#ifndef LEMT_ACCEPTOR__H
#define LEMT_ACCEPTOR__H
#include "listen_socket.hpp"

namespace lemt {
    class Acceptor {
    public:
        Acceptor();
        Acceptor(const Acceptor&) = delete;
        Acceptor& operator=(const Acceptor&) = delete;
        ~Acceptor() {}
        
        std::error_code start();
        void accept_socket();

    public:
        int lfd;
    private:
        ListenSocket lsocket;
    };
}


#endif