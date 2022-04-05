#ifndef LEMT__SUB_REACTOR_SOCKET__H
#define LEMT__SUB_REACTOR_SOCKET__H

#include "reactor.hpp"
#include "client_socket.hpp"
#include <map>

namespace lemt {
    class SubReactorSocket : public Reactor {
    public:
        SubReactorSocket(int idx) : idx(idx) {
            Reactor();
        }
        SubReactorSocket(SubReactorSocket&& other) : idx(other.idx) {
            Reactor(std::move(other)); // 必须显示调用父类的移动构造函数
            clients.swap(other.clients);
        }
    public:
        std::error_code register_fd(int fd, poll_event poll_events) {
            std::error_code ec = Reactor::register_fd(fd, poll_events);
            if (ec.value() == 0){
                clients.insert(std::make_pair(fd, std::make_shared<ClientSocket>(fd, idx))); // 这里可以使用mialloc等优化内存的第三方内存分配库
            }
            return ec;
        }

        std::error_code unregister_sub_reactor(int fd) {
            std::error_code ec = Reactor::unregister_fd(fd);
            clients.erase(fd);
            return ec;
        }

    protected:
        virtual void dispatch(epoll_event& ev) override {
            clients[ev.data.fd]->handle(ev.events);
        }

    private:
        std::map<int, std::shared_ptr<ClientSocket>> clients;
        int idx;
    };
}

#endif