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

#include <cstring>
#include <cmath>

#include "settings.hpp"
#include "gfx/pixbuf.hpp"
#include "cave/colors.hpp"

/* somewhat optimized implementation of the Scale2x algorithm. */
/* http://scale2x.sourceforge.net */
void scale2x(const Pixbuf &src, Pixbuf &dest) {
    int sh = src.get_height(), sw = src.get_width();

    for (int y = 0; y < sh; ++y) {
        // wraparound
        int ym = (y + sh - 1) % sh;
        int yp = (y + sh + 1) % sh;

        for (int x = 0; x < sw; ++x) {
            int xm = (x + sw - 1) % sw;
            int xp = (x + sw + 1) % sw;

            guint32 B = src(x, ym);
            guint32 D = src(xm, y);
            guint32 E = src(x, y);
            guint32 F = src(xp, y);
            guint32 H = src(x, yp);

            guint32 E0, E1, E2, E3;
            if (B != H && D != F) {
                E0 = D == B ? D : E;
                E1 = B == F ? F : E;
                E2 = D == H ? D : E;
                E3 = H == F ? F : E;
            } else {
                E0 = E;
                E1 = E;
                E2 = E;
                E3 = E;
            }

            dest(x * 2, y * 2) = E0;
            dest(x * 2 + 1, y * 2) = E1;
            dest(x * 2, y * 2 + 1) = E2;
            dest(x * 2 + 1, y * 2 + 1) = E3;
        }
    }
}


void scale2xnearest(const Pixbuf &src, Pixbuf &dest) {
    int const width = src.get_width(), height = src.get_height();
    for (int y = 0; y < height; ++y) {
        // first double a line horizontally
        guint32 const *srcpix = src.get_row(y);
        guint32 *dstpix = dest.get_row(2 * y);
        for (int x = 0; x < width; ++x) {
            *dstpix++ = *srcpix;
            *dstpix++ = *srcpix;
            srcpix++;
        }
        // then make a fast copy of the doubled line
        memcpy(dest.get_row(2 * y + 1), dest.get_row(2 * y), 4 * dest.get_width());
    }
}


void scale3xnearest(const Pixbuf &src, Pixbuf &dest) {
    int const width = src.get_width(), height = src.get_height();
    for (int y = 0; y < height; ++y) {
        // first double a line horizontally
        guint32 const *srcpix = src.get_row(y);
        guint32 *dstpix = dest.get_row(3 * y);
        for (int x = 0; x < width; ++x) {
            *dstpix++ = *srcpix;
            *dstpix++ = *srcpix;
            *dstpix++ = *srcpix;
            srcpix++;
        }
        // then make a fast copy of the tripled line
        // 4* is because of 32-bit pixbufs (4 bytes/pixel)
        memcpy(dest.get_row(3 * y + 1), dest.get_row(3 * y), 4 * dest.get_width());
        memcpy(dest.get_row(3 * y + 2), dest.get_row(3 * y), 4 * dest.get_width());
    }
}


void scale3x(const Pixbuf &src, Pixbuf &dest) {
    int sh = src.get_height(), sw = src.get_width();

    for (int y = 0; y < sh; ++y) {
        int ny = y * 3; /* new coordinate */
        // wraparound
        int ym = (y + sh - 1) % sh;
        int yp = (y + sh + 1) % sh;

        for (int x = 0; x < sw; ++x) {
            int nx = x * 3; /* new coordinate */
            int xm = (x + sw - 1) % sw;
            int xp = (x + sw + 1) % sw;

            guint32 A = src(xm, ym);
            guint32 B = src(x, ym);
            guint32 C = src(xp, ym);
            guint32 D = src(xm, y);
            guint32 E = src(x, y);
            guint32 F = src(xp, y);
            guint32 G = src(xm, yp);
            guint32 H = src(x, yp);
            guint32 I = src(xp, yp);

            guint32 E0, E1, E2, E3, E4, E5, E6, E7, E8;
            if (B != H && D != F) {
                E0 = D == B ? D : E;
                E1 = (D == B && E != C) || (B == F && E != A) ? B : E;
                E2 = B == F ? F : E;
                E3 = (D == B && E != G) || (D == H && E != A) ? D : E;
                E4 = E;
                E5 = (B == F && E != I) || (H == F && E != C) ? F : E;
                E6 = D == H ? D : E;
                E7 = (D == H && E != I) || (H == F && E != G) ? H : E;
                E8 = H == F ? F : E;
            } else {
                E0 = E;
                E1 = E;
                E2 = E;
                E3 = E;
                E4 = E;
                E5 = E;
                E6 = E;
                E7 = E;
                E8 = E;
            }

            dest(nx, ny) = E0;
            dest(nx + 1, ny) = E1;
            dest(nx + 2, ny) = E2;
            dest(nx, ny + 1) = E3;
            dest(nx + 1, ny + 1) = E4;
            dest(nx + 2, ny + 1) = E5;
            dest(nx, ny + 2) = E6;
            dest(nx + 1, ny + 2) = E7;
            dest(nx + 2, ny + 2) = E8;
        }
    }
}


