#include "client_socket.hpp"
#include <sys/epoll.h>
#include <iostream>
#include "reactor_mgr.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>

namespace lemt {
    void ClientSocket::handle(poll_event events) {
        if (events & EPOLLIN) { // 读取数据
            handle_read();
            return;
        }   
        if (events & EPOLLOUT) { // 发送数据
            handle_send();
            return;
        }
        return;
    }

    void ClientSocket::handle_read() {
        int n = recv(fd, &recv_buf[0], buff_size, 0); // 这里会出现信号11 导致core dump，这个是recv_buf的原因
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) { // try agin
                std::cout << fd << " recv ewould_block: " << errno << std::endl;
                return;
            }
            else { // 错误
                std::cout << fd << " recv error: " << errno << std::endl;
                ReactorMgr::get_instance()->unregister_sub_reactor(reactor_idx, fd);
                return;
            }
        }
        if (n == 0) { // 关闭
            ReactorMgr::get_instance()->unregister_sub_reactor(reactor_idx, fd);
            return;
        }
//        std::cout << fd << " recv: " << n << " " << &recv_buf[0] << std::endl;
        std::memcpy(&send_buf[0], &recv_buf[0], n);
        std::memset(&recv_buf[0], 0, buff_size);
        handle_send();
    }

    void ClientSocket::handle_send() {
        int err = send(fd, &send_buf[0], buff_size, 0);
        if (err == -1) { // EPOLLOUT
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                ReactorMgr::get_instance()->update_fd_events(fd, EPOLLIN|EPOLLOUT);
                return;
            }
            else {
                std::cout << fd << " send error: " << errno << std::endl;
                ReactorMgr::get_instance()->unregister_sub_reactor(reactor_idx, fd);
                return;
            }
        }
        else { // success
            ReactorMgr::get_instance()->update_fd_events(fd, EPOLLIN); // 去除EPOLLOUT
            return;
        }        
    }

}