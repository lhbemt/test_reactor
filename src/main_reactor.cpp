#include "main_reactor.hpp"
#include <cassert>
#include "reactor_exception.hpp"
//#include "reactor_api.hpp"

namespace lemt {

    void MainReactor::start() {
        std::error_code ec;// = r_api::register_main_reactor(acceptor.lfd, EPOLLIN);
        if (ec.value() != 0) {
            throw ReactorException(ec);
        }
        Reactor::start();
        acceptor.start();
    }

    void MainReactor::dispatch(epoll_event& ev) {
        acceptor.accept_socket();
    }
}