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
#include <set>

#include "editor/exportcrli.hpp"

#include "cave/cavetypes.hpp"
#include "cave/cavestored.hpp"
#include "cave/caverendered.hpp"
#include "fileops/c64import.hpp"
#include "misc/logger.hpp"
#include "cave/elementproperties.hpp"


static int crli_steel_code = 0x38;  /* magic value: the code of steel wall in crli */

static int element_to_crli(GdElementEnum e, std::set<GdElementEnum>& unknown) {
    int code = -1;

    /* hack: there is no separate horizontal and vertical growing wall in crli. */
    /* only the switch can determine the direction. */
    if (e == O_V_EXPANDING_WALL)
        e = O_H_EXPANDING_WALL;

    /* 128 is the number of elements in import_table_crli */
    /// @todo fixed size for array
    for (int i = 0; i < 128; i++)
        if (C64Import::import_table_crli[i] == e) {
            code = i;
            break;  /* if found, exit loop */
        }

    // add some elements from CrDr engine
    // http://www.gratissaugen.de/erbsen/BD-Inside-FAQ.html#Objects
    if (code == -1) {
       switch (e) {
       case O_TELEPORTER:
          code = 0x95;
          gd_warning("using CrDr element: '%s -> 0x%x", visible_name(e), code);
          break;
       default:
          break;
       }
    }

    if (code == -1) {
        if (unknown.count(e) == 0) {
            gd_warning("the element '%s' can not be saved in crli, saving steel wall instead", visible_name(e));
            unknown.insert(e);
        }

        code = crli_steel_code;
    }

    return code;
}

/**
 * Convert a probability to c64 representation.
 *
 * Probability values that could be exactly expressed using
 * the c64 6-bit scheme:
 * 1000000  -> 0x03
 *  500000  -> 0x07
 *  250000  -> 0x0f
 *  125000  -> 0x1f
 *   62500  -> 0x3f
 *   31250  -> 0x7f
 *   15625  -> 0xff
 *
 * Also check C64Import.
 */
static unsigned char amoeba_probability(GdProbability input) {
    // these values can be converted exactly.
    switch (int(input)) {
        case 1000000:
            return 0x03;
        case  500000:
            return 0x07;
        case  250000:
            return 0x0f;
        case  125000:
            return 0x1f;
        case   62500:
            return 0x3f;
        case   31250:
            return 0x7f;
        case   15625:
            return 0xff;
    }
    // otherwise, report and convert non-exactly
    gd_warning("Cannot convert amoeba probability %g%% exactly", visible_name(input));
    /*
     Give an approximation. The scale could be logarithmic, but who cares.
        1000000: 0x03;
        *750000
         500000: 0x07;
        *375000
         250000: 0x0f;
        *187500
         125000: 0x1f;
        * 93750
          62500: 0x3f;
        * 46875
          31250: 0x7f;
        * 23438
          15625: 0xff;
     */

    if (input > 750000) return 0x03;
    if (input > 375000) return 0x07;
    if (input > 187500) return 0x0f;
    if (input > 93750) return 0x1f;
    if (input > 46875) return 0x3f;
    if (input > 23438) return 0x7f;
    return 0xff;
}


