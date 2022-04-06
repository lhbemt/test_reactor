#include <iostream>
#include "utility.hpp"
#include <thread>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/select.h>
#include <unistd.h>
#include <algorithm>

void do_thread(int);
void handle_client(int);

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "usage: " << argv[0] << " thread_num " << "client_num" << std::endl;
        return -1;
    }

    int thread_num = atoi(argv[1]);
    int client_num = atoi(argv[2]);

    std::vector<std::thread> threads;
    threads.reserve(thread_num);
    for (int i = 0; i < thread_num; ++i) {
        threads.emplace_back(std::thread(&do_thread, client_num));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "done!" << std::endl;
}

void do_thread(int client_num) {
    struct sockaddr_in serv_addr;
	std::memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(lemt::port);
	serv_addr.sin_addr.s_addr = inet_addr(lemt::ip.c_str());
    fd_set set;
    FD_ZERO(&set);
    int connect_cout = 0;
    int maxfd = 0;
    std::vector<int> client_fds;
    client_fds.reserve(client_num);
    for (int i = 0; i < client_num; ++i) {
        int clientfd = socket(AF_INET, SOCK_STREAM, 0);
        std::error_code ec = lemt::set_nonblocking(clientfd);
        if (ec.value() != 0) {
            std::cout << "socket error: " << ec.message() << std::endl;
        }
        int err = connect(clientfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (err == 0) { // 连接马上成功了
            handle_client(clientfd);
            connect_cout += 1;
        }
        else {
            if (errno == EALREADY) { // 该socket的前一次连接还未完成 出现于同一个socket的多次连接 第一次不会出现
                std::cout << "ealready" << std::endl;
                continue;
            }
            if (errno == EINPROGRESS) { // 不能马上建立连接 建立中
                FD_SET(clientfd, &set);
                client_fds.push_back(clientfd);
                maxfd = clientfd > maxfd ? clientfd : maxfd;
                continue;
            }
            std::cout << "unkown error " << errno << std::endl;
        }
    }

    // 超时检测是否连接成功
    struct timeval timeo;
    timeo.tv_sec = 10; // 10s
    timeo.tv_usec = 0;
    while(!client_fds.empty()) {
        int err = select(maxfd + 1, NULL, &set, NULL, &timeo);
        if (err == -1) {
            std::cout << "select erro: " << std::endl;
            return;
        }
        for (int fd = 0; fd < maxfd + 1; ++fd) {
            if (FD_ISSET(fd, &set)) { // 也许成功了 用getsockopt 判断
                err = 0;
                socklen_t len = sizeof(err);
                auto iter = std::find(client_fds.begin(), client_fds.end(), fd);
                client_fds.erase(iter);
                if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) { // 连接建立失败
                    std::cout << " getsockopt error: " << errno << std::endl;
                    close(fd);
                    continue;
                }
                else {
                    if (err != 0) { // 失败
                        std::cout << " getsockopt connect error: " << errno << std::endl;
                        continue;
                    }
                    handle_client(fd);
                    connect_cout += 1;
                }        
            }
        }
        FD_ZERO(&set);
        maxfd = 0;
        for (auto& left : client_fds) {
            maxfd = left > maxfd ? left : maxfd;
            FD_SET(left, &set);
        }
    }

    std::cout << "connect_count: " << connect_cout << std::endl;
}

void handle_client(int fd) {
    while(true) {
        int n = write(fd, "ping", std::strlen("ping"));
        if (n > 0) {
            close(fd);
            return;
        }
        if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        } 
        else {
            std::cout << "send error: " << errno << std::endl;
            close(fd);
            return;
        } 
    }
}