/* pal emulation for 32-bit rgba images. */

/* used:
    y=0.299r+0.587g+0.114b
    u=b-y=-0.299r-0.587g+0.886b
    v=r-y=0.701r-0.587g-0.114b

    r=(r-y)+y=v+y
    b=(b-y)+y=u+y
    g=(y-0.299r-0.114b)/0.587=...=y-0.509v-0.194u

    we multiply every floating point value with 256:

    y=77r+150g+29b
    u=g-y=-77r-150g+227b
    v=r-y=179r-150g-29b

    256*r=v+y
    65536*g=256y-130v-50u
    256*b=u+y
*/

struct YUV {
    /* we use values *256 here for fixed point math, so 8bits is not enough */
    gint32 y, u, v;
};

static void luma_blur(YUV **yuv, YUV **work, int width, int height) {
    /* convolution "matrices" could be 5 numbers, ie. x-2, x-1, x, x+1, x+2... */
    /* but the output already has problems for x-1 and x+1. as the game only
       pal_emus cells, not complete screens - so they are only 3 pixels wide */
    /* convolution "matrix" for luminance */
    static const int lconv[] = { 1, 3, 1, }, ldiv = lconv[0] + lconv[1] + lconv[2];
    /* for left edge of image. */
    static const int lconv_left[] = { 1, 10, 6, }, ldiv_left = lconv_left[0] + lconv_left[1] + lconv_left[2];
    /* for right edge of image. */
    static const int lconv_right[] = { 6, 10, 1, }, ldiv_right = lconv_right[0] + lconv_right[1] + lconv_right[2];

    /* apply convolution matrix */
    /* luma blur */
    for (int y = 0; y < height; y++) {
        YUV n;    /* new value of this pixel in yuv */
        int x, xm, xp;

        /* for x = 0 (left edge) */
        xm = width - 1;
        x = 0;
        xp = 1;
        n.y = (yuv[y][xm].y * lconv_left[0] + yuv[y][x].y * lconv_left[1] + yuv[y][xp].y * lconv_left[2]) / ldiv_left;
        work[y][x].y = n.y;

        /* for x = 1..width-2 */
        for (int x = 1; x < width - 1; x++) {
            int xm = x - 1;
            int xp = x + 1;
            n.y = (yuv[y][xm].y * lconv[0] + yuv[y][x].y * lconv[1] + yuv[y][xp].y * lconv[2]) / ldiv;
            work[y][x].y = n.y;
        }

        /* for x = width-1 (right edge) */
        xm = width - 2;
        x = width - 1;
        xp = 0;
        n.y = (yuv[y][xm].y * lconv_right[0] + yuv[y][x].y * lconv_right[1] + yuv[y][xp].y * lconv_right[2]) / ldiv_right;
        work[y][x].y = n.y;
    }

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            yuv[y][x].y = work[y][x].y;
}


