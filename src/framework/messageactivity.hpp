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

#ifndef MESSAGEACTIVITY_HPP_INCLUDED
#define MESSAGEACTIVITY_HPP_INCLUDED

#include "framework/activity.hpp"

#include <vector>
#include <string>

class Command;

/**
 * Show a short message to the user, and wait for a keypress.
 * After the keypress, execute a Command.
 * The message can be of two parts, a primary and a secondary text. The secondary one is optional,
 * it may be an empty text - it just gives some further information to the user. */
class MessageActivity: public Activity {
public:
    /** Ctor of a MessageActivity.
     * @param app The parent app.
     * @param primary The text to show.
     * @param secondary Supplementary text to show (extra information). May be omitted.
     * @param command_after_exit Execute the command after showing the text and keypress from the user. May be omitted. */
    MessageActivity(App *app, std::string const &primary, std::string const &secondary = "", std::unique_ptr<Command> command_after_exit = nullptr);
    ~MessageActivity();

    virtual void redraw_event(bool full) const;
    virtual void keypress_event(KeyCode keycode, int gfxlib_keycode);

private:
    std::unique_ptr<Command> command_after_exit;
    std::vector<std::string> wrapped_text;
};

#endif
