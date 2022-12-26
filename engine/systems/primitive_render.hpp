#pragma once

#include "systems.hpp"
#include "components/box2D.hpp"
#include "components/tilebody.hpp"
#include "render.hpp"

/* quick and dirty debug rendering for Box2D shapes or primitive shapes */

namespace spk {
    struct comp_primitive_render_t {
        glm::vec3 color = {1.0f, 1.0f, 1.0f};
    };

    struct comp_box_t {
        glm::vec3 color = {1.0f, 1.0f, 1.0f};
        glm::vec2 position;
        glm::vec2 size;
    };

    struct primitive_render_system_ctx_t {
        struct vertex_t {
            glm::vec2 pos; // position
            glm::vec3 color; // color
        };

        vertex_buffer_t vertex_buffer;
        vertex_layout_t vertex_layout;
        vertex_array_t  vertex_array;
        program_t program;

        std::vector<vertex_t> mesh;
        uint32_t vertices; // of vertex vvv

        void init();
        void free();

        void render_box(glm::mat4& vp, static_index_buffer_t& ind, comp_primitive_render_t* info, const comp_box_t* box);
        void render_polygon(glm::mat4& vp, static_index_buffer_t& ind, comp_primitive_render_t* info, b2Body* body, b2PolygonShape* polygon);
        void render_circle(glm::mat4& vp, comp_primitive_render_t* info, b2Body* body, b2CircleShape* circle);
        void render_edge(glm::mat4& vp, comp_primitive_render_t* info, b2Body* body, b2EdgeShape* circle);

    };

    void primitive_render_cs_init(system_ctx_allocater_t& ctx_alloc, flecs::world& world);
}