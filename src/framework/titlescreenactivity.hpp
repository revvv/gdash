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

#ifndef TITLESCREENACTIVITY_HPP_INCLUDED
#define TITLESCREENACTIVITY_HPP_INCLUDED

#include "framework/activity.hpp"
#include "gfx/pixmapstorage.hpp"

#include <vector>
#include <memory>

class Pixmap;

class TitleScreenActivity: public Activity, public PixmapStorage {
public:
    explicit TitleScreenActivity(App *app);
    virtual void redraw_event(bool full) const;
    virtual void keypress_event(KeyCode keycode, int gfxlib_keycode);
    virtual void timer_event(int ms_elapsed);
    virtual void shown_event();
    virtual void hidden_event();
    ~TitleScreenActivity() = default;

    /// Implement PixbufStorage
    virtual void release_pixmaps();

private:
    const int scale;
    const int image_centered_threshold;
    mutable std::vector<std::unique_ptr<Pixmap>> animation;
    int frames, time_ms;
    mutable int animcycle;
    /* positions on screen */
    int image_h, y_gameline, y_caveline;
    bool show_status;
    int which_status;
    bool caveset_has_levels;
    int cavenum, levelnum;

    void render_animation() const;
    void clear_animation();
};

/**
 * A variable of this enum type stores the action which
 * should be done after finishing the App.
 * It is used outside the App, by the running environment! */
enum NextAction {
    StartTitle,    ///< Start the app with the title screen.
#ifdef HAVE_GTK
    StartEditor,   ///< Start the editor.
#endif
    Restart,       ///< Restart with new sound settings & etc.
    Quit           ///< Quit the program.
};

#endif
