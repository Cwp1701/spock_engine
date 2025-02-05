#include "resource_manager.hpp"

namespace spk {
    bool resource_manager_t::init() {
        if(!fm_init())
            return false;

        if(!am_init())
            return false;

        td_init();

        return true;
    }

    void resource_manager_t::free() {
        am_free();
        fm_free();
        td_free();
    }
}