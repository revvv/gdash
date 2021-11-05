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

#include <memory>

#include "gfx/cellrenderer.hpp"

#include "misc/logger.hpp"
#include "gfx/pixbuf.hpp"
#include "gfx/pixbuffactory.hpp"
#include "gfx/screen.hpp"


/* data */
#include "c64_gfx.cpp"

CellRenderer::CellRenderer(Screen &screen, const std::string &theme_file)
    :   PixmapStorage(screen),
        is_c64_colored(false),
        cell_size(0),
        color0(GD_GDASH_BLACK),
        color1(GD_GDASH_MIDDLEBLUE),
        color2(GD_GDASH_LIGHTRED),
        color3(GD_GDASH_WHITE),
        color4(GD_GDASH_WHITE),
        color5(GD_GDASH_WHITE),
        screen(screen) {
    load_theme_file(theme_file);
}


/** Remove colored Pixbufs and Pixmaps created. */
void CellRenderer::remove_cached() {
    for (unsigned i = 0; i < G_N_ELEMENTS(cells_pixbufs); ++i) {
        cells_pixbufs[i].reset();
    }
    CellRenderer::release_pixmaps();
    if (is_c64_colored) {
        cells_all.reset();
    }
}


void CellRenderer::release_pixmaps() {
    for (unsigned i = 0; i < G_N_ELEMENTS(cells); ++i) {
        cells[i].reset();
    }
}



Pixbuf &CellRenderer::cell_pixbuf(unsigned i) {
    g_assert(i < G_N_ELEMENTS(cells_pixbufs));
    if (cells_all == NULL)
        create_colorized_cells();
    g_assert(cells_all != NULL);
    if (cells_pixbufs[i] == NULL)
        cells_pixbufs[i] = screen.pixbuf_factory.create_subpixbuf(*cells_all, (i % NUM_OF_CELLS_X) * cell_size, (i / NUM_OF_CELLS_X) * cell_size, cell_size, cell_size);
    return *cells_pixbufs[i];
}

Pixmap &CellRenderer::cell(unsigned i) {
    g_assert(i < G_N_ELEMENTS(cells));
    if (cells[i] == NULL) {
        int type = i / NUM_OF_CELLS;  // 0=normal, 1=colored1, 2=colored2
        int index = i % NUM_OF_CELLS;
        Pixbuf &pb = cell_pixbuf(index);    // this is to be rendered as a pixmap, but may be colored

        switch (type) {
            case 0:
                cells[i] = screen.create_scaled_pixmap_from_pixbuf(pb, false);
                break;
            case 1: {
                std::unique_ptr<Pixbuf> colored(screen.pixbuf_factory.create_composite_color(pb, gd_flash_color));
                cells[i] = screen.create_scaled_pixmap_from_pixbuf(*colored, false);
            }
            break;
            case 2: {
                std::unique_ptr<Pixbuf> colored(screen.pixbuf_factory.create_composite_color(pb, gd_select_color));
                cells[i] = screen.create_scaled_pixmap_from_pixbuf(*colored, false);
            }
            break;
            default:
                g_assert_not_reached();
                break;
        }
    }
    return *cells[i];
}

/* check if given surface is ok to be a gdash theme. */
bool CellRenderer::is_pixbuf_ok_for_theme(const Pixbuf &surface) {
    if ((surface.get_width() % NUM_OF_CELLS_X != 0)
            || (surface.get_height() % NUM_OF_CELLS_Y != 0)
            || (surface.get_width() / NUM_OF_CELLS_X != surface.get_height() / NUM_OF_CELLS_Y)) {
        gd_critical("Image should contain %d cells in a row and %d in a column!", int(NUM_OF_CELLS_X), int(NUM_OF_CELLS_Y));
        return false;
    }
    if (surface.get_width() / NUM_OF_CELLS_X < 16) {
        gd_critical("The image should contain cells which are at least 16x16 pixels in size!");
    }

    return true;    /* passed checks */
}

