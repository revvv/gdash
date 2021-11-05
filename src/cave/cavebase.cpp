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

#include <glib.h>
#include "cave/cavebase.hpp"

/**
  Select random colors for a given cave.
  This function will select colors so that they should look somewhat nice; for example
  brick walls won't be the darkest color.
*/
void gd_cave_set_random_c64_colors(CaveBase &cave) {
    const int bright_colors[] = {1, 3, 7};
    const int dark_colors[] = {2, 6, 8, 9, 11};

    /* always black */
    cave.colorb = GdColor::from_c64(0);
    cave.color0 = GdColor::from_c64(0);
    /* choose some bright color for brick */
    cave.color3 = GdColor::from_c64(bright_colors[g_random_int_range(0, G_N_ELEMENTS(bright_colors))]);
    /* choose a dark color for dirt, but should not be == color of brick */
    do {
        cave.color1 = GdColor::from_c64(dark_colors[g_random_int_range(0, G_N_ELEMENTS(dark_colors))]);
    } while (cave.color1 == cave.color3); /* so it is not the same as color 1 */
    /* choose any but black for steel wall, but should not be == brick or dirt */
    do {
        /* between 1 and 15 - do not use black for this. */
        cave.color2 = GdColor::from_c64(g_random_int_range(1, 16));
    } while (cave.color1 == cave.color2 || cave.color2 == cave.color3); /* so colors are not the same */
    /* copy amoeba and slime color */
    cave.color4 = cave.color3;
    cave.color5 = cave.color1;
}

static void
cave_set_random_indexed_colors(CaveBase &cave, GdColor(*color_indexer_func)(unsigned, unsigned)) {
    int hue = g_random_int_range(0, 15);
    int hue_spread = g_random_int_range(1, 6); /* 1..5 */
    /* we only use 0..6, as saturation 15 is too bright (almost always white) */
    /* also, saturation 0..1..2 is too dark. the color0=black is there for dark. */
    int bri_spread = 6 - hue_spread; /* so this is also 1..5. when hue spread is low, brightness spread is high */
    int bri1 = 8, bri2 = 8 - bri_spread, bri3 = 8 + bri_spread;
    /* there are 15 valid choices for hue, so we do a %15 */
    int col1 = hue, col2 = (hue + hue_spread + 15) % 15, col3 = (hue - hue_spread + 15) % 15;

    /* this makes up a random color, and selects a color triad by hue+5 and hue+10. */
    /* also creates a random saturation. */
    /* color of brick is 8+sat, so it is always a bright color. */
    /* another two are 8-sat and 8. */
    /* order of colors is also changed randomly. */

    if (g_random_boolean())
        std::swap(bri1, bri2);
    /* we do not touch bri3 (8+sat), as it should be a bright color */
    if (g_random_boolean())
        std::swap(col1, col2);
    if (g_random_boolean())
        std::swap(col2, col3);
    if (g_random_boolean())
        std::swap(col1, col3);

    cave.colorb = color_indexer_func(0, 0);
    cave.color0 = color_indexer_func(0, 0);
    cave.color1 = color_indexer_func(col1 + 1, bri1);
    cave.color2 = color_indexer_func(col2 + 1, bri2);
    cave.color3 = color_indexer_func(col3 + 1, bri3);
    /* amoeba and slime are different */
    cave.color4 = color_indexer_func(g_random_int_range(11, 13), g_random_int_range(6, 12)); /* some green thing */
    cave.color5 = color_indexer_func(g_random_int_range(7, 10), g_random_int_range(0, 6));  /* some blueish thing */
}

void
gd_cave_set_random_atari_colors(CaveBase &cave) {
    cave_set_random_indexed_colors(cave, &GdColor::from_atari_huesat);
}

void
gd_cave_set_random_c64dtv_colors(CaveBase &cave) {
    cave_set_random_indexed_colors(cave, &GdColor::from_c64dtv_huesat);
}

