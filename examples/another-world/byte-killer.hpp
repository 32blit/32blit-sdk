#pragma once

#include <cstdint>

struct ByteKiller {
  private:
  uint32_t bit_stream;
  uint32_t crc;
  uint8_t *ps;
  uint8_t *pd;

  uint16_t read_uint16_bigendian(const void* p) {
    const uint8_t* b = (const uint8_t*)p;
    return (b[0] << 8) | b[1];
  }

  uint32_t read_uint32_bigendian(const void* p) {
    const uint8_t* b = (const uint8_t*)p;
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
  }

  bool get_stream_bit() {
    bool result;

    if(bit_stream == 0b1) {
      // the final bit tells us we're at the end of this block
      // so we need to load in the next block
      bit_stream = read_uint32_bigendian(ps); ps -= 4;
      crc ^= bit_stream;

      // grab the low command bit and shift the stream
      result = bit_stream & 0b1;
      bit_stream >>= 1;

      // set the high bit so we detect the next time we run out
      // of command bits
      bit_stream |= 0x80000000;    
    }else{
      // grab the low command bit and shift the stream
      result = bit_stream & 0b1;
      bit_stream >>= 1;
    }

    return result;
  }

  uint16_t get_value(uint8_t bit_count) {
    uint16_t value = 0;

    while(bit_count--) {
      value <<= 1;
      value |= get_stream_bit() ? 0b1 : 0b0;
    }  

    return value;
  }

  void copy(uint16_t count) {
    while(count--) {
      *pd = get_value(8);
      pd--;
    }
  }

  void repeat_from_offset(uint16_t count, uint16_t offset) {
    while(count--) {
      *pd = *(pd + offset);
      pd--;
    }
  }

  public:
  ByteKiller() {}

  bool unpack(uint8_t *buffer, uint32_t packed_size) {
    // the data is unpacked from end to start using two pointers
    // the source pointer `ps` and destination pointer `pd`
    // because the packed data is smaller than the unpacked data
    // it can be unpacked inline in the same buffer without any risk
    // over the destination pointer overruning the source pointer.
    // effectively both pointers "race" to the start of the buffer
    // meeting there once the unpacking is complete.

    // source pointer starts by pointing at the last 32-bit word of 
    // the packed resource data
    ps = buffer + packed_size - 4;

    // the last 32-bits of the source data contain a 32-bit unsigned
    // integer which tells us the unpacked size of the data
    uint32_t unpacked_size = read_uint32_bigendian(ps); ps -= 4;

    // destination pointer starts by pointing at the last byte of
    // the unpacked resource memory area
    pd = buffer + unpacked_size - 1;

    // crc is only used to confirm file loaded correctly, it can be 
    // ignored with regard to the actual unpacking of the data
    crc = read_uint32_bigendian(ps); ps -= 4;

    // high bit is always set in `command` when read from the file, as
    // the `command` bits are shifted off one by one it needs the high
    // bit set to detect when the final shift occurs and to then
    // fetch the next command
    bit_stream = read_uint32_bigendian(ps); ps -= 4;
    crc ^= bit_stream;

    // the stream is composed of a mixture of commands and data
    // the commands are variable length encoded (either two or three 
    // bits) and tell the unpacker what to do with the following data
    //
    // all commands boil down to two options; either copy bytes from
    // the bitstream (copy() method) or copy bytes from the unpacked 
    // buffer at the specified offset (repeat_from_offset())
    do { 
      bool b0 = get_stream_bit(); 
      if(b0) {
        bool b1 = get_stream_bit();
        bool b2 = get_stream_bit();
        
        if(b1 && b2) {
          // 111 xxxxxxxx
          // copy between 9 and 275 (xxx + 9) bytes from bitstream to destination
          uint16_t count = get_value(8) + 9;
          copy(count);
        }
        
        if(b1 && !b2) {
          // 110 xxxxxxxx oooooooooooo
          // repeat between 2 and 256 bytes from destination + offset to destination
          uint16_t count = get_value(8) + 1;
          uint16_t offset = get_value(12);
          repeat_from_offset(count, offset);
        }
        
        if(!b1 && b2) {
          // 101 oooooooooo
          // repeat 4 bytes from from destination + offset to destination
          uint16_t count = 4;
          uint16_t offset = get_value(10);
          repeat_from_offset(count, offset);
        } 
      
        if(!b1 && !b2) {
          // 100 ooooooooo
          // repeat 3 bytes from from destination + offset to destination
          uint16_t count = 3;
          uint16_t offset = get_value(9);
          repeat_from_offset(count, offset);
        }      
      } 
      
      if(!b0) {
        bool b1 = get_stream_bit();

        if(b1) {
          // 11 oooooooo
          // repeat 2 bytes from from destination + offset to destination
          uint16_t offset = get_value(8);
          repeat_from_offset(2, offset);
        }
        
        if(!b1) {
          // 00 xxx
          // copy between 2 and 8 (xxx + 1) bytes from bitstream to destination
          uint16_t count = get_value(3) + 1;
          copy(count);
        }
      }

      if (pd - buffer < 100) {
        static int a = 0;
        a++;
      }
    } while((pd - buffer) != -1); // until we reach the end of the bitstream

    return crc == 0;
  }
};