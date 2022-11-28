#pragma once

#include "../systems.hpp"
#include "../components/ui.hpp"

#define MAX_LETTERS 0xfff

namespace spk {
    struct font_render_t {
        struct vertex_t {
            float x, y;
            glm::vec2 uv;
            glm::vec3 rgb;
        };
        
        sfk::vertex_layout_t layout;
        sfk::vertex_array_t vao;
        sfk::vertex_buffer_t vbo;
        sfk::program_t program;

        std::vector<vertex_t> buffer;

        /* letters */
        uint32_t indexes;
        uint32_t vertices;
        const uint32_t indexes_per_letter = 4;
        const uint32_t vertices_per_letter = 6;

        void init();
        void render(sfk::static_index_buffer_t& ibo, font_t* font, ui_comp_canvas_t* canvas);
        void free();

        void add_ui_text(font_t* font, ui_text_t* text);
    };

    struct button_render_t {
        struct vertex_t {
            float x, y;
            float r, g, b;
        };

        sfk::vertex_layout_t layout;
        sfk::vertex_array_t vao;
        sfk::vertex_buffer_t vbo;
        sfk::program_t program;

        std::vector<vertex_t> buffer;

        /* letters */
        uint32_t indexes;
        uint32_t vertices;
        const uint32_t indexes_per_button = 4;
        const uint32_t vertices_per_button = 6;

        void init();
        void render(sfk::static_index_buffer_t& ibo, ui_comp_canvas_t* canvas);
        void free();

        void add_ui_button(ui_button_t* button);
    };

    struct ui_render_system_ctx_t {
        ui_render_system_ctx_t() {
            font_render.init();
            button_render.init();

            ibo.init(GL_ELEMENT_ARRAY_BUFFER);
            ibo.generate_quad_indexes(MAX_LETTERS);     
        }

        ~ui_render_system_ctx_t() {
            ibo.free();
            button_render.free();
            font_render.free();
        }

        ui_render_system_ctx_t(ui_render_system_ctx_t&& other) {
            memcpy(this, static_cast<const void*>(&other), sizeof(ui_render_system_ctx_t));
        }

        ui_render_system_ctx_t& operator=(ui_render_system_ctx_t&& other) {
            memcpy(this, static_cast<const void*>(&other), sizeof(ui_render_system_ctx_t));
            
            return *this;    
        }

        sfk::static_index_buffer_t ibo;
        font_render_t font_render;
        button_render_t button_render;
    };

    void ui_cs_init(system_ctx_allocater_t& ctx_alloc, flecs::world& world);
}