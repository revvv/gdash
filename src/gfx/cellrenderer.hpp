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

#ifndef CELLRENDERER_HPP_INCLUDED
#define CELLRENDERER_HPP_INCLUDED

#include <vector>

#include "cave/cavetypes.hpp"
#include "cave/colors.hpp"
#include "gfx/pixmapstorage.hpp"
#include "gfx/pixbuf.hpp"

class PixbufFactory;
class Pixmap;
class Screen;

/// @ingroup Graphics
/// @brief The class which is responsible for rendering the cave pixbufs.
/// In its constructor, it takes a PixbufFactory as a parameter (for the graphics
/// subsystem), and the name of a theme file to load.
/// If the theme file cannot be loaded, or an empty string is given as file name,
/// it switches to the builtin default theme.
/// After loading the theme file, select_pixbuf_colors is used to select a color theme.
/// A Pixbuf of a cell can be retrieved using cell(i).
class CellRenderer : public PixmapStorage {
protected:
    /// The pixbuf which was loaded from the disk.
    std::unique_ptr<Pixbuf> loaded;

    /// The pixbuf which stores rgb data of all images.
    std::unique_ptr<Pixbuf> cells_all;

    bool is_c64_colored;

    /// The size of the loaded pixbufs
    unsigned cell_size;

    /// The cache to store the pixbufs already rendered.
    std::unique_ptr<Pixbuf> cells_pixbufs[NUM_OF_CELLS];

    /// The cache to store the pixbufs already rendered.
    std::unique_ptr<Pixmap> cells[3 * NUM_OF_CELLS];

    /// If using c64 gfx, these store the current color theme.
    GdColor color0, color1, color2, color3, color4, color5;

    void create_colorized_cells();
    bool loadcells_image(std::unique_ptr<Pixbuf> loadcells_image);
    bool loadcells_file(const std::string &filename);
    virtual void remove_cached();

public:
    /// The Screen for which the CellRenderer is drawing.
    Screen &screen;

    /// Constructor. Loads a theme file (or nothing); uses pixbuf_factory as a gfx engine.
    CellRenderer(Screen &screen, std::string const &theme_file);

    /// To implement PixbufStorage.
    virtual void release_pixmaps();

    /// Destructor.
    virtual ~CellRenderer() = default;

    /// @brief Loads a new theme.
    /// The theme_file can be a file name of a png file, or empty.
    void load_theme_file(const std::string &theme_file);

    /// @brief Returns a particular cell.
    Pixbuf &cell_pixbuf(unsigned i);

    /// @brief Returns a particular cell.
    Pixmap &cell(unsigned i);

    /// @brief Returns the size of the pixmaps stored.
    /// They are squares, so there is only one function, not two for width and height.
    int get_cell_size();

    /// @brief Returns the size of the pixbufs
    /// They are squares, so there is only one function, not two for width and height.
    int get_cell_pixbuf_size() {
        return cell_size;
    }

    /// Returns the background color currently used.
    const GdColor &get_background_color() const {
        return color0;
    }

    /// @brief Select a color theme, when using C64 graphics.
    /// If no c64 graphics is used, then this function does nothing.
    virtual void select_pixbuf_colors(GdColor c0, GdColor c1, GdColor c2, GdColor c3, GdColor c4, GdColor c5);

    /// @brief This function checks if a file is suitable to be used as a GDash theme.
    static bool is_image_ok_for_theme(PixbufFactory &pixbuf_factory, const char *image);
    static bool is_pixbuf_ok_for_theme(Pixbuf const &surface);
    static bool check_if_pixbuf_c64_png(Pixbuf const &image);
};


#endif
