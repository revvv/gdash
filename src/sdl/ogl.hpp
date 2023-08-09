/*
 * Copyright (c) 2007-2018, GDash Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:

 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef NEWOGL_HPP_INCLUDED
#define NEWOGL_HPP_INCLUDED

#include <SDL_opengl.h>

#include <glib.h>
#include <string>
#include <vector>
#include <algorithm>
#include "sdl/sdlabstractscreen.hpp"
#include "misc/deleter.hpp"

class SDLOGLScreen: public SDLAbstractScreen {
private:
    template <void (*DELETER)(GLuint)>
    class GlResource {
      private:
        GLuint resource;
        
      public:
        explicit GlResource(GLuint resource = 0) : resource(resource) {}
        GlResource(GlResource const &) = delete;
        GlResource(GlResource && other) {
            resource = other.resource;
            other.resource = 0;
        }
        GlResource& operator=(GlResource other) {
            std::swap(resource, other.resource);
            return *this;
        }
        ~GlResource() {
            if (resource)
                DELETER(resource);
        }
        
        GLuint get() const {
            return resource;
        }
        explicit operator bool() const {
            return resource != 0;
        }
        void reset(GLuint resource = 0) {
            *this = GlResource(resource);
        }
    };

    bool shader_support;
    bool timed_flips;
    double oglscaling;
    
    static void glDeleteProgram_wrapper(GLuint texture);
    GlResource<glDeleteProgram_wrapper> glprogram;
    
    static void glDeleteShader_wrapper(GLuint texture);
    std::vector<GlResource<glDeleteShader_wrapper>> shaders;
    
    static void glDeleteTexture_wrapper(GLuint texture) {
        if (texture != 0)
            glDeleteTextures(1, &texture);
    }
    GlResource<glDeleteTexture_wrapper> texture;
    
    std::unique_ptr<SDL_Window, Deleter<SDL_Window, SDL_DestroyWindow>> window;
    std::unique_ptr<void, Deleter<void, SDL_GL_DeleteContext>> context;

    /// used when loading the xml
    std::string shadertext;
    static void start_element(GMarkupParseContext *context, const gchar *element_name, const gchar **attribute_names, const gchar **attribute_values, gpointer user_data, GError **error);
    static void end_element(GMarkupParseContext *context, const gchar *element_name, gpointer user_data, GError **error);
    static void text(GMarkupParseContext *context, const gchar *text, gsize text_len, gpointer user_data, GError **error);
    
    void set_uniform_float(char const *name, GLfloat value);
    void set_uniform_2float(char const *name, GLfloat value1, GLfloat value2);
    void set_texture_bilinear(bool bilinear);

public:
    SDLOGLScreen(PixbufFactory &pixbuf_factory);
    virtual void set_properties(double scaling_factor_, GdScalingType scaling_type_, bool pal_emulation_) override;
    virtual void set_title(char const *) override;
    virtual void configure_size() override;
    virtual void flip() override;
    virtual bool has_timed_flips() const override;
    virtual std::unique_ptr<Pixmap> create_pixmap_from_pixbuf(Pixbuf const &pb, bool keep_alpha) const override;
};

#endif
