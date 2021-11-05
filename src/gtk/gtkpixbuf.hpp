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
#ifndef GTKPIXBUF_HPP_INCLUDED
#define GTKPIXBUF_HPP_INCLUDED

#include "config.h"

#include <gtk/gtk.h>
#include "gfx/pixbuf.hpp"

/**
 * Implementation of the Pixbuf interface, using GDK-pixbuf
 */
class GTKPixbuf: public Pixbuf {
private:
    GdkPixbuf *pixbuf;

    GTKPixbuf(const GTKPixbuf &) = delete;
    GTKPixbuf &operator=(const GTKPixbuf &) = delete;

public:
    /** This constructor adopts the pixbuf. it does not increase ref count! */
    explicit GTKPixbuf(GdkPixbuf *pb);
    GTKPixbuf(int w, int h);
    virtual ~GTKPixbuf();

    virtual int get_width() const;
    virtual int get_height() const;
    virtual void blit_full(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const;
    virtual void copy_full(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const;
    virtual void fill_rect(int x, int y, int w, int h, const GdColor &c);

    virtual unsigned char *get_pixels() const;
    virtual int get_pitch() const;

    /** Return the GdkPixbuf* associated with the object. */
    GdkPixbuf *get_gdk_pixbuf() {
        return pixbuf;
    }
    /** Return the GdkPixbuf* associated with the object. */
    GdkPixbuf const *get_gdk_pixbuf() const {
        return pixbuf;
    }
};
#endif
