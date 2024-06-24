#include <cstdio>
#include <cstdlib>
#include <tuple>

#include "hardware/flash.h"

#include "usb.hpp"
#include "blit-launch.hpp"
#include "multiplayer.hpp"

#include "engine/engine.hpp"
#include "executable.hpp"

void init();

static uint8_t cur_header[8];
static int header_pos = 0;
static CDCCommand *cur_command = nullptr;

static constexpr uint32_t to_cmd_id(const char str[4]) {
  return str[0] | str[1] << 8 | str[2] << 16 | str[3] << 24;
}

#define MAX_FILENAME 256+1
#define MAX_FILELEN 16+1

class CDCProgCommand final : public CDCCommand {
  void init() override {
    parse_state = ParseState::Filename;
    buf_off = 0;
  }

  Status update() override {
    while(true) {
      switch(parse_state) {
        case ParseState::Filename: {
          if(!usb_cdc_read(buf + buf_off, 1))
            return Status::Continue;

          // end of string
          if(buf[buf_off] == 0) {
            printf("PROG: file %s\n", buf);
            parse_state = ParseState::Length;
            buf_off = 0;
            continue;
          }

          buf_off++;

          // too long
          if(buf_off == MAX_FILENAME)
            return Status::Error;

          break;
        }

        case ParseState::Length: {
          if(!usb_cdc_read(buf + buf_off, 1))
            return Status::Continue;

          // end of string
          if(buf[buf_off] == 0) {
            auto file_len = strtoul((const char *)buf, nullptr, 10);
            printf("PROG: len %lu\n", file_len);
            parse_state = ParseState::Data;
            buf_off = 0;

            writer.init(file_len);
            continue;
          }

          buf_off++;

          // too long
          if(buf_off == MAX_FILELEN)
            return Status::Error;

          break;
        }

        case ParseState::Data: {
          // read data
          auto max = std::min(uint32_t(FLASH_PAGE_SIZE), writer.get_remaining()) - buf_off;
          auto read = usb_cdc_read(buf + buf_off, max);

          if(!read)
            return Status::Continue;

          buf_off += read;

          // got full page or final part of file
          if(buf_off == FLASH_PAGE_SIZE || buf_off == writer.get_remaining()) {
            if(!writer.write(buf, buf_off))
              return Status::Error;

            buf_off = 0;
          }

          // end of file
          if(writer.get_remaining() == 0) {
            // send response
            auto block = writer.get_flash_offset() >> 16;
            uint8_t res_data[]{'3', '2', 'B', 'L', '_', '_', 'O', 'K', uint8_t(block), uint8_t(block >> 8)};
            usb_cdc_write(res_data, sizeof(res_data));
            usb_cdc_flush_write();

            // reinit loader
            ::init();

            return Status::Done;
          }

          break;
        }
      }
    }

    return Status::Continue;
  }

  enum class ParseState {
    Filename,
    Length,
    Data
  } parse_state = ParseState::Filename;

  static const size_t buf_size = std::max((unsigned)std::max(MAX_FILELEN, MAX_FILENAME), FLASH_PAGE_SIZE);
  uint8_t buf[buf_size];
  uint32_t buf_off = 0;

  BlitWriter writer;
};

static CDCHandshakeCommand handshake_command;
static CDCUserCommand user_command;

#if defined(BUILD_LOADER) && !defined(USB_HOST)
#define FLASH_COMMANDS
static CDCProgCommand prog_command;
#endif

const std::tuple<uint32_t, CDCCommand *> cdc_commands[]{
  {to_cmd_id("MLTI"), &handshake_command},
  {to_cmd_id("USER"), &user_command},

#ifdef FLASH_COMMANDS
  {to_cmd_id("PROG"), &prog_command},
#endif
};

void usb_cdc_update() {
  while(usb_cdc_read_available()) {
    // command in progress
    if(cur_command) {
      auto res = cur_command->update();

      if(res == CDCCommand::Status::Continue) {
        break;
      } else {
        // done/error
        cur_command = nullptr;
      }

      continue;
    }

    // match header
    if(header_pos < 8) {
      usb_cdc_read(cur_header + header_pos, 1);

      const char *expected = "32BL";
      if(header_pos >= 4 || cur_header[header_pos] == expected[header_pos])
        header_pos++;
      else
        header_pos = 0;
    } else {

      // got header
      auto command_id = to_cmd_id((char *)cur_header + 4);

      // find command
      for(auto &cmd : cdc_commands) {
        if(std::get<0>(cmd) == command_id) {
          cur_command = std::get<1>(cmd);
          break;
        }
      }

      if(!cur_command)
        printf("got: %c%c%c%c%c%c%c%c\n", cur_header[0], cur_header[1], cur_header[2], cur_header[3], cur_header[4], cur_header[5], cur_header[6], cur_header[7]);
      else
        cur_command->init();

      header_pos = 0;
    }
  }
}
