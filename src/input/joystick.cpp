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
static SDL_GameController **gamepads = NULL;
static int num_joysticks = 0;
#endif

void Joystick::init() {
#ifdef HAVE_SDL
    if (!SDL_WasInit(SDL_INIT_JOYSTICK)) {
        int res = SDL_Init(SDL_INIT_JOYSTICK);
        if (res < 0)
            gd_warning("SDL init joystick failed: %s", SDL_GetError());
    }
    num_joysticks = SDL_NumJoysticks();
    gd_debug("Number of joysticks: %d", num_joysticks);
    if (num_joysticks > 0) {
        int res = SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
        gd_debug("gamecontrollerdb.txt: new mappings found: %d", res); // doesn't tell how many were updated
        gd_debug("Game Controller events: %d", SDL_GameControllerEventState(SDL_QUERY));
        gamepads = (SDL_GameController **) malloc(sizeof(SDL_GameController *) * num_joysticks);
        for (int i = 0; i < num_joysticks; i++) {
            gamepads[i] = SDL_GameControllerOpen(i);
            char guid_str[33];
            SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i), guid_str, 33);
            gd_debug("joystick %d: GUID=%s name=%s", i, guid_str, SDL_JoystickNameForIndex(i));
            gd_debug("joystick %d: map =%s", i, SDL_GameControllerMappingForGUID(SDL_JoystickGetDeviceGUID(i)));

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
    bool res = false;
    for (int i = 0; i < num_joysticks; i++) {
        res |= gamepads[i] != NULL && (SDL_GameControllerGetAxis(gamepads[i], SDL_CONTROLLER_AXIS_LEFTY) < -JoystickThreshold
                || SDL_GameControllerGetButton(gamepads[i], SDL_CONTROLLER_BUTTON_DPAD_UP));
    }
    return res;
#else
    return false;
#endif
}

bool Joystick::down() {
#ifdef HAVE_SDL
    bool res = false;
    for (int i = 0; i < num_joysticks; i++) {
        res |= gamepads[i] != NULL && (SDL_GameControllerGetAxis(gamepads[i], SDL_CONTROLLER_AXIS_LEFTY) > JoystickThreshold
                || SDL_GameControllerGetButton(gamepads[i], SDL_CONTROLLER_BUTTON_DPAD_DOWN));
    }
    return res;
#else
    return false;
#endif
}

bool Joystick::left() {
#ifdef HAVE_SDL
    bool res = false;
    for (int i = 0; i < num_joysticks; i++) {
        res |= gamepads[i] != NULL && (SDL_GameControllerGetAxis(gamepads[i], SDL_CONTROLLER_AXIS_LEFTX) < -JoystickThreshold
                || SDL_GameControllerGetButton(gamepads[i], SDL_CONTROLLER_BUTTON_DPAD_LEFT));
    }
    return res;
#else
    return false;
#endif
}

bool Joystick::right() {
#ifdef HAVE_SDL
    bool res = false;
    for (int i = 0; i < num_joysticks; i++) {
        res |= gamepads[i] != NULL && (SDL_GameControllerGetAxis(gamepads[i], SDL_CONTROLLER_AXIS_LEFTX) > JoystickThreshold
                || SDL_GameControllerGetButton(gamepads[i], SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
    }
    return res;
#else
    return false;
#endif
}

bool Joystick::fire1() {
#ifdef HAVE_SDL
    // NOTE: Although SDL_GameControllerEventState() is true, we have to poll for joystick events. Not needed for all gamepads.
    if (num_joysticks > 0)
        SDL_JoystickUpdate();
    bool res = false;
    for (int i = 0; i < num_joysticks; i++)
        res |= gamepads[i] != NULL && (SDL_GameControllerGetButton(gamepads[i], SDL_CONTROLLER_BUTTON_A));
    return res;
#else
    return false;
#endif
}

bool Joystick::fire2() {
#ifdef HAVE_SDL
    bool res = false;
    for (int i = 0; i < num_joysticks; i++)
        res |= gamepads[i] != NULL && (SDL_GameControllerGetButton(gamepads[i], SDL_CONTROLLER_BUTTON_X));
    return res;
#else
    return false;
#endif
}
