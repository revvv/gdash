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

#ifndef GAMEACTIVITY_HPP_INCLUDED
#define GAMEACTIVITY_HPP_INCLUDED

#include <memory>

#include "framework/app.hpp"
#include "gfx/cellrenderer.hpp"
#include "cave/gamerender.hpp"

class GameControl;
class Command;

/**
 * This activity allows the user playing the game.
 * It must be given a dinamically allocated GameControl object,
 * and passes all keypresses and stuff to this object.
 */
class GameActivity: public Activity {
public:
    /** Constructor of the GameActivity.
     * @param app The parent App.
     * @param game A GameControl object, which will be passed the keypresses. */
    GameActivity(App *app, std::unique_ptr<GameControl> game);
    ~GameActivity();
    virtual void shown_event();
    virtual void hidden_event();
    virtual void redraw_event(bool full) const;
    virtual void keypress_event(KeyCode keycode, int gfxlib_keycode);
    virtual void timer_event(int ms_elapsed);

    /**
     * Shortcut keys in the GameActivity.
     * Public for the GTK+ frontend to emulate these keypresses.
     * If you change these, change the help strings as well. */
    enum Keys {
        EndGameKey = App::F1,
        RandomColorKey = App::F2,
        TakeSnapshotKey = App::F3,
        RevertToSnapshotKey = App::F4,
        PauseKey = ' ',
        CaveVariablesKey = App::F8,
    };

private:
    std::unique_ptr<GameControl> game;
    CellRenderer cellrenderer;
    GameRenderer gamerenderer;
    bool exit_game, show_highscore, paused;
};

#endif

