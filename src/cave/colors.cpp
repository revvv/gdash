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

#include <cstdio>
#include <cmath>
#include <glib.h>
#include <glib/gi18n.h>

#include "settings.hpp"
#include "cave/colors.hpp"
#include "misc/logger.hpp"
#include "misc/util.hpp"

/* These are in the include/ directory. */
/* Stores the atari palettes. */
#include "ataripal.cpp"
/* Stores the DTV palettes. */
#include "dtvpal.cpp"

static const char *c64_color_bdcff_names[] = {
    "Black", "White", "Red", "Cyan", "Purple", "Green", "Blue", "Yellow",
    "Orange", "Brown", "LightRed", "Gray1", "Gray2", "LightGreen", "LightBlue", "Gray3",
};

static const char *c64_color_visible_names[] = {
    N_("Black"), N_("White"), N_("Red"), N_("Cyan"), N_("Purple"), N_("Green"), N_("Blue"), N_("Yellow"),
    N_("Orange"), N_("Brown"), N_("Light red"), N_("Dark gray"), N_("Gray"), N_("Light green"), N_("Light blue"), N_("Light gray"),
};

static guint32 c64_colors_vice_old[] = {
    0x000000, 0xFFFFFF, 0x68372b, 0x70a4b2, 0x6f3d86, 0x588d43, 0x352879, 0xb8c76f,
    0x6f4f25, 0x433900, 0x9a6759, 0x444444, 0x6c6c6c, 0x9ad284, 0x6c5eb5, 0x959595,
};

static guint32 c64_colors_vice_new[] = {
    0x000000, 0xffffff, 0x894036, 0x7abfc7, 0x8a46ae, 0x68a941, 0x3e31a2, 0xd0dc71,
    0x905f25, 0x5c4700, 0xbb776d, 0x555555, 0x808080, 0xacea88, 0x7c70da, 0xababab,
};

static guint32 c64_colors_c64_hq[] = {
    0x0A0A0A, 0xFFF8FF, 0x851F02, 0x65CDA8, 0xA73B9F, 0x4DAB19, 0x1A0C92, 0xEBE353,
    0xA94B02, 0x441E00, 0xD28074, 0x464646, 0x8B8B8B, 0x8EF68E, 0x4D91D1, 0xBABABA,
};

static guint32 c64_colors_c64s[] = {
    0x000000, 0xFCFCFC, 0xA80000, 0x54FCFC, 0xA800A8, 0x00A800, 0x0000A8, 0xFCFC00,
    0xA85400, 0x802C00, 0xFC5454, 0x545454, 0x808080, 0x54FC54, 0x5454FC, 0xA8A8A8,
};

static guint32 c64_colors_ccs64[] = {
    0x101010, 0xFFFFFF, 0xE04040, 0x60FFFF, 0xE060E0, 0x40E040, 0x4040E0, 0xFFFF40,
    0xE0A040, 0x9C7448, 0xFFA0A0, 0x545454, 0x888888, 0xA0FFA0, 0xA0A0FF, 0xC0C0C0,
};

static guint32 c64_colors_vice_default[] = {
    0x000000, 0xFDFEFC, 0xBE1A24, 0x30E6C6, 0xB41AE2, 0x1FD21E, 0x211BAE, 0xDFF60A,
    0xB84104, 0x6A3304, 0xFE4A57, 0x424540, 0x70746F, 0x59FE59, 0x5F53FE, 0xA4A7A2,
};

static guint32 c64_colors_frodo[] = {
    0x000000, 0xFFFFFF, 0xCC0000, 0x00FFCC, 0xFF00FF, 0x00CC00, 0x0000CC, 0xFFFF00,
    0xFF8800, 0x884400, 0xFF8888, 0x444444, 0x888888, 0x88FF88, 0x8888FF, 0xCCCCCC,
};

static guint32 c64_colors_godot[] = {
    0x000000, 0xFFFFFF, 0x880000, 0xAAFFEE, 0xCC44CC, 0x00CC55, 0x0000AA, 0xEEEE77,
    0xDD8855, 0x664400, 0xFE7777, 0x333333, 0x777777, 0xAAFF66, 0x0088FF, 0xBBBBBB,
};

static guint32 c64_colors_pc64[] = {
    0x212121, 0xFFFFFF, 0xB52121, 0x73FFFF, 0xB521B5, 0x21B521, 0x2121B5, 0xFFFF21,
    0xB57321, 0x944221, 0xFF7373, 0x737373, 0x949494, 0x73FF73, 0x7373FF, 0xB5B5B5,
};

