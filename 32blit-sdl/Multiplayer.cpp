#include <iostream>

#include "Multiplayer.hpp"

#include "engine/api_private.hpp"

using namespace blit;

Multiplayer::Multiplayer(Mode mode, const std::string &address) : mode(mode), address(address) {
    // shouldn't fail unless we ran out of memory
    sock_set = SDLNet_AllocSocketSet(2);
}

Multiplayer::~Multiplayer() {
    if(sock_set)
        SDLNet_FreeSocketSet(sock_set);

    if(socket)
        SDLNet_TCP_Close(socket);

    if(listen_socket)
        SDLNet_TCP_Close(listen_socket);
}

void Multiplayer::update() {
    if(!socket && !listen_socket) {
        // attempt to reconnect
        auto now = SDL_GetTicks();
        if((now - last_connect_time) > retry_interval) {
            setup();

            last_connect_time = now;
        }
        return;
    }

    int num_ready;

    while((num_ready = SDLNet_CheckSockets(sock_set, 0))) {
      if(num_ready == -1) {
          std::cerr << "Failed to check socket: " << SDLNet_GetError() << std::endl;
          return;
      }

      if(listen_socket && SDLNet_SocketReady(listen_socket)) {
          // new connection
          socket = SDLNet_TCP_Accept(listen_socket);
          if(socket) {
              auto remote_addr = SDLNet_TCP_GetPeerAddress(socket);
              auto ip = SDL_SwapBE32(remote_addr->host);
              std::cout << (ip >> 24) << "." << ((ip >> 16) & 0xFF) << "." << ((ip >> 8) & 0xFF) << "." << (ip & 0xFF) << " connected" << std::endl;

              SDLNet_TCP_AddSocket(sock_set, socket);

              SDLNet_TCP_Send(socket, "32BLMLTI\1", 9);

              // stop listening now
              stop_listening();
          }
      }

      if(!socket || !SDLNet_SocketReady(socket))
          return;

      if(!recv_buf) {
          // read header and setup

          auto read = SDLNet_TCP_Recv(socket, head_buf + head_off, 8 - head_off);

          if(read <= 0) {
            disconnect();
            return;
          }

          head_off += read;

          if(head_off < 8)
            continue;

          if(memcmp(head_buf, "32BLUSER", 8) == 0) {
            read = SDLNet_TCP_Recv(socket, head_buf, 2);

            if(read <= 0) {
              disconnect();
              return;
            }

            recv_len = head_buf[0] | (head_buf[1] << 8);
            recv_buf = new uint8_t[recv_len];
            recv_off = 0;
            head_off = 0;

          } else if(memcmp(head_buf, "32BLMLTI", 8) == 0) {
            // handle the handshake packet
            SDLNet_TCP_Recv(socket, head_buf, 1);
            handshake = head_buf[0] != 0;

            if(mode == Mode::Connect && head_buf[0] == 1)
              SDLNet_TCP_Send(socket, "32BLMLTI\2", 9);

            head_off = 0;
          } else {
            std::cerr << "Unexpected header: " << std::string(reinterpret_cast<char *>(head_buf), 8) << std::endl;
            head_off = 0;
          }

          if(!recv_buf || SDLNet_CheckSockets(sock_set, 0) <= 0)
              return;
      }

      auto read = SDLNet_TCP_Recv(socket, recv_buf + recv_off, recv_len - recv_off);
      if(read <= 0) {
          // failed/disconnected
          delete[] recv_buf;
          recv_buf = nullptr;
          disconnect();
          return;
      }

      recv_off += read;

      // got message, pass to user
      if(recv_off == recv_len) {
          if(api.message_received)
              api.message_received(recv_buf, recv_len);

          delete[] recv_buf;
          recv_buf = nullptr;
      }
    }
}

bool Multiplayer::is_connected() const {
    return socket != nullptr && handshake;
}

void Multiplayer::set_enabled(bool enabled) {
    if(enabled) {
        setup();
    } else {
        disconnect();

        if(listen_socket) {
            SDLNet_TCP_DelSocket(sock_set, listen_socket);
            SDLNet_TCP_Close(listen_socket);
        }
    }

    this->enabled = enabled;
}

void Multiplayer::send_message(const uint8_t *data, uint16_t length) {
    if(!socket)
        return;

    uint8_t head[]{
        '3', '2', 'B', 'L',
        'U', 'S', 'E', 'R',
        static_cast<uint8_t>(length),
        static_cast<uint8_t>(length >> 8)
    };

    if(SDLNet_TCP_Send(socket, head, 10) != 10) {
        // failed
        disconnect();
        return;
    }

    auto sent = SDLNet_TCP_Send(socket, data, length);
    if(sent < length) {
        // failed
        disconnect();
    }
}

void Multiplayer::setup() {
    const uint16_t port = 0x32B1;

    IPaddress ip;

    // try connecting first for auto
    if(mode != Mode::Listen) {
        if(SDLNet_ResolveHost(&ip, address.c_str(), port) == -1) {
            std::cerr << "Failed to resolve host: " << SDLNet_GetError() << std::endl;
            return;
        }

        socket = SDLNet_TCP_Open(&ip);
    }

    if(!socket && mode != Mode::Connect) {
        // try hosting instead unless connecting was specified
        if(SDLNet_ResolveHost(&ip, nullptr, port) == -1) {
            std::cerr << "Failed to resolve host: " << SDLNet_GetError() << std::endl;
            return;
        }

        listen_socket = SDLNet_TCP_Open(&ip);
    }

    if(!socket && !listen_socket) {
        std::cerr << "Failed to open socket: " << SDLNet_GetError() << std::endl;
        return;
    }

    if(listen_socket) {
        SDLNet_TCP_AddSocket(sock_set, listen_socket);
        mode = Mode::Listen;
    } else {
        SDLNet_TCP_AddSocket(sock_set, socket);
        mode = Mode::Connect;
    }
}

void Multiplayer::disconnect() {
    if(!socket)
        return;

    SDLNet_TCP_DelSocket(sock_set, socket);

    SDLNet_TCP_Close(socket);
    socket = nullptr;

    handshake = false;
}

void Multiplayer::stop_listening() {
    SDLNet_TCP_DelSocket(sock_set, listen_socket);

    SDLNet_TCP_Close(listen_socket);
    listen_socket = nullptr;
}
