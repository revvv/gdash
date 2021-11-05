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
#ifndef PIXBUFFACTORY_HPP_INCLUDED
#define PIXBUFFACTORY_HPP_INCLUDED

#include "config.h"

#include <memory>

class Pixbuf;
class GdColor;

/// Scaling types supported by the pixbuf engine
enum GdScalingType {
    GD_SCALING_NEAREST,    ///< nearest neighbor
    GD_SCALING_SCALE2X,    ///< Scale2X algorithm by Andrea Mazzoleni
    GD_SCALING_HQX,        ///< HQX algorithm by Maxim Stepin and Cameron Zemek
    GD_SCALING_MAX,
};

/// Names of scaling types supported.
extern const char *gd_scaling_names[];


/// @ingroup Graphics
class PixbufFactory {
public:
    /// @brief Create a new pixbuf factory.
    PixbufFactory() = default;

    /// @brief Virtual destructor.
    virtual ~PixbufFactory() = default;

    /// @brief Create a new pixbuf, with non-initialized memory.
    /// @param w The width of the pixbuf.
    /// @param h The height of the pixbuf.
    /// @return A newly allocated pixbuf object. Free with delete.
    virtual std::unique_ptr<Pixbuf> create(int w, int h) const = 0;

    /// @brief Create a new pixbuf, and load an image from memory.
    /// @param length The number of bytes of the image.
    /// @param data Pointer to the image in memory.
    /// @return A newly allocated pixbuf object. Free with delete.
    virtual std::unique_ptr<Pixbuf> create_from_inline(int length, unsigned char const *data) const = 0;

    /// @brief Create a new pixbuf, and load an image from a file.
    /// @param filename The name of the file to load.
    /// @return A newly allocated pixbuf object. Free with delete.
    virtual std::unique_ptr<Pixbuf> create_from_file(const char *filename) const = 0;

    /// @brief Create a new pixbuf, and load an image file from memory, which is base64 encoded..
    /// @param base64 Base64 encoded image string, delimited with zero.
    /// @return A newly allocated pixbuf object. Free with delete.
    std::unique_ptr<Pixbuf> create_from_base64(const char *base64) const;

    /// @brief Composite pixbuf with color c, and return the new (composited) one
    /// @param c Color
    /// @param a Alpha value; 0 will be invisible, 255 will totally cover. Default is 128, which looks nice, and is accelerated by SDL.
    /// @return The new pixbuf.
    virtual std::unique_ptr<Pixbuf> create_composite_color(const Pixbuf &src, const GdColor &c, unsigned char alpha = 128) const = 0;

    /// @brief Create a pixbuf, which is a part of this one.
    /// Pixels will be shared!
    virtual std::unique_ptr<Pixbuf> create_subpixbuf(Pixbuf &src, int x, int y, int w, int h) const = 0;

    /// @brief Use the selected software scaled to create a new, enlarged pixbuf.
    /// @param src The pixbuf to scale.
    /// @param scaling_factor The factor of enlargement, 1x, 2x, 3x or 4x.
    /// @param scaling_type The scaling algorithm.
    /// @param pal_emulation Whether to add a PAL TV effect.
    /// @return The scaled pixbuf, to be freed by the caller.
    std::unique_ptr<Pixbuf> create_scaled(const Pixbuf &src, int scaling_factor, GdScalingType scaling_type, bool pal_emulation) const;

    /// Names of rotations.
    enum Rotation {
        None,
        CounterClockWise,
        UpsideDown,
        ClockWise
    };

    /// Creates a new, rotated pixbuf.
    virtual std::unique_ptr<Pixbuf> create_rotated(const Pixbuf &src, Rotation r) const = 0;
};

#endif
