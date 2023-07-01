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
#ifndef PIXBUF_HPP_INCLUDED
#define PIXBUF_HPP_INCLUDED

#include <glib.h>
#include <vector>

#include "gfx/pixbuffactory.hpp"
#include "config.h"

class GdColor;

/// @defgroup Graphics

/// @ingroup Graphics
/// A class which represents a 32-bit RGBA image in memory.
class Pixbuf {
public:

    /* these masks are always for RGBA ordering in memory.
       that is what gdk-pixbuf uses, and also what sdl uses.
       no BGR, no ABGR.
     */
#if G_BYTE_ORDER == G_BIG_ENDIAN
    static const guint32 rmask = 0xff000000;
    static const guint32 gmask = 0x00ff0000;
    static const guint32 bmask = 0x0000ff00;
    static const guint32 amask = 0x000000ff;

    static const guint32 rshift = 24;
    static const guint32 gshift = 16;
    static const guint32 bshift = 8;
    static const guint32 ashift = 0;
#else
    static const guint32 rmask = 0x000000ff;
    static const guint32 gmask = 0x0000ff00;
    static const guint32 bmask = 0x00ff0000;
    static const guint32 amask = 0xff000000;

    static const guint32 rshift = 0;
    static const guint32 gshift = 8;
    static const guint32 bshift = 16;
    static const guint32 ashift = 24;
#endif

    /// Creates a guint32 value, which can be raw-written to a pixbuf memory area.
    static guint32 rgba_pixel_from_color(const GdColor &col, unsigned a);

    /// Get the width of the pixbuf in pixels.
    virtual int get_width() const = 0;

    /// Get the height of the pixbuf in pixels.
    virtual int get_height() const = 0;

    /// @brief Blit pixbuf to another pixbuf (for an area).
    /// @param dest The destination pixbuf.
    /// @param x Upper left coordinate of source area to copy.
    /// @param y Upper left coordinate of source area to copy.
    /// @param w Width of area to copy.
    /// @param h Height of area to copy.
    /// @param dx Destination coordinate, upper left corner.
    /// @param dy Destination coordinate, upper left corner.
    virtual void blit_full(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const = 0;

    /// @brief Blit pixbuf to another pixbuf (for an area).
    /// @param dest The destination pixbuf.
    /// @param x Upper left coordinate of source area to copy.
    /// @param y Upper left coordinate of source area to copy.
    /// @param w Width of area to copy.
    /// @param h Height of area to copy.
    /// @param dx Destination coordinate, upper left corner.
    /// @param dy Destination coordinate, upper left corner.
    /// The purpose of this function is to give an overloaded name for blit_full,
    /// without requiring derived classes to say "using Pixbuf::blit".
    void blit(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const {
        blit_full(x, y, w, h, dest, dx, dy);
    }

    /// @brief Blit the whole pixbuf to another pixbuf.
    /// @param dest The destination pixbuf.
    /// @param dx Destination coordinate, upper left corner.
    /// @param dy Destination coordinate, upper left corner.
    void blit(Pixbuf &dest, int dx, int dy) const {
        blit_full(0, 0, get_width(), get_height(), dest, dx, dy);
    }

    /// @brief Copy pixbuf to another pixbuf (copy area). No alpha-blending, no nothing, just copies data.
    /// @param dest The destination pixbuf.
    /// @param x Upper left coordinate of source area to copy.
    /// @param y Upper left coordinate of source area to copy.
    /// @param w Width of area to copy.
    /// @param h Height of area to copy.
    /// @param dx Destination coordinate, upper left corner.
    /// @param dy Destination coordinate, upper left corner.
    virtual void copy_full(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const = 0;

    /// @brief Copy pixbuf to another pixbuf (copy area). No alpha-blending, no nothing, just copies data.
    /// @param dest The destination pixbuf.
    /// @param x Upper left coordinate of source area to copy.
    /// @param y Upper left coordinate of source area to copy.
    /// @param w Width of area to copy.
    /// @param h Height of area to copy.
    /// @param dx Destination coordinate, upper left corner.
    /// @param dy Destination coordinate, upper left corner.
    /// The purpose of this function is to give an overloaded name for blit_full,
    /// without requiring derived classes to say "using Pixbuf::blit".
    void copy(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const {
        copy_full(x, y, w, h, dest, dx, dy);
    }

    /// @brief Copy pixbuf to another pixbuf (copy area). No alpha-blending, no nothing, just copies data.
    /// @param dest The destination pixbuf.
    /// @param dx Destination coordinate, upper left corner.
    /// @param dy Destination coordinate, upper left corner.
    void copy(Pixbuf &dest, int dx, int dy) const {
        copy_full(0, 0, get_width(), get_height(), dest, dx, dy);
    }

    /// @brief Scale the whole pixbuf to another pixbuf.
    /// @param dest The destination pixbuf.
    /// @param scaling_factor The scaling factor.
    virtual void scale_full(Pixbuf &dest, double scaling_factor, GdScalingType scaling_type) const = 0;

    /// @brief Scale the whole pixbuf to another pixbuf.
    /// @param dest The destination pixbuf.
    /// @param scaling_factor The scaling factor.
    void scale(Pixbuf &dest, double scaling_factor, GdScalingType scaling_type) const {
        scale_full(dest, scaling_factor, scaling_type);
    }

    /// @brief Fill the given area of the pixbuf with the specified color.
    /// No blending takes place!
    virtual void fill_rect(int x, int y, int w, int h, const GdColor &c) = 0;
    void fill(const GdColor &c) {
        fill_rect(0, 0, get_width(), get_height(), c);
    }

    virtual unsigned char *get_pixels() const = 0;
    virtual int get_pitch() const = 0;

    /// Get pointer to row number n, cast to a 32-bit unsigned integer.
    /// Using this, one can: pixbuf.get_row(y)[x]=0xRRGGBBAA or 0xAABBGGRR;
    guint32 *get_row(int y) {
        return reinterpret_cast<guint32 *>(get_pixels() + y * get_pitch());
    }
    const guint32 *get_row(int y) const {
        return reinterpret_cast<guint32 *>(get_pixels() + y * get_pitch());
    }
    /// Generic get/putpixel.
    guint32 &operator()(int x, int y) {
        return get_row(y)[x];
    }
    /// Generic getpixel.
    guint32 const &operator()(int x, int y) const {
        return get_row(y)[x];
    }

    virtual ~Pixbuf() = default;

    static std::vector<unsigned char> c64_gfx_data_from_pixbuf(Pixbuf const &image);
};

#endif