bool CellRenderer::is_image_ok_for_theme(PixbufFactory &pixbuf_factory, const char *filename) {
    try {
        SetLoggerContextForFunction scf(filename);
        std::unique_ptr<Pixbuf> image(pixbuf_factory.create_from_file(filename));
        return is_pixbuf_ok_for_theme(*image);
    } catch (...) {
        return false;
    }
}

/* load theme from image file. */
/* return true if successful. */
bool CellRenderer::loadcells_image(std::unique_ptr<Pixbuf> image) {
    /* do some checks. if those fail, the error is already reported by the function */
    if (!is_pixbuf_ok_for_theme(*image))
        return false;

    /* remove old stuff */
    remove_cached();
    loaded.reset();

    /* load new stuff */
    cell_size = image->get_width() / NUM_OF_CELLS_X;
    if (check_if_pixbuf_c64_png(*image)) {
        /* c64 pixbuf with a small number of colors which can be changed */
        loaded = std::move(image);
        cells_all = NULL;
        is_c64_colored = true;
    } else {
        /* normal, "truecolor" pixbuf */
        loaded = NULL;
        cells_all = std::move(image);
        is_c64_colored = false;
    }
    return true;
}


/* load theme from image file. */
/* return true if successful. */
bool CellRenderer::loadcells_file(const std::string &filename) {
    /* load cell graphics */
    /* load from file */
    try {
        std::unique_ptr<Pixbuf> image = screen.pixbuf_factory.create_from_file(filename.c_str());
        return loadcells_image(std::move(image));
    } catch (std::exception &e) {
        gd_critical("%s: unable to load image (%s)", filename, e.what());
        return false;
    }
}

/* load the theme specified in theme_file. */
/* if successful, ok. */
/* if fails, or no theme specified, load the builtin */
void CellRenderer::load_theme_file(const std::string &theme_file) {
    if (theme_file != "" && loadcells_file(theme_file)) {
        /* loaded from png file */
    } else {
        std::unique_ptr<Pixbuf> image = screen.pixbuf_factory.create_from_inline(sizeof(c64_gfx), c64_gfx);
        loadcells_image(std::move(image));
    }
}

int CellRenderer::get_cell_size() {
    return cell_size * screen.get_pixmap_scale();
}


void CellRenderer::select_pixbuf_colors(GdColor c0, GdColor c1, GdColor c2, GdColor c3, GdColor c4, GdColor c5) {
    if (c0 != color0 || c1 != color1 || c2 != color2 || c3 != color3 || c4 != color4 || c5 != color5) {
        /* if not the same colors as requested before */
        color0 = c0;
        color1 = c1;
        color2 = c2;
        color3 = c3;
        color4 = c4;
        color5 = c5;
        if (is_c64_colored)
            remove_cached();
    }
}



static inline int
c64_color_index(int h, int s, int v, int a) {
    if (a < 0x80)
        return 8;   /* transparent */
    if (v < 0x10)
        return 0;   /* black */
    if (s < 0x10)
        return 7;   /* editor white, arrows & etc */

    if (h < 30 || h >= 330)  /* around 0 */
        return 1;            /* red - foreg1 */
    if (h >= 270 && h < 330) /* around 300 */
        return 2;            /* purple - foreg2 */
    if (h >= 30 && h < 90)   /* around 60 */
        return 3;            /* yellow - foreg3 */
    if (h >= 90 && h < 150)  /* around 120 */
        return 4;            /* green - amoeba */
    if (h >= 210 && h < 270) /* around 240 */
        return 5;            /* slime */

    if (h >= 150 && h < 210) /* around 180 */
        return 6;            /* cyan - editor black */

    return 0;                /* should be unreachable */
}


/* returns true, if the given pixbuf seems to be a c64 imported image. */
bool CellRenderer::check_if_pixbuf_c64_png(Pixbuf const &image) {
    int wx = image.get_width() * 4;   // 4 bytes/pixel
    int h = image.get_height();

    bool c64_png = true;
    for (int y = 0; y < h; y++) {
        const unsigned char *p = (const unsigned char *) image.get_row(y);
        for (int x = 0; x < wx; x++)
            if (p[x] != 0 && p[x] != 255)
                c64_png = false;
    }

    return c64_png;
}


