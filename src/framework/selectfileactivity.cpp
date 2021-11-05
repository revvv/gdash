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

#include "framework/selectfileactivity.hpp"
#include "cave/colors.hpp"
#include "framework/app.hpp"
#include <glib/gi18n.h>
#include "framework/commands.hpp"
#include "gfx/fontmanager.hpp"

#include <glib.h>
#include <glib/gstdio.h>
#include <algorithm>

#include "gfx/screen.hpp"
#include "misc/util.hpp"
#include "misc/autogfreeptr.hpp"

// TODO utf8-filename charset audit


class JumpToDirectoryCommand: public Command1Param<std::string> {
public:
    JumpToDirectoryCommand(App *app, SelectFileActivity *activity)
        :
        Command1Param<std::string>(app),
        directory(p1),
        activity(activity) {
    }
private:
    std::string &directory;
    SelectFileActivity *activity;
    virtual void execute() {
        AutoGFreePtr<char> filename_in_locale_charset(g_filename_from_utf8(directory.c_str(), -1, NULL, NULL, NULL));
        activity->jump_to_directory(filename_in_locale_charset);
    }
};


class FileNameEnteredCommand: public Command1Param<std::string> {
public:
    FileNameEnteredCommand(App *app, SelectFileActivity *activity)
        :
        Command1Param<std::string>(app),
        filepath(p1),
        activity(activity) {
    }
private:
    std::string &filepath;
    SelectFileActivity *activity;
    virtual void execute() {
        activity->file_selected(filepath.c_str());
    }
};


/* returns a string which contains the utf8 representation of the filename originally in system encoding */
static std::string filename_to_utf8(const char *filename) {
    GError *error = NULL;
    AutoGFreePtr<char> utf8(g_filename_to_utf8(filename, -1,  NULL, NULL, &error));
    if (error) {
        g_error_free(error);
        return filename;        // return with filename without conversion
    }
    return std::string(utf8);
}


/* filename sort. directories on the top. */
static bool filename_sort(std::string const &s1, std::string const &s2) {
    if (s1.empty())
        return true;
    if (s2.empty())
        return false;
    bool const s1_dir = s1[s1.length() - 1] == G_DIR_SEPARATOR;
    bool const s2_dir = s2[s2.length() - 1] == G_DIR_SEPARATOR;
    if (s1_dir && s2_dir)
        return s1 < s2;
    if (s1_dir)
        return true;
    if (s2_dir)
        return false;
    return s1 < s2;
}


SelectFileActivity::SelectFileActivity(App *app, const char *title, const char *start_dir, const char *glob, bool for_save, const char *defaultname, std::unique_ptr<Command1Param<std::string>> command_when_successful)
    :
    Activity(app),
    command_when_successful(std::move(command_when_successful)),
    title(title),
    for_save(for_save),
    defaultname(defaultname),
    start_dir(start_dir ? start_dir : "") {
    yd = app->font_manager->get_line_height();
    names_per_page = app->screen->get_height() / yd - 5;
    if (glob == NULL || g_str_equal(glob, ""))
        glob = "*";
    globs = g_strsplit_set(glob, ";", -1);

    /* remember current directory, as we step into others */
    directory_of_process = g_get_current_dir();
    directory = g_strdup(directory_of_process);
}


void SelectFileActivity::pushed_event() {
    if (start_dir != "")
        jump_to_directory(start_dir.c_str());
    else
        jump_to_directory(directory_of_process);
}


SelectFileActivity::~SelectFileActivity() {
    g_strfreev(globs);
}


void SelectFileActivity::jump_to_directory(char const *jump_to) {
    GDir *dir;
    /* directory we are looking at, and then to the selected one (which may be relative path!) */
    if (g_chdir(directory) == -1 || g_chdir(jump_to) == -1 || NULL == (dir = g_dir_open(".", 0, NULL))) {
        g_chdir(directory_of_process);    /* step back to directory where we started */
        dir = g_dir_open(".", 0, NULL);
        if (!dir) {
            /* problem: cannot change to requested directory, and cannot change to the process
             * original directory as well. cannot read any of them. this is critical, the
             * selectfileactivity cannot continue, so it deletes itself. */
            app->enqueue_command(std::make_unique<PopActivityCommand>(app));
            app->show_message(Printf(_("Cannot change to directory: %s."), jump_to), Printf("Jumped back to directory: %s.", directory_of_process));
        } else {
            /* cannot read the new directory, but managed to jump back to the original. issue
             * an error then continue in the original dir. */
            app->show_message(Printf(_("Cannot change to directory: %s."), jump_to), Printf("Jumped back to directory: %s.", directory_of_process));
        }
    }
    /* now get the directory we have stepped into, and it is readable as well. remember! */
    g_free(directory);
    directory = g_get_current_dir();

    files.clear();
    char const *name;
    while ((name = g_dir_read_name(dir)) != NULL) {
#ifdef G_OS_WIN32
        /* on windows, skip hidden files? */
#else
        /* on unix, skip file names starting with a '.' - those are hidden files */
        if (name[0] == '.')
            continue;
#endif
        if (g_file_test(name, G_FILE_TEST_IS_DIR))
            files.push_back(std::string(name) + G_DIR_SEPARATOR_S);    /* dirname/ or dirname\ */
        else {
            bool match = false;
            for (int i = 0; globs[i] != NULL && !match; i++)
                if (g_pattern_match_simple(globs[i], name))
                    match = true;
            if (match)
                files.push_back(filename_to_utf8(name));
        }
    }
    g_dir_close(dir);

    /* add "directory up" if we are NOT in a root directory */
#ifdef G_OS_WIN32
    if (!g_str_has_suffix(directory, ":\\"))    /* root directory is "X:\" */
        files.push_back(std::string("..") + G_DIR_SEPARATOR_S);  /* ..\ */
#else
    if (!g_str_equal(directory, "/"))
        files.push_back(std::string("..") + G_DIR_SEPARATOR_S);  /* ../ */
#endif
    /* sort the array */
    sort(files.begin(), files.end(), filename_sort);
    sel = 0;

    /* step back to directory where we started */
    g_chdir(directory_of_process);

    queue_redraw();
}