static guint32 c64_colors_rtadash[] = {
    0x000000, 0xffffff, 0xea3418, 0x58ffff, 0xd82cff, 0x55fb00, 0x4925ff, 0xffff09,
    0xe66c00, 0x935f00, 0xff7c64, 0x6c6c6c, 0xa1a1a1, 0xafff4d, 0x9778ff, 0xd8d8d8,
};


/* these values are taken from the title screen, drawn by cws. */
/* so menus and everything else will look nice! */
/* the 16 colors that can be used are the same as on c64. */
/* "Black", "White", "Red", "Cyan", "Purple", "Green", "Blue", "Yellow", */
/* "Orange", "Brown", "LightRed", "Gray1", "Gray2", "LightGreen", "LightBlue", "Gray3", */
/* not in the png: cyan, purple. gray3 is darker in the png. */
/* 17th color is the player's leg in the png. i not connected it to any c64 */
/* color, but it is used for theme images for example. */
static const guint32 gdash_colors[] = {
    0x000000, 0xffffff, 0xe33939, 0x55aaaa, 0xaa55aa, 0x71aa55, 0x0039ff, 0xffff55,
    0xe37139, 0xaa7139, 0xe09080, 0x555555, 0x717171, 0xc6e38e, 0xaaaaff, 0x8e8e8e,

    0x5555aa,
};


/* make sure that pointeres and names match! */
static guint32 *c64_palette_pointers[] = {
    c64_colors_vice_new,
    c64_colors_vice_old, c64_colors_vice_default, c64_colors_c64_hq, c64_colors_c64s,
    c64_colors_ccs64, c64_colors_frodo, c64_colors_godot, c64_colors_pc64, c64_colors_rtadash,
    NULL
};
static const char *c64_palettes_names[] = {
    "Vice new",
    "Vice old", "Vice default", "C64HQ", "C64S",
    "CCS64", "Frodo", "GoDot", "PC64", "RTADash",
    NULL
};

/* indexes in this array must match GdColorType */
static const char *palette_types_names[] = {
    N_("RGB colors"),
    N_("C64 colors"),
    N_("C64DTV colors"),
    N_("Atari colors"),
    NULL
};


/// Color of flashing the screen, gate opening to exit.
const GdColor gd_flash_color = GdColor::from_rgb(0xff, 0xff, 0xff);
/// Color of selected object in editor.
const GdColor gd_select_color = GdColor::from_rgb(0x80, 0x80, 0xff);


/// Get an array of C strings, which stores the color types known.
const char **GdColor::get_palette_types_names() {
    return palette_types_names;
}

/// Get an array of C strings, which stores the names of C64 palettes known.
const char **GdColor::get_c64_palette_names() {
    return c64_palettes_names;
}

/// Get an array of C strings, which stores the names of C64DTV palettes known.
const char **GdColor::get_c64dtv_palette_names() {
    return c64dtv_palettes_names;
}

/// Get an array of C strings, which stores the names of Atari palettes known.
const char **GdColor::get_atari_palette_names() {
    return atari_palettes_names;
}

/// Constructor to create an RGB color from a 0x00RRGGBB value.
/// Used internally, as palettes can easily be stored this way.
/// @param rgb The 0x00RRGGBB value.
GdColor::GdColor(unsigned int rgbval) {
    g_assert(rgbval < 0x1000000);
    type = TypeRGB;
    rgb.b = (rgbval >> 0) & 0xff;
    rgb.g = (rgbval >> 8) & 0xff;
    rgb.r = (rgbval >> 16) & 0xff;
}

/// Create a GDash color. These are similar to C64 colors,
/// but always use the same palette. Internally, they are
/// RGB colors.
/// @param c Color index, 0..16.
GdColor GdColor::from_gdash_index(unsigned c) {
    g_assert(c >= 0 && c < G_N_ELEMENTS(gdash_colors));
    return GdColor(gdash_colors[c]);
}

/// Create a C64 color.
/// @param index C64 color index, 0..15
GdColor GdColor::from_c64(unsigned index) {
    g_assert(index >= 0 && index <= 15);
    return GdColor(TypeC64, index);
}


/// Create an Atari color.
/// @param index Color index, 0..255
GdColor GdColor::from_atari(unsigned index) {
    g_assert(index >= 0 && index <= 255);
    return GdColor(TypeAtari, index);
}


