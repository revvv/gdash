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

#include "config.h"

#include <gtk/gtk.h>
#include <stdexcept>

#include "cave/colors.hpp"
#include "gtk/gtkpixbuf.hpp"


GTKPixbuf::GTKPixbuf(int w, int h) {
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, w, h);
}


GTKPixbuf::GTKPixbuf(GdkPixbuf *pb)
    : pixbuf(pb) {
    /* convert to our own format if necessary */
    if (!gdk_pixbuf_get_has_alpha(pixbuf) || gdk_pixbuf_get_bits_per_sample(pixbuf) != 8
            || gdk_pixbuf_get_n_channels(pixbuf) != 4) {
        int w = gdk_pixbuf_get_width(pixbuf);
        int h = gdk_pixbuf_get_height(pixbuf);
        GdkPixbuf *newpixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, w, h);
        gdk_pixbuf_copy_area(pixbuf, 0, 0, w, h, newpixbuf, 0, 0);
        g_object_unref(pixbuf);
        pixbuf = newpixbuf;
    }
}


int GTKPixbuf::get_width() const {
    return gdk_pixbuf_get_width(pixbuf);
}


int GTKPixbuf::get_height() const {
    return gdk_pixbuf_get_height(pixbuf);
}


void GTKPixbuf::blit_full(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const {
    GdkPixbuf *destpb = static_cast<GTKPixbuf &>(dest).pixbuf;

    gdk_pixbuf_composite(pixbuf, destpb, dx, dy, w, h, dx - x, dy - y, 1, 1, GDK_INTERP_NEAREST, 255);
}


void GTKPixbuf::copy_full(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const {
    // gdk_pixbuf_copy_area does not like clipping
    if (dy + h > dest.get_height()) {
        h = dest.get_height() - dy;
        if (h < 0)
            return;
    }
    if (dx + w > dest.get_width()) {
        w = dest.get_width() - dx;
        if (w < 0)
            return;
    }
    GdkPixbuf *destpb = static_cast<GTKPixbuf &>(dest).pixbuf;
    gdk_pixbuf_copy_area(pixbuf, x, y, w, h, destpb, dx, dy);
}

void GTKPixbuf::scale_full(Pixbuf &dest, double scaling_factor, GdScalingType scaling_type) const {
    GdkPixbuf *destpb = static_cast<GTKPixbuf &>(dest).pixbuf;

    GdkInterpType interp_type;
    switch (scaling_type) {
        case GD_SCALING_NEAREST:
            interp_type = GDK_INTERP_NEAREST;
            break;
        case GD_SCALING_SCALE2X:
            interp_type = GDK_INTERP_TILES;
            break;
        case GD_SCALING_HQX:
            interp_type = GDK_INTERP_BILINEAR;
            break;
        case GD_SCALING_MAX:
            interp_type = GDK_INTERP_HYPER; // unreachable
            g_assert_not_reached();
            break;
    }

    gdk_pixbuf_scale(pixbuf, destpb, 0, 0, dest.get_width(), dest.get_height(), 0, 0, scaling_factor, scaling_factor, interp_type);
}

void GTKPixbuf::fill_rect(int x, int y, int w, int h, const GdColor &c) {
    GdkPixbuf *sub = gdk_pixbuf_new_subpixbuf(pixbuf, x, y, w, h);
    unsigned char r, g, b;
    c.get_rgb(r, g, b);
    gdk_pixbuf_fill(sub, (guint32(r) << 24) | (guint32(g) << 16) | (guint32(b) << 8) | (0xff << 0));
    g_object_unref(sub);
}


unsigned char *GTKPixbuf::get_pixels() const {
    return gdk_pixbuf_get_pixels(pixbuf);
}


int GTKPixbuf::get_pitch() const {
    return gdk_pixbuf_get_rowstride(pixbuf);
}


GTKPixbuf::~GTKPixbuf() {
    g_object_unref(pixbuf);
}
