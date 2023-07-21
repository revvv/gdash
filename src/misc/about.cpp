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

#include <glib/gi18n.h>

#include "about.hpp"

const char *About::license =
    N_(
        "Permission is hereby granted, free of charge, to any person obtaining "
        "a copy of this software and associated documentation files (the "
        "\"Software\"), to deal in the Software without restriction, including "
        "without limitation the rights to use, copy, modify, merge, publish, "
        "distribute, sublicense, and/or sell copies of the Software, and to "
        "permit persons to whom the Software is furnished to do so, subject to "
        "the following conditions:\n"
        "\n"
        "The above copyright notice and this permission notice shall be "
        "included in all copies or substantial portions of the Software.\n"
        "\n"
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, "
        "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "
        "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. "
        "IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR "
        "ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF "
        "CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION "
        "WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE."
    );

const char *About::artists[] = { "CWS",
                                 0
                               };

const char *About::authors[] = { "Czirkos Zoltan <czirkos.zoltan@gmail.com>",
                                 "Scale2x: Andrea Mazzoleni",
                                 "hqx: Maxim Stepin, Cameron Zemek",
                                 "specifications/test: CWS",
                                 "misc: revvv",
#ifdef HAVE_SDL
                                 "SDL: Sam Lantinga <slouken@libsdl.org>",
                                 "SDL png saver: Philip D. Bober <wildfire1138@mchsi.com>",
#endif
                                 0
                               };

const char *About::documenters[] = {
    "Original engine docs: LogicDeLuxe",
    "Playing hints: Sendy",
    0
};


const char *About::comments =
    N_("The primary objective of this game is to collect required "
       "number of diamonds in defined time and exit the cave. You control your "
       "player's movement to solve given cave.");

const char *About::translator_credits = N_("translator-credits");

const char *About::copyright = "Copyright 2007-2023, GDash Project";

const char *About::website = "https://github.com/revvv/gdash-export-CrLi";
