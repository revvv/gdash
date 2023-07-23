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

/**
 * @defgroup Framework
 *
 * This group is the documentation of the Activity the App and
 * the Command class, which together control the graphical presentation
 * of everything in GDash (except the editor) to the user.
 *
 * An Activity object is a screen on which the user can do something,
 * for example play the game, select a file or read about a cave. Most
 * of the activities fill the whole screen. (Some of them are small
 * windows centered on the screen, but still they are modal.) An
 * activity receives events from the application, and can organize its
 * inner workings according to these events. There are keypress, timer,
 * redraw and other kinds of events.
 *
 * The activities are contained in an App object which organizes them into
 * a stack. Every time an event is received from the user or the timer,
 * the App object forwards the events (maybe after some preprocessing)
 * to the topmost activity on the stack. New activities are put on top
 * of the stack, and an activity can quit by requesting the App to
 * pop it off the top of the stack.
 *
 * The App object has the responsibility to connect the low level user
 * interface (screen, keyboard, graphics objects) to the activities.
 * The main program (which may be an SDL or a GTK implementation)
 * creates the App object, and translates the operating system and
 * graphics library specific events to function calls on the App
 * object. For example, when the user presses the Escape key, the
 * .keyboard_event() method of the App is to be called; and after that,
 * the App will forward this event to the topmost activity. If it is
 * the title screen, the program will presumably quit. If it is the
 * game, the cave will be restarted. It is always only the topmost
 * activity, which will receive the events from the app. Those which
 * are occluded by these do not receive any events, and so they are
 * stopped.
 *
 * In order to provide a means of communication between activities,
 * Command objects can be created. Each activity is allowed to push
 * as many command objects as it wants into the command queue of the
 * App from any of its event methods. The App will check the command
 * queue after calling any of the event methods of the topmost activity,
 * and execute all commands which are queued.
 *
 * The starting of the game from the title screen is a typical use of
 * these commands objects. When the user presses space, his name is asked
 * for. After that, he can start the game by typing his name and pressing
 * enter. This can be implemented by the TitleScreenActivity, the
 * InputTextActivity and the GameActivity classes.
 * - First, the TitleScreenActivity creates the InputTextActivity.
 * - The InputTextActivity does not know, what it should do with the
 *   text the user typed.
 * - However, it is also given a NewGameCommand, which is a command
 *   of one string parameter. When this command is executed, it creates
 *   a GameActivity and gives it to the App to push it onto the stack.
 * - If the user presses enter after typing his name, the
 *   InputTextActivity is popped from the stack, and the GameActivity
 *   is pushed.
 * - If the user presses escape, the command is not executed, so after
 *   popping the InputTextActivity nothing else happens. And so the
 *   TitleScreenActivity is on the top again.
 *
 * The command queue also serves the purpose of preventing object
 * creation/destruction ordering problems from happening. For example,
 * in the above scenario, the InputTextActivity object should not be
 * destructed before setting the player name parameter in the NewGameCommand
 * and executing the command itself. So activities are also pushed and
 * popped by means of using PopActivityCommand and PushActivityCommand
 * commands. The above interaction in detail:
 * - The user presses enter after typing his name.
 * - The NewGameCommand is parametrized with the name of the user.
 * - A PopActivityCommand is enqueued in the App.
 * - The NewGameCommand is also enqueued.
 * - The App checks the command queue.
 * - First, it executes the PopActivityCommand, so the InputTextActivity
 *   is destructed. (The name and starting cave parameters are not lost,
 *   because they are in the NewGameCommand.)
 * - The NewGameCommand is executed. It creates the GameActivity and
 *   pushes it onto the top of the activity stack.
 * - The game runs.
 *
 * The App class has some methods which can be called for common tasks like
 * selecting a file or asking a user a yes or no question. By default, these
 * methods push the corresponding activity. However, they can be overridden
 * by inheriting from the App, to provide the user with toolkit-specific
 * dialog boxes, for example GTK+ file open dialogs or message dialogs.
 * Activities should use these methods to perform these tasks instead of
 * hard-coding anything.
 */

#include "config.h"

#include <algorithm>
#include <glib/gi18n.h>

#include "framework/app.hpp"

#include "framework/commands.hpp"
#include "gfx/pixbuf.hpp"
#include "gfx/fontmanager.hpp"
#include "gfx/pixbuffactory.hpp"
#include "gfx/screen.hpp"
#include "input/gameinputhandler.hpp"