static void chroma_blur(YUV **yuv, YUV **work, int width, int height) {
    /* convolution "matrix" for chrominance */
    /* x-2, x-1, x, x+1, x+2 */
    static const int cconv[] = { 1, 1, 1, 1, 1, }, cdiv = cconv[0] + cconv[1] + cconv[2] + cconv[3] + cconv[4];

    /* apply convolution matrix */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int xm, xm2, xp, xp2;

            /* turnaround coordinates */
            xm2 = (x - 2 + width) % width;
            xm = (x - 1 + width) % width;
            xp = (x + 1) % width;
            xp2 = (x + 2) % width;

            /* chroma blur */
            work[y][x].u = (yuv[y][xm2].u * cconv[0] + yuv[y][xm].u * cconv[1] + yuv[y][x].u * cconv[2] + yuv[y][xp].u * cconv[3] + yuv[y][xp2].u * cconv[4]) / cdiv;
            work[y][x].v = (yuv[y][xm2].v * cconv[0] + yuv[y][xm].v * cconv[1] + yuv[y][x].v * cconv[2] + yuv[y][xp].v * cconv[3] + yuv[y][xp2].v * cconv[4]) / cdiv;
        }
    }

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            yuv[y][x].u = work[y][x].u;
            yuv[y][x].v = work[y][x].v;
        }
}

#define CROSSTALK_SIZE 16

static void chroma_crosstalk_to_luma(YUV **yuv, YUV **work, int width, int height) {
    /* arrays to store things */
    static int crosstalk_sin[CROSSTALK_SIZE];
    static int crosstalk_cos[CROSSTALK_SIZE];
    /* crosstalk will be amplitude/div; we use these two to have integer arithmetics */
    const int crosstalk_amplitude = 384;
    const int crosstalk_div = 256;
    static bool crosstalk_calculated = false;

    if (!crosstalk_calculated) {
        crosstalk_calculated = true;
        for (int i = 0; i < CROSSTALK_SIZE; i++) {
            double f = (double)i / CROSSTALK_SIZE * 2.0 * G_PI * 2;
            crosstalk_sin[i] = crosstalk_amplitude * sin(f);
            crosstalk_cos[i] = crosstalk_amplitude * cos(f);
        }
    }

    /* apply edge detection matrix */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const int conv[] = { -1, 1, 0, 0, 0,};

            /* turnaround coordinates */
            int xm2 = (x - 2 + width) % width;
            int xm = (x - 1 + width) % width;
            int xp = (x + 1) % width;
            int xp2 = (x + 2) % width;

            /* edge detect */
            work[y][x].u = yuv[y][xm2].u * conv[0] + yuv[y][xm].u * conv[1] + yuv[y][x].u * conv[2] + yuv[y][xp].u * conv[3] + yuv[y][xp2].u * conv[4];
            work[y][x].v = yuv[y][xm2].v * conv[0] + yuv[y][xm].v * conv[1] + yuv[y][x].v * conv[2] + yuv[y][xp].v * conv[3] + yuv[y][xp2].v * conv[4];
        }
    }

    for (int y = 0; y < height; y++)
        if (y / 2 % 2 == 1) /* rows 3&4 */
            for (int x = 0; x < width; x++)
                yuv[y][x].y += (crosstalk_sin[x % CROSSTALK_SIZE] * work[y][x].u - crosstalk_cos[x % CROSSTALK_SIZE] * work[y][x].v) / crosstalk_div; /* odd lines (/2) */
        else          /* rows 1&2 */
            for (int x = 0; x < width; x++)
                yuv[y][x].y += (crosstalk_sin[x % CROSSTALK_SIZE] * work[y][x].u + crosstalk_cos[x % CROSSTALK_SIZE] * work[y][x].v) / crosstalk_div; /* even lines (/2) */
}

#undef CROSSTALK_SIZE


static void scanline_shade(YUV **yuv, YUV **work, int width, int height) {
    if (gd_pal_emu_scanline_shade < 0)
        gd_pal_emu_scanline_shade = 0;
    if (gd_pal_emu_scanline_shade > 100)
        gd_pal_emu_scanline_shade = 100;
    int shade = gd_pal_emu_scanline_shade * 256 / 100;

    /* apply shade for every second row */
    for (int y = 1; y < height; y += 2)
        for (int x = 0; x < width; x++)
            yuv[y][x].y = yuv[y][x].y * shade / 256;
}


