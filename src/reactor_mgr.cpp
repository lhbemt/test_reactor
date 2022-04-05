#include "reactor_mgr.hpp"

namespace lemt {
    std::unique_ptr<ReactorMgr> ReactorMgr::instance{ nullptr };
    std::once_flag ReactorMgr::init_flag;
}