static int crli_export(CaveStored const &to_convert, const int level, guint8 *compressed) {
    guint8 output[0x3b0];
    /* for rle */
    std::set<GdElementEnum> unknown;        /* hash table to help reporting non-convertable elements only once */

    /* render cave with seed=0 */
    CaveRendered cave(to_convert, level, 0);
    SetLoggerContextForFunction scf(to_convert.name);

    /* do some checks */
    if (!to_convert.lineshift)
        gd_warning("crli only supports line shifting map wraparound");
    if (!to_convert.pal_timing || to_convert.scheduling != GD_SCHEDULING_PLCK)
        gd_warning("applicable timing settings for crli are pal timing=true, scheduling=plck");
    if (to_convert.amoeba_timer_started_immediately)
        gd_warning("crli amoeba timer is only started when the amoeba is let free!");
    if (!to_convert.amoeba_timer_wait_for_hatching)
        gd_message("crli amoeba timer waits for hatching");
    if (!to_convert.voodoo_dies_by_stone || !to_convert.voodoo_collects_diamonds || to_convert.voodoo_disappear_in_explosion)
        gd_warning("crli voodoo dies by stone hit, can collect diamonds and can't be destroyed");
    if (to_convert.short_explosions)
        gd_warning("crli explosions are slower than original");
    if (!to_convert.magic_timer_wait_for_hatching)
        gd_message("crli magic wall timer waits for hatching");
    if (!to_convert.slime_predictable)
        gd_message("crli only supports predictable slime");

    /* fill data bytes with some defaults */
    g_assert(C64Import::import_table_crli[crli_steel_code] == O_STEEL); /* check magic value */
    for (unsigned i = 0; i < 40 * 22; i++) /* fill map with steel wall */
        output[i] = crli_steel_code;
    for (unsigned i = 40 * 22; i < G_N_ELEMENTS(output); i++) /* fill properties with zero */
        output[i] = 0;

    /* check cave sizes */
    if (to_convert.w != 40 || to_convert.h != 22)
        gd_critical("cave sizes out of range, should be 40x22 instead of %dx%d", to_convert.w, to_convert.h);
    gd_cave_correct_visible_size(cave);
    if (to_convert.intermission) { /* visible size for intermissions */
        if (to_convert.x1 != 0 || to_convert.y1 != 0 || to_convert.x2 != 19 || to_convert.y2 != 11)
            gd_critical("for intermissions, the upper left 20x12 elements should be visible");
    } else {    /* visible size for normal caves */
        if (to_convert.x1 != 0 || to_convert.y1 != 0 || to_convert.x2 != to_convert.w - 1 || to_convert.y2 != to_convert.h - 1)
            gd_warning("for normal caves, the whole cave should be visible");
    }

    /* convert map */
    bool has_horizontal = false;
    bool has_vertical = false;
    for (int y = 0; y < to_convert.h && y < 22; y++)
        for (int x = 0; x < to_convert.w && x < 40; x++) {
            output[y * 40 + x] = element_to_crli(cave.map(x, y), unknown);

            if (cave.map(x, y) == O_H_EXPANDING_WALL)
                has_horizontal = true;
            if (cave.map(x, y) == O_V_EXPANDING_WALL)
                has_vertical = true;
        }

    output[0x370] = to_convert.level_time[level] / 100 % 10;
    output[0x371] = to_convert.level_time[level] / 10 % 10;
    output[0x372] = to_convert.level_time[level] / 1 % 10;

    output[0x373] = to_convert.level_diamonds[level] / 100 % 10;
    output[0x374] = to_convert.level_diamonds[level] / 10 % 10;
    output[0x375] = to_convert.level_diamonds[level] / 1 % 10;

    output[0x376] = to_convert.extra_diamond_value / 100 % 10;
    output[0x377] = to_convert.extra_diamond_value / 10 % 10;
    output[0x378] = to_convert.extra_diamond_value / 1 % 10;

    output[0x379] = to_convert.diamond_value / 100 % 10;
    output[0x37a] = to_convert.diamond_value / 10 % 10;
    output[0x37b] = to_convert.diamond_value / 1 % 10;

    output[0x37c] = to_convert.level_amoeba_time[level] / 256;
    output[0x37d] = to_convert.level_amoeba_time[level] % 256;

    output[0x37e] = to_convert.level_magic_wall_time[level] / 256;
    output[0x37f] = to_convert.level_magic_wall_time[level] % 256;

    if (to_convert.creatures_direction_auto_change_time) {
        output[0x380] = 1;
        output[0x381] = to_convert.creatures_direction_auto_change_time;
    }

    output[0x382] = amoeba_probability(to_convert.amoeba_growth_prob);
    output[0x383] = amoeba_probability(to_convert.amoeba_fast_growth_prob);

    output[0x384] = to_convert.colorb.get_c64_index();
    output[0x385] = to_convert.color0.get_c64_index();
    output[0x386] = to_convert.color1.get_c64_index();
    output[0x387] = to_convert.color2.get_c64_index();
    int x = to_convert.color3.get_c64_index();
    if (x > 7)
        gd_message("allowed colors for color3 are Black..Yellow, but it is %d", x);
    output[0x388] = x | 8;

    output[0x389] = to_convert.intermission ? 1 : 0;
    output[0x38a] = to_convert.level_ckdelay[level];

    output[0x38b] = to_convert.level_slime_permeability_c64[level];
    output[0x38c] = 0;  /* always "normal" intermission, scrolling intermission is said to be buggy */
    output[0x38d] = 0xf1; /* magic wall sound on */

    /* if changed direction, we swap the flags here. */
    /* effects are already converted by element_to_crli */
    if (to_convert.expanding_wall_changed)
        std::swap(has_horizontal, has_vertical);
    output[0x38e] = 0x2e;
    if (has_vertical && !has_horizontal)
        output[0x38e] = 0x2f;
    if (has_horizontal && has_vertical)
        gd_warning("a crli map cannot contain horizontal and vertical growing walls at the same time");
    output[0x38f] = to_convert.creatures_backwards ? 0x2d : 0x2c;

    output[0x390] = to_convert.level_amoeba_threshold[level] / 256;
    output[0x391] = to_convert.level_amoeba_threshold[level] % 256;

    output[0x392] = to_convert.level_bonus_time[level];
    output[0x393] = to_convert.level_penalty_time[level];
    output[0x394] = to_convert.biter_delay_frame;
    output[0x395] = to_convert.magic_wall_stops_amoeba ? 0 : 1; /* inverted! */

    output[0x396] = element_to_crli(scanned_pair(to_convert.bomb_explosion_effect), unknown);
    output[0x397] = element_to_crli(scanned_pair(to_convert.explosion_3_effect), unknown);
    if (to_convert.stone_falling_effect != O_STONE_F)
        gd_warning("crli does not support 'falling stone to' effect");
    output[0x398] = element_to_crli(scanned_pair(to_convert.stone_bouncing_effect), unknown);
    output[0x399] = element_to_crli(scanned_pair(to_convert.diamond_birth_effect), unknown);
    output[0x39a] = element_to_crli(scanned_pair(to_convert.magic_diamond_to), unknown);
    if (to_convert.diamond_bouncing_effect != O_DIAMOND)
        gd_warning("crli does not support 'bouncing diamond turns to' effect");
    output[0x39b] = element_to_crli(to_convert.bladder_converts_by, unknown);
    output[0x39c] = element_to_crli(scanned_pair(to_convert.diamond_falling_effect), unknown);
    if (to_convert.diamond_bouncing_effect != O_DIAMOND)
        gd_warning("crli does not support 'bouncing diamond turns to' effect");
    output[0x39d] = element_to_crli(to_convert.biter_eat, unknown);
    output[0x39e] = element_to_crli(to_convert.slime_eats_1, unknown);
    if (element_to_crli(to_convert.slime_eats_1, unknown) + 3 != element_to_crli(scanned_pair(to_convert.slime_converts_1), unknown))
        gd_warning("cannot convert slime setting: %s to %s", visible_name(to_convert.slime_eats_1), visible_name(to_convert.slime_converts_1));
    output[0x39f] = element_to_crli(to_convert.slime_eats_2, unknown);
    if (element_to_crli(to_convert.slime_eats_2, unknown) + 3 != element_to_crli(scanned_pair(to_convert.slime_converts_2), unknown))
        gd_warning("cannot convert slime setting: %s to %s", visible_name(to_convert.slime_eats_2), visible_name(to_convert.slime_converts_2));

    output[0x3a0] = 'V'; /* version number */
    output[0x3a1] = '3';
    output[0x3a2] = '.';
    output[0x3a3] = '0';

    output[0x3a4] = to_convert.diagonal_movements ? 1 : 0;
    output[0x3a6] = element_to_crli(scanned_pair(to_convert.amoeba_too_big_effect), unknown);
    output[0x3a7] = element_to_crli(scanned_pair(to_convert.amoeba_enclosed_effect), unknown);
    output[0x3a8] = to_convert.acid_spread_ratio * 255.0;
    output[0x3a9] = element_to_crli(to_convert.acid_eats_this, unknown);
    output[0x3ab] = element_to_crli(to_convert.expanding_wall_looks_like, unknown);
    output[0x3ac] = element_to_crli(to_convert.dirt_looks_like, unknown);

    guint8 prev = output[0];
    int out = 0;
    int count = 1;
    unsigned i = 1;     // because output[0] is already in prev
    while (i < G_N_ELEMENTS(output)) {
        if (output[i] == prev) {        /* same as previous */
            /* if it would be too much for the length to be fit in a byte, write now */
            if (count > 253) {
                compressed[out++] = 0xbf;
                compressed[out++] = prev; /* which is == 0xbf */
                compressed[out++] = count;

                count = 0;
            }
            count++;
        } else {
            /* not the same as the previous ones, so write out those */
            if (count == 1 && prev != 0xbf) /* output previous character */
                compressed[out++] = prev;
            else if (count == 1 && prev == 0xbf) { /* output previous character, but it is accidentally the escape code */
                compressed[out++] = 0xbf;
                compressed[out++] = prev; /* which is == 0xbf */
                compressed[out++] = 1;
            } else if (count == 2) {
                /* count=2 is not written as escape, byte, count, as it would make it longer */
                if (prev != 0xbf) {
                    /* if the byte to write is not the escape byte */
                    compressed[out++] = prev; /* which is == 0xbf */
                    compressed[out++] = prev; /* which is == 0xbf */
                } else {
                    /* we have two escape bytes */
                    compressed[out++] = 0xbf;
                    compressed[out++] = prev; /* which is == 0xbf */
                    compressed[out++] = 2;
                }
            } else {
                /* count > 2 */
                compressed[out++] = 0xbf;
                compressed[out++] = prev;
                compressed[out++] = count;
            }

            /* and process this one. */
            prev = output[i];
            count = 1;
        }

        i++;    /* next byte to compress */
    }
    /* process remaining bytes; always write them as compressed with escape byte */
    compressed[out++] = 0xbf;
    compressed[out++] = prev;
    compressed[out++] = count;

    /* return number of bytes */
    return out;
}

