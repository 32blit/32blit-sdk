/* 
  resource definitions are stored in MEMLIST.BIN

  each record is twenty bytes long and numbers are stored as
  big endian
*/

#include <cassert>
#include <cstring>
#include <algorithm>
#include <ctime>


#include "virtual-machine.hpp"

namespace another_world {

  #define REG_RANDOM_SEED 0x3c
  #define THREAD_INACTIVE 0xffff
  #define THREAD_LOCK 0x01
  #define THREAD_UNLOCK 0x02
  #define NO_UPDATE 0xffff

  // helpers to fetch values stored in big endian and convert
  // them into stdint types
  uint16_t read_uint16_bigendian(const void *p) {
    const uint8_t* b = (const uint8_t*)p;
    return (b[0] << 8) | b[1];
  }

  uint32_t read_uint32_bigendian(const void* p) { 
    const uint8_t* b = (const uint8_t*)p;
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
  }

  bool (*read_file)(std::string filename, uint32_t offset, uint32_t length, char* buffer) = nullptr;
  void (*debug)(const char *fmt, ...) = nullptr;
  void (*debug_display_update)() = nullptr;
  void (*update_screen)(uint8_t* buffer) = nullptr;
  void (*set_palette)(uint16_t* palette) = nullptr;

  
  uint8_t vram0[320 * 200 / 2];
  uint8_t vram1[320 * 200 / 2];
  uint8_t vram2[320 * 200 / 2];
  uint8_t vram3[320 * 200 / 2];

  Input input;

  uint8_t* vram[4] = {
    vram0,  // background 1 (also used for clone drawing operations)
    vram1,  // framebuffer 1
    vram2,  // framebuffer 2
    vram3   // background 2
  };

  void VirtualMachine::init() {
    init_resources();

    memset(registers, 0, REGISTER_COUNT * sizeof(int16_t));

    // TODO: some special register values that need setting,
    // perhaps one day we'll have a dig around and figure out why...
    registers[0x54] = 0x81;
    registers[0xBC] = 0x10;
    registers[0xC6] = 0x80;
    registers[0xF2] = 4000;
    registers[0xDC] = 33;
    registers[0xE4] = 20;

    // number selected by committee, guaranteed random
    registers[REG_RANDOM_SEED] = 2322; 
  }

  void VirtualMachine::initialise_chapter(uint16_t id) {
    /* TODO: player->stop();
	  mixer->stopAll();*/

    // reset the heap and resource states
    

    for(auto resource : resources) {
      resource->state = Resource::State::NOT_NEEDED;
    }

    // according to Eric Chahi's original notes the chapters are:
    //
    // 16000 = Title
    // 16001 = Intro
    // 16002 = Cave
    // 16003 = Prison
    // 16004 = Citadel?
    // 16005 = Arena
    // 16006 = ???
    // 16007 = Final
    // 16008 = ???
    // 16009 = ???
    
    chapter_id = id - 16000;

    registers[0xE4] = 0x14; // TODO: erm?      

    palette = resources[chapter_resources[chapter_id].palette];
    code = resources[chapter_resources[chapter_id].code];
    background = resources[chapter_resources[chapter_id].background];  

    // load the chapter resources
    palette->state = Resource::State::NEEDS_LOADING;
    code->state = Resource::State::NEEDS_LOADING;
    background->state = Resource::State::NEEDS_LOADING;

    if(chapter_resources[chapter_id].characters) {
      characters = resources[chapter_resources[chapter_id].characters];
      characters->state = Resource::State::NEEDS_LOADING;
    }
    
    load_needed_resources();

    // set all thread program counters to 0xffff (inactive)
    for (auto thread : threads) {
      thread.pc = 0xffff;
      thread.paused = false;
    }

    // reset program counter for first thread
    threads[0].pc = 0;
  }

  uint8_t VirtualMachine::fetch_byte(uint8_t *b, uint32_t *c) {
    uint8_t v = b[*c];
    (*c)++;
    return v;
  }

  uint16_t VirtualMachine::fetch_word(uint8_t *b, uint32_t *c) {
    uint16_t v = read_uint16_bigendian(&b[*c]);
    (*c)++;
    (*c)++;
    return v;
  }

