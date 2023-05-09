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

#ifdef HAVE_SDL
#include <SDL.h>
#endif

#include "input/joystick.hpp"
#include "misc/logger.hpp"

enum { JoystickThreshold = 32768 / 2 };

#ifdef HAVE_SDL
static SDL_Joystick **joysticks = NULL;
static int num_joysticks = 0;
#endif

void Joystick::init() {
#ifdef HAVE_SDL
    if (!SDL_WasInit(SDL_INIT_JOYSTICK))
        SDL_Init(SDL_INIT_JOYSTICK);
    num_joysticks = SDL_NumJoysticks();
    gd_message("Number of joysticks: %d", num_joysticks);
    if (num_joysticks > 0) {
        joysticks = (SDL_Joystick **) malloc(sizeof(SDL_Joystick *) * num_joysticks);
        for (int i = 0; i < num_joysticks; i++) {
            joysticks[i] = SDL_JoystickOpen(i);
            gd_message("joystick %d: %s", i, SDL_JoystickNameForIndex(i));
        }
    }
#endif
}

bool Joystick::have_joystick() {
#ifdef HAVE_SDL
    return num_joysticks > 0;
#else
    return false;
#endif
}

bool Joystick::up() {
#ifdef HAVE_SDL
    SDL_JoystickUpdate();
    bool res = false;
    for (int i = 0; i < num_joysticks; i++)
        res |= joysticks[i] != NULL && SDL_JoystickGetAxis(joysticks[i], 1) < -JoystickThreshold;
    return res;
#else
    return false;
#endif
}

bool Joystick::down() {
#ifdef HAVE_SDL
    SDL_JoystickUpdate();
    bool res = false;
    for (int i = 0; i < num_joysticks; i++)
        res |= joysticks[i] != NULL && SDL_JoystickGetAxis(joysticks[i], 1) > JoystickThreshold;
    return res;
#else
    return false;
#endif
}

bool Joystick::left() {
#ifdef HAVE_SDL
    SDL_JoystickUpdate();
    bool res = false;
    for (int i = 0; i < num_joysticks; i++)
        res |= joysticks[i] != NULL && SDL_JoystickGetAxis(joysticks[i], 0) < -JoystickThreshold;
    return res;
#else
    return false;
#endif
}

bool Joystick::right() {
#ifdef HAVE_SDL
    SDL_JoystickUpdate();
    bool res = false;
    for (int i = 0; i < num_joysticks; i++)
        res |= joysticks[i] != NULL && SDL_JoystickGetAxis(joysticks[i], 0) > JoystickThreshold;
    return res;
#else
    return false;
#endif
}

bool Joystick::fire1() {
#ifdef HAVE_SDL
    SDL_JoystickUpdate();
    bool res = false;
    for (int i = 0; i < num_joysticks; i++)
        res |= joysticks[i] != NULL && (SDL_JoystickGetButton(joysticks[i], 0));
    return res;
#else
    return false;
#endif
}

bool Joystick::fire2() {
#ifdef HAVE_SDL
    SDL_JoystickUpdate();
    bool res = false;
    for (int i = 0; i < num_joysticks; i++)
        res |= joysticks[i] != NULL && (SDL_JoystickGetButton(joysticks[i], 1));
    return res;
#else
    return false;
#endif
}
