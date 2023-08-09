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

#ifndef SCREEN_HPP_INCLUDED
#define SCREEN_HPP_INCLUDED

#include "config.h"

#include <vector>
#include <stdexcept>
#include <memory>
#include "gfx/pixbuffactory.hpp"

class GdColor;
class ParticleSet;
class Pixbuf;
class PixmapStorage;



/// @ingroup Graphics
/// A class which represents a drawable image.
/// It is different from a pixbuf in the sense that it is already converted to the graphics format of the screen.
class Pixmap {
public:
    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
    virtual ~Pixmap() = default;
};


/// @ingroup Graphics
/// A type of exception that a Screen object can throw, if it cannot
/// configure the screen to the given resolution.
class ScreenConfigureException : public std::runtime_error {
public:
    explicit ScreenConfigureException(std::string const & what)
      : std::runtime_error(what) {
    }
};


/// @ingroup Graphics
/// A class which represents a screen (or a drawing area in a window) to draw pixmaps to.
/// A Screen object can know about one or mor PixmapStorage objects, which are notified
/// before a video mode change.
class Screen {
private:
    std::vector<PixmapStorage *> pixmap_storages;
    void register_pixmap_storage(PixmapStorage *ps);
    void unregister_pixmap_storage(PixmapStorage *ps);
    friend class PixmapStorage;

    virtual void configure_size() = 0;
    virtual void flip() {}

protected:
    int w, h;
    bool fullscreen;
    bool did_some_drawing;
    double scaling_factor;
    GdScalingType scaling_type;
    bool pal_emulation;

public:
    PixbufFactory &pixbuf_factory;

    Screen(PixbufFactory &pixbuf_factory)
        : w(0), h(0),
          fullscreen(false), did_some_drawing(false),
          scaling_factor(1.0), scaling_type(GD_SCALING_NEAREST), pal_emulation(false),
          pixbuf_factory(pixbuf_factory) {}
    virtual ~Screen() = default;

    virtual void set_properties(double scaling_factor_, GdScalingType scaling_type_, bool pal_emulation_);

    /// @brief Return the scale factor of the pixbuf->pixmap.
    /// @return The scale factor, 1 is unscaled (original size)
    double get_pixmap_scale() const {
        return scaling_factor;
    }

    /// @brief Returns true, if the screen uses software pal emulation.
    bool get_pal_emulation() const {
        return pal_emulation;
    }

    int get_width() const {
        return w;
    }
    int get_height() const {
        return h;
    }
    void set_size(int w, int h, bool fullscreen);
    virtual void set_title(char const *title) = 0;


    /// @brief Create a newly allocated, scaled pixmap.
    /// @param pb The pixbuf to set the contents from.
    /// @return A newly allocated pixmap object. Free with delete.
    virtual std::unique_ptr<Pixmap> create_pixmap_from_pixbuf(Pixbuf const &pb, bool keep_alpha) const = 0;
    std::unique_ptr<Pixmap> create_scaled_pixmap_from_pixbuf(Pixbuf const &pb, bool keep_alpha) const;

    virtual void fill_rect(int x, int y, int w, int h, const GdColor &c) = 0;
    void fill(const GdColor &c) {
        fill_rect(0, 0, get_width(), get_height(), c);
    }
    virtual void blit(Pixmap const &src, int dx, int dy) const = 0;
    void blit_pixbuf(Pixbuf const &src, int dx, int dy, bool keep_alpha);

    virtual void set_clip_rect(int x1, int y1, int w, int h) = 0;
    virtual void remove_clip_rect() = 0;

    virtual void draw_particle_set(int dx, int dy, ParticleSet const &ps) {}

    /** 
     * Tell the graphics system to accept text input;
     * mainly used for SDL. See https://wiki.libsdl.org/SDL_StartTextInput
     */
    virtual void start_text_input() {}
    
    /**
     * See start_text_input.
     */
    virtual void stop_text_input() {}

    /**
     * Returns if the screen is double buffered, which means that everything must be redrawn
     * before flipping. */
    virtual bool must_redraw_all_before_flip() const {
        return false;
    }

    /**
     * Returns if the do_the_flip()-s wait for vertical retrace,
     * and therefore can be used for timing. */
    virtual bool has_timed_flips() const {
        return false;
    }

    /**
     * When the application has finished its drawing, it must call this function.
     */
    void drawing_finished() {
        did_some_drawing = true;
    }

    /**
     * Check if there was some drawing.
     */
    bool is_drawn() const {
        return did_some_drawing;
    }

    /**
     *
     */
    void do_the_flip() {
        did_some_drawing = false;
        flip();
    }
    
    static unsigned char const *gdash_icon_32_png;
    static unsigned const gdash_icon_32_size;
    static unsigned char const *gdash_icon_48_png;
    static unsigned const gdash_icon_48_size;
};

#endif