  uint8_t VirtualMachine::fetch_byte(uint16_t *pc) {
    uint8_t v = code->data[*pc];
    (*pc)++;
    return v;
  }

  uint16_t VirtualMachine::fetch_word(uint16_t *pc) {
    uint16_t v = read_uint16_bigendian(&code->data[*pc]);
    (*pc)++;
    (*pc)++;
    return v;
  }

  void VirtualMachine::point(uint8_t* target, uint8_t color, Point* p) {
    uint32_t offset = (p->y * 160) + (p->x / 2);
    uint8_t* pd = target + offset;
    uint8_t mask = p->x & 0b1 ? 0x0f : 0xf0;

    if (p->x < 0 || p->x >= 320 || p->y < 0 || p->y >= 200) {
      return;
    }

    if (color == 0x10) {
      // special blend mode, set the high bit of the colour (to offset the drawn color
      // palette index by 8. This is used to overlay colours (like the headlights of
      // the car during the intro animation) and requires the palettes to be carefully
      // setup to achieve the effect.
      (*pd) |= 0x88 & mask;  // set the high bit in the masked nibble
    } else if (color > 0x10) {
      // theory - this mode only draws the pixel if the equivalent pixel in the background
      // has the high bit set, effectively allowing the masking of shapes
      (*pd) &= (~mask); // clear the nibble in the target
      uint8_t* ps = get_vram_from_id(0) + offset; // get same offset in copy from buffer
      (*pd) |= (*ps) & mask; // copy the nibble from visible vram
    } else {
      // draw in the colour requested
      uint8_t c = color & 0x0f;
      c = c | (c << 4);
      (*pd) &= (~mask);  // clear the nibble in the target
      (*pd) |= c & mask; // mask in the new colour
    }
  }

  void VirtualMachine::polygon(uint8_t *target, uint8_t color, Point *points, uint8_t point_count) {
    static int32_t nodes[256]; // maximum allowed number of nodes per scanline for polygon rendering    

    Rect clip = { 0, 0, 320, 200 };
    int16_t miny = points[0].y, maxy = points[0].y;

    // copy the colour value into the high and low nibbles making
    // it easier to use later
    uint8_t c = color & 0x0f;
    c = c | (c << 4);

    for (uint16_t i = 1; i < point_count; i++) {
      miny = std::min(miny, points[i].y);
      maxy = std::max(maxy, points[i].y);
    }

    // for each scanline within the polygon bounds (clipped to clip rect)
    Point p;

    for (p.y = std::max(clip.y, miny); p.y <= std::min(int16_t(clip.y + clip.h), maxy); p.y++) {
      uint8_t n = 0;
      for (uint16_t i = 0; i < point_count; i++) {
        uint16_t j = (i + 1) % point_count;
        int32_t sy = points[i].y;
        int32_t ey = points[j].y;
        int32_t fy = p.y;
        if ((sy < fy && ey >= fy) || (ey < fy && sy >= fy)) {
          int32_t sx = points[i].x;
          int32_t ex = points[j].x;
          int32_t px = int32_t(sx + float(fy - sy) / float(ey - sy) * float(ex - sx));

          nodes[n++] = px < clip.x ? clip.x : (px >= clip.x + clip.w ? clip.x + clip.w - 1 : px);
        }
      }

      uint16_t i = 0;
      while (i < n - 1) {
        if (nodes[i] > nodes[i + 1]) {
          int32_t s = nodes[i]; nodes[i] = nodes[i + 1]; nodes[i + 1] = s;
          if (i) i--;
        }
        else {
          i++;
        }
      }

      for (uint16_t i = 0; i < n; i += 2) {
        for (p.x = nodes[i]; p.x <= nodes[i + 1]; p.x++) {
          point(target, color, &p);
        }
      }      
    }

    if (debug_display_update) {
      debug_display_update();
    }    
  }