void SelectFileActivity::file_selected_do_command() {
    app->enqueue_command(std::move(command_when_successful));
    app->enqueue_command(std::make_unique<PopActivityCommand>(app));
}


/** This command forces the SelectFileActivity object to do the save.
 * It can be used as a command to be processed after a "do you really want to save"
 * question shown by an AskYesNoActivity. . */
class SelectFileForceSaveCommand: public Command {
public:
    SelectFileForceSaveCommand(App *app, SelectFileActivity *activity)
        : Command(app),
          activity(activity) {
    }
private:
    SelectFileActivity *activity;
    virtual void execute() {
        activity->file_selected_do_command();
    }
};


void SelectFileActivity::file_selected(char const *filename) {
    /* ok so set the filename in the pending command object. */
    AutoGFreePtr<char> result_filename(g_build_path(G_DIR_SEPARATOR_S, directory, filename, NULL));
    command_when_successful->set_param1(std::string(result_filename));

    /* a file is selected, so enqueue the command. */
    /* but first - check if it is an overwrite! if the user does not allow the overwriting, we
     * should do nothing. so create an AskYesNoActivity, which will call the
     * file_selected_do_command() method on the SelectFileActivity, when the user
     * accepts the overwrite. */
    if (for_save && g_file_test(filename, G_FILE_TEST_EXISTS)) {
        /* ask the overwrite. */
        app->ask_yesorno_and_do_command(_("File exists. Overwrite?"), "Yes", "No", std::make_unique<SelectFileForceSaveCommand>(app, this));
    } else {
        /* not a "save file" activity, so no problem if an existing file is
         * selected. */
        file_selected_do_command();
    }
}


void SelectFileActivity::process_enter() {
    if (g_str_has_suffix(files[sel].c_str(), G_DIR_SEPARATOR_S)) {
        /* directory selected */
        jump_to_directory(files[sel].c_str());
    } else {
        /* file selected */
        file_selected(files[sel].c_str());
    }
}


void SelectFileActivity::keypress_event(KeyCode keycode, int gfxlib_keycode) {
    switch (keycode) {
            /* movements */
        case App::PageUp:
            sel = gd_clamp(sel - names_per_page, 0, files.size() - 1);
            queue_redraw();
            break;
        case App::PageDown:
            sel = gd_clamp(sel + names_per_page, 0, files.size() - 1);
            queue_redraw();
            break;
        case App::Up:
            sel = gd_clamp(sel - 1, 0, files.size() - 1);
            queue_redraw();
            break;
        case App::Down:
            sel = gd_clamp(sel + 1, 0, files.size() - 1);
            queue_redraw();
            break;
        case App::Home:
            sel = 0;
            queue_redraw();
            break;
        case App::End:
            sel = files.size() - 1;
            queue_redraw();
            break;
        case App::Enter:
            process_enter();
            break;

            /* jump to directory (name will be typed) */
        case 'j':
        case 'J':
            // TRANSLATORS: 35 chars max
            app->input_text_and_do_command(_("Jump to directory"), directory, std::make_unique<JumpToDirectoryCommand>(app, this));
            break;
            /* enter new filename - only if saving allowed */
        case 'n':
        case 'N':
            if (for_save) {
                // TRANSLATORS: 35 chars max
                app->input_text_and_do_command(_("Enter new file name"), defaultname.c_str(), std::make_unique<FileNameEnteredCommand>(app, this));
            }
            break;

        case App::Escape:
            app->enqueue_command(std::make_unique<PopActivityCommand>(app));
            break;

        default:
            /* other keys do nothing */
            break;
    }
}


void SelectFileActivity::redraw_event(bool full) const {
    app->clear_screen();

    /* show current directory */
    app->title_line(title.c_str());
    app->set_color(GD_GDASH_YELLOW);
    app->font_manager->blittext_n(-1, 1 * yd, filename_to_utf8(directory).c_str());
    if (for_save) {
        // TRANSLATORS: 40 chars max
        app->status_line(_("Crsr:select  N:new  J:jump  Esc:cancel"));   /* for saving, we allow the user to select a new filename. */
    } else {
        // TRANSLATORS: 40 chars max
        app->status_line(_("Crsr: select   J: jump   Esc: cancel"));
    }
    unsigned i, page = sel / names_per_page, cur;
    for (i = 0, cur = page * names_per_page; i < names_per_page; i++, cur++) {
        if (cur < files.size()) {  /* may not be as much filenames as it would fit on the screen */
            app->set_color((cur == unsigned(sel)) ? GD_GDASH_YELLOW : GD_GDASH_LIGHTBLUE);
            app->font_manager->blittext_n(app->font_manager->get_font_width_narrow(), (i + 3)*yd, files[cur].c_str());
        }
    }

    if (files.size() > names_per_page)
        app->draw_scrollbar(0, sel, files.size() - 1);

    app->screen->drawing_finished();
}
