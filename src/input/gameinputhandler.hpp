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

#ifndef GAMEINPUTHANDLER_HPP_INCLUDED
#define GAMEINPUTHANDLER_HPP_INCLUDED

#include "config.h"

#include <glib.h>

class GameInputHandler {
public:
    enum Keys {
        KeyUp,
        KeyDown,
        KeyLeft,
        KeyRight,
        KeyFire1,
        KeyFire2,
        KeySuicide,
        KeyFastForward,
        KeyStatusBar,
        KeyRestartLevel,
        KeysNum
    };
    struct KeyAssignment {
        Keys index;
        int &gfxlib_keycode;
        bool GameInputHandler::*ptr;
    };

    GameInputHandler();
    virtual ~GameInputHandler() {}
    void keyrelease(int gfxlib_keycode);
    void keypress(int gfxlib_keycode);

    char const *get_key_name(Keys keyindex);
    virtual char const *get_key_name_from_keycode(int gfxlib_keycode) = 0;
    int &get_key_variable(Keys keyindex);
    virtual KeyAssignment const &get_key(Keys keyindex) const = 0;
    void clear_all_keypresses();

    /// This is for the GTK+ version - emulates a pressed restart key.
    void set_restart();

    bool up();
    bool down();
    bool left();
    bool right();
    bool fire1();
    bool fire2();
    bool restart();

    bool up_k, down_k, left_k, right_k, fire1_k, fire2_k, restart_k;

    bool suicide, fast_forward, alternate_status;
};

#endif
