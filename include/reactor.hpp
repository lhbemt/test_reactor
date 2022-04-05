#ifndef LEMT_REACTOR__H
#define LEMT_REACTOR__H

#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include <system_error>
#include "utility.hpp"
#include <array>
#include <thread>
#include <cstring>
#include <iostream>

namespace lemt {
    class Reactor {
    public:
        Reactor();
        ~Reactor() {
            if (t.joinable()) {
                t.join();
            }
            close(epoll_fd);
            close(stop_fd[0]);
            close(stop_fd[1]);
        }

        Reactor(Reactor&& other) : epoll_fd(other.epoll_fd), t(std::move(t)) {
            std::memcpy(stop_fd, other.stop_fd, sizeof(stop_fd));
            std::memcpy(&event_array[0], &other.event_array[0], sizeof(event_array));
        }

        std::error_code register_fd(int fd, poll_event poll_events);
        std::error_code unregister_fd(int fd);
        std::error_code update_fd_events(int fd, poll_event poll_events);
        
        virtual void start() {
            t = std::move(std::thread(&Reactor::run, this));
        }

        void stop() {
            std::cout << "reactor stop begin: " << stop_fd[1] << std::endl;
            write(stop_fd[1], "stop", strlen("stop"));
        }

    protected:
        virtual void dispatch(epoll_event& ev) {}
        virtual void clear() {} // 清理

    private:
        void run();
        std::error_code set_fd_events(int fd, poll_event poll_events, int e_type);
        
    private:
        int epoll_fd;
        int stop_fd[2];
        std::thread t;
        std::array<epoll_event, event_num> event_array;
    };
}

#endif