#include "text.hpp"

namespace spk {
        bool font_tt::init() {
        if(!char_map.init(sfk::xor_int_hash<u_char>))
            return false;

        if(!texture.init()) {
            char_map.free();
            return false;
        }

        return true;
    }

    bool font_tt::load_ascii_font(FT_Library lib, int f_width, int f_height, const char* file_path) {
        /* introducing bullshit by bullshit dev */
        uint32_t x = 0; // cursor within this->texture's data, used for writing
        tallest_glyph = 0;
        widest_glyph = 0;
        width = 0;
        height = 0;

        if(FT_New_Face(lib, file_path, 0, &face)) {
            return false;
        }

        FT_Set_Pixel_Sizes(face, f_width, f_height);

        for(u_char c = 0; c < UCHAR_MAX; c++) {
            character_tt* c_data = nullptr;

            DEBUG_VALUE(bool, ret =) char_map.register_key(c); 
            /* this should never happen */
            assert(ret);
        
            if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                continue;
            }

            width += face->glyph->bitmap.width;
            widest_glyph = std::max(widest_glyph, face->glyph->bitmap.width);
            tallest_glyph = std::max(tallest_glyph, face->glyph->bitmap.rows);

            c_data = &char_map[c];
            c_data->advance = face->glyph->advance.x >> 6;
            c_data->size    = {face->glyph->bitmap.width, face->glyph->bitmap.rows};
            c_data->bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top};
        }

        height = tallest_glyph;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        texture.allocate(GL_UNSIGNED_BYTE, GL_RED, GL_RED, width, height, nullptr);
        texture.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        for(u_char c = 0; c < UCHAR_MAX; c++) {
            character_tt* c_data = nullptr;
            float gx, gwidth, gheight; // g for glyph
            
            if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                continue;
            }

            texture.subdata(GL_UNSIGNED_BYTE, x, 0, GL_RED, face->glyph->bitmap.width, 
                face->glyph->bitmap.rows, face->glyph->bitmap.buffer);

            c_data = &char_map[c];

            gx = (float)x / width;
            gwidth = (float)c_data->size.x / width;
            gheight = (float)c_data->size.y / height;

            // ccw indexing, keep that in mind
            c_data->tex_indices[0] = { gx         , gheight };
            c_data->tex_indices[1] = { gx + gwidth, gheight };
            c_data->tex_indices[2] = { gx + gwidth, 0       };
            c_data->tex_indices[3] = { gx         , 0       };

            x += c_data->size.x;
        }

        // texture still bound
        glGenerateMipmap(GL_TEXTURE_2D);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // restore previous alignment

        one_fourth_tallest_glyph = (float)tallest_glyph / 4;

        return true;
    }

    void font_tt::free() {
        char_map.free();
        texture.free();
    }

    bool font_manager_tt::init() {
        if(!font_pool.init())
            return false;

        if(FT_Init_FreeType(&ft_lib)) {
            return false;
        } else {
            return true;
        }
    }

    font_tt* font_manager_tt::load_ascii_font(int f_width, int f_height, const char* file_path) {
        font_tt* font = font_pool.malloc();
        if(!font)
            return nullptr;

        if(!font->init()) {
            goto failure;
        } else {
            if(!font->load_ascii_font(ft_lib, f_width, f_height, file_path)) {
                goto failure;
            } else {
                goto success;
            }
        }

    failure:
        font_pool.letgo(font);
        return nullptr;
    success:    
        return font;
    }

    void font_manager_tt::free() {
        FT_Done_FreeType(ft_lib);
        font_pool.free();
    }
}