/** This function takes the loaded image, and transforms it using the selected
 * cave colors, to create cells_all.
 *
 * The process is as follows. All pixels are converted to HSV (hue, saturation,
 * value). The hues of the pixels should be 0 (red), 60 (yellow), 120 (green)
 * etc, n*60. This way will the routine recognize the colors.
 * After converting the pixels to HSV, the hue selects the cave color to use.
 * The resulting color will use the hue of the selected cave color, the product
 * of the saturations of the cave color and the original color, and the
 * product of the values:
 *
 *   Hresult = Hcave
 *   Sresult = Scave * Sloadedimage
 *   Vresult = Vcave * Vloadedimage.
 *
 * This allows for modulating the cave colors in saturation and value. If the
 * loaded image contains a dark purple color instead of RGB(255;0;255) purple,
 * the cave color will also be darkened at that pixel and so on. */
void CellRenderer::create_colorized_cells() {
    g_assert(is_c64_colored);
    g_assert(loaded != NULL);

    GdColor colshsv[9], colsrgb[9];

    /* colors used, in hsv */
    colshsv[0] = color0.to_hsv(); /* c64 background */
    colshsv[1] = color1.to_hsv(); /* foreg1 */
    colshsv[2] = color2.to_hsv(); /* foreg2 */
    colshsv[3] = color3.to_hsv(); /* foreg3 */
    colshsv[4] = color4.to_hsv(); /* amoeba */
    colshsv[5] = color5.to_hsv(); /* slime */
    colshsv[6] = GdColor::from_hsv(0, 0, 0);    /* black, opaque */
    colshsv[7] = GdColor::from_hsv(0, 0, 100);  /* white, opaque */
    colshsv[8] = GdColor::from_hsv(0, 0, 0);    /* for the transparent */
    /* the same with rgb values */
    colsrgb[0] = color0.to_rgb(); /* c64 background */
    colsrgb[1] = color1.to_rgb(); /* foreg1 */
    colsrgb[2] = color2.to_rgb(); /* foreg2 */
    colsrgb[3] = color3.to_rgb(); /* foreg3 */
    colsrgb[4] = color4.to_rgb(); /* amoeba */
    colsrgb[5] = color5.to_rgb(); /* slime */
    colsrgb[6] = colshsv[6].to_rgb();  /* black, opaque */
    colsrgb[7] = colshsv[7].to_rgb();  /* white, opaque */
    colsrgb[8] = colshsv[8].to_rgb();  /* for the transparent */

    int w = loaded->get_width(), h = loaded->get_height();
    cells_all = screen.pixbuf_factory.create(w, h);

    for (int y = 0; y < h; y++) {
        const guint32 *p = loaded->get_row(y);
        guint32 *to = cells_all->get_row(y);
        for (int x = 0; x < w; x++) {
            /* rgb values found in image */
            unsigned r = (p[x] & loaded->rmask) >> loaded->rshift;
            unsigned g = (p[x] & loaded->gmask) >> loaded->gshift;
            unsigned b = (p[x] & loaded->bmask) >> loaded->bshift;
            unsigned a = (p[x] & loaded->amask) >> loaded->ashift;
            unsigned short inh;
            unsigned char ins, inv;
            GdColor::from_rgb(r, g, b).get_hsv(inh, ins, inv);

            /* the color code from the original image (essentially the hue) will select the color index */
            unsigned index = c64_color_index(inh, ins, inv, a);

            /* and then shade it, and convert to rgb */
            unsigned char resr, resg, resb;
            if (index == 0 || index >= 6) {
                /* for the background and the editor colors, no shading is used */
                colsrgb[index].get_rgb(resr, resg, resb);
            } else {
                /* otherwise the saturation and value from the original image will modify it */
                unsigned short pixh;
                unsigned char pixs, pixv;
                colshsv[index].get_hsv(pixh, pixs, pixv);
                GdColor::from_hsv(pixh, pixs * ins / 100, pixv * inv / 100).get_rgb(resr, resg, resb);
            }

            guint32 newcolword =
                resr << cells_all->rshift | resg << cells_all->gshift | resb << cells_all->bshift | a << cells_all->ashift;

            to[x] = newcolword;
        }
    }
}
