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
#ifndef COLORS_HPP_INCLUDED
#define COLORS_HPP_INCLUDED

#include "config.h"

#include <iostream>

#include "misc/printf.hpp"

/// @brief A class which stores a color in a cave.
///
/// Can store different kind of colors:
/// - C64 colors,
/// - C64DTV colors,
/// - Atari colors,
/// - RGB colors.
///
/// Can convert any of them to RGB color during playing the game.
/// The conversion from paletted to RGB colors uses a palette,
/// determined by user preferences.
class GdColor {
public:
    /// This enum selects the type of the color used.
    enum Type {
        TypeRGB,
        TypeC64,
        TypeC64DTV,
        TypeAtari,
        TypeHSV,
        TypeInvalid,
    };
    /// Constructor, which created a color object, initialized to invalid.
    /// For convenience only; any color which is to be used must be initialized.
    GdColor() : type(TypeInvalid) { }
    static GdColor from_rgb(unsigned r, unsigned g, unsigned b);
    static GdColor from_hsv(unsigned short h, unsigned char s, unsigned char v);
    static GdColor from_c64(unsigned index);
    static GdColor from_atari(unsigned index);
    static GdColor from_atari_huesat(unsigned hue, unsigned sat);
    static GdColor from_c64dtv(unsigned index);
    static GdColor from_c64dtv_huesat(unsigned hue, unsigned sat);
    static GdColor from_gdash_index(unsigned c);    /* similar to c64 colors */

    bool operator==(const GdColor &rhs) const;
    /// Compare two color objects for inequality.
    /// @return True, if they are not equal.
    bool operator!=(const GdColor &rhs) const {
        return !(*this == rhs);
    }

    void get_rgb(unsigned char & r, unsigned char & g, unsigned char & b) const;
    void get_hsv(unsigned short & h, unsigned char & s, unsigned char & v) const;
    unsigned int get_uint_0rgb() const;
    GdColor to_rgb() const;
    GdColor to_hsv() const;

    bool is_c64() const;

    int get_c64_index() const;

    static const char **get_c64_palette_names();
    static const char **get_atari_palette_names();
    static const char **get_c64dtv_palette_names();
    static const char **get_palette_types_names();

private:
    Type type;
    union {
        unsigned char index;
        struct {
            unsigned char r;
            unsigned char g;
            unsigned char b;
        } rgb;
        struct {
            unsigned short h;  /* [0;360) */
            unsigned char s;   /* [0;100] */
            unsigned char v;   /* [0;100] */
        } hsv;
    };

    GdColor(unsigned char r, unsigned char g, unsigned char b) :
        type(TypeRGB) {
        rgb.r = r;
        rgb.g = g;
        rgb.b = b;
    }
    GdColor(Type _type, int i) :
        type(_type) {
        index = i;
    }
    GdColor(unsigned int rgb);

    friend std::string visible_name(const GdColor &c);
    friend std::ostream &operator<<(std::ostream &os, const GdColor &c);
};


/// Get red, green, blue (0..255) component of color.
/// Uses the current user palette, if needed.
/// Inlined, because it is used millions of times when creating a colorized game theme.
inline void GdColor::get_rgb(unsigned char & r, unsigned char & g, unsigned char & b) const {
    if (type != TypeRGB)
        to_rgb().get_rgb(r, g, b);
    else {
        r = rgb.r;
        g = rgb.g;
        b = rgb.b;
    }
}


/// Get h, s, v (0..359, 0..99, 0..99) component of color.
/// Inlined, because it is used millions of times when creating a colorized game theme.
inline void GdColor::get_hsv(unsigned short & h, unsigned char & s, unsigned char & v) const {
    if (type != TypeHSV)
        to_hsv().get_hsv(h, s, v);
    else {
        h = hsv.h;
        s = hsv.s;
        v = hsv.v;
    }
}


/// Create a color from a given r, g, b value.
/// Inlined, because it is used millions of times when creating a colorized game theme.
/// @param r Red value 0..255.
/// @param g Green value 0..255.
/// @param b Blue value 0..255.
inline GdColor GdColor::from_rgb(unsigned r, unsigned g, unsigned b) {
    return GdColor(r, g, b);
}


