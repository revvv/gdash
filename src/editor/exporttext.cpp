/*
 * Copyright (c) 2007-2020, GDash Project
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
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "editor/exporttext.hpp"
#include "cave/cavetypes.hpp"
#include "cave/caveset.hpp"
#include "cave/caverendered.hpp"
#include "misc/logger.hpp"

/**
 * Save caveset as BDCFF plain text
 * @param textname filename
 */
void gd_save_text(char *textname, CaveSet &caveset) {
    char *textoutbasename; /* used as a base name for text output files */

    textoutbasename = g_strdup(textname);
    if (g_str_has_suffix(textoutbasename, ".bd")) {
        /* has BDCFF extension */
        *g_strrstr(textoutbasename, ".bd") = 0;
    }

    for (unsigned i = 0; i < caveset.caves.size(); i++) {
        std::string contents;
        GError *error = NULL;

        CaveStored &cave = caveset.caves[i];
        CaveRendered rendered(cave, 0, 0);

        int w = cave.w;
        int h = cave.h;
        int diamonds = cave.level_diamonds[0];
        int time = cave.level_time[0];
        int glibber = cave.level_amoeba_threshold[0];
        int growing = cave.level_amoeba_time[0];
        int milling = cave.level_magic_wall_time[0];

        if (growing >= 999) growing = 0;
        if (milling >= 999) milling = 0;

        contents += Printf("BDCFF cave dump\n");
        contents += Printf("size=%dx%d\n", w, h);
        contents += Printf("diamonds=%d\n", diamonds);
        contents += Printf("time=%d\n", time);
        contents += Printf("glibber=%d\n", glibber);
        contents += Printf("growing=%d\n", growing);
        contents += Printf("milling=%d\n", milling);

        for (int y = 0; y < cave.h; y++) {
            for (int x = 0; x < cave.w; x++) {
                int c = rendered.map(x, y);

                switch (c) {
                    case O_SPACE: c = ' '; break;
                    case O_DIRT: c = '.'; break;
                    case O_BRICK: c = 'w'; break;
                    case O_MAGIC_WALL: c = 'M'; break;
                    case O_OUTBOX: c = 'X'; break;
                    case O_PRE_OUTBOX: c = 'H'; break;
                    case O_PRE_INVIS_OUTBOX: c = 'H'; break;
                    case O_STEEL: c = 'W'; break;
                    case O_STEEL_EXPLODABLE: c = 'E'; break;
                    case O_FIREFLY_1: c = 'Q'; break;
                    case O_FIREFLY_2: c = 'o'; break;
                    case O_FIREFLY_3: c = 'O'; break;
                    case O_FIREFLY_4: c = 'q'; break;
                    case O_BITER_1: c = 'i'; break;
                    case O_BITER_2: c = 'J'; break;
                    case O_BITER_3: c = 'I'; break;
                    case O_BITER_4: c = 'j'; break;
                    case O_STONE: c = 'r'; break;
                    case O_STONE_F: c = 'R'; break;
                    case O_DIAMOND: c = 'd'; break;
                    case O_DIAMOND_F: c = 'D'; break;
                    case O_NITRO_PACK: c = 't'; break;
                    case O_NITRO_PACK_F: c = 'T'; break;
                    case O_INBOX: c = 'P'; break;
                    case O_H_EXPANDING_WALL: c = 'x'; break;
                    case O_V_EXPANDING_WALL: c = 'v'; break;
                    case O_EXPANDING_WALL: c = 'V'; break;
                    case O_BUTTER_1: c = 'C'; break;
                    case O_BUTTER_2: c = 'c'; break;
                    case O_BUTTER_3: c = 'B'; break;
                    case O_BUTTER_4: c = 'b'; break;
                    case O_AMOEBA: c = 'a'; break;
                    case O_SLIME: c = 's'; break;
                    case O_BOMB: c = 'N'; break;
                    case O_VOODOO: c = 'F'; break;
                    case O_GHOST: c = 'g'; break;
                    case O_GRAVESTONE: c = 'G'; break;
                    default:
                        printf("encountered unknown code: %d\n", c);
                        c = '?';
                        break;
                }

                contents += c;
            }

            contents += "\n";
        }

        if (cave.name != "")
            contents += Printf("%ms\n", cave.name);
        if (cave.description != "")
            contents += Printf("%ms\n", cave.description);

        char *filename = g_strdup_printf("%s-%03d.txt", textoutbasename, i+1);
        if (!g_file_set_contents(filename, contents.c_str(), contents.size(), &error)) {
            /* could not save properly */
            gd_critical(error->message);
            g_error_free(error);
        }
        g_free(filename);
    }
}