static inline int clamp(int value, int min, int max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

void pal_emulate(Pixbuf &pb) {
    int width = pb.get_width();
    int height = pb.get_height();

    /* memory for yuv images */
    YUV **yuv = new YUV*[height];
    for (int y = 0; y < height; y++)
        yuv[y] = new YUV[width];
    YUV **work = new YUV*[height];
    for (int y = 0; y < height; y++)
        work[y] = new YUV[width];
    YUV **alpha = new YUV*[height]; /* alpha is not really yuv, only y will be used. luma blur touches only y. */
    for (int y = 0; y < height; y++)
        alpha[y] = new YUV[width];

    /* convert to yuv */
    for (int y = 0; y < height; y++) {
        guint32 *row = pb.get_row(y);

        for (int x = 0; x < width; x++) {
            int r = (row[x] >> pb.rshift) & 0xff;
            int g = (row[x] >> pb.gshift) & 0xff;
            int b = (row[x] >> pb.bshift) & 0xff;

            /* now y, u, v will contain values * 256 */
            yuv[y][x].y = 77 * r + 150 * g + 29 * b; /* always pos */
            yuv[y][x].u = -37 * r - 74 * g + 111 * b; /* pos or neg */
            yuv[y][x].v = 157 * r - 131 * g - 26 * b; /* pos or neg */

            /* alpha is copied as is, and is not *256 */
            alpha[y][x].y = (row[x] >> pb.ashift) & 0xff;
        }
    }

    /* we give them an array to "work" in, so that is not free()d and malloc() four times */
    luma_blur(yuv, work, width, height);
    chroma_blur(yuv, work, width, height);
    chroma_crosstalk_to_luma(yuv, work, width, height);
    scanline_shade(yuv, work, width, height);

    luma_blur(alpha, work, width, height);

    /* convert back to rgb */
    for (int y = 0; y < height; y++) {
        guint32 *row = pb.get_row(y);

        for (int x = 0; x < width; x++) {
            /* back to rgb */
            int r = clamp((256 * yuv[y][x].y                + 292 * yuv[y][x].v + 32768) / 65536, 0, 255);
            int g = clamp((256 * yuv[y][x].y - 101 * yuv[y][x].u - 149 * yuv[y][x].v + 32768) / 65536, 0, 255);
            int b = clamp((256 * yuv[y][x].y + 519 * yuv[y][x].u                + 32768) / 65536, 0, 255);

            /* alpha channel is preserved, others are converted back from yuv */
            row[x] = (alpha[y][x].y << pb.ashift) | (r << pb.rshift) | (g << pb.gshift) | (b << pb.bshift);
        }

    }

    /* free arrays */
    for (int y = 0; y < height; y++)
        delete[] yuv[y];
    delete[] yuv;
    for (int y = 0; y < height; y++)
        delete[] alpha[y];
    delete[] alpha;
    for (int y = 0; y < height; y++)
        delete[] work[y];
    delete[] work;
}


GdColor average_nonblack_colors_in_pixbuf(Pixbuf const &pb) {
    guint32 red = 0, green = 0, blue = 0, count = 0;
    int w = pb.get_width(), h = pb.get_height();
    for (int y = 0; y < h; ++y) {
        guint32 const *row = pb.get_row(y);
        for (int x = 0; x < w; ++x) {
            guint32 pixel = row[x];
            unsigned char tred = (pixel >> pb.rshift) & 0xFF;
            unsigned char tgreen = (pixel >> pb.gshift) & 0xFF;
            unsigned char tblue = (pixel >> pb.bshift) & 0xFF;
            // if not almost black (otherwise skip)
            if ((tred + tgreen + tblue) / 3 >= 16) {
                red += tred;
                green += tgreen;
                blue += tblue;
                count++;
            }
        }
    }
    if (count > 0)
        return GdColor::from_rgb(red / count, green / count, blue / count);
    else
        return GdColor::from_rgb(0, 0, 0);
    /* if no colors counted, all pixels were almost black - so simply return black. */
}


GdColor lightest_color_in_pixbuf(Pixbuf const &pb) {
    int red = 0, green = 0, blue = 0;
    int w = pb.get_width(), h = pb.get_height();
    for (int y = 0; y < h; ++y) {
        guint32 const *row = pb.get_row(y);
        for (int x = 0; x < w; ++x) {
            guint32 pixel = row[x];
            int tred = (pixel >> pb.rshift) & 0xFF;
            int tgreen = (pixel >> pb.gshift) & 0xFF;
            int tblue = (pixel >> pb.bshift) & 0xFF;
            // if lighter than previous
            if (tred + tgreen + tblue > red + green + blue) {
                red = tred;
                green = tgreen;
                blue = tblue;
            }
        }
    }
    return GdColor::from_rgb(red, green, blue);
}
