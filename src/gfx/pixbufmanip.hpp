/*
 * Copyright (c) 2007-2013, Czirkos Zoltan http://code.google.com/p/gdash/
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef _GD_pixbufmanip
#define _GD_pixbufmanip

#include "config.h"

#include <vector>
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