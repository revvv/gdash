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
#ifndef GTKPIXBUFFACTORY_HPP_INCLUDED
#define GTKPIXBUFFACTORY_HPP_INCLUDED

#include "config.h"

#include "gfx/pixbuffactory.hpp"

class GTKPixbufFactory: public PixbufFactory {
public:
    virtual std::unique_ptr<Pixbuf> create(int w, int h) const;
    virtual std::unique_ptr<Pixbuf> create_from_inline(int length, unsigned char const *data) const;
    virtual std::unique_ptr<Pixbuf> create_from_file(const char *filename) const;
    virtual std::unique_ptr<Pixbuf> create_rotated(const Pixbuf &src, Rotation r) const;
    virtual std::unique_ptr<Pixbuf> create_composite_color(const Pixbuf &src, const GdColor &c, unsigned char alpha) const;
    virtual std::unique_ptr<Pixbuf> create_subpixbuf(Pixbuf &src, int x, int y, int w, int h) const;
};

#endif

