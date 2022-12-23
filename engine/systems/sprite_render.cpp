#include "sprite_render.hpp"
#include "../state.hpp"
#include "../spock.hpp"
#include <glm/gtx/rotate_vector.hpp> 

const char* vs_sprite = R"###(
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex_coords;

out vec2 v_tex_coords;

uniform mat4 u_vp;

void main() {
    gl_Position = u_vp * vec4(a_pos, 0.0, 1.0);
    v_tex_coords = a_tex_coords;
})###";

const char* fs_sprite = R"###(
#version 330 core
in vec2 v_tex_coords;

uniform sampler2D atlas;

out vec4 fragment_color;

void main() {
    fragment_color = texture(atlas, v_tex_coords);
})###";

namespace spk {

    void sprite_render_system_ctx_t::init() {
        vertex_array.init();
        vertex_array.bind();
        
        vertex_buffer.init(GL_ARRAY_BUFFER);
        vertex_buffer.buffer_data(sizeof(vertex_t) * 4 * 1000, nullptr, GL_DYNAMIC_DRAW);
        vertex_layout.add(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0, vertex_buffer);
        vertex_layout.add(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, sizeof(float) * 2, vertex_buffer);
        vertex_array.bind_layout(vertex_layout);

        program.init();
        bool ret = program.load_shader_str(vs_sprite, fs_sprite);
        sfk_assert(ret && "sprite render shaders invalid");

        for(uint32_t i = 0; i < SPK_MAX_ATLAS; i++) {
            meshes[i].mesh.resize(1000 * 4); // 4 vertex per sprite
            meshes[i].sprites = 0;
        }
    }

    void sprite_render_system_ctx_t::free() {
        vertex_array.free();
        vertex_buffer.free();
        program.free();
    }

    void sprite_render_system_ctx_t::add_sprite_mesh(b2Body* body, comp_sprite_t& sprite, glm::vec2 offset) {
        resource_manager_t* rsrc_mng = &state.engine->rsrc_mng;

        if(sprite.atlas_id == UINT32_MAX)
            return;

        const uint32_t atlas_id = sprite.atlas_id;
        sprite_atlas_t* atlas = rsrc_mng->get_atlas(atlas_id);
        uint32_t index = meshes[atlas_id].sprites * indexes_per_sprite;
        std::array<glm::vec2, 4> tex_coords = atlas->gen_tex_coords(sprite.tax, sprite.tay);

        meshes[atlas_id].mesh[index + 0] = { {sfk::to_glm_vec2(body->GetWorldPoint((b2Vec2){-sprite.size.x, -sprite.size.y} + sfk::to_box_vec2(offset)))},  tex_coords[0]};
        meshes[atlas_id].mesh[index + 1] = { {sfk::to_glm_vec2(body->GetWorldPoint((b2Vec2){ sprite.size.x, -sprite.size.y} + sfk::to_box_vec2(offset)))},  tex_coords[1]};
        meshes[atlas_id].mesh[index + 2] = { {sfk::to_glm_vec2(body->GetWorldPoint((b2Vec2){ sprite.size.x,  sprite.size.y} + sfk::to_box_vec2(offset)))},  tex_coords[2]};
        meshes[atlas_id].mesh[index + 3] = { {sfk::to_glm_vec2(body->GetWorldPoint((b2Vec2){-sprite.size.x,  sprite.size.y} + sfk::to_box_vec2(offset)))},  tex_coords[3]};
        meshes[atlas_id].sprites += 1;
    }

    void sprite_render_system_ctx_t::draw_atlas_meshes() {
        auto render_ctx = state.get_current_renderer().get_ref<render_system_ctx_t>(); // this is a safe op as render system only has pre and post update
        resource_manager_t* rsrc_mng = &state.engine->rsrc_mng;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for(uint32_t i = 0; i < SPK_MAX_ATLAS; i++) {
            if(!rsrc_mng->atlas_is_in_use(i))
                continue;

            sprite_render_system_ctx_t::atlas_mesh_t& atlas_mesh = meshes[i];
            sprite_atlas_t* atlas = rsrc_mng->get_atlas(i);

            if(0 < atlas_mesh.sprites) {
                vertex_buffer.buffer_sub_data(0, 
                    atlas_mesh.sprites * indexes_per_sprite * sizeof(sprite_render_system_ctx_t::vertex_t), atlas_mesh.mesh.data());
                vertex_array.bind();

                render_ctx->quad_index_buffer.bind();
                program.use();
                program.set_mat4("u_vp", render_ctx->vp);
                program.set_int("atlas", 0);
                atlas->texture.active_texture(GL_TEXTURE0);
                glDrawElements(GL_TRIANGLES, atlas_mesh.sprites * vertexes_per_sprite, GL_UNSIGNED_INT, nullptr);   

                atlas_mesh.sprites = 0;
            }
        }
        glDisable(GL_BLEND);
    }

    void sprite_render_system_tilebody_update(flecs::iter& iter, comp_tilebody_t* tilebodies) {
        auto ctx = SPK_GET_CTX_REF(iter, sprite_render_system_ctx_t);
        resource_manager_t* rsrc_mng = &state.engine->rsrc_mng;

        for(auto i : iter) {
            comp_tilebody_t& tilebody = tilebodies[i];
            glm::vec2 half_size = (glm::vec2)tilebody.tilemap.size / 2.0f;

            for(uint32_t x = 0; x < tilebody.tilemap.size.x; x++) {
                for(uint32_t y = 0; y < tilebody.tilemap.size.y; y++) {
                    tile_t& tile = tilebody.tilemap.tiles[x][y];

                    if(!(tile.flags & TILE_FLAGS_EMPTY)) {
                        ctx->add_sprite_mesh(tilebody.body, tilebody.dictionary[tile.id].sprite, 
                            {x, y});
                    }
                }
            }
        }

        ctx->draw_atlas_meshes();
    }

    void sprite_render_system_update(flecs::iter& iter, comp_b2Body_t* bodies, comp_sprite_t* sprites) {
        auto ctx = SPK_GET_CTX_REF(iter, sprite_render_system_ctx_t);
        resource_manager_t* rsrc_mng = &state.engine->rsrc_mng;

        for(auto i : iter) {
            comp_b2Body_t&  body = bodies[i];
            comp_sprite_t&  sprite = sprites[i];

            ctx->add_sprite_mesh(body.body, sprite);
        }

        ctx->draw_atlas_meshes();
    }

    void sprite_render_cs_init(system_ctx_allocater_t& allocater, flecs::world& world) {
        sprite_comp_init(world);
        tile_comp_init(world);

        sfk_register_component(world, sprite_render_system_ctx_t);
        
        auto ctx = allocater.allocate_ctx<sprite_render_system_ctx_t>();

        world.system<comp_b2Body_t, comp_sprite_t>().ctx(ctx).kind(flecs::OnUpdate)
            .iter(sprite_render_system_update);
    
        world.system<comp_tilebody_t>().ctx(ctx).kind(flecs::OnUpdate)
            .iter(sprite_render_system_tilebody_update);
    }
}