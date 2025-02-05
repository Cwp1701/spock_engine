#pragma once

#include "data/tilemap.hpp"
#include "debug.hpp"
#include "box2D.hpp"

namespace spk {
    struct comp_tilebody_t : public comp_b2Body_t {
        tilemap_t tilemap;

        void init();
        void free();

        void add_fixtures();
    };
    
    void tile_comp_init(flecs::world& world);
}