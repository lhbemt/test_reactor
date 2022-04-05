#include "listen_socket.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace lemt {
    ListenSocket::ListenSocket() {
        init();
    }

    void ListenSocket::init() {
        lfd = socket(AF_INET, SOCK_STREAM, 0);
        if (lfd == -1) {
            throw SocketException(std::error_code(errno, std::system_category()));
        }
        struct sockaddr_in s_addr;
        std::memset(&s_addr, 0, sizeof(s_addr));
        s_addr.sin_family = AF_INET;
        s_addr.sin_port = htons(port);
        s_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        int one = 1;
        int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        if (ret == -1) {
            close(lfd);
            throw SocketException(std::error_code(errno, std::system_category()));
        }
        std::error_code err = set_nonblocking(lfd);
        if (err.value() != 0) {
            close(lfd);
            throw SocketException(err);
        }
        ret = bind(lfd, (struct sockaddr*)(&s_addr), sizeof(s_addr));
        if (ret == -1) {
            close(lfd);
            throw SocketException(std::error_code(errno, std::system_category()));
        }
        ret = listen(lfd, 5);
        if (ret == -1) {
            close(lfd);
            throw SocketException(std::error_code(errno, std::system_category()));
        }
    }
}