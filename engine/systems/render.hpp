#pragma once

#include "./window.hpp"
#include "systems.hpp"
#include "components/camera.hpp"
#include "utility/opengl.hpp"

namespace spk {
    struct render_system_ctx_t {

        // quad indexes can be used in many places
        static_index_buffer_t quad_index_buffer;

        void init();
        void free();
    };

    void render_cs_init(system_ctx_allocater_t& ctx_alloc, flecs::world& world); 
}