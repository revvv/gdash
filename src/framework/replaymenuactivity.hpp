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

#ifndef REPLAYMENUACTIVITY_HPP_INCLUDED
#define REPLAYMENUACTIVITY_HPP_INCLUDED

#include "config.h"

#include "framework/activity.hpp"

class CaveStored;
class CaveReplay;

/** Show the replays of the current caveset to the user,
 * and also allow him to play them.
 * If SDL support is compiled, this Activity may also launch a ReplaySaverActivity
 * to save the replay to a WAV/PNG combo. */
class ReplayMenuActivity: public Activity {
public:
    /** Ctor.
     * @param app The parent app. */
    ReplayMenuActivity(App *app);
    virtual void pushed_event();
    virtual void redraw_event(bool full) const;
    virtual void keypress_event(KeyCode keycode, int gfxlib_keycode);

private:
    /** This is a line of the list shown to the user.
     * For replays, both pointers are non-null. For cave names, replay is null.
     * For an empty (separator) line, both are null. */
    struct ReplayItem {
        CaveStored *cave;       ///< The cave to which the replay belongs.
        CaveReplay *replay;     ///< The replay object.
    };
    /** List of replays - or, the list of lines to be shown to the user. */
    std::vector<ReplayItem> items;
    /** Lines to show on one page. */
    unsigned lines_per_page;
    /** The current line selected. */
    unsigned current;
};

#endif