  void VirtualMachine::draw_shape(uint8_t color, Point pos, int16_t zoom, uint8_t *buffer, uint32_t *offset) {
    uint8_t shape_header = fetch_byte(buffer, offset);

    // the top two bits of the shape header determine what to draw
    //
    // 11xxxxxx - single polygon
    // 01xxxxxx - ???
    // 10xxxxxx - ???
    // 00xxxxxx - ???
    //
    // Eric Chahi documents another special case where the header is
    // 63 (11000000) which states "fin du bloc matrice" - i suspect originally
    // the plan was to mark the end of a polygon group this way but ultimately
    // he decided to include the count in the bytecode? not sure...

    if ((shape_header & 0b11000000) == 0b11000000) {
      // draw a single polygon
      // bits 0-5 of the header contain the colour of the polygon being drawn
      // TODO: if the colour is set here why are we passing it in as a parameter?      
      if(color & 0x80) {
        color = shape_header & 0x3f;      
      }
      draw_polygon(color, pos, zoom, buffer, offset);
    }
    else {
      // draw a polygon group
      // bits 0-5 of the header seem to always contain the number 2. 
      // why? we just don't know
      if ((shape_header & 0x3f) == 2) {
        draw_shape_group(color, pos, zoom, buffer, offset);
      } else {
      }
    }

  }

  void VirtualMachine::draw_polygon(uint8_t color, Point pos, int16_t zoom, uint8_t *buffer, uint32_t *offset) {
    static Point points[256];
    
    // polygons are drawn offset by the centre of their bounding box
    Rect bounds;
    bounds.w = fetch_byte(buffer, offset) * zoom / 64;
    bounds.h = fetch_byte(buffer, offset) * zoom / 64;    
    bounds.x = pos.x - bounds.w / 2;
    bounds.y = pos.y - bounds.h / 2;     

    // TODO: why is this needed? Without it some scenes show a glitchy top row of pixels
    bounds.y--; 
    
    // TODO: could do a quick bounds check here to test if on screen at all

    // load in the point data for this polygon and offset/scale accordingly
    int16_t point_count = fetch_byte(buffer, offset);

    for (uint8_t i = 0; i < point_count; i++) {
      points[i].x = bounds.x + fetch_byte(buffer, offset) * zoom / 64;
      points[i].y = bounds.y + fetch_byte(buffer, offset) * zoom / 64;
    }

    polygon(working_vram, color, points, point_count);
  }

  void VirtualMachine::draw_shape_group(uint8_t color, Point pos, int16_t zoom, uint8_t* buffer, uint32_t *offset) {
    pos.x -= fetch_byte(buffer, offset) * zoom / 64;
    pos.y -= fetch_byte(buffer, offset) * zoom / 64;

    int8_t count = fetch_byte(buffer, offset);

    // TODO: this seems wrong, but produces much better output until it crashes - what gives?
    // surely it should be "i < count"?
    for (uint8_t i = 0; i <= count; i++) {  
      uint16_t header = fetch_word(buffer, offset);

      // absolute position of shape (added to relative positions later)
      Point polygon_pos = { pos.x, pos.y };
      polygon_pos.x += fetch_byte(buffer, offset) * zoom / 64;
      polygon_pos.y += fetch_byte(buffer, offset) * zoom / 64;

      uint8_t child_color = 0xff; // TODO: why reset the colour here? not sure...

      // the high bit of the header tells us whether this shape has a custom colour
      // or uses the colour we used previously.
      // if it is set then we need to pull the new colour from the bytecode
      if (header & 0x8000) {
        child_color = fetch_byte(buffer, offset) & 0x7f;

        fetch_byte(buffer, offset); // TODO: what is this?
      }

      uint32_t child_offset = (header & 0x7fff) * 2;
      draw_shape(child_color, polygon_pos, zoom, buffer, &child_offset);
    }
  }

  uint8_t* VirtualMachine::get_vram_from_id(uint8_t id) {
    // screen id 0 is unclear from Eric Chahi's notes he says 
    // "set pour la couleur masque" which translates to "set for
    // the colour mask"? not sure what that means...
    //
    //   0 - vram[2] background framebuffer (used for mask color - "sert pour la coleur masque")
    //   1 - vram[0] foreground framebuffer 1 
    //   2 - vram[1] foreground framebuffer 2
    //   3 - vram[2] background framebuffer
    //
    // 254 - currently visible foreground framebuffer
    // 255 - currently invisible foreground framebuffer
    /*if (id == 0) {
      // special case for masking?
      return vram[2];
    }
    if (id >= 1 && id <= 3) {
      // return the requested framebuffer
      return vram[id - 1];
    }*/

    if (id >= 0 && id <= 3) {
      return vram[id];
    }

    if (id == 254) {
      // visible screen "ecran visible"
      return visible_vram;
      
    }

    if (id == 255) {
      // invisible screen "ecran invisible"
      return visible_vram == vram[1] ? vram[2] : vram[1];
    }

    return nullptr;
  }

