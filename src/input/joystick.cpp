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

enum { JoystickThreshold = 32768 / 2 };

#ifdef HAVE_SDL
static SDL_Joystick *joystick_1 = NULL;
#endif

void Joystick::init() {
#ifdef HAVE_SDL
    if (!SDL_WasInit(SDL_INIT_JOYSTICK))
        SDL_Init(SDL_INIT_JOYSTICK);
    if (SDL_NumJoysticks() > 0)
        joystick_1 = SDL_JoystickOpen(0);
#endif
}

bool Joystick::have_joystick() {
#ifdef HAVE_SDL
    return joystick_1 != NULL;
#else
    return false;
#endif
}

bool Joystick::up() {
#ifdef HAVE_SDL
    return joystick_1 != NULL && SDL_JoystickGetAxis(joystick_1, 1) < -JoystickThreshold;
#else
    return false;
#endif
}

bool Joystick::down() {
#ifdef HAVE_SDL
    return joystick_1 != NULL && SDL_JoystickGetAxis(joystick_1, 1) > JoystickThreshold;
#else
    return false;
#endif
}

bool Joystick::left() {
#ifdef HAVE_SDL
    return joystick_1 != NULL && SDL_JoystickGetAxis(joystick_1, 0) < -JoystickThreshold;
#else
    return false;
#endif
}

bool Joystick::right() {
#ifdef HAVE_SDL
    return joystick_1 != NULL && SDL_JoystickGetAxis(joystick_1, 0) > JoystickThreshold;
#else
    return false;
#endif
}

bool Joystick::fire1() {
#ifdef HAVE_SDL
    return joystick_1 != NULL && (SDL_JoystickGetButton(joystick_1, 0));
#else
    return false;
#endif
}

bool Joystick::fire2() {
#ifdef HAVE_SDL
    return joystick_1 != NULL && (SDL_JoystickGetButton(joystick_1, 1));
#else
    return false;
#endif
}
