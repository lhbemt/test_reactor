#ifndef LEMT_REACTOR_MGR__H
#define LEMT_REACTOR_MGR__H

#include <mutex>
#include <memory>
#include <vector>
#include "main_reactor.hpp"
#include "sub_reactor_socket.hpp"
#include "utility.hpp"
#include <random>

// reactor管理类单例

// 不同的线程对同一个epollfd操作到底是不是线程安全的，在网上看到这样的说法：
// 但是实际程序运行过程出现了这样的现象: A线程正好从某次epoll_wait调用退出的时候, B线程加入的那个socket上发生的事件消失了
// (对应epoll_ctl返回值是0, 没有显示错误).
// Google后得到的信息都是认为前述写法不存在问题, 但是偶然在一个github的项目的issue看到不一样的说法: 
// epoll_wait() is oblivious to concurrent updates via epoll_ctl() · Issue #331 · cloudius-systems/osv · GitHub,
// 所以将原来的写法改了, B线程不能直接调用epoll_ctl, 而是写一个pipe唤醒A线程, 在A线程执行对应的操作. 改了之后bug没再出现了.

// 但是其实上面是有问题的：除了一个线程在epoll或者select中监控一个socket时候另外一个线程对这个socket进行close这种情况，
// 我就可以认为多个线程操作同一个epoll fd的行为是安全的。这是陈硕的观点，即只要是在相同线程close，就没问题。
// https://blog.csdn.net/cws1214/article/details/47909323

namespace lemt {
    class ReactorMgr {
    private:
        ReactorMgr() {
            sub_reactors.reserve(sub_reactor_num);
            e = std::default_random_engine(r());
            uniform_dist = std::uniform_int_distribution<int>(0, sub_reactor_num);
            for (int i = 0; i < sub_reactor_num; ++i) {
                sub_reactors.emplace_back(SubReactorSocket(i));
            }
        }
    public:
        static std::unique_ptr<ReactorMgr>& get_instance() {
            if (!instance) {
                std::call_once(init_flag, [&](){ instance.reset(new ReactorMgr());});
            }
            return instance;
        }

        inline std::error_code register_main_reactor(int fd, poll_event events) {
            return main_reactor.register_fd(fd, events);
        }

        inline std::error_code unregister_main_reactor(int fd) {
            return main_reactor.unregister_fd(fd);
        }

        inline std::error_code update_fd_events(int fd, poll_event events) {
            return main_reactor.update_fd_events(fd, events);
        }

        inline std::pair<std::error_code, int> register_sub_reactor(int fd, poll_event events) {
            int idx = uniform_dist(e);
            std::error_code ec = sub_reactors[idx].register_fd(fd, events);
            return std::make_pair(ec, idx);
        }

        inline std::error_code unregister_sub_reactor(int idx, int fd) {
            return sub_reactors[idx].unregister_fd(fd);
        }

        inline std::error_code update_fd_events(int idx, int fd, poll_event events) {
            return sub_reactors[idx].update_fd_events(fd, events);
        }

        void start() {
            main_reactor.start();
            for (auto& sub_reactor : sub_reactors) {
                sub_reactor.start();
            }
        }

        void stop() {
            main_reactor.stop();
            for (auto& sub_reactor : sub_reactors) {
                sub_reactor.stop();
            }
        }

    private:
        static std::unique_ptr<ReactorMgr> instance;

        MainReactor main_reactor;
        std::vector<SubReactorSocket> sub_reactors;
        static std::once_flag init_flag;
        // 随机数
        std::random_device r;
        std::default_random_engine e;
        std::uniform_int_distribution<int> uniform_dist;
    };
}


#endif