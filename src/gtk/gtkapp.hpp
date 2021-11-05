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

#ifndef GTKAPP_HPP_INCLUDED
#define GTKAPP_HPP_INCLUDED

#include "config.h"

#include "framework/app.hpp"

class CaveSet;
class GTKScreen;

class GTKApp: public App {
private:
    GtkWidget *toplevel;
    GtkActionGroup *actions_game;

public:
    GTKApp(GTKScreen &screenref, GtkWidget *toplevel, GtkActionGroup *actions_game);

    virtual void select_file_and_do_command(const char *title, const char *start_dir, const char *glob, bool for_save, const char *defaultname, std::unique_ptr<Command1Param<std::string> > command_when_successful);
    virtual void ask_yesorno_and_do_command(char const *question, const char *yes_answer, char const *no_answer, std::unique_ptr<Command> command_when_yes, std::unique_ptr<Command> command_when_no);
    virtual void show_about_info();
    virtual void input_text_and_do_command(char const *title_line, char const *default_text, std::unique_ptr<Command1Param<std::string> > command_when_successful);
    virtual void game_active(bool active);
    virtual void show_settings(Setting *settings);
    virtual void show_message(std::string const &primary, std::string const &secondary, std::unique_ptr<Command> command_after_exit);
    virtual void show_help(helpdata const help_text[]);
};

#endif
