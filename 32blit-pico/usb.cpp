#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <tuple>

#include "hardware/flash.h"

#include "usb.hpp"
#include "blit_launch.hpp"
#include "multiplayer.hpp"
#include "overlay.hpp"

#include "engine/engine.hpp"
#include "executable.hpp"

#define MAX_FILENAME 256+1
#define MAX_FILELEN 16+1

void init();

static uint8_t cur_header[8];
static int header_pos = 0;
static CDCCommand *cur_command = nullptr;

static constexpr uint32_t to_cmd_id(const char str[4]) {
  return str[0] | str[1] << 8 | str[2] << 16 | str[3] << 24;
}

class CDCParseBuffer final {
public:
  void reset() {
    offset = 0;
  }

  uint8_t *get_data() {
    return buffer;
  }

  uint8_t *get_current_ptr() {
    return buffer + offset;
  }

  void add_read(uint32_t count) {
    offset += count;
    assert(offset <= buffer_size);
  }

  uint32_t get_offset() const {
    return offset;
  }

private:
  static const size_t buffer_size = std::max((unsigned)std::max(MAX_FILELEN, MAX_FILENAME), FLASH_PAGE_SIZE);
  uint8_t buffer[buffer_size];

  uint32_t offset = 0;
};

/*
  Helper to read a string
  returns Done when the string is ready (in buf.get_ptr())
  returns Error when we've reached the max length without a terminator
  returns Continue when there is currently no more data to read
*/
static CDCCommand::Status cdc_read_string(CDCParseBuffer &buf, uint32_t max_len) {
  while(true) {
    auto buf_ptr = buf.get_current_ptr();

    if(!usb_cdc_read(buf_ptr, 1))
      return CDCCommand::Status::Continue;

    // end of string
    if(*buf_ptr == 0)
      return CDCCommand::Status::Done;

    buf.add_read(1);

    // too long
    if(buf.get_offset() == max_len)
      return CDCCommand::Status::Error;
  }
}

// helper in an attempt to keep commands more generic...
static void cdc_command_progress(const char *message, uint32_t progress, uint32_t total) {
  set_render_overlay_enabled(message != nullptr || total);

  // null message preserves the old one
  if(message)
    set_overlay_message(message);

  set_overlay_progress(progress, total);
}

class CDCProgCommand final : public CDCCommand {
public:
  CDCProgCommand(CDCParseBuffer &buf) : buf(buf) {}

  void init() override {
    parse_state = ParseState::Filename;
    buf.reset();
  }

