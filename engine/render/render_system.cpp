#include "render_system.hpp"
#include "state.hpp"

namespace spk {
#ifndef CALLBACK
    #define CALLBACK 
#endif
  
    void CALLBACK opengl_debug_callback(GLenum source,
            GLenum type,
            GLenum severity,
            GLuint id,                  
            GLsizei length,
            const GLchar *message,
            const void *userParam) {
        switch(type) {
        case GL_DEBUG_TYPE_ERROR:
            log.log(spk::LOG_TYPE_ERROR, message);
            break;

        default:
            log.flags &= ~spk::LOG_FLAGS_ENABLE_STD_PIPE;
            log.log(spk::LOG_TYPE_INFO, message); 
            log.flags |= spk::LOG_FLAGS_ENABLE_STD_PIPE;
        }
    }

 
    void render_system_t::init() {
#ifndef NDEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(opengl_debug_callback, nullptr);
#endif

        attachments.init();
        framebuffers.init();
        render_passes.init();

        // setup the default framebuffer and render pass
        fb_default_init();

        rp_init();
        rp_set_fb(0, 0);

        // setup utilities
        quad_index_buffer.init(GL_ELEMENT_ARRAY_BUFFER);
        quad_index_buffer.generate_quad_indexes(100000);

        copy_buffer.init(GL_ARRAY_BUFFER);
    }

    void render_system_t::free() {
        copy_buffer.free();
        quad_index_buffer.free();
        render_passes.free();
        framebuffers.free();
        attachments.free();
    }

    key_t render_system_t::atch_init() {
        attachment_t atch;
        atch.attachment      = UINT32_MAX;
        atch.data_format     = UINT32_MAX;
        atch.internal_format = UINT32_MAX;

        glGenTextures(1, &atch.attachment);        

        attachments.push_back(atch);

        return attachments.size() - 1;
    }

    void render_system_t::resize(uint32_t width, uint32_t height) {
        for(attachment_t& atch : attachments) {
            glBindTexture(GL_TEXTURE_2D, atch.texture);
            glTexImage2D(GL_TEXTURE_2D, 0, atch.internal_format, width, height, 0, atch.data_format, GL_UNSIGNED_BYTE, nullptr);
        }

        glViewport(0, 0, width, height);
    }

    void render_system_t::atch_set(key_t atch_id, uint32_t type, uint32_t internal_format, uint32_t data_format) {
        attachments[atch_id] = {type, internal_format, data_format};
    }
    
    attachment_t& render_system_t::atch_get(key_t atch_id) {
        return attachments[atch_id];
    }

    void render_system_t::_atch_free(key_t atch_id) {
        framebuffers[atch_id].attachments.free();
        glDeleteTextures(1, &attachments[atch_id].texture);
    }
    
    void render_system_t::atch_remove(key_t atch_id) {
        attachments.erase(atch_id);
    }

    void render_system_t::fb_default_init() {
        framebuffer_t fb;
        fb.attachments.init();
        fb.clear_bits  = GL_COLOR_BUFFER_BIT;
        fb.clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
        fb.framebuffer = 0;        

        framebuffers.push_back(fb);
    }

    key_t render_system_t::fb_init() {
        framebuffer_t fb;
        fb.attachments.init();
        fb.clear_bits  = GL_COLOR_BUFFER_BIT;
        fb.clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
        
        glGenFramebuffers(1, &fb.framebuffer);

        framebuffers.push_back(fb);
    
        return framebuffers.size() - 1;
    }

    framebuffer_t& render_system_t::fb_get(key_t key) {
        return framebuffers[key];
    }

    void render_system_t::fb_set_clear_bits(key_t fb_id, uint32_t bits) {
        framebuffers[fb_id].clear_bits = bits;
    }

    void render_system_t::fb_set_clear_color(key_t fb_id, float r, float g, float b, float a) {
        framebuffers[fb_id].clear_color = {r, g, b, a};
    }

    void render_system_t::fb_attach(key_t fb_id, key_t atch_id) {
        framebuffers[fb_id].attachments.push_back(atch_id);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[fb_id].framebuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, attachments[atch_id].attachment, attachments[atch_id].texture, 0);
    }

    void render_system_t::_fb_free(key_t fb_id) {
        framebuffers[fb_id].attachments.free();
        glDeleteFramebuffers(1, &framebuffers[fb_id].framebuffer);
    }

    void render_system_t::fb_remove(key_t fb_id) {
        framebuffers.erase(fb_id);
    }

    key_t render_system_t::rp_init() {
        render_pass_t rp;
        rp.framebuffer = 0;
        rp.renderer.init();
        rp.fb_renderer = nullptr;

        render_passes.push_back(rp);
    
        return render_passes.size() - 1;
    }

    render_pass_t& render_system_t::rp_get(key_t rp_id) {
        return render_passes[rp_id];
    }

    void  render_system_t::rp_set_fb(key_t rp_id, key_t fb_id) {
        render_passes[rp_id].framebuffer = fb_id;
    }

    void  render_system_t::rp_add_renderer(key_t rp_id, base_renderer_t* renderer) {
        render_passes[rp_id].renderer.push_back(renderer);
    }

    void  render_system_t::_rp_free(key_t rp_id) {
        render_passes[rp_id].renderer.free();
    }

    void render_system_t::rp_remove(key_t rp_id) {
        render_passes.erase(rp_id);
    }

    void render_system_t::begin_frame() {

    }

    void render_system_t::draw_frame() {
        for(auto& rp : render_passes) {
            spk_assert(rp.framebuffer < framebuffers.size());
            framebuffer_t* fb = &framebuffers[rp.framebuffer];

            glBindFramebuffer(GL_FRAMEBUFFER, fb->framebuffer);
            glClearColor(fb->clear_color.r, fb->clear_color.g, fb->clear_color.b, fb->clear_color.a);
            glClear(fb->clear_bits);

            for(auto renderer : rp.renderer) {
                for(auto& system : renderer->systems) {
                    flecs::system(system.world().m_world, system.id()).run(stats.delta_time);
                }

                renderer->draw();
            }

            if(rp.fb_renderer)
                rp.fb_renderer->draw(this, rp.framebuffer);
        }
    }

    void render_system_t::end_frame(SDL_Window* window) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0].framebuffer);

        SDL_GL_SwapWindow(window);
    }
}