#include "misc/about.hpp"
#include "misc/helptext.hpp"
#include "sound/sound.hpp"
#include "settings.hpp"

#include "framework/selectfileactivity.hpp"
#include "framework/askyesnoactivity.hpp"
#include "framework/showtextactivity.hpp"
#include "framework/inputtextactivity.hpp"
#include "framework/settingsactivity.hpp"
#include "framework/messageactivity.hpp"
#ifdef HAVE_SDL
    #include "framework/volumeactivity.hpp"
#endif

#include "gamebackground.cpp"

std::string help_strings_to_string(char const **strings) {
    std::string s;
    for (int n = 0; strings[n] != NULL; n += 2)
        s += Printf("%c%s  %c%s\n", GD_COLOR_INDEX_YELLOW, strings[n], GD_COLOR_INDEX_LIGHTBLUE, strings[n + 1]);
    return s;
}


static std::string help_text_to_string(helpdata const help_text[]) {
    std::string s = Printf("%c", GD_COLOR_INDEX_LIGHTBLUE);
    for (int i = 0; g_strcmp0(help_text[i].stock_id, HELP_LAST_LINE) != 0; ++i) {
        GdElementEnum element = help_text[i].element;
        if (element != O_NONE) {
            // pixbuf missing here
            if (help_text[i].heading == NULL) {
                /* add element name only if no other text given */
                s += Printf("%c%s%c\n", GD_COLOR_INDEX_YELLOW, visible_name_no_attribute(element));
            }
        }
        if (help_text[i].heading) {
            /* some words in big letters */
            s += Printf("%c%s%c\n", GD_COLOR_INDEX_YELLOW, _(help_text[i].heading));
        }
        if (help_text[i].keyname)
            s += Printf(" %c%s\t", GD_COLOR_INDEX_YELLOW, _(help_text[i].keyname));
        if (help_text[i].description)
            s += Printf("%c%s\n", GD_COLOR_INDEX_LIGHTBLUE, _(help_text[i].description));
        s += "\n";
    }
    return s;
}


App::App(Screen &screenref)
    : PixmapStorage(screenref),
      screen(&screenref) {
    font_manager = NULL;
    gameinput = NULL;
    caveset = NULL;
}


App::~App() {
    pop_all_activities();
    gd_music_stop();
    release_pixmaps();
    delete gameinput;
    delete font_manager;
}


void App::release_pixmaps() {
    background_image.reset();
}


void App::set_no_activity_command(std::unique_ptr<Command> command) {
    no_activity_command = std::move(command);
}


void App::set_quit_event_command(std::unique_ptr<Command> command) {
    quit_event_command = std::move(command);
}


void App::set_request_restart_command(std::unique_ptr<Command> command) {
    request_restart_command = std::move(command);
}


void App::set_start_editor_command(std::unique_ptr<Command> command) {
    start_editor_command = std::move(command);
}


void App::clear_screen() {
    /* create if needed */
    if (!background_image) {
        std::unique_ptr<Pixbuf> pixbuf = screen->pixbuf_factory.create_from_inline(sizeof(gamebackground), gamebackground);
        background_image = screen->create_scaled_pixmap_from_pixbuf(*pixbuf, false);
    }
    /* tile */
    for (int y = 0; y < screen->get_height(); y += background_image->get_height())
        for (int x = 0; x < screen->get_width(); x += background_image->get_width())
            screen->blit(*background_image, x, y);
}


void App::draw_window(int rx, int ry, int rw, int rh) {
    /* if we have a "redraw all", we cannot build on the fact that the
     * background stayed. so rather clear the screen befure drawing a window */
    if (screen->must_redraw_all_before_flip())
        clear_screen();

    int const size = screen->get_pixmap_scale();
    screen->fill_rect(rx - 2 * size, ry - 2 * size, rw + 4 * size, rh + 4 * size, GdColor::from_rgb(64, 64, 64));
    screen->fill_rect(rx - 1 * size, ry - 1 * size, rw + 2 * size, rh + 2 * size, GdColor::from_rgb(128, 128, 128));
    screen->fill_rect(rx, ry, rw, rh, GdColor::from_rgb(0, 0, 0));
}


