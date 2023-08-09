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

#include <algorithm>
#include <memory>
#include "gfx/pixbuf.hpp"
#include "gfx/screen.hpp"
#include "gfx/pixbuffactory.hpp"
#include "gfx/pixmapstorage.hpp"


#include "gdash_icon_32.cpp"
#include "gdash_icon_48.cpp"

/* point pointers to the images so they are nonstatic. the included cpp files
 * make them static. */
unsigned char const *Screen::gdash_icon_32_png = gdash_icon_32;
unsigned const Screen::gdash_icon_32_size = sizeof(gdash_icon_32);
unsigned char const *Screen::gdash_icon_48_png = gdash_icon_48;
unsigned const Screen::gdash_icon_48_size = sizeof(gdash_icon_48);


void Screen::set_properties(double scaling_factor_, GdScalingType scaling_type_, bool pal_emulation_) {
    scaling_factor = scaling_factor_;
    scaling_type = scaling_type_;
    pal_emulation = pal_emulation_;
}


void Screen::set_size(int w, int h, bool fullscreen) {
    if (w != this->w || h != this->h || fullscreen != this->fullscreen) {
        /* notify pixmap storages before the actual change */
        for (std::vector<PixmapStorage *>::const_iterator it = pixmap_storages.begin(); it != pixmap_storages.end(); ++it)
            (*it)->release_pixmaps();
        /* then change the resolution */
        this->w = w;
        this->h = h;
        this->fullscreen = fullscreen;
        configure_size();

        did_some_drawing = false;
    }
}


void Screen::register_pixmap_storage(PixmapStorage *ps) {
    unregister_pixmap_storage(ps);
    pixmap_storages.push_back(ps);
}


void Screen::unregister_pixmap_storage(PixmapStorage *ps) {
    pixmap_storages.erase(std::remove(pixmap_storages.begin(), pixmap_storages.end(), ps), pixmap_storages.end());
}


std::unique_ptr<Pixmap> Screen::create_scaled_pixmap_from_pixbuf(const Pixbuf &pb, bool keep_alpha) const {
    std::unique_ptr<Pixbuf> scaled(pixbuf_factory.create_scaled(pb, scaling_factor, scaling_type, pal_emulation));
    return create_pixmap_from_pixbuf(*scaled, keep_alpha);
}


void Screen::blit_pixbuf(const Pixbuf &pb, int dx, int dy, bool keep_alpha) {
    std::unique_ptr<Pixmap> pm(create_pixmap_from_pixbuf(pb, keep_alpha));
    blit(*pm, dx, dy);
}
