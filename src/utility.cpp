#include "utility.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

namespace lemt {
    std::error_code set_nonblocking(int fd) {
        int opts;
        opts = fcntl(fd, F_GETFL);
        if (opts < 0) {
            return std::error_code(errno, std::system_category());
        }
        opts = opts|O_NONBLOCK;
        if (fcntl(fd, F_SETFL, opts) < 0) {
            return std::error_code(errno, std::system_category());
        }
        return std::error_code();
    }
}