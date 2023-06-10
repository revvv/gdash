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
#include <memory>
#include <cmath>

#include "gtk/gtkpixbuf.hpp"
#include "gtk/gtkpixbuffactory.hpp"
#include "gtk/gtkscreen.hpp"
#include "cave/colors.hpp"
#include "cave/particle.hpp"
#include "settings.hpp"
#include "misc/logger.hpp"

int GTKPixmap::get_width() const {
    return gdk_pixbuf_get_width(pixbuf.get());
}


int GTKPixmap::get_height() const {
    return gdk_pixbuf_get_height(pixbuf.get());
}


GTKScreen::GTKScreen(PixbufFactory &pixbuf_factory, GtkWidget *drawing_area)
    :   Screen(pixbuf_factory),
        drawing_area(drawing_area) {
    set_drawing_area(drawing_area);
}


void GTKScreen::free_buffer() {
    cr.reset();
    back.reset();
}


void GTKScreen::configure_size() {
    free_buffer();
    /* invalid size? do not configure yet. */
    if (w == 0 || h == 0)
        return;
    if (drawing_area != NULL) {
        /* rendering to a screen. */
        gtk_widget_set_size_request(drawing_area, w, h);
        /* when using particle effects, a client-side image is created for the back buffer.
         * when not using particles, a server-side image.
         * this is because drawing particle effects is much more efficient on the client side.
         * if we were drawing on the serverside, it would be very slow, as rgba rendering requires
         * roundtrips to the server.
         * the chosen back image will also affect the pixmap objects created, but that is handled
         * automatically by cairo. */
        if (gd_particle_effects)
            back.reset(cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h));
        else {
            if (gtk_widget_get_window(drawing_area) != NULL) {
                back.reset(gdk_window_create_similar_surface(gtk_widget_get_window(drawing_area), CAIRO_CONTENT_COLOR, w, h));
            } else {
                // FIX Issue #1
                gd_message("configure_size: window w/o particle effects");
                back.reset(cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h));
            }
        }
    } else {
        /* rendering to a software buffer. */
        back.reset(cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h));
    }
    cr.reset(cairo_create(back.get()));
}

bool destroyed = false;

void GTKScreen::set_title(char const *title) {

    /* This method is sometimes called on a destroyed window.
     * Test case: Start game with space bar -> Start editor through menu item = Crash
     * As I don't know how to prevent this, I mark this object as destroyed
     * by calling set_title(NULL).
     */
    if (destroyed) {
       destroyed = false;
       gd_message("set_title: ignoring '%s'", title);
       return;
    }
    if (title == NULL) {
       destroyed = true;
       return;
    }

    /* Without above workaround:
     * cygwin: Gtk-CRITICAL **: gtk_widget_get_toplevel: assertion 'GTK_IS_WIDGET (widget)' failed
     * mingw: SIGSEGV, Segmentation fault
     */

    if (drawing_area != NULL) {
        GtkWidget *toplevel = gtk_widget_get_toplevel(drawing_area);
        gtk_window_set_title(GTK_WINDOW(toplevel), title);
    }
}


void GTKScreen::set_drawing_area(GtkWidget *drawing_area) {
    free_buffer();
    this->drawing_area = drawing_area;
    if (drawing_area == NULL) {
        /* make sure that the new set size request will do its job */
        w = h = 0;
    } else {
        g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_event), this);
    }
    /* call this to reconfigure back buffer & stuff */
    configure_size();
}


void GTKScreen::set_clip_rect(int x1, int y1, int w, int h) {
    cairo_rectangle(cr.get(), x1, y1, w, h);
    cairo_clip(cr.get());
}


void GTKScreen::remove_clip_rect() {
    cairo_reset_clip(cr.get());
}

