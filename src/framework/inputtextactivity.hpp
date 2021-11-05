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

#ifndef INPUTTEXTACTIVITY_HPP_INCLUDED
#define INPUTTEXTACTIVITY_HPP_INCLUDED

#include <string>
#include <glib.h>

#include "framework/activity.hpp"
#include "framework/commands.hpp"

template <typename T> class Command1Param;

/** Ask the user to type one line of text, and when successful, call the Command parametrized with the text.
 *  See App::input_text_and_do_command() as well. */
class InputTextActivity: public Activity {
public:
    /** Ctor.
     * @param title_line The title of the window.
     * @param default_text The default value of the text box.
     * @param command_when_successful The command of one string parameter to be parametrized with the line
     *        typed and executed, if the user accepts the input. */
    InputTextActivity(App *app, char const *title_line, const char *default_text, std::unique_ptr<Command1Param<std::string>> command_when_successful);
    ~InputTextActivity();
    virtual void keypress_event(KeyCode keycode, int gfxlib_keycode);
    virtual void textinput_event(char *appendtext);
    virtual void textediting_event(char *overwritetext);
    virtual void redraw_event(bool full) const;
    virtual void timer_event(int ms_elapsed);
    virtual void shown_event();
    virtual void hidden_event();

private:
    std::string title;
    std::unique_ptr<Command1Param<std::string>> command_when_successful;
    GString *text;
    int ms;
    bool blink;
};

#endif
