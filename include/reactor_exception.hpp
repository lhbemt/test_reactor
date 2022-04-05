#ifndef LEMT_REACTOR_EXCEPTION__H
#define LEMT_REACTOR_EXCEPTION__H

#include <exception>
#include <system_error>
#include <string>

namespace lemt {
    class ReactorException : public std::exception {
    public:
        ReactorException(std::error_code ec) : err_code(std::move(ec)) {
        }

        virtual const char* what() const noexcept override {
            std::string err_s = "reactor error: " + err_code.message();
            return err_s.c_str();
        }

    protected:
        std::error_code err_code;
    };

    class SocketException : std::exception {
    public:
        SocketException(std::error_code ec) : err_code(std::move(ec)) {
        }

        virtual const char* what() const noexcept override{
            std::string err_s = "socket error: " + err_code.message();
            return err_s.c_str();
        }
    protected:
        std::error_code err_code;
    };
}

#endif