void
gd_export_cave_to_crli_cavefile(CaveStored const &cave, int level, const char *filename) {
    guint8 data[1024];
    int size;
    GError *error = NULL;

    data[0x0] = 0x00;
    data[0x1] = 0xc4;
    data[0x2] = 'D';
    data[0x3] = 'L';
    data[0x4] = 'P';
    size = crli_export(cave, level, data + 5);
    if (!g_file_set_contents(filename, (gchar *)data, size + 5, &error)) {
        /* could not save properly */
        gd_warning("%s: %s", filename, error->message);
        g_error_free(error);
    }
}


void gd_export_caves_to_crli_cavepack(const std::vector<CaveStored> &caves, int level, const char *filename) {
    GError *error = NULL;
    const int start = 0x6ffa;
    guint8 out[0xcc00 - start + 1024]; /* max number of cavepack + 1024 so dont worry about buffer overrun :P */
    int pos;
    int i;

    /* start address */
    out[0] = 0xfc;
    out[1] = 0x6f;
    /* cavepack version number */
    out[0x6ffc - start] = 'V';
    out[0x6ffd - start] = '3';
    out[0x6ffe - start] = '.';
    out[0x6fff - start] = '0';

    for (i = 0; i < 48; i++) {
        out[0x7000 - start + i] = 0xff;
        out[0x7030 - start + i] = 0xff;
        out[0x7060 - start + i] = 0xff; /* no cave present */
    }
    out[0x7090 - start] = 0xff;
    out[0x7091 - start] = 0x60;
    out[0x7092 - start] = 0x60;
    out[0x7093 - start] = 0x60;
    pos = 0x70a2; /* first available byte; before that we have space for the name */

    for (unsigned n = 0; n < caves.size(); ++n) {
        if (n >= 48) {
            gd_critical("maximum of 48 caves in a crli cavepack");
            break;
        }

        CaveStored const &cave = caves[n];
        int bytes = crli_export(cave, level, out + pos - start);

        if (pos + bytes > 0xcbff) {
            gd_critical("run out of data space, not writing this cave");
            break;
        }

        out[0x7000 - start + i] = pos % 256; /* lsb */
        out[0x7030 - start + i] = pos / 256; /* msb */
        out[0x7060 - start + i] = cave.selectable ? 0 : 1; /* selection table (inverted!) */

        /* write name */
        for (int c = 0; c < 14; c++) /* fill with space */
            out[pos - start - 14 + c] = 0x20;

        gboolean exportname = TRUE;
        const char *namepos = cave.name.c_str();
        int c = 0;
        gunichar ch = g_utf8_get_char(namepos);
        while (c < 14 && ch != 0) {
            bool succ = false;

            out[pos - start - 14 + c] = ' '; /* space is default, also for unknown characters */
            if (ch == ' ')      /* treat space as different, as bd_internal_character_encoding has lots of spaces at unknown character positions */
                succ = true;
            else if (ch < 256) {
                for (int j = 0; C64Import::bd_internal_character_encoding[j] != 0; j++)
                    if (C64Import::bd_internal_character_encoding[j] == g_ascii_toupper(ch)) { /* search for the character */
                        out[pos - start - 14 + c] = j;
                        succ = true;
                    }
            }

            if (!succ)
                exportname = false;

            c++;
            namepos = g_utf8_next_char(namepos);
            ch = g_utf8_get_char(namepos);
        }
        if (!exportname)
            gd_message("couldn't export cave name properly");

        pos += bytes + 14; /* jump number of bytes + place for next cave name */
    }
    pos -= 14;  /* subtract 14 for place we left for next cave name, but there was no more cave */

    if (!g_file_set_contents(filename, (gchar *)out, pos - start, &error)) {
        /* could not save properly */
        gd_critical("%s: %s", filename, error->message);
        g_error_free(error);
    }
}