/// Create an Atari color, hue+sat version.
/// Makes generating random Atari palettes easier.
/// @param hue Hue, 0..15
/// @param sat Saturation, 0..15
GdColor GdColor::from_atari_huesat(unsigned hue, unsigned sat) {
    g_assert(hue >= 0 && hue <= 15);
    g_assert(sat >= 0 && sat <= 15);
    return GdColor(TypeAtari, 16 * hue + sat);
}

/// Create a C64DTV color.
/// @param index Color index, 0..255
GdColor GdColor::from_c64dtv(unsigned index) {
    g_assert(index >= 0 && index <= 255);
    return GdColor(TypeC64DTV, index);
}

/// Create a C64DTV color, hue+sat version.
/// @param hue Hue, 0..15
/// @param sat Saturation, 0..15
GdColor GdColor::from_c64dtv_huesat(unsigned hue, unsigned sat) {
    g_assert(hue >= 0 && hue <= 15);
    g_assert(sat >= 0 && sat <= 15);
    return GdColor(TypeC64DTV, 16 * hue + sat);
}


/// Create a color from a BDCFF description.
/// @param color The string which contains the BDCFF representation.
/// @return The new color object.
bool read_from_string(std::string const &str, GdColor &c) {
    // check if it is a name of a c64 color
    for (unsigned i = 0; i < G_N_ELEMENTS(c64_color_bdcff_names); i++)
        if (gd_str_ascii_caseequal(str, c64_color_bdcff_names[i])) {
            c = GdColor::from_c64(i);
            return true;
        }

    std::string strupper(str);
    for (unsigned i = 0; i < strupper.size(); ++i)
        strupper[i] = toupper(strupper[i]);
    const char *cstr = strupper.c_str();

    int x;
    // check if atari color.
    if (sscanf(cstr, "ATARI%02x", &x) == 1) {
        c = GdColor::from_atari(x);
        return true;
    }

    // check if c64dtv color.
    if (sscanf(cstr, "C64DTV%02x", &x) == 1) {
        c = GdColor::from_c64dtv(x);
        return true;
    }

    // rgb color? may or may not have a #
    if (cstr[0] == '#')
        ++cstr;
    int r, g, b;
    if (sscanf(cstr, "%02x%02x%02x", &r, &g, &b) == 3) {
        c = GdColor::from_rgb(r, g, b);
        return true;
    }

    // could not read in any way
    return false;
}


/// Create a color object, which is RGB internally.
/// Uses the current user preferences for conversion.
/// The color object created should not be saved, but
/// only used on-screen.
/// @param color The color to convert.
GdColor GdColor::to_rgb() const {
    switch (type) {
        case TypeRGB:
            /* is already rgb */
            return *this;

        case TypeC64: {
            if (gd_c64_palette < 0 || gd_c64_palette >= (int)G_N_ELEMENTS(c64_palette_pointers) - 1)
                gd_c64_palette = 0;  /* silently switch to default, if invalid value */
            const guint32 *c64_pal = c64_palette_pointers[gd_c64_palette];
            return GdColor(c64_pal[index]);
        }

        case TypeC64DTV: {
            if (gd_c64dtv_palette < 0 || gd_c64dtv_palette >= (int)G_N_ELEMENTS(c64dtv_palettes_pointers) - 1)
                gd_c64dtv_palette = 0;
            const guint8 *c64dtv_pal = c64dtv_palettes_pointers[gd_c64dtv_palette];
            return from_rgb(c64dtv_pal[index * 3], c64dtv_pal[index * 3 + 1], c64dtv_pal[index * 3 + 2]);
        }

        case TypeAtari: {
            if (gd_atari_palette < 0 || gd_atari_palette >= (int)G_N_ELEMENTS(atari_palettes_pointers) - 1)
                gd_atari_palette = 0;
            const guint8 *atari_pal = atari_palettes_pointers[gd_atari_palette];
            return from_rgb(atari_pal[index * 3], atari_pal[index * 3 + 1], atari_pal[index * 3 + 2]);
        }

        case TypeHSV: {
            double h = hsv.h, s = hsv.s / 100.0, v = hsv.v / 100.0;
            int i = (int)(h / 60) % 6;   /* divided by 60 degrees */
            double f = h / 60 - (int)(h / 60); /* fractional part */
            double p = v * (1 - s);
            double q = v * (1 - s * f);
            double t = v * (1 - s * (1 - f));

            v *= 255.0;
            p *= 255.0;
            q *= 255.0;
            t *= 255.0;

            switch (i) {
                case 0:
                    return GdColor::from_rgb(v, t, p);
                case 1:
                    return GdColor::from_rgb(q, v, p);
                case 2:
                    return GdColor::from_rgb(p, v, t);
                case 3:
                    return GdColor::from_rgb(p, q, v);
                case 4:
                    return GdColor::from_rgb(t, p, v);
                case 5:
                    return GdColor::from_rgb(v, p, q);
            }
            /* no way we reach this */
            g_assert_not_reached();
            return GdColor::from_rgb(0, 0, 0);
        }
        
        case TypeInvalid:
            g_assert_not_reached();
            break;
    }
    g_assert_not_reached();
}


