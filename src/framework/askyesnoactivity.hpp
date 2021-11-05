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

#ifndef ASKYESNOACTIVITY_HPP_INCLUDED
#define ASKYESNOACTIVITY_HPP_INCLUDED

#include <string>
#include <glib.h>

#include "framework/activity.hpp"

class App;
class Command;


/** An activity which shows a question to the user, and asks yes or no.
 * Two commands are given to such an activity; one of which will be executed
 * if the answer was yes, the other if the answer was no.
 * See App::ask_yesorno_and_do_command(). */
class AskYesNoActivity: public Activity {
public:
    /** Ctor.
     * @param app The parent app.
     * @param question The text of the question to ask.
     *      Should not be too long, so it fits on the screen in the SDL version as well.
     * @param yes_answer The text of the answer treated as "yes".
     * @param no_answer The text of the answer treated as "no".
     * @param command_when_yes The command to be executed if the user said yes. May be NULL.
     * @param command_when_no The command to be executed if the user said no. May be NULL. */
    AskYesNoActivity(App *app, char const *question, const char *yes_answer, char const *no_answer, std::unique_ptr<Command> command_when_yes, std::unique_ptr<Command> command_when_no);
    virtual void keypress_event(KeyCode keycode, int gfxlib_keycode);
    virtual void redraw_event(bool full) const;

private:
    std::string question, yes_answer, no_answer;
    std::unique_ptr<Command> command_when_yes, command_when_no;
};

#endif
