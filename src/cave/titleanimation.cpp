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

#include "title.cpp"

#include <memory>

#include "misc/logger.hpp"
#include "gfx/pixbuf.hpp"
#include "gfx/pixbuffactory.hpp"
#include "gfx/screen.hpp"
#include "cave/colors.hpp"
#include "cave/cavetypes.hpp"
#include "cave/titleanimation.hpp"

std::vector<std::unique_ptr<Pixbuf>> get_title_animation_pixbuf(const GdString &title_screen, const GdString &title_screen_scroll, bool one_frame_only, PixbufFactory &pixbuf_factory) {
    std::vector<std::unique_ptr<Pixbuf>> animation;

    std::unique_ptr<Pixbuf> screen, tile;
    try {
        if (title_screen != "")
            screen = pixbuf_factory.create_from_base64(title_screen.c_str());
        if (screen.get() != NULL && title_screen_scroll != "")
            tile = pixbuf_factory.create_from_base64(title_screen_scroll.c_str());
    } catch (std::exception &e) {
        gd_message("Caveset is storing an invalid title screen image: %s", e.what());
        return animation;
    }

    if (tile.get() != NULL && tile->get_height() > 40) {
        gd_message("Caveset is storing an oversized tile image");
        tile.release();
    }

    /* if no special title image or unable to load that one, load the built-in */
    if (screen.get() == NULL) {
        /* the screen */
        screen = pixbuf_factory.create_from_inline(sizeof(gdash_screen), gdash_screen);
        /* the tile to be put under the screen */
        tile = pixbuf_factory.create_from_inline(sizeof(gdash_tile), gdash_tile);
        g_assert(screen.get() != NULL);
        g_assert(tile.get() != NULL);
    }

    /* if no tile, let it be black. */
    if (tile.get() == NULL) {
        /* one-row pixbuf, so no animation; totally black. */
        tile = pixbuf_factory.create(screen->get_width(), 1);
        tile->fill(GdColor::from_rgb(0, 0, 0));
    }

    /* do not allow more than 40 frames of animation */
    g_assert(tile->get_height() < 40);

    /* create a big image, which is one tile larger than the title image size */
    std::unique_ptr<Pixbuf> bigone = pixbuf_factory.create(screen->get_width(), screen->get_height() + tile->get_height());
    /* and fill it with the tile. use copy(), so pixbuf data is initialized! */
    for (int y = 0; y < screen->get_height() + tile->get_height(); y += tile->get_height())
        for (int x = 0; x < screen->get_width(); x += tile->get_width())
            tile->copy(*bigone, x, y);

    int framenum = one_frame_only ? 1 : tile->get_height();
    for (int i = 0; i < framenum; i++) {
        std::unique_ptr<Pixbuf> frame = pixbuf_factory.create(screen->get_width(), screen->get_height());
        // copy part of the big tiled image
        bigone->copy(0, i, screen->get_width(), screen->get_height(), *frame, 0, 0);
        // and composite it with the title image
        screen->blit(*frame, 0, 0);
        // copy to array
        animation.push_back(std::move(frame));
    }

    return animation;
}

std::vector<std::unique_ptr<Pixmap>> get_title_animation_pixmap(const GdString &title_screen, const GdString &title_screen_scroll, bool one_frame_only, Screen &screen, PixbufFactory &pixbuf_factory) {
    std::vector<std::unique_ptr<Pixbuf>> pixbufs = get_title_animation_pixbuf(title_screen, title_screen_scroll, one_frame_only, pixbuf_factory);
    if (pixbufs.empty())
        pixbufs = get_title_animation_pixbuf(GdString(), GdString(), one_frame_only, pixbuf_factory);

    std::vector<std::unique_ptr<Pixmap>> pixmaps;
    for (unsigned i = 0; i < pixbufs.size(); ++i)
        pixmaps.push_back(screen.create_scaled_pixmap_from_pixbuf(*pixbufs[i], false));

    return pixmaps;
}
