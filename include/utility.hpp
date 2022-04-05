#ifndef LEMT_UTILITY__H
#define LEMT_UTILITY__H

#include <system_error>

namespace lemt {
    using poll_event = uint32_t;
    const int sub_reactor_num = 3;
    const int event_num = 1000; // 每次epoll_wait返回时可处理的最大event数量
    const std::string ip = "127.0.0.1";
    const int port = 8000;
    const int buff_size = 1024;

    std::error_code set_nonblocking(int fd);
}

#endif