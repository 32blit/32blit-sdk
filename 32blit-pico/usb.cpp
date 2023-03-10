#include <cstdio>
#include <tuple>

#include "usb.hpp"
#include "multiplayer.hpp"

static uint8_t cur_header[8];
static int header_pos = 0;
static CDCCommand *cur_command = nullptr;

static constexpr uint32_t to_cmd_id(const char str[4]) {
  return str[0] | str[1] << 8 | str[2] << 16 | str[3] << 24;
}

static CDCHandshakeCommand handshake_command;
static CDCUserCommand user_command;

const std::tuple<uint32_t, CDCCommand *> cdc_commands[]{
  {to_cmd_id("MLTI"), &handshake_command},
  {to_cmd_id("USER"), &user_command}
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

      cur_command->init();
      header_pos = 0;
    }
  }
}