  void VirtualMachine::process_input() {
    uint8_t input_mask = 0;

    registers[0xE5] = 0;
    registers[0xFB] = 0;
    registers[0xFC] = 0;
    registers[0xFA] = 0;

    if (input.up && !input.down) {
      input_mask |= 0b00001000;
      // TODO: why both?
      registers[0xE5] = -1;
      registers[0xFB] = -1;
    }

    if (input.down && !input.up) {
      input_mask |= 0b00000100;
      // TODO: why both?
      registers[0xE5] = 1;
      registers[0xFB] = 1;
    }

    if (input.left && !input.right) {
      input_mask |= 0b00000010;
      registers[0xFC] = -1;
    }

    if (input.right && !input.left) {
      input_mask |= 0b00000001;
      registers[0xFC] = 1;
    }

    if (input.action) {
      input_mask |= 0b10000000;
      registers[0xFA] = 1;
    }
 
    // TODO: why both?
    registers[0xFD] = input_mask;
    registers[0xFE] = input_mask;
  }

  void VirtualMachine::execute_threads() {
    if (debug) {
      debug("--- execute threads ---");
    }
    
    // TODO: switch part if needed (can't this be done in the op code processing?)
  
        //  //Check if a part switch has been requested.
        // 	if (res->requestedNextPart != 0) {
        // 		initForPart(res->requestedNextPart);
        // 		res->requestedNextPart = 0;
        // 	}

        // TODO: handle input and player update
	    //	vm.inp_updatePlayer();
	    //	processInput();

    // ensure the call stack is empty before starting
    call_stack.clear();

    process_input();

    // during thread execution the svec opcode allows a thread
    // to be given a new program counter for the next cycle of
    // execution, we store those here and update the program
    // counters after all threads have been processed if needed
/*    uint16_t  new_program_counter[THREAD_COUNT];
    for (uint8_t i = 0; i < THREAD_COUNT; i++) {
      new_program_counter[i] = NO_UPDATE;
    }

    uint16_t new_paused_threads[THREAD_COUNT];
    for (uint8_t i = 0; i < THREAD_COUNT; i++) {
      new_paused_threads[i] = NO_UPDATE;
    }*/
    
    std::map<uint8_t, Thread> requested_thread_state;
    
    // step through each thread and execute the active ones
    for(auto &thread : threads) {
      if (thread.pc == THREAD_INACTIVE || thread.paused) {
        continue;
      }

      uint16_t* pc = &thread.pc;

      bool next_thread = false;
      while(!next_thread) {          
        uint8_t opcode = fetch_byte(pc);

        std::string opcode_name = "----";
        if (opcode <= 0x1a) {
          opcode_name = opcode_names[opcode];
        } else if (opcode < 0x40) {
          // invalid
        } else if (opcode < 0x80) {
          opcode_name = "plyl";
        } else {
          opcode_name = "plys";
        }
          
        if(debug) {
          debug("%6i)  %2i [%05u] > %02x:%-6s", ticks, 0, *(pc)-1, opcode, opcode_name.c_str());
        }          

        // opcodes come in three different flavours depending on the status
        // of the two highest bits
        //
        // 00xxxxxx = standard opcode instruction number in bits 0-5
        // 01xxxxxx = polygon opcode long format (translated from Eric Chahi's "different format de donnees pour spr.l")
        // 1xxxxxxx = polygon opcode short format (high part of address in bits 0-6)

        if (ticks == 48) {
            uint8_t a = 0;
        }

        if (opcode & 0x80) {
          // contains offset for polygon data in cinematic data resource  
          // the high bits of the address are 0-6 from the opcode
          uint32_t offset = (((opcode & 0x7f) << 8) | fetch_byte(pc)) * 2;

          uint8_t* polygon_data = background->data;

          // absolute position of shape (added to relative positions later)
          Point pos;
          pos.x = fetch_byte(pc);
          pos.y = fetch_byte(pc);

          // slightly weird one this. if the y value is greater than 199
          // then the extra is added onto the x value. i assume this is because
          // the screen resolution is 320 pixels but a byte can only hold 
          // numbers up to 255. this "hack" allows bigger numbers (up to 311) to 
          // be represented in the x byte (at the cost that it can only happen
          // when y is greater than 199 (so is effectively clamped to the 
          // bottom of the screen).
          if (pos.y > 199) {
            pos.x += pos.y - 199;
            pos.y = 199;
          }                              

          draw_shape(0xff, pos, 64, polygon_data, &offset);

          ticks++;
          continue;
        }
          
        if(opcode & 0x40) {   
          // contains offset for polygon data in cinematic data resource    
          // the offset is contained in the next two bytes in the bytecode
          uint32_t offset = fetch_word(pc) * 2;
            
          uint8_t* polygon_data = background->data;

          Point pos;
            
          // bits 0-5 of the opcode have special meaning that manipulate the
          // x and y coordinates for this polygon.
          //
          // the bits 0-5 are laid out aabbcc with each pair of bits (e.g "aa")
          // selecting an operation to perform.

          if ((opcode & 0b00110000) == 0b00110000) {
            // if xx == 11 then add 256 to x (essentially x gains an extra
            // bit of resolution)
            pos.x = fetch_byte(pc) + 256;
          } else if ((opcode & 0b00110000) == 0b00010000)  {
            // if xx == 01 then the x value is selected from the specified register
            pos.x = registers[fetch_byte(pc)];
          } else if ((opcode & 0b00110000) == 0b00000000) {
            // if xx == 00 then the x value is read from the next two bytes of
            // bytecode
            pos.x = fetch_word(pc);
          }
          else {
            // otherwise the x value is simply the next byte of bytecode
            pos.x = fetch_byte(pc);
          }

          if ((opcode & 0b00001100) == 0b00001100) {
            // if yy == 11 then add 256 to y (essentially y gains an extra
            // bit of resolution)
            pos.y = fetch_byte(pc) + 256;
          }
          else if ((opcode & 0b00001100) == 0b00000100) {
            // if yy == 01 then the y value is selected from the specified register
            pos.y = registers[fetch_byte(pc)];
          }
          else if ((opcode & 0b00001100) == 0b00000000) {
            // if yy == 00 then the y value is read from the next two bytes of
            // bytecode
            pos.y = fetch_word(pc);
          }
          else {
            // otherwise the y value is simply the next byte of bytecode
            pos.y = fetch_byte(pc);
          }

          int16_t zoom = 64;

          if ((opcode & 0b00000011) == 0b00000011) {
            // if zz == 11 then something special happens...
            // why? we don't know, but it does! the notes in Eric
            // Chahi's document are not really legible, perhaps
            // something like... "11 si Z utiliser Z~~~~ Banque et Z = 64"?
            // Fabien Sanglard has this special case change the source of
            // polygon data to "SegVideo2" which I think is meant to be the
            // character data, anyway, let's try that...
            polygon_data = characters->data;

    //         assert(false); // i don't think we should end up here...
          }
          else if ((opcode & 0b00000011) == 0b00000001) {
            // if zz == 01 then the z value is selected from the specified register
            zoom = registers[fetch_byte(pc)];
          }
          else if ((opcode & 0b00000011) == 0b00000000) {
            // default zoom level, already set above
          }
          else {
            // otherwise the z value is simply the next byte of bytecode
            zoom = fetch_byte(pc);
          }

          draw_shape(0xff, pos, zoom, polygon_data, &offset);

          ticks++;
          continue;}

        switch(opcode) {
          case 0x00: {
            // movi   d0, #1234
            // copy immediate word to register d0
            uint8_t d0 = fetch_byte(pc);
            int16_t w = fetch_word(pc);
            registers[d0] = w;
            break;
          }

          case 0x01: {
            // mov    d0, d1
            // copy value in register d1 into register d0
            uint8_t d0 = fetch_byte(pc);
            uint8_t d1 = fetch_byte(pc);
            registers[d0] = registers[d1];
            break;
          }

          case 0x02: {
            // add    d0, d1
            // add value in register d1 to to register d0
            uint8_t d0 = fetch_byte(pc);
            uint8_t d1 = fetch_byte(pc);
            registers[d0] += registers[d1];
            break;
          }

          case 0x03: {
            // addi   d0, #1234
            // add immediate word to register d0
            uint8_t d0 = fetch_byte(pc);
            int16_t w = fetch_word(pc);
            registers[d0] += w;
            break;
          }

          case 0x04: {              
            // call   #1234
            // push current program counter onto stack then jump to specified address
            int16_t w = fetch_word(pc);
            call_stack.push_back(*pc);
            *pc = w;
            break;
          }

          case 0x05: {
            // ret
            // pop last address off the stack and jump there (return from a call)
            *pc = call_stack.back();
            call_stack.pop_back();
            break;
          }

          case 0x06: {
            // brk
            // stop execution of this thread and switch execution to the next thread              
            next_thread = true;
            break;
          }

          case 0x07: {
            // jmp    #1234
            // jump to specified address
            int16_t w = fetch_word(pc);
            *pc = w;
            break;
          }

          case 0x08: {
            // svec   #12, #1234
            // request the change of a program counter of a thread to be applied after
            // the current execution cycle has completed
            uint8_t thread_id = fetch_byte(pc);
            int16_t new_pc = fetch_word(pc);

            Thread new_thread_state = threads[thread_id];
            new_thread_state.pc = new_pc;
            requested_thread_state[thread_id] = new_thread_state;
  
            break;
          }

          case 0x09: {
            // djnz   d0, #1234
            // decrement register and jump to specified address if not zero
            uint8_t d0 = fetch_byte(pc);
            int16_t w = fetch_word(pc);

            registers[d0]--;

            if(registers[d0] != 0) {
              *pc = w;
            }
            break;
          }

          case 0x0a: {
            // cjmp   #12, d0, d1 or #1234, #1234
            // conditional jump for expression when d0 compared to either
            // d1 or an immediate byte or word value if expression result 
            // is true then jump to specified address
            uint8_t t = fetch_byte(pc);          
            int16_t a = registers[fetch_byte(pc)];
            int16_t b = fetch_byte(pc);

            if(t & 0x80) {
              // register to register comparison
              b = registers[b];         
            } else if (t & 0x40) {
              // register to 16-bit literal comparison
              b = (b << 8) | fetch_byte(pc);   
            }

            int16_t w = fetch_word(pc);

            bool result = false;
            
            // mask out just the expression bits
            t &= 0b111;
            if(t == 0) { result = a == b; }
            if(t == 1) { result = a != b; }
            if(t == 2) { result = a  > b; }
            if(t == 3) { result = a >= b; }
            if(t == 4) { result = a  < b; }
            if(t == 5) { result = a <= b; }

            if(result) {
              *pc = w;
            }           
            break;
          }

          case 0x0b: {
            // pal    #12, #12
            // specify the index of the palette to use
            uint8_t id = fetch_byte(pc);
    
            // TODO: from Eric Chahi's original notes the second byte of
            // this instruction appears to be a speed ("a la vitesse") 
            // for the palette change - but then parts of the notes are
            // crossed out suggesting it was never implemented?
            uint8_t speed = fetch_byte(pc);

            if (id != 0xff) {
              // calculate the offset for the requested palette
              uint16_t offset = id * 32;

              // the first 32 palettes are for the Amiga/VGA version, the
              // following 32 palettes are for the MSDOS version              
              //offset += (32 * 32); // offset to EGA/TGA
              set_palette((uint16_t*)&palette->data[offset]);
            }
                            
            break;
          }

          case 0x0c: {              
            // ???    #12, #12, #12
            // this one is a bit cryptic with Eric Chahi's notes 
            // referring  to the first "1st affecte"/"start" and last 
            // "dernier affecte"/"end" vectors affected along with a 
            // "type" of action (unlock, lock, clear)
            // it suggests that this opcode should affect a range of
            // threads, perhaps updating their state in bulk?

            uint8_t first = fetch_byte(pc);
            uint8_t last = fetch_byte(pc);
            uint8_t type = fetch_byte(pc);

            for (uint8_t thread_id = first; thread_id <= last; thread_id++) {
              Thread new_thread_state = threads[thread_id];

              if (type == 0) {
                // unlock
                new_thread_state.paused = false;
                requested_thread_state[thread_id] = new_thread_state;                  
              }
              if (type == 1) {
                // lock 
                new_thread_state.paused = true;
                requested_thread_state[thread_id] = new_thread_state;
              }
              if (type == 2) {
                // kill threads
                new_thread_state.pc = THREAD_INACTIVE;
                requested_thread_state[thread_id] = new_thread_state;
              }
            }
            break;
          }

          // framebuffer manipulation op codes
          // 
          case 0x0d: {
            // setws    #12
            // set the working screen for drawing operations
            uint8_t id = fetch_byte(pc);
            uint8_t *b = get_vram_from_id(id);

            if(b) {
              // TODO: why would we ever be given an invalid screen id?
              // that doesn't seem right...

              working_vram = b;
            }
            else {
              assert(false);
            }
            break;
          }

          case 0x0e: {
            // vclr   #12, #12
            // clears an entire backbuffer with the specified palette
            // colour              
            uint8_t id = fetch_byte(pc);
            uint8_t* d = get_vram_from_id(id);              

            uint8_t color = fetch_byte(pc);
            color |= color << 4;

            if(d) {
              // TODO: why would we ever be given an invalid screen id?
              // that doesn't seem right...
              memset(d, color, 320 * 200 / 2);
            }

            if (debug_display_update) {
              debug_display_update();
            }

            break;
          }

          case 0x0f: {
            // vcpy   #12, #12
            // copy contents of one backbuffer into another

            uint8_t src_id = fetch_byte(pc);
            uint8_t dest_id = fetch_byte(pc);

            if (src_id >= 0xFE || ((src_id &= ~0x40) & 0x80) == 0) {

            }
            else {
              // assert(false); // TODO: vscroll?
            }


            //src_id &= ~0x40;
            uint8_t* s = get_vram_from_id(src_id);
            uint8_t* d = get_vram_from_id(dest_id);

            if (s && d) {
              // TODO: why would we ever be given an invalid screen id?
              // that doesn't seem right...
              memcpy(d, s, 320 * 200 / 2);
            }

            /*
            uint8_t src_id = fetch_byte(pc);
            uint8_t dest_id = fetch_byte(pc);

            debug("Copy buffer %d to %d", src_id, dest_id);

            //src_id &= ~0x40;
            uint8_t* s = get_vram_from_id(src_id);
            uint8_t* d = get_vram_from_id(dest_id);

           
            // TODO: why would we ever be given an invalid screen id?
            // that doesn't seem right...
            if (s && d) {
              int16_t v_scroll = registers[0xF9];
              uint16_t h = 200;

              if (v_scroll != 0) {
                uint8_t a = 0;
              }
              h -= abs(v_scroll);
              s -= v_scroll < 0 ? (v_scroll * 160) : 0;
              d += v_scroll > 0 ? (v_scroll * 160) : 0;

              memcpy(d, s, 320 * h / 2);
            }*/
              
              
            if (debug_display_update) {
              debug_display_update();
            }

            // TODO: this should support vertical scrolling by looking the
            // value in register VM_VARIABLE_SCROLL_Y
            // e.g. video->copyPage(srcPageId, dstPageId, vmVariables[VM_VARIABLE_SCROLL_Y]);
            break;
          }

          case 0x10: {
            // vshw   #12
            // copy specified backbuffer to screen
            uint8_t id = fetch_byte(pc);

            registers[0xF7] = 0; // TODO:  why?
              
            if(id == 0xff) {                              
              // from Eric Chahi's notes:
              // "si n == 255 on flip invisi et visi" so in case the
              // id specified is 255 we swap which of the backbuffers
              // is the woring framebuffer
              visible_vram = visible_vram == vram[1] ? vram[2] : vram[1];                
            }

              
            update_screen(visible_vram);

            if (debug_display_update) {
              debug_display_update();
            }

            break;
          }

          case 0x11: {
            // kill
            // set current threads program counter to 0xffff (inactive) and 
            // moveto the next thread
            *pc = THREAD_INACTIVE;
            next_thread = true;
            break;
          }

          case 0x12: {
            // text   #1234, #12, #12, #12
            uint16_t string_id = fetch_word(pc);
            uint8_t x = fetch_byte(pc);
            uint8_t y = fetch_byte(pc);
            uint8_t colour = fetch_byte(pc);

            /*if (string_id < string_table.size()) {
              const std::string& string_entry = string_table.at(string_id);
                
            }
            else {
              // TODO: why would we ever get an invalid string id?
              //assert(false);
            }*/

            // TODO: make this work?
            break;
          }

          case 0x13: {          
            // sub  d0, d1
            // subtract value in register d1 from register d0
            uint8_t d = fetch_byte(pc);
            uint8_t s = fetch_byte(pc);
            registers[d] -= registers[s];           
            break;
          }

          case 0x14: {
            // andi  d0, #1234
            // bitwise AND register d0 with the value provided
            uint8_t r = fetch_byte(pc);
            int16_t v = fetch_word(pc);
            registers[r] = (uint16_t)registers[r] & v;
            break;
          }

          case 0x15: {
            // andi  d0, #1234
            // bitwise OR register d0 with the value provided
            uint8_t r = fetch_byte(pc);
            int16_t v = fetch_word(pc);
            registers[r] = (uint16_t)registers[r] | v;
            break;
          }

          case 0x16: {
            // shli  d0, #1234
            // shift value in register d0 left by value provided

            // TODO: seems odd the shift value is 16-bit since
            // shifting by anything more than 16 will zero out the
            // register
            uint8_t r = fetch_byte(pc);
            int16_t v = fetch_word(pc);
            registers[r] = (uint16_t)registers[r] << v;
            break;
          }

          case 0x17: {
            // shri  d0, #1234
            // shift value in register d0 right by value provided
            // note: this shift is intentionally unsigned so new bits 
            // are zero filled
              
            // TODO: seems odd the shift value is 16-bit since
            // shifting by anything more than 16 will zero out the
            // register
            uint8_t r = fetch_byte(pc);
            int16_t v = fetch_word(pc);
            registers[r] = (uint16_t)registers[r] >> v;
            break;
          }

          case 0x18: {
            // snd  #1234, #12, #12, #12
            fetch_word(pc);
            fetch_byte(pc);
            fetch_byte(pc);
            fetch_byte(pc);
            break;
          }

          case 0x19: {
            // load   #1234
            // loads either a resource or the next chapter of the
            // game.
            uint16_t i = fetch_word(pc);

            if (i == 0) {
              // TODO: Eric Chahi's notes are hard to read here but say
              // something like "libere la memoire annuler"
              // sounds like perhaps this is "exit the game"?
              // not sure - let's leave an assert here and see if it
              // ever happens...
              assert(false);
            } else {
              if (i <= resources.size()) {
                // load a resource
                resources[i]->state = Resource::State::NEEDS_LOADING;
                load_needed_resources();
              }
              else {
                // switch to a new chapter
                initialise_chapter(i);
              }
            }
              
            break;
          }

          case 0x1a: {
            // music #1234, #1234, #12
            fetch_word(pc);
            fetch_word(pc);
            fetch_byte(pc);
            break;
          }

          default: {
            // debug("- Invalid opcode " + std::to_string(opcode) + " on thread " + std::to_string(i));
            break;
          }
        }
      }
    }

    // set thread program counters and pause states if new values
    // have been requested
    for (auto const& p : requested_thread_state) {      
      threads[p.first] = p.second;
    }
    
    /*
    for (uint8_t i = 0; i < THREAD_COUNT; i++) {
      if (new_program_counter[i] == 0xfffe) {
        program_counter[i] = THREAD_INACTIVE;
        new_program_counter[i] = THREAD_INACTIVE;
      } else {
        if (new_program_counter[i] != NO_UPDATE) {
          program_counter[i] = new_program_counter[i];
        }
      }

      if (new_paused_threads[i] == THREAD_LOCK) {
        paused_thread[i] = true;
      }

      if (new_paused_threads[i] == THREAD_UNLOCK) {
        paused_thread[i] = false;
      }
    }*/
  }
}
