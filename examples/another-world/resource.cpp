/*
  resource definitions are stored in MEMLIST.BIN

  each record is twenty bytes long and numbers are stored as
  big endian
*/

#include <assert.h>
#include <string.h>
#include <algorithm>
#include <ctime>

#include "virtual-machine.hpp"

namespace another_world {

  uint8_t resource_heap[HEAP_SIZE];
  uint32_t resource_heap_offset = 0;
  std::vector<Resource *> resources;

  ChapterResources chapter_resources[10] = {
  {0x14, 0x15, 0x16, 0x00},
  {0x17, 0x18, 0x19, 0x00},
  {0x1a, 0x1b, 0x1c, 0x11},
  {0x1d, 0x1e, 0x1f, 0x11},
  {0x20, 0x21, 0x22, 0x11},
  {0x23, 0x24, 0x25, 0x00},
  {0x26, 0x27, 0x28, 0x11},
  {0x29, 0x2a, 0x2b, 0x11},
  {0x7d, 0x7e, 0x7f, 0x00},
  {0x7d, 0x7e, 0x7f, 0x00}
  };


  // load the resource definitions from MEMLIST.BIN
  // you must provide a pointer to a buffer than contains the
  // file contents
  void init_resources() {

    // TODO: move file access out of here by requiring a basic set 
    // of system calls to be provided
    uint8_t memlist[2940];
    uint8_t* p = memlist;

    read_file("memlist.bin", 0, 2940, (char*)memlist);

    while (static_cast<Resource::State>(p[0]) != Resource::State::END_OF_MEMLIST) {
      Resource* resource = new Resource();

      // each memlist entry (resource) contains 20 bytes:
      //
      //  0     : state
      //  1     : type
      //  2 -  6: unknown (always zero)
      //  7     : bank id
      //  8 - 11: bank data start offset
      // 12 - 15: packed size
      // 16 - 19: unpacked size

      resource->state = (Resource::State)p[0];
      resource->type = (Resource::Type)p[1];
      resource->bank_id = p[7];
      resource->bank_offset = read_uint32_bigendian(p + 8);
      resource->packed_size = read_uint32_bigendian(p + 12);
      resource->size = read_uint32_bigendian(p + 16);

      resources.push_back(resource);

      p += 20;
    }
  }

  void load_chapter_resources(uint16_t id) {
    
  }

  // loads all resources that are currently in the NEEDS_LOADING state
  void load_needed_resources() {
    for (auto resource : resources) {
      if (resource->state == Resource::State::NEEDS_LOADING) {

        if (resource->type == Resource::Type::SOUND || resource->type == Resource::Type::MUSIC) {
          // TODO: we're not currently supporting audio so no point in loading the 
          // resources just yet...
          continue;
        }

        uint8_t* destination;
        if (resource->type == Resource::Type::IMAGE) {
          destination = vram[0];
        }
        else {
          destination = resource_heap + resource_heap_offset;
          resource_heap_offset += resource->size;
        }

        //debug("Loading resource of type " + std::to_string(resource->type) + " at offset " + std::to_string(heap_offset));
        resource->load(destination);

        // TODO: if the resource was an image then it's encoded as 4 bitplanes a la mode 9
        // we need to shuffle the pixels around to get it into our buffer format
        if (resource->type == Resource::Type::IMAGE) {
          uint8_t temp[320 * 200 / 2];
          memcpy(temp, destination, 320 * 200 / 2);
          uint8_t* p = destination;
          for (uint16_t y = 0; y < 200; y++) {
            for (uint16_t x = 0; x < 320; x += 8) {
              uint8_t b1 = temp[y * 40 + x / 8 + 0];
              uint8_t b2 = temp[y * 40 + x / 8 + 8000];
              uint8_t b3 = temp[y * 40 + x / 8 + 16000];
              uint8_t b4 = temp[y * 40 + x / 8 + 24000];

              for (uint8_t i = 0; i < 4; i++) {
                uint8_t v1 = (b1 & 0b10000000) >> 0;
                uint8_t v2 = (b2 & 0b10000000) >> 1;
                uint8_t v3 = (b3 & 0b10000000) >> 2;
                uint8_t v4 = (b4 & 0b10000000) >> 3;

                b1 <<= 1;
                b2 <<= 1;
                b3 <<= 1;
                b4 <<= 1;

                uint8_t v5 = (b1 & 0b10000000) >> 4;
                uint8_t v6 = (b2 & 0b10000000) >> 5;
                uint8_t v7 = (b3 & 0b10000000) >> 6;
                uint8_t v8 = (b4 & 0b10000000) >> 7;

                b1 <<= 1;
                b2 <<= 1;
                b3 <<= 1;
                b4 <<= 1;

                *p++ = v1 | v2 | v3 | v4 | v5 | v6 | v7 | v8;
              }
            }
          }
        }

        if (resource->type == Resource::Type::IMAGE) {
          resource->state = Resource::State::NOT_NEEDED;
        }
        else {
          resource->state = Resource::State::LOADED;
        }
      }
    }
  }

  bool Resource::load(uint8_t* destination) {
    static std::string hex[16] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f" };

    // TODO: move file access out of here by requiring a basic set 
    // of system calls to be provided
    std::string bank_filename = "bank0" + hex[this->bank_id];
    read_file(bank_filename, this->bank_offset, this->packed_size, (char*)destination);

    bool success = false;

    if (this->packed_size != this->size) {
      ByteKiller bk;
      success = bk.unpack(destination, this->packed_size);
    }

    this->data = destination;
    
    return success;
  }

}