gboolean GTKScreen::draw_event(GtkWidget *drawing_area, cairo_t *cairo_context, gpointer data) {
    GTKScreen *dis = static_cast<GTKScreen*>(data);
    cairo_set_source_surface(cairo_context, dis->back.get(), 0, 0);
    cairo_rectangle(cairo_context, 0, 0, dis->w, dis->h);
    cairo_fill(cairo_context);
    
    return TRUE;
}

void GTKScreen::flip() {
    /* if rendering to a software buffer, flipping does nothing. */
    if (drawing_area == NULL)
        return;
    /* otherwise queue a draw event */
    gtk_widget_queue_draw_area(drawing_area, 0, 0, w, h);
}


void GTKScreen::fill_rect(int x, int y, int w, int h, const GdColor &c) {
    unsigned char r, g, b;
    c.get_rgb(r, g, b);
    cairo_set_source_rgb(cr.get(), r / 255.0, g / 255.0, b / 255.0);
    cairo_rectangle(cr.get(), x, y, w, h);
    cairo_fill(cr.get());
}


void GTKScreen::blit(Pixmap const &src, int dx, int dy) const {
    int sw = src.get_width(), sh = src.get_height();
    /* if totally out of window, skip */
    if (dx + sw < 0 || dy + sh < 0 || dx > w || dy > h)
        return;
    /* "The x and y parameters give the user-space coordinate at which the surface origin should appear." */
    GTKPixmap &srcgtk = const_cast<GTKPixmap &>(static_cast<GTKPixmap const &>(src));
    cairo_set_source_surface(cr.get(), srcgtk.get_cairo_surface(), dx, dy);
    cairo_rectangle(cr.get(), dx, dy, sw, sh);
    cairo_fill(cr.get());
}


std::unique_ptr<Pixmap> GTKScreen::create_pixmap_from_pixbuf(const Pixbuf &pb, bool keep_alpha) const {
    GdkPixbuf *pixbuf = (GdkPixbuf *) static_cast<GTKPixbuf const &>(pb).get_gdk_pixbuf();
    /* we keep the pixmap in a surface that is similar to the back buffer.
     * if the back buffer is an xlib pixmap, then this will be an xlib pixmap as well,
     * and the blitting will be very fast, and done by the x server.
     * if it is a cairo image, this one also will be a cairo image, and cairo will
     * do the blitting. */
    cairo_surface_t *surface = cairo_surface_create_similar(back.get(),
                               keep_alpha ? CAIRO_CONTENT_COLOR_ALPHA : CAIRO_CONTENT_COLOR, w, h);
    cairo_t *crs = cairo_create(surface);
    gdk_cairo_set_source_pixbuf(crs, pixbuf, 0, 0);
    cairo_rectangle(crs, 0, 0, w, h);
    cairo_fill(crs);
    cairo_destroy(crs);
    return std::make_unique<GTKPixmap>(pixbuf, surface);
}


void GTKScreen::draw_particle_set(int dx, int dy, ParticleSet const &ps) {
    unsigned char r, g, b;
    ps.color.get_rgb(r, g, b);
    cairo_set_source_rgba(cr.get(), r / 255.0, g / 255.0, b / 255.0, ps.life / 1000.0 * ps.opacity);
    int size = ceil(ps.size);
    /* cairo gets the center of the pixel, like opengl. because it works with
     * float coordinates, not integers.
     * dx0, dy0 are the center, and the sides "outgrow". */
    double dxm = dx - size, dx0 = dx + 0.5, dxp = dx + size + 1;
    double dym = dy - size, dy0 = dy + 0.5, dyp = dy + size + 1;
    for (ParticleSet::const_iterator it = ps.begin(); it != ps.end(); ++it) {
        cairo_move_to(cr.get(), (int)it->px + dx0, (int)it->py + dym);
        cairo_line_to(cr.get(), (int)it->px + dxp, (int)it->py + dy0);
        cairo_line_to(cr.get(), (int)it->px + dx0, (int)it->py + dyp);
        cairo_line_to(cr.get(), (int)it->px + dxm, (int)it->py + dy0);
        cairo_fill(cr.get());
    }
}
