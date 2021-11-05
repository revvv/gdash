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

#include "input/gameinputhandler.hpp"
#include "input/joystick.hpp"

GameInputHandler::GameInputHandler() {
    clear_all_keypresses();
}


void GameInputHandler::clear_all_keypresses() {
    up_k = false;
    down_k = false;
    left_k = false;
    right_k = false;
    fire1_k = false;
    fire2_k = false;
    suicide = false;
    fast_forward = false;
    alternate_status = false;
    restart_k = false;
}


void GameInputHandler::keyrelease(int gfxlib_keycode) {
    for (unsigned i = 0; i != KeysNum; ++i) {
        KeyAssignment const &key = get_key(Keys(i));
        if (gfxlib_keycode == key.gfxlib_keycode) {
            this->*key.ptr = false;
        }
    }
}


void GameInputHandler::keypress(int gfxlib_keycode) {
    for (unsigned i = 0; i != KeysNum; ++i) {
        KeyAssignment const &key = get_key(Keys(i));
        if (gfxlib_keycode == key.gfxlib_keycode) {
            this->*key.ptr = true;
        }
    }
}


char const *GameInputHandler::get_key_name(Keys keyindex) {
    return get_key_name_from_keycode(get_key(keyindex).gfxlib_keycode);
}


int &GameInputHandler::get_key_variable(Keys keyindex) {
    return get_key(keyindex).gfxlib_keycode;
}


bool GameInputHandler::up() {
    return up_k || Joystick::up();
}


bool GameInputHandler::down() {
    return down_k || Joystick::down();
}


bool GameInputHandler::left() {
    return left_k || Joystick::left();
}


bool GameInputHandler::right() {
    return right_k || Joystick::right();
}


bool GameInputHandler::fire1() {
    return fire1_k || Joystick::fire1();
}


bool GameInputHandler::fire2() {
    return fire2_k || Joystick::fire2();
}

bool GameInputHandler::restart() {
    bool ret = restart_k;
    restart_k = false;
    return ret;
}

void GameInputHandler::set_restart() {
    restart_k = true;
}