/// Create a color object, which is HSV internally.
GdColor GdColor::to_hsv() const {
    if (type == TypeHSV)
        return *this;

    unsigned char r, g, b;
    get_rgb(r, g, b);
    double R = r / 255.0, G = g / 255.0, B = b / 255.0;
    double M = std::max(std::max(R, G), B);
    double m = std::min(std::min(R, G), B);
    double C = M - m;
    double H, S, V;

    V = M;
    if (V > 0)
        S = C / V;
    else
        S = 0;
    if (R >= M)
        H = (G - B) / C;
    else if (G >= M)
        H = 2 + (B - R) / C;
    else
        H = 4 + (R - G) / C;
    return GdColor::from_hsv(fmod(H * 60.0 + 360, 360), S * 100.0, V * 100.0);
}


/// Get RGB components of color as an unsigned int 0x00RRGGBB
unsigned int GdColor::get_uint_0rgb() const {
    unsigned char r, g, b;
    get_rgb(r, g, b);
    return (r << 16) + (g << 8) + b;
}


/// Standard operator<< to write BDCFF info of the color to an ostream.
std::ostream &operator<<(std::ostream &os, const GdColor &c) {
    char text[32];

    switch (c.type) {
        case GdColor::TypeC64:
            sprintf(text, "%s", c64_color_bdcff_names[c.index]);
            break;

        case GdColor::TypeAtari:
            sprintf(text, "Atari%02x", c.index);
            break;

        case GdColor::TypeC64DTV:
            sprintf(text, "C64DTV%02x", c.index);
            break;

        case GdColor::TypeRGB:
        case GdColor::TypeHSV:
            {
                unsigned char r, g, b;
                c.get_rgb(r, g, b);
                sprintf(text, "#%02x%02x%02x", r, g, b);
            }
            break;
        case GdColor::TypeInvalid:
            g_assert_not_reached();
            break;
    }
    os << text;
    return os;
}

/// Return on-screen visible name of color.
/// Returns strings which are already translated.
/// @todo throw?
std::string visible_name(const GdColor &c) {
    char text[32];

    switch (c.type) {
        case GdColor::TypeC64:
            return _(c64_color_visible_names[c.index]);
            break;

        case GdColor::TypeAtari:
            sprintf(text, "Atari #%02X", c.index);
            return text;

        case GdColor::TypeC64DTV:
            sprintf(text, "C64DTV #%02X", c.index);
            return text;

        case GdColor::TypeRGB:
        case GdColor::TypeHSV:
            {
                unsigned char r, g, b;
                c.get_rgb(r, g, b);
                sprintf(text, "RGB #%02X%02X%02X", r, g, b);
            }
            return text;
        case GdColor::TypeInvalid:
            g_assert_not_reached();
            break;
    }
    /* should not happen */
    return _("Invalid");
}


/// Check if the color is a C64 color.
/// @return True, if C64 color.
bool GdColor::is_c64() const {
    return type == TypeC64;
}

/// Return the "traditional" c64 index of color.
/// If not found, reports the error, and gives a random color.
/// @return The C64 index.
int GdColor::get_c64_index() const {
    if (type == TypeC64)
        return index;
    gd_message("Non-C64 color: %s", visible_name(*this));
    return g_random_int_range(0, 16);
}

/// Compare two color objects for equality.
/// @return True, if they are equal. They must be also of the same type, same rgb values are not enough!
bool GdColor::operator==(const GdColor &rhs) const {
    if (type != rhs.type)
        return false;

    /* for rgb, all must match. */
    if (type == TypeRGB)
        return rgb.r == rhs.rgb.r && rgb.g == rhs.rgb.g && rgb.b == rhs.rgb.b;
    if (type == TypeHSV)
        return hsv.h == rhs.hsv.h && hsv.s == rhs.hsv.s && hsv.v == rhs.hsv.v;

    /* for others, we check only r which is used as an index. */
    return index == rhs.index;
}
