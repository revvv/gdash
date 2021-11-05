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

#include <glib.h>

#include "config.h"
#include "cave/colors.hpp"
#include "gfx/pixbuf.hpp"


/// Creates a guint32 value, which can be raw-written to a pixbuf memory area.
guint32 Pixbuf::rgba_pixel_from_color(const GdColor &col, unsigned a) {
    unsigned char r, g, b;
    col.get_rgb(r, g, b);
    return (guint32(r) << rshift) | (guint32(g) << gshift) | (guint32(b) << bshift) | (a << ashift);
}

















static inline int
c64_png_colors(int r, int g, int b, int a) {
    static const int c64_png_cols[] = {
        /* abgr */
        /* 0000 */ 0,    /* transparent */
        /* 0001 */ 0,
        /* 0010 */ 0,
        /* 0011 */ 0,
        /* 0100 */ 0,
        /* 0101 */ 0,
        /* 0110 */ 0,
        /* 0111 */ 0,
        /* 1000 */ 1, /* black - background */
        /* 1001 */ 2, /* red - foreg1 */
        /* 1010 */ 5, /* green - amoeba */
        /* 1011 */ 4, /* yellow - foreg3 */
        /* 1100 */ 6, /* blue - slime */
        /* 1101 */ 3, /* purple - foreg2 */
        /* 1110 */ 7, /* black around arrows (used in editor) is coded as cyan */
        /* 1111 */ 8, /* white is the arrow */
    };

    /* take most significant bit of each */
    int c = (a >> 7) * 8 + (b >> 7) * 4 + (g >> 7) * 2 + (r >> 7) * 1;

    return c64_png_cols[c];
}


/* takes a c64_gfx.png-coded 32-bit pixbuf, and creates a paletted pixbuf in our internal format. */
std::vector<unsigned char> Pixbuf::c64_gfx_data_from_pixbuf(Pixbuf const &image) {
    std::vector<unsigned char> c64_gfx_data(image.get_width()*image.get_height());

    int out = 0;
    for (int y = 0; y < image.get_height(); y++) {
        const guint32 *p = image.get_row(y);
        for (int x = 0; x < image.get_width(); x++) {
            int r = (p[x] & image.rmask) >> image.rshift;
            int g = (p[x] & image.gmask) >> image.gshift;
            int b = (p[x] & image.bmask) >> image.bshift;
            int a = (p[x] & image.amask) >> image.ashift;
            c64_gfx_data[out++] = c64_png_colors(r, g, b, a);
        }
    }

    return c64_gfx_data;
}
