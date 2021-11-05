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

#include "fileops/brcimport.hpp"
#include "cave/cavetypes.hpp"
#include "cave/colors.hpp"
#include "cave/cavestored.hpp"
#include "cave/caveset.hpp"
#include "misc/logger.hpp"
#include "cave/elementproperties.hpp"

static GdElementEnum brc_import_table[] = {
    /* 0 */
    O_SPACE, O_DIRT, O_BRICK, O_MAGIC_WALL, O_PRE_OUTBOX, O_OUTBOX, O_UNKNOWN, O_STEEL,
    O_H_EXPANDING_WALL, O_H_EXPANDING_WALL_scanned, O_FIREFLY_1_scanned, O_FIREFLY_1_scanned, O_FIREFLY_1, O_FIREFLY_2, O_FIREFLY_3, O_FIREFLY_4,
    /* 1 */
    O_BUTTER_1_scanned, O_BUTTER_1_scanned, O_BUTTER_1, O_BUTTER_2, O_BUTTER_3, O_BUTTER_4, O_PLAYER, O_PLAYER_scanned,
    O_STONE, O_STONE_scanned, O_STONE_F, O_STONE_F_scanned, O_DIAMOND, O_DIAMOND_scanned, O_DIAMOND_F, O_DIAMOND_F_scanned,
    /* 2 */
    O_NONE /* WILL_EXPLODE_THING */, O_EXPLODE_1, O_EXPLODE_2, O_EXPLODE_3, O_EXPLODE_4, O_EXPLODE_5, O_NONE /* WILL EXPLODE TO DIAMOND_THING */, O_PRE_DIA_1,
    O_PRE_DIA_2, O_PRE_DIA_3, O_PRE_DIA_4, O_PRE_DIA_5, O_AMOEBA, O_AMOEBA_scanned, O_SLIME, O_NONE,
    /* 3 */
    O_CLOCK, O_NONE /* clock eaten */, O_INBOX, O_PRE_PL_1, O_PRE_PL_2, O_PRE_PL_3, O_NONE, O_NONE,
    O_NONE, O_NONE, O_V_EXPANDING_WALL, O_NONE, O_VOODOO, O_UNKNOWN, O_EXPANDING_WALL, O_EXPANDING_WALL_scanned,
    /* 4 */
    O_FALLING_WALL, O_FALLING_WALL_F, O_FALLING_WALL_F_scanned, O_UNKNOWN, O_ACID, O_ACID_scanned, O_NITRO_PACK, O_NITRO_PACK_scanned,
    O_NITRO_PACK_F, O_NITRO_PACK_F_scanned, O_NONE, O_NONE, O_NONE, O_NONE, O_NONE, O_NONE,
    /* 5 */
    O_NONE /* bomb explosion utolso */, O_UNKNOWN, O_NONE /* solid bomb glued */, O_UNKNOWN, O_STONE_GLUED, O_UNKNOWN, O_DIAMOND_GLUED, O_UNKNOWN,
    O_UNKNOWN, O_UNKNOWN, O_NONE, O_NONE, O_NONE, O_NONE, O_NONE, O_NONE,
    /* 6 */
    O_ALT_FIREFLY_1_scanned, O_ALT_FIREFLY_1_scanned, O_ALT_FIREFLY_1, O_ALT_FIREFLY_2, O_ALT_FIREFLY_3, O_ALT_FIREFLY_4, O_PLAYER_BOMB, O_PLAYER_BOMB_scanned,
    O_BOMB, O_BOMB_TICK_1, O_BOMB_TICK_2, O_BOMB_TICK_3, O_BOMB_TICK_4, O_BOMB_TICK_5, O_BOMB_TICK_6, O_BOMB_TICK_7,
    /* 7 */
    O_BOMB_TICK_7, O_BOMB_EXPL_1, O_BOMB_EXPL_2, O_BOMB_EXPL_3, O_BOMB_EXPL_4, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
    O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
};