  Status update() override {

    while(true) {
      switch(parse_state) {
        case ParseState::Filename: {
          auto status = cdc_read_string(buf, MAX_FILENAME);
          if(status == Status::Done) {
            parse_state = ParseState::Length;
            buf.reset();
            continue;
          }

          return status;
        }

        case ParseState::Length: {
          auto status = cdc_read_string(buf, MAX_FILELEN);
          if(status == Status::Done) {
            auto file_len = strtoul((const char *)buf.get_data(), nullptr, 10);
            parse_state = ParseState::Data;
            buf.reset();

            writer.init(file_len);
            continue;
          }

          return status;
        }

        case ParseState::Data: {
          // read data
          auto max = std::min(uint32_t(FLASH_PAGE_SIZE), writer.get_remaining()) - buf.get_offset();
          auto read = usb_cdc_read(buf.get_current_ptr(), max);

          if(!read)
            return Status::Continue;

          buf.add_read(read);

          // got full page or final part of file
          auto buf_off = buf.get_offset();
          if(buf_off == FLASH_PAGE_SIZE || buf_off == writer.get_remaining()) {
            if(!writer.write(buf.get_data(), buf_off))
              return Status::Error;

            buf.reset();
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

  CDCParseBuffer &buf;

  BlitWriter writer;
};

class CDCSaveCommand final : public CDCCommand {
public:
  CDCSaveCommand(CDCParseBuffer &buf) : buf(buf) {}

  void init() override {
    parse_state = ParseState::Filename;
    file_offset = 0;
    buf.reset();
  }

  Status update() override {
    while(true) {
      switch(parse_state) {
        case ParseState::Filename: {
          auto status = cdc_read_string(buf, MAX_FILENAME);
          if(status == Status::Done) {
            auto filename = (const char *)buf.get_data();

            file = blit::api.open_file(filename, blit::OpenMode::write);
            if(!file) {
              blit::debugf("Failed to open %s", filename);
              return Status::Error;
            }

            // setup progress message
            char message_buf[300];
            snprintf(message_buf, sizeof(message_buf), "Saving %s...", filename);
            cdc_command_progress(message_buf, 0, 0);

            parse_state = ParseState::Length;
            buf.reset();
            continue;
          }

          return status;
        }

        case ParseState::Length: {
          auto status = cdc_read_string(buf, MAX_FILELEN);
          if(status == Status::Done) {
            file_length = strtoul((const char *)buf.get_data(), nullptr, 10);

            cdc_command_progress(nullptr, 0, file_length);

            parse_state = ParseState::Data;
            buf.reset();
            continue;
          }

          return status;
        }

        case ParseState::Data: {
          // read data
          auto max = std::min(uint32_t(FLASH_PAGE_SIZE), file_length - file_offset);
          auto read = usb_cdc_read(buf.get_current_ptr(), max);

          if(!read)
            return Status::Continue;

          // write whatever we got, the fs is going to buffer anyway
          auto written = blit::api.write_file(file, file_offset, read, (const char *)buf.get_data());
          if(written != read) {
            blit::api.close_file(file);
            return Status::Error;
          }

          file_offset += read;

          cdc_command_progress(nullptr, file_offset, file_length);

          // end of file
          if(file_offset == file_length) {
            blit::api.close_file(file);
            cdc_command_progress(nullptr, 0, 0); // clear progress

            // send response
            uint8_t res_data[]{'3', '2', 'B', 'L', '_', '_', 'O', 'K'};
            usb_cdc_write(res_data, sizeof(res_data));
            usb_cdc_flush_write();

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

  CDCParseBuffer &buf;

  void *file = nullptr;
  uint32_t file_length = 0, file_offset = 0;
};

class CDCListCommand final : public CDCCommand {
  void init() override {
  }

  Status update() override {
    blit::api.list_installed_games([](const uint8_t *ptr, uint32_t block, uint32_t size) {
      // bit of a mismatch between the API and the CDC API
      // this wants offset in bytes and the size WITHOUT metadata
      uint32_t offset_bytes = block * game_block_size;
      uint32_t header_size = ((BlitGameHeader *)ptr)->end & 0x1FFFFFF;

      usb_cdc_write((uint8_t *)&offset_bytes, sizeof(uint32_t));
      usb_cdc_write((uint8_t *)&header_size, sizeof(uint32_t));
      usb_cdc_flush_write();

      // write metadata if found
      auto meta = ptr + header_size;

      if(memcmp(meta, "BLITMETA", 8) == 0) {
        auto meta_size = *(uint16_t *)(meta + 8);
        usb_cdc_write(meta, meta_size + 10);
      } else {
        // no meta, write header + 0 len
        usb_cdc_write((const uint8_t *)"BLITMETA\0", 10);
      }
      usb_cdc_flush_write();
    });

    // end marker
    uint32_t end = 0xFFFFFFFF;
    usb_cdc_write((uint8_t *)&end, sizeof(uint32_t));
    usb_cdc_flush_write();

    return Status::Done;
  }
};

class CDCLaunchCommand final : public CDCCommand {
public:
  CDCLaunchCommand(CDCParseBuffer &buf) : buf(buf) {}

  void init() override {
    buf.reset();
  }

  Status update() override {
    auto status = cdc_read_string(buf, MAX_FILENAME);

    if(status == Status::Done)
      blit::api.launch((const char *)buf.get_data());

    return status;
  }

  CDCParseBuffer &buf;
};

class CDCEraseCommand final : public CDCCommand {
public:
  CDCEraseCommand(CDCParseBuffer &buf) : buf(buf) {}

  void init() override {
    buf.reset();
  }

  Status update() override {
    while(true) {
        auto buf_ptr = buf.get_current_ptr();
        if(!usb_cdc_read(buf_ptr, 1))
          return Status::Continue;

        buf.add_read(1);

        // end of word
        if(buf.get_offset() == 4) {
          blit::api.erase_game(*(uint32_t *)buf.get_data());
          return Status::Done;
        }
    }

    return Status::Continue;
  }

  CDCParseBuffer &buf;
};


static CDCHandshakeCommand handshake_command;
static CDCUserCommand user_command;

#if defined(BUILD_LOADER) && !defined(USB_HOST)
#define FLASH_COMMANDS
static CDCParseBuffer parse_buffer;
static CDCProgCommand prog_command(parse_buffer);
static CDCSaveCommand save_command(parse_buffer);
static CDCListCommand list_command;
static CDCLaunchCommand launch_command(parse_buffer);
static CDCEraseCommand erase_command(parse_buffer);
#endif

const std::tuple<uint32_t, CDCCommand *> cdc_commands[]{
  {to_cmd_id("MLTI"), &handshake_command},
  {to_cmd_id("USER"), &user_command},

#ifdef FLASH_COMMANDS
  {to_cmd_id("PROG"), &prog_command},
  {to_cmd_id("SAVE"), &save_command},
  {to_cmd_id("__LS"), &list_command},
  {to_cmd_id("LNCH"), &launch_command},
  {to_cmd_id("ERSE"), &erase_command},
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
