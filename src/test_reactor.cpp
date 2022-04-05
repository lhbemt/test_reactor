#include "reactor_mgr.hpp"
#include <signal.h>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "reactor_exception.hpp"

std::mutex m;
std::condition_variable cv;
bool stop = false;

void sig_handle(int sig) {
    stop = true;
    cv.notify_one();
}

int main(int argc, char* argv[]) {
    try {
        signal(SIGPIPE, SIG_IGN);
        signal(SIGINT, sig_handle);
        signal(SIGTERM, sig_handle);
        lemt::ReactorMgr::get_instance()->start();
        while (true) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [](){ return stop; });
            std::cout << "notify stop" << std::endl;
            lemt::ReactorMgr::get_instance()->stop();
            std::cout << "reactor stop end" << std::endl;
            return 0;
        }
    }
    catch(lemt::ReactorException& e) {
        std::cout << e.what() << std::endl;
    }
    catch(lemt::SocketException& e) {
        std::cout << e.what() << std::endl;
    }
    catch(...) {
        std::cout << errno << std::endl; 
    }
    
    return 0;
}
