#pragma once
#include <cstdint>

#include "SDL_net.h"

class Multiplayer final {
    public:
        Multiplayer();
        ~Multiplayer();

        void update();

        void send_message(const uint8_t *data, uint16_t length);

    private:
        void disconnect();

        TCPsocket socket = nullptr, listen_socket = nullptr;
        SDLNet_SocketSet sock_set = nullptr;

        uint8_t *recv_buf = nullptr;
        uint16_t recv_len = 0, recv_off = 0;
};