void App::draw_scrollbar(int min, int current, int max) {
    /* positions in char heights */
    const int upper = 3, lower = 3;
    /* if nothing to scroll, return immediately. */
    if (max <= min)
        return;
    set_color(GD_GDASH_GRAY2);
    /* bar */
    screen->fill_rect(screen->get_width() - font_manager->get_font_width_narrow(), 
        upper * font_manager->get_font_height(),
        font_manager->get_font_width_narrow(),
        screen->get_height() - (upper+lower) * font_manager->get_font_height(),
        GdColor::from_rgb(0, 0, 0));
    /* up & down arrow */
    font_manager->blittext_n(screen->get_width() - font_manager->get_font_width_narrow(),
                screen->get_height() - (lower+1) * font_manager->get_font_height(), Printf("%c", GD_DOWN_CHAR).c_str());
    font_manager->blittext_n(screen->get_width() - font_manager->get_font_width_narrow(),
                upper * font_manager->get_font_height(), Printf("%c", GD_UP_CHAR).c_str());
    /* slider */
    double pos = current / double(max-min);
    pos = pos * (screen->get_height() - font_manager->get_font_height() * (upper+lower+2) - font_manager->get_font_height());
    pos = pos + font_manager->get_font_height() * (upper+1);
    font_manager->blittext_n(screen->get_width() - font_manager->get_font_width_narrow(),
                pos, Printf("%c", GD_FULL_BOX_CHAR).c_str());
}


void App::select_file_and_do_command(const char *title, const char *start_dir, const char *glob, bool for_save, const char *defaultname, std::unique_ptr<Command1Param<std::string>> command_when_successful) {
    enqueue_command(std::make_unique<PushActivityCommand>(this, std::make_unique<SelectFileActivity>(this, title, start_dir, glob, for_save, defaultname, std::move(command_when_successful))));
}


void App::ask_yesorno_and_do_command(char const *question, const char *yes_answer, char const *no_answer, std::unique_ptr<Command> command_when_yes, std::unique_ptr<Command> command_when_no) {
    enqueue_command(std::make_unique<PushActivityCommand>(this, std::make_unique<AskYesNoActivity>(this, question, yes_answer, no_answer, std::move(command_when_yes), std::move(command_when_no))));
}

void App::show_text_and_do_command(char const *title_line, std::string const &text, std::unique_ptr<Command> command_after_exit) {
    enqueue_command(std::make_unique<PushActivityCommand>(this, std::make_unique<ShowTextActivity>(this, title_line, text, std::move(command_after_exit))));
}


void App::show_settings(Setting *settings) {
    enqueue_command(std::make_unique<PushActivityCommand>(this, std::make_unique<SettingsActivity>(this, settings)));
}

void App::show_message(std::string const &primary, std::string const &secondary, std::unique_ptr<Command> command_after_exit) {
    enqueue_command(std::make_unique<PushActivityCommand>(this, std::make_unique<MessageActivity>(this, primary, secondary, std::move(command_after_exit))));
}

void App::show_help(helpdata const help_text[]) {
    // TRANSLATORS: 'H' uppercase because of title capitalization in English.
    show_text_and_do_command(_("GDash Help"), help_text_to_string(help_text));
}

void App::show_about_info() {
    // TRANSLATORS: about dialog box categories.
    struct String {
        char const *title;
        char const *text;
    } strings[] = {
        { _("About GDash"), _(About::comments) },
        { _("Copyright"), _(About::copyright) },
        { _("License"), _(About::license) },
        { _("Website"), _(About::website) },
        { NULL, NULL }
    };
    // TRANSLATORS: about dialog box categories.
    struct StringArray {
        char const *title;
        char const **texts;
    } stringarrays[] = {
        { _("Authors"), About::authors },
        { _("Artists"), About::artists },
        { _("Documenters"), About::documenters },
        { NULL, NULL }
    };
    std::string text;

    for (String *s = strings; s->title != NULL; ++s) {
        text += Printf("%c%s\n%c%s\n\n", GD_COLOR_INDEX_YELLOW, s->title, GD_COLOR_INDEX_LIGHTBLUE, s->text);
    }
    for (StringArray *s = stringarrays; s->title != NULL; ++s)  {
        text += Printf("%c%s%c\n", GD_COLOR_INDEX_YELLOW, s->title, GD_COLOR_INDEX_LIGHTBLUE);
        for (char const **t = s->texts; *t != NULL; ++t) {
            text += Printf("%c%c %c%s\n", GD_COLOR_INDEX_YELLOW, GD_PLAYER_CHAR, GD_COLOR_INDEX_LIGHTBLUE, *t);
        }
        text += '\n';
    }

    show_text_and_do_command(PACKAGE_NAME PACKAGE_VERSION, text);
}


