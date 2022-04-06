#ifndef LEMT_CLIENT_SOCKET__H
#define LEMT_CLIENT_SOCKET__H

#include "utility.hpp"
#include <array>
#include <unistd.h>

namespace lemt {
    class ClientSocket {
    public:
        ClientSocket(int fd, int idx) : fd(fd), reactor_idx(idx) {}
        ClientSocket(const ClientSocket&) = delete;
        ClientSocket& operator=(const ClientSocket&) = delete;
        ~ClientSocket() {
            close(fd);
        }

        void handle(poll_event events); // 处理socket数据

    private:
        void handle_read();
        void handle_send();

    private:
        int fd;
        int reactor_idx;
        std::array<char, buff_size> send_buf;
        std::array<char, buff_size> recv_buf;
    };
}




#endif
