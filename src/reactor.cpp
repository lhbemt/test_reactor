#include <cerrno>
#include "reactor.hpp"
#include "reactor_exception.hpp"
#include <sys/epoll.h>
#include <iostream>

namespace lemt {
    Reactor::Reactor() {
        epoll_fd = epoll_create(5); // 自linux2.6起，epoll_create的参数被忽略，大于0即可。
        if (epoll_fd == -1) {
            throw ReactorException(std::error_code(errno, std::system_category()));
        }
        if (pipe(stop_fd) == -1) {
            close(epoll_fd);
            throw ReactorException(std::error_code(errno, std::system_category()));
        }
        std::error_code ec = register_fd(stop_fd[0], EPOLLIN);
        if (ec.value() != 0) {
            close(epoll_fd);
            close(stop_fd[0]);
            close(stop_fd[1]);
            throw ReactorException(std::error_code(errno, std::system_category()));
        }
    }

    void Reactor::run() {
        while(true) {
            int ret = epoll_wait(epoll_fd, &event_array[0], event_num, -1);
            if (ret == -1 && errno == EINTR)
                continue;
            for (int i = 0; i < ret; ++i) {
                if (event_array[i].data.fd == stop_fd[0]) {
                    std::cout << "stop reactor: " << stop_fd[0] << std::endl;
                    return; // stop
                }
                dispatch(event_array[i]);
            }
        }
    }

    std::error_code Reactor::register_fd(int fd, poll_event events) {
        std::error_code err = set_nonblocking(fd);
        if (err.value() != 0) {
            return err;
        }
        return set_fd_events(fd, events, EPOLL_CTL_ADD);
    }

    std::error_code Reactor::unregister_fd(int fd) {
        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
            return std::error_code(errno, std::system_category());
        }
        return std::error_code();
    }

    std::error_code Reactor::update_fd_events(int fd, poll_event events) {
        return set_fd_events(fd, events, EPOLL_CTL_MOD);
    }

    std::error_code Reactor::set_fd_events(int fd, poll_event events, int etype) {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd, etype, fd, &ev) == -1) {
            return std::error_code(errno, std::system_category());
        }
        return std::error_code();
    }
}