void App::input_text_and_do_command(char const *title_line, char const *default_text, std::unique_ptr<Command1Param<std::string> > command_when_successful) {
    enqueue_command(std::make_unique<PushActivityCommand>(this, std::make_unique<InputTextActivity>(this, title_line, default_text, std::move(command_when_successful))));
}


/** select color for the next text drawing */
void App::set_color(const GdColor &color) {
    font_manager->set_color(color);
}


/** set status line (the last line in the screen) to text */
void App::status_line(const char *text) {
    font_manager->blittext_n(-1, screen->get_height() - font_manager->get_font_height(), GD_GDASH_GRAY2, text);
}


/** set title line (the first line in the screen) to text */
void App::title_line(const char *text) {
    font_manager->blittext_n(-1, 0, GD_GDASH_WHITE, text);
}


void App::timer_event(int ms_elapsed) {
    if (have_activity()) {
        topmost_activity().timer_event(ms_elapsed);
    }
    process_commands();
}


void App::timer2_event() {
    if (have_activity()) {
        topmost_activity().timer2_event();
        process_commands();
    }
}


void App::keypress_event(Activity::KeyCode keycode, int gfxlib_keycode) {
    /* send it to the gameinput object. */
    gameinput->keypress(gfxlib_keycode);

    /* and process it. */
    switch (keycode) {
        case F9:
            /* if we have sound, key f9 will push a volume activity.
             * that activity will receive the keypresses.
             * but only if not the volumeactivity app is active. */
#ifdef HAVE_SDL
            if (gd_sound_enabled && have_activity()) {
                try {
                    dynamic_cast<VolumeActivity&>(topmost_activity());
                } catch (std::bad_cast &) {
                    enqueue_command(std::make_unique<PushActivityCommand>(this, std::make_unique<VolumeActivity>(this)));
                }
            }
#endif // ifdef HAVE_SDL
            break;
        case F10:
            /* f10 is reserved for the menu in the gtk version. do not pass to the activity,
             * if we get this somehow. */
            break;
        case F11:
            /* fullscreen switch. do not pass to the app. */
            break;
        default:
            /* other key. pass to the activity. */
            if (have_activity()) {
                topmost_activity().keypress_event(keycode, gfxlib_keycode);
                process_commands();
            }
            break;
    }
}


void App::redraw_event(bool full) {
    if (have_activity()) {
        auto & activity = topmost_activity();
        activity.redraw_event(full);
        activity.redraw_queued = false;
    }
}


void App::quit_event() {
    if (quit_event_command)
        quit_event_command->execute();
}


/* process pending commands */
void App::process_commands() {
    while (!command_queue.empty()) {
        std::unique_ptr<Command> command = std::move(command_queue.front());
        command_queue.pop();
        command->execute();
    }

    if (running_activities.empty() && no_activity_command) {
        no_activity_command->execute();
        no_activity_command.reset();  /* process this one only once! */
    }
}


void App::request_restart() {
    request_restart_command->execute();
}


void App::start_editor() {
    start_editor_command->execute();
}


/* enqueue a command for executing after the event.
 * and empty command is allowed to be pushed, and has no effect. */
void App::enqueue_command(std::unique_ptr<Command> the_command) {
    if (the_command)
        command_queue.push(std::move(the_command));
}


void App::push_activity(std::unique_ptr<Activity> the_activity) {
    /* the currently topmost activity will be hidden by the new one */
    if (have_activity()) {
        topmost_activity().hidden_event();
    }
    /* push the new one to the top */
    auto & act = *the_activity;
    running_activities.push(std::move(the_activity));
    act.pushed_event();
    act.shown_event();
    redraw_event(true);
}


void App::pop_activity() {
    if (!running_activities.empty()) {
        auto popped_activity = std::move(running_activities.top());
        popped_activity->hidden_event();
        running_activities.pop();
        /* send a redraw to the topmost one */
        if (have_activity()) {
            topmost_activity().shown_event();
            redraw_event(true);
        }
    }
}


void App::pop_all_activities() {
    while (have_activity())
        pop_activity();
}


bool App::redraw_queued() const {
    if (have_activity())
        topmost_activity().redraw_queued;
    return have_activity();
}