void
gd_cave_set_random_rgb_colors(CaveBase &cave) {
    const double hue_max = 10.0 / 30.0;
    double hue = g_random_int_range(0, 1000000) / 1000000.0; /* any hue allowed */
    double hue_spread = g_random_int_range(1000000.0 * 2.0 / 30.0, 1000000.0 * hue_max) / 1000000.0; /* hue 360 degress=1.  hue spread is min. 24 degrees, max 120 degrees (1/3) */
    double h1 = hue, h2 = hue + hue_spread, h3 = hue + 2 * hue_spread;
    double v1, v2, v3;
    double s1, s2, s3;

    if (g_random_boolean()) {
        /* when hue spread is low, brightness(saturation) spread is high */
        /* this formula gives a number (x) between 0.1 and 0.4, which will be 0.5-x and 0.5+x, so the range is 0.1->0.9 */
        double spread = 0.1 + 0.3 * (1 - hue_spread / hue_max);
        v1 = 0.6;           /* brightness variation, too */
        v2 = 0.7;
        v3 = 0.8;
        s1 = 0.5;           /* saturation is different */
        s2 = 0.5 - spread;
        s3 = 0.5 + spread;
    } else {
        /* when hue spread is low, brightness(saturation) spread is high */
        /* this formula gives a number (x) between 0.1 and 0.25, which will be 0.5+x and 0.5+2x, so the range is 0.5->0.9 */
        double spread = 0.1 + 0.15 * (1 - hue_spread / hue_max);
        v1 = 0.5;           /* brightness is different */
        v2 = 0.5 + spread;
        v3 = 0.5 + 2 * spread;
        s1 = 0.7;           /* saturation is same - a not fully saturated one */
        s2 = 0.8;
        s3 = 0.9;
    }
    /* randomly change values, but do not touch v3, as cave.color3 should be a bright color */
    if (g_random_boolean()) std::swap(v1, v2);
    /* randomly change hues and saturations */
    if (g_random_boolean()) std::swap(h1, h2);
    if (g_random_boolean()) std::swap(h2, h3);
    if (g_random_boolean()) std::swap(h1, h3);
    if (g_random_boolean()) std::swap(s1, s2);
    if (g_random_boolean()) std::swap(s2, s3);
    if (g_random_boolean()) std::swap(s1, s3);

    h1 *= 360.0;
    s1 *= 100.0;
    v1 *= 100.0;
    h2 *= 360.0;
    s2 *= 100.0;
    v2 *= 100.0;
    h3 *= 360.0;
    s3 *= 100.0;
    v3 *= 100.0;

    cave.colorb = GdColor::from_hsv(0, 0, 0).to_rgb();
    cave.color0 = GdColor::from_hsv(0, 0, 0).to_rgb();     /* black for background */
    cave.color1 = GdColor::from_hsv(h1, s1, v1).to_rgb();  /* dirt */
    cave.color2 = GdColor::from_hsv(h2, s2, v2).to_rgb();  /* steel */
    cave.color3 = GdColor::from_hsv(h3, s3, v3).to_rgb();  /* brick */
    cave.color4 = GdColor::from_hsv(g_random_int_range(100, 140), s2, v2).to_rgb();  /* green(120+-20) with the saturation and brightness of brick */
    cave.color5 = GdColor::from_hsv(g_random_int_range(220, 260), s1, v1).to_rgb();  /* blue(240+-20) with saturation and brightness of dirt */
}


void gd_cave_set_random_colors(CaveBase &cave, GdColor::Type type) {
    switch (type) {
        case GdColor::TypeRGB:
            gd_cave_set_random_rgb_colors(cave);
            break;
        case GdColor::TypeC64:
            gd_cave_set_random_c64_colors(cave);
            break;
        case GdColor::TypeC64DTV:
            gd_cave_set_random_c64dtv_colors(cave);
            break;
        case GdColor::TypeAtari:
            gd_cave_set_random_atari_colors(cave);
            break;
        default:
            g_assert_not_reached();
    }
}


/* check if cave visible part coordinates
   are outside cave sizes, or not in the right order.
   correct them if needed.
*/
void gd_cave_correct_visible_size(CaveBase &cave) {
    /* change visible coordinates if they do not point to upperleft and lowerright */
    if (cave.x2 < cave.x1) {
        int t = cave.x2;
        cave.x2 = cave.x1;
        cave.x1 = t;
    }
    if (cave.y2 < cave.y1) {
        int t = cave.y2;
        cave.y2 = cave.y1;
        cave.y1 = t;
    }
    if (cave.x1 < 0)
        cave.x1 = 0;
    if (cave.y1 < 0)
        cave.y1 = 0;
    if (cave.x2 > cave.w - 1)
        cave.x2 = cave.w - 1;
    if (cave.y2 > cave.h - 1)
        cave.y2 = cave.h - 1;
}


/* from the key press booleans, create a direction */
GdDirectionEnum
gd_direction_from_keypress(bool up, bool down, bool left, bool right) {
    GdDirectionEnum player_move;

    /* from the key press booleans, create a direction */
    if (up && right)
        player_move = MV_UP_RIGHT;
    else if (down && right)
        player_move = MV_DOWN_RIGHT;
    else if (down && left)
        player_move = MV_DOWN_LEFT;
    else if (up && left)
        player_move = MV_UP_LEFT;
    else if (up)
        player_move = MV_UP;
    else if (down)
        player_move = MV_DOWN;
    else if (left)
        player_move = MV_LEFT;
    else if (right)
        player_move = MV_RIGHT;
    else
        player_move = MV_STILL;

    return player_move;
}
