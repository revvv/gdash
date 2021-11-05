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

#ifndef EDITORCELLRENDERER_HPP_INCLUDED
#define EDITORCELLRENDERER_HPP_INCLUDED

#include "config.h"

#include <gtk/gtk.h>

#include "gfx/cellrenderer.hpp"
#include "gfx/pixbuffactory.hpp"

class GTKPixbufFactory;
class CaveRendered;

/**
 * @brief The EditorCellRenderer is a special cell renderer used in the GTK
 * version of the game, which has cells used only in the editor.
 *
 * When created (or colors are changed), some cells are also drawn. Most of
 * these cells are for example creatures with direction of movement arrows.
 */
class EditorCellRenderer: public CellRenderer {
private:
    /** Small pixbufs for the editor. */
    GdkPixbuf *combo_pixbufs[NUM_OF_CELLS];

    void add_arrow_to_cell(GdElementEnum dest, GdElementEnum src, GdElementEnum arrow, PixbufFactory::Rotation r = PixbufFactory::None);
    void copy_cell(int dest, int src);
    void create_composite_cell_pixbuf(GdElementEnum dest, GdElementEnum src1, GdElementEnum src2);
    void draw_editor_pixbufs();
    GdkPixbuf *get_element_pixbuf_with_border(int index);

public:
    /**
     * @brief Constructor.
     *
     * Loads a theme file (or "" for default theme) to draw on the screen.
     * @param screen The screen and graphics engine to use.
     * @param theme_name The theme name from the caveset "charset" field.
     * @param default_theme_file The name of the file to load a theme from, if the specified theme is not found. Can be the empty string, to load the built-in theme.
     */
    EditorCellRenderer(Screen &screen, const std::string &theme_file);
    virtual void remove_cached();
    virtual void select_pixbuf_colors(GdColor c0, GdColor c1, GdColor c2, GdColor c3, GdColor c4, GdColor c5);

    /** Returns cave background color currently used by the cell renderer. */
    const GdColor &background_color() const {
        return color0;
    }

    /** Convenience function which returns the GdkPixbuf* of a cell. */
    GdkPixbuf *cell_gdk_pixbuf(unsigned i);

    /** Convenience function which returns the scaled GdkPixbuf* of a cell. */
    cairo_surface_t *cell_cairo_surface(unsigned i);

    /** @brief Returns a small picture of the element, for use in the editor.
     *  The picture will be GTK_ICON_SIZE_MENU, +1 pixel black border.
     *  For convenience, the picture is returned as a GdkPixbuf *.
     */
    GdkPixbuf *combo_pixbuf(GdElementEnum element);

    /** @brief Returns a small picture of the element, for use in the editor.
     *  The picture is the simplistic one (not showing movement arrows).
     *  The picture will be GTK_ICON_SIZE_MENU, +1 pixel black border.
     *  For convenience, the picture is returned as a GdkPixbuf *.
     */
    GdkPixbuf *combo_pixbuf_simple(GdElementEnum element);
};

GdkPixbuf *gd_drawcave_to_pixbuf(const CaveRendered &cave, EditorCellRenderer &cr, int width, int height, bool game_view, bool border);

#endif
