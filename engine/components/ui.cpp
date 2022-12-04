#include "ui.hpp"
#include "../state.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace spk {
    ui_element_t::ui_element_t() {
        sfk::zero(&pos);
        sfk::zero(&size);
        sfk::zero(&flags);

        in_use.reset();
        elements.fill(nullptr);
        parent = nullptr;   
        abs_pos  = { 0.0f, 0.0f };
    }

    void ui_element_t::init() {
    }

    void ui_element_t::iter_children(children_callback_t callback) {
        for(auto ele : elements) {
            if(ele) {
                if(callback(*ele)) {
                    continue;
                } else {
                   ele->iter_children(callback);
                }
            }
        }
    }

    void ui_element_t::free() {

    }

    void ui_comp_canvas_t::init() {

        DEBUG_VALUE(bool, ret =) texts.init();
        sfk_assert(ret);
        DEBUG_EXPR(ret =) btns.init();
        sfk_assert(ret);

        font = nullptr;

        flags |= spk::UI_ELEMENT_FLAGS_ROOT |
                 spk::UI_ELEMENT_FLAGS_ENABLED;

        size = { std::nanf("nan"), std::nanf("nan") };
        pos = { std::nanf("nan"), std::nanf("nan") };
    }

    void ui_comp_canvas_t::resize(int width, int height) {
        glm::mat4 view, proj;

        view = glm::identity<glm::mat4>();
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -1.0f));
        proj = glm::ortho(0.0f, (float)width, 0.0f, (float)height, 0.01f, 100.0f);

        vp = proj * view;
 
        abs_size = { (float)width, (float)height };
    }

    void ui_comp_canvas_t::free() {
        texts.free();
        btns.free();
    }

    void ui_tag_current_canvas_on_add(flecs::entity e, ui_comp_canvas_t& canvas) {
        if(state._get_current_canvas().id() != UINT64_MAX) {
            state._get_current_canvas().remove<ui_tag_current_canvas_t>();
        }

        state._set_current_canvas(e);
    }

    void ui_canvas_init(flecs::world& world) {
        sfk_register_component(world, ui_comp_canvas_t);

        world.observer<ui_comp_canvas_t>().term<ui_tag_current_canvas_t>()
            .event(flecs::OnAdd).each(ui_tag_current_canvas_on_add);
    }
}