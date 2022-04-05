#include "acceptor.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include "reactor_mgr.hpp"

namespace lemt {

    Acceptor::Acceptor() {
        lfd = lsocket.get_fd();
    }

    std::error_code Acceptor::start() {
        return ReactorMgr::get_instance()->register_main_reactor(lfd, EPOLLIN);
    }

    void Acceptor::accept_socket() {
        int client_fd = accept(lfd, NULL, NULL);
        if (client_fd == -1) {
            return;
        }
        // client_fd分到subreactor里
        ReactorMgr::get_instance()->register_sub_reactor(client_fd, EPOLLIN);
    }
}