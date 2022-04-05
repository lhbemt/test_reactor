#ifndef LEMT_MAIN_REACTOR__H
#define LEMT_MAIN_REACTOR__H

#include "reactor.hpp"
#include "acceptor.hpp"

// 处理accept fd
namespace lemt {
    class MainReactor : public Reactor {
    public:
        MainReactor() {
            Reactor();
        }

        virtual void start() override;

    protected:
        virtual void dispatch(epoll_event& ev) override;

    private:
        Acceptor acceptor;
    };
}

#endif