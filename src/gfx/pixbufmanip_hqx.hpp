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

#ifndef PIXBUFMANIP_HQX_HPP_INCLUDED
#define PIXBUFMANIP_HQX_HPP_INCLUDED

#include "config.h"

#include <glib.h>
#include <cstdlib>
#include "gfx/pixbuf.hpp"

#define Ymask 0x00FF0000
#define Umask 0x0000FF00
#define Vmask 0x000000FF
#define trY   0x00300000
#define trU   0x00000700
#define trV   0x00000006

// These masks are used to mask out two bytes from an guint32
// out of four.
// When interpolating, all r g b and a values must be interpolated.
// These enable the interpolation functions to do two multiplications
// at a time.
#define MASK_24 0xFF00FF00
#define MASK_13 0x00FF00FF


inline guint32 RGBtoYUV(guint32 c) {
    guint32 r, g, b, y, u, v;
    r = (c & Pixbuf::rmask) >> Pixbuf::rshift;
    g = (c & Pixbuf::gmask) >> Pixbuf::gshift;
    b = (c & Pixbuf::bmask) >> Pixbuf::bshift;
    y = (77 * r + 150 * g + 29 * b) >> 8;           // y = (0.299*r + 0.587*g + 0.114*b);
    u = (32768 + -43 * r - 85 * g + 128 * b) >> 8;  // u = (-0.169*r - 0.331*g + 0.5*b) + 128;
    v = (32768 + 128 * r - 107 * g - 21 * b) >> 8;  // v = (0.5*r - 0.419*g - 0.081*b) + 128;
    return (y << 16) + (u << 8) + v;
}

/* Test if there is difference in color */
inline int Diff(guint32 w1, guint32 w2) {
    guint32 YUV1 = RGBtoYUV(w1);
    guint32 YUV2 = RGBtoYUV(w2);
    return (abs(gint32(YUV1 & Ymask) - gint32(YUV2 & Ymask)) > trY)
           || (abs(gint32(YUV1 & Umask) - gint32(YUV2 & Umask)) > trU)
           || (abs(gint32(YUV1 & Vmask) - gint32(YUV2 & Vmask)) > trV);
}

/* Interpolate functions */
inline void Interp1(guint32 *pc, guint32 c1, guint32 c2) {
    //*pc = (c1*3+c2)/4;
    if (c1 == c2) {
        *pc = c1;
        return;
    }
    *pc = ((((c1 & MASK_24) / 4 * 3 + (c2 & MASK_24) / 4)) & MASK_24)
          + ((((c1 & MASK_13) / 4 * 3 + (c2 & MASK_13) / 4)) & MASK_13);
}

inline void Interp2(guint32 *pc, guint32 c1, guint32 c2, guint32 c3) {
    //*pc = (c1*2+c2+c3)/4;
    *pc = ((((c1 & MASK_24) / 4 * 2 + (c2 & MASK_24) / 4 + (c3 & MASK_24) / 4)) & MASK_24)
          + ((((c1 & MASK_13) / 4 * 2 + (c2 & MASK_13) / 4 + (c3 & MASK_13) / 4)) & MASK_13);
}

inline void Interp3(guint32 *pc, guint32 c1, guint32 c2) {
    //*pc = (c1*7+c2)/8;
    if (c1 == c2) {
        *pc = c1;
        return;
    }
    *pc = ((((c1 & MASK_24) / 8 * 7 + (c2 & MASK_24) / 8)) & MASK_24) +
          ((((c1 & MASK_13) / 8 * 7 + (c2 & MASK_13) / 8)) & MASK_13);
}

inline void Interp4(guint32 *pc, guint32 c1, guint32 c2, guint32 c3) {
    //*pc = (c1*2+(c2+c3)*7)/16;
    *pc = ((((c1 & MASK_24) / 16 * 2 + (c2 & MASK_24) / 16 * 7 + (c3 & MASK_24) / 16 * 7)) & MASK_24) +
          ((((c1 & MASK_13) / 16 * 2 + (c2 & MASK_13) / 16 * 7 + (c3 & MASK_13) / 16 * 7)) & MASK_13);
}

inline void Interp5(guint32 *pc, guint32 c1, guint32 c2) {
    //*pc = (c1+c2)/2;
    if (c1 == c2) {
        *pc = c1;
        return;
    }
    *pc = ((((c1 & MASK_24) / 2 + (c2 & MASK_24) / 2)) & MASK_24) +
          ((((c1 & MASK_13) / 2 + (c2 & MASK_13) / 2)) & MASK_13);
}

inline void Interp6(guint32 *pc, guint32 c1, guint32 c2, guint32 c3) {
    //*pc = (c1*5+c2*2+c3)/8;
    *pc = ((((c1 & MASK_24) / 8 * 5 + (c2 & MASK_24) / 8 * 2 + (c3 & MASK_24) / 8)) & MASK_24) +
          ((((c1 & MASK_13) / 8 * 5 + (c2 & MASK_13) / 8 * 2 + (c3 & MASK_13) / 8)) & MASK_13);
}

inline void Interp7(guint32 *pc, guint32 c1, guint32 c2, guint32 c3) {
    //*pc = (c1*6+c2+c3)/8;
    *pc = ((((c1 & MASK_24) / 8 * 6 + (c2 & MASK_24) / 8 + (c3 & MASK_24) / 8)) & MASK_24) +
          ((((c1 & MASK_13) / 8 * 6 + (c2 & MASK_13) / 8 + (c3 & MASK_13) / 8)) & MASK_13);
}

inline void Interp8(guint32 *pc, guint32 c1, guint32 c2) {
    //*pc = (c1*5+c2*3)/8;
    if (c1 == c2) {
        *pc = c1;
        return;
    }
    *pc = ((((c1 & MASK_24) / 8 * 5 + (c2 & MASK_24) / 8 * 3)) & MASK_24) +
          ((((c1 & MASK_13) / 8 * 5 + (c2 & MASK_13) / 8 * 3)) & MASK_13);
}

inline void Interp9(guint32 *pc, guint32 c1, guint32 c2, guint32 c3) {
    //*pc = (c1*2+(c2+c3)*3)/8;
    *pc = ((((c1 & MASK_24) / 8 * 2 + (c2 & MASK_24) / 8 * 3 + (c3 & MASK_24) / 8 * 3)) & MASK_24) +
          ((((c1 & MASK_13) / 8 * 2 + (c2 & MASK_13) / 8 * 3 + (c3 & MASK_13) / 8 * 3)) & MASK_13);
}

inline void Interp10(guint32 *pc, guint32 c1, guint32 c2, guint32 c3) {
    //*pc = (c1*14+c2+c3)/16;
    *pc = ((((c1 & MASK_24) / 16 * 14 + (c2 & MASK_24) / 16 + (c3 & MASK_24) / 16)) & MASK_24) +
          ((((c1 & MASK_13) / 16 * 14 + (c2 & MASK_13) / 16 + (c3 & MASK_13) / 16)) & MASK_13);
}

#endif
