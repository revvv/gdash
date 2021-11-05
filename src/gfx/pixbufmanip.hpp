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
#ifndef PIXBUFMANIP_HPP_INCLUDED
#define PIXBUFMANIP_HPP_INCLUDED

#include "config.h"

#include <glib.h>

class Pixbuf;
class GdColor;

void scale2x(const Pixbuf &src, Pixbuf &dest);
void scale3x(const Pixbuf &src, Pixbuf &dest);
void scale2xnearest(const Pixbuf &src, Pixbuf &dest);
void scale3xnearest(const Pixbuf &src, Pixbuf &dest);
void pal_emulate(Pixbuf &pb);
void hq2x(Pixbuf const &src, Pixbuf &dst);
void hq3x(Pixbuf const &src, Pixbuf &dst);
void hq4x(Pixbuf const &src, Pixbuf &dst);
GdColor average_nonblack_colors_in_pixbuf(Pixbuf const &pb);
GdColor lightest_color_in_pixbuf(Pixbuf const &pb);

#endif