static GdElementEnum brc_effect_table[] = {
    O_STEEL, O_DIRT, O_SPACE, O_STONE, O_STONE_F, O_STONE_GLUED, O_DIAMOND, O_DIAMOND_F, O_DIAMOND_GLUED, O_PRE_DIA_1,
    O_PLAYER, O_PRE_PL_1, O_PLAYER_BOMB, O_PRE_OUTBOX, O_OUTBOX, O_FIREFLY_1, O_FIREFLY_2, O_FIREFLY_3, O_FIREFLY_4,
    O_BUTTER_1, O_BUTTER_2, O_BUTTER_3, O_BUTTER_4, O_BRICK, O_MAGIC_WALL, O_H_EXPANDING_WALL, O_V_EXPANDING_WALL, O_EXPANDING_WALL,
    O_FALLING_WALL, O_FALLING_WALL_F, O_AMOEBA, O_SLIME, O_ACID, O_VOODOO, O_CLOCK, O_BOMB, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
    O_ALT_FIREFLY_1, O_ALT_FIREFLY_2, O_ALT_FIREFLY_3, O_ALT_FIREFLY_4, O_ALT_BUTTER_1, O_ALT_BUTTER_2, O_ALT_BUTTER_3, O_ALT_BUTTER_4,
    O_EXPLODE_1, O_BOMB_EXPL_1, O_UNKNOWN,
};

static double brc_hue_table[] = { 92, 211, 29, 346, 263, 147, 317, 10, 64 };

static GdElementEnum brc_effect(unsigned char byt) {
    if (byt >= G_N_ELEMENTS(brc_effect_table)) {
        gd_warning("invalid element identifier for brc effect: %02x", unsigned(byt));
        return O_UNKNOWN;
    }

    return brc_effect_table[byt];
}

GdElementEnum brc_import_elem(unsigned char c) {
    if (c >= G_N_ELEMENTS(brc_import_table)) {
        gd_warning("invalid brc element byte %x", unsigned(c));
        return O_UNKNOWN;
    }
    return nonscanned_pair(brc_import_table[c]);
}