/// Make up GdColor from h,s,v values.
/// Inlined, because it is used millions of times when creating a colorized game theme.
/// @param h Hue, 0..360
/// @param s Saturation, 0..100
/// @param v Value, 0..100
inline GdColor GdColor::from_hsv(unsigned short h, unsigned char s, unsigned char v) {
    GdColor n;
    n.type = TypeHSV;
    n.hsv.h = h;
    n.hsv.s = s;
    n.hsv.v = v;
    return n;
}


/* i/o operators and functions like for all other cave types */
std::ostream &operator<<(std::ostream &os, const GdColor &c);
std::string visible_name(const GdColor &c);
bool read_from_string(std::string const &s, GdColor &c);


/// Traditional C64 color indexes, plus one GDash special color.
enum GdColorIndex {
    GD_COLOR_INDEX_BLACK = 0,
    GD_COLOR_INDEX_WHITE = 1,
    GD_COLOR_INDEX_RED = 2,
    GD_COLOR_INDEX_PURPLE = 4,
    GD_COLOR_INDEX_CYAN = 3,
    GD_COLOR_INDEX_GREEN = 5,
    GD_COLOR_INDEX_BLUE = 6,
    GD_COLOR_INDEX_YELLOW = 7,
    GD_COLOR_INDEX_ORANGE = 8,
    GD_COLOR_INDEX_BROWN = 9,
    GD_COLOR_INDEX_LIGHTRED = 10,
    GD_COLOR_INDEX_GRAY1 = 11,
    GD_COLOR_INDEX_GRAY2 = 12,
    GD_COLOR_INDEX_LIGHTGREEN = 13,
    GD_COLOR_INDEX_LIGHTBLUE = 14,
    GD_COLOR_INDEX_GRAY3 = 15,
    GD_COLOR_INDEX_MIDDLEBLUE = 16,
};
enum GdColorIndexHelper {
    GD_COLOR_SETCOLOR = 31, /* for blittext */
};

// specialized for color codes for font manager
template <>
inline void Printf::feed_one(GdColorIndex const &colorindex) {
    /* +64 is needed so it is a normal ascii char in the encoding, not a string limiter \0
     * or a \n or whatever */
    char s[3] = { GD_COLOR_SETCOLOR, char(colorindex + 64), 0 };
    feed_one(s);
}


#define GD_GDASH_BLACK (GdColor::from_gdash_index(GD_COLOR_INDEX_BLACK))
#define GD_GDASH_WHITE (GdColor::from_gdash_index(GD_COLOR_INDEX_WHITE))
#define GD_GDASH_RED (GdColor::from_gdash_index(GD_COLOR_INDEX_RED))
#define GD_GDASH_PURPLE (GdColor::from_gdash_index(GD_COLOR_INDEX_PURPLE))
#define GD_GDASH_CYAN (GdColor::from_gdash_index(GD_COLOR_INDEX_CYAN))
#define GD_GDASH_GREEN (GdColor::from_gdash_index(GD_COLOR_INDEX_GREEN))
#define GD_GDASH_BLUE (GdColor::from_gdash_index(GD_COLOR_INDEX_BLUE))
#define GD_GDASH_YELLOW (GdColor::from_gdash_index(GD_COLOR_INDEX_YELLOW))
#define GD_GDASH_ORANGE (GdColor::from_gdash_index(GD_COLOR_INDEX_ORANGE))
#define GD_GDASH_BROWN (GdColor::from_gdash_index(GD_COLOR_INDEX_BROWN))
#define GD_GDASH_LIGHTRED (GdColor::from_gdash_index(GD_COLOR_INDEX_LIGHTRED))
#define GD_GDASH_GRAY1 (GdColor::from_gdash_index(GD_COLOR_INDEX_GRAY1))
#define GD_GDASH_GRAY2 (GdColor::from_gdash_index(GD_COLOR_INDEX_GRAY2))
#define GD_GDASH_LIGHTGREEN (GdColor::from_gdash_index(GD_COLOR_INDEX_LIGHTGREEN))
#define GD_GDASH_LIGHTBLUE (GdColor::from_gdash_index(GD_COLOR_INDEX_LIGHTBLUE))
#define GD_GDASH_GRAY3 (GdColor::from_gdash_index(GD_COLOR_INDEX_GRAY3))
#define GD_GDASH_MIDDLEBLUE (GdColor::from_gdash_index(GD_COLOR_INDEX_MIDDLEBLUE))

extern const GdColor gd_flash_color;
extern const GdColor gd_select_color;

#endif

