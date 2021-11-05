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

#ifndef SELECTFILEACTIVITY_HPP_INCLUDED
#define SELECTFILEACTIVITY_HPP_INCLUDED

#include <vector>
#include <string>
#include <memory>

#include "framework/activity.hpp"

template <typename T> class Command1Param;

/**
 * Allow the user to select a file (maybe type the name of a new file),
 * parametrize a command with the filename and finally execute the parametrized command.
 * This Activity is like a file selection dialog in modern UIs.
 */
class SelectFileActivity: public Activity {
public:
    /** Constructor for file selection Activity.
     * @param app The parent App.
     * @param title Window title - this should describe the purpose of file selection.
     * @param start_dir The directory to start the selecting in.
     * @param glob Glob pattern - semicolon separated list of globs. For example "*.png;*.jpg"
     * @param for_save Set to true, if the file selection is for saving a file, and therefore typing a new name should be allowed.
     * @param defaultname The default name to set.
     * @param command_when_successful Parametrize this command with the filename and execute it - if the file selection is successful. */
    SelectFileActivity(App *app, const char *title, const char *start_dir, const char *glob, bool for_save, const char *defaultname, std::unique_ptr<Command1Param<std::string>> command_when_successful);
    ~SelectFileActivity();

    virtual void keypress_event(KeyCode keycode, int gfxlib_keycode);
    virtual void redraw_event(bool full) const;
    virtual void pushed_event();

    void jump_to_directory(char const *jump_to);
    void file_selected(char const *filename);
    void file_selected_do_command();

private:
    std::unique_ptr<Command1Param<std::string>> command_when_successful;
    std::string title;
    bool for_save;
    int yd;
    unsigned names_per_page;
    char **globs;
    char *directory;
    char *directory_of_process;
    int sel;

    std::vector<std::string> files;
    std::string defaultname;
    std::string start_dir;

    void read_dir();
    void process_enter();
};

#endif
