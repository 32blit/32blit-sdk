#pragma once
#include <cstdint>
#include <string>

#include "SDL_net.h"

class Multiplayer final {
    public:
        enum class Mode {
            Auto,
            Listen,
            Connect
        };

        Multiplayer(Mode mode, const std::string &address);
        ~Multiplayer();

        void update();

        bool is_connected() const;
        void set_enabled(bool enabled);

        void send_message(const uint8_t *data, uint16_t length);

    private:
        void setup();
        void disconnect();

        Mode mode;
        std::string address;
        bool enabled = false;

        TCPsocket socket = nullptr, listen_socket = nullptr;
        SDLNet_SocketSet sock_set = nullptr;

        static const int retry_interval = 5000;
        Uint32 last_connect_time = 0;

        uint8_t *recv_buf = nullptr;
        uint16_t recv_len = 0, recv_off = 0;
};
