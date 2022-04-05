#ifndef LEMT_LISTEN_SOCKET__H
#define LEMT_LISTEN_SOCKET__H
#include "reactor_exception.hpp"
#include "utility.hpp"
#include <unistd.h>

namespace lemt{
    class ListenSocket {
    public:
        ListenSocket();
        ListenSocket(const ListenSocket& other) = delete;
        ListenSocket& operator=(const ListenSocket& other) = delete;
        ~ListenSocket() {
            close(lfd);
        }

        inline int get_fd() {
            return lfd;
        }

    private:
        void init();

    private:
        int lfd;
    };
}









#endif