void brc_import(CaveSet &caveset, guint8 *data) {
    /* we import 100 caves, and the put them in the correct order. */
    CaveStored imported[100];
    bool import_effect;

    /* this is some kind of a version number */
    import_effect = false;
    switch (data[23]) {
        case 0x0:
            /* nothing to do */
            break;
        case 0xde:
            /* import effects */
            import_effect = true;
            break;
        default:
            gd_warning("unknown brc version %02x", unsigned(data[23]));
            break;
    }

    for (int level = 0; level < 5; level++) {
        for (int cavenum = 0; cavenum < 20; cavenum++) {
            CaveStored &cave = imported[level * 20 + cavenum];
            char s[128];

            int c = 5 * 20 * 24; /* 5 levels, 20 caves, 24 bytes - max 40*2 properties for each cave */
            int datapos = (cavenum * 5 + level) * 24 + 22;
            int colind;

            if (cavenum < 16)
                g_snprintf(s, sizeof(s), "Cave %c/%d", 'A' + cavenum, level + 1);
            else
                g_snprintf(s, sizeof(s), "Intermission %d/%d", cavenum - 15, level + 1);
            cave.name = s;

            /* fixed intermission caves; are smaller. */
            if (cavenum >= 16) {
                cave.w = 20;
                cave.h = 12;
            }
            cave.map.set_size(cave.w, cave.h);

            for (int y = 0; y < cave.h; y++) {
                for (int x = 0; x < cave.w; x++) {
                    guint8 import = data[y + level * 24 + cavenum * 24 * 5 + x * 24 * 5 * 20];
                    cave.map(x, y) = brc_import_elem(import);
                }
            }

            for (int i = 0; i < 5; i++) {
                cave.level_time[i] = data[0 * c + datapos];
                cave.level_diamonds[i] = data[1 * c + datapos];
                cave.level_magic_wall_time[i] = data[4 * c + datapos];
                cave.level_amoeba_time[i] = data[5 * c + datapos];
                cave.level_amoeba_threshold[i] = data[6 * c + datapos];
                /* bonus time: 100 was added, so it could also be negative */
                cave.level_bonus_time[i] = (int)data[11 * c + datapos + 1] - 100;
                cave.level_hatching_delay_frame[i] = data[10 * c + datapos];
                cave.level_slime_permeability[i] = 1.0 / data[9 * c + datapos];

                /* this was not set in boulder remake. */
                cave.level_speed[i] = 150;
            }
            cave.diamond_value = data[2 * c + datapos];
            cave.extra_diamond_value = data[3 * c + datapos];
            /* BRC PROBABILITIES */
            /* a typical code example:
                   46:if (random(slime*4)<4) and (tab[x,y+2]=0) then
                      Begin tab[x,y]:=0;col[x,y+2]:=col[x,y];tab[x,y+2]:=27;mat[x,y+2]:=9;Voice4:=2;end;
               where slime is the byte loaded from the file as it is.
               pascal random function generates a random number between 0..limit-1, inclusive, for random(limit).

               so a random number between 0..limit*4-1 is generated.
               for limit=1, 0..3, which is always < 4, so P=1.
               for limit=2, 0..7, 0..7 is < 4 in P=50%.
               for limit=3, 0..11, is < 4 in P=33%.
               So the probability is exactly 100%/limit.
               just make sure we do not divide by zero for some broken input.
            */
            if (data[7 * c + datapos] != 0)
                cave.amoeba_growth_prob = 1.0 / data[7 * c + datapos];
            else
                gd_warning("amoeba growth cannot be zero, error at byte %d", unsigned(data[7 * c + datapos]));
            if (data[8 * c + datapos] != 0)
                cave.amoeba_fast_growth_prob = 1.0 / data[8 * c + datapos];
            else
                gd_warning("amoeba growth cannot be zero, error at byte %d", unsigned(data[8 * c + datapos]));
            cave.slime_predictable = false;
            cave.acid_spread_ratio = 1.0 / data[10 * c + datapos];
            cave.pushing_stone_prob = 1.0 / data[11 * c + datapos]; /* br only allowed values 1..8 in here, but works the same way. */
            cave.magic_wall_stops_amoeba = data[12 * c + datapos + 1] != 0;
            cave.intermission = cavenum >= 16 || data[14 * c + datapos + 1] != 0;

            /* colors */
            colind = data[31 * c + datapos] % G_N_ELEMENTS(brc_hue_table);
            cave.colorb = GdColor::from_rgb(0, 0, 0);  /* fixed rgb black */
            cave.color0 = GdColor::from_rgb(0, 0, 0);  /* fixed rgb black */
            cave.color1 = GdColor::from_hsv(brc_hue_table[colind], 70, 70); /* brc specified dirt color */
            cave.color2 = GdColor::from_hsv(brc_hue_table[colind] + 120, 85, 80);
            cave.color3 = GdColor::from_hsv(brc_hue_table[colind] + 240, 15, 90); /* almost white for brick */
            cave.color4 = GdColor::from_hsv(120, 90, 90);  /* fixed for amoeba */
            cave.color5 = GdColor::from_hsv(240, 90, 90);  /* fixed for slime */

            if (import_effect) {
                cave.amoeba_enclosed_effect = brc_effect(data[14 * c + datapos + 1]);
                cave.amoeba_too_big_effect = brc_effect(data[15 * c + datapos + 1]);
                cave.explosion_effect = brc_effect(data[16 * c + datapos + 1]);
                cave.bomb_explosion_effect = brc_effect(data[17 * c + datapos + 1]);
                /* 18 solid bomb explode to */
                cave.diamond_birth_effect = brc_effect(data[19 * c + datapos + 1]);
                cave.stone_bouncing_effect = brc_effect(data[20 * c + datapos + 1]);
                cave.diamond_bouncing_effect = brc_effect(data[21 * c + datapos + 1]);
                cave.magic_diamond_to = brc_effect(data[22 * c + datapos + 1]);
                cave.acid_eats_this = brc_effect(data[23 * c + datapos + 1]);
                /* slime eats: (diamond,boulder,bomb), (diamond,boulder), (diamond,bomb), (boulder,bomb) */
                cave.amoeba_enclosed_effect = brc_effect(data[14 * c + datapos + 1]);
            }
        }
    }

    /* put them in the caveset - take correct order into consideration. */
    for (int level = 0; level < 5; level++) {
        for (int cavenum = 0; cavenum < 20; cavenum++) {
            static const int reorder[] = {0, 1, 2, 3, 16, 4, 5, 6, 7, 17, 8, 9, 10, 11, 18, 12, 13, 14, 15, 19};
            int i = level * 20 + reorder[cavenum];
            CaveStored &cave = imported[i];

            /* check if cave contains only dirt. that is an empty cave, and do not import. */
            bool only_dirt = true;
            for (int y = 1; y < cave.h - 1 && only_dirt; y++)
                for (int x = 1; x < cave.w - 1 && only_dirt; x++)
                    if (cave.map(x, y) != O_DIRT)
                        only_dirt = false;

            /* append to caveset if not empty */
            if (!only_dirt)
                caveset.caves.push_back(std::move(cave));
        }
    }
}

