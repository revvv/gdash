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

#ifndef APP_HPP_INCLUDED
#define APP_HPP_INCLUDED

#include "config.h"

#include <glib.h>
#include <stack>
#include <queue>
#include <string>
#include <memory>

#include "framework/activity.hpp"
#include "gfx/pixmapstorage.hpp"

class GdColor;
class PixbufFactory;
class FontManager;
class Screen;
class Pixmap;
class CaveSet;
class Command;
class GameInputHandler;
class App;
class Activity;
class Setting;
struct helpdata;
template <typename T> class Command1Param;


std::string help_strings_to_string(char const **strings);

/// @ingroup Framework
/**
 * The App object manages Activity objects by organizing them in a stack,
 * and always sending timer and user events to the topmost one.
 *
 * The App object is responsible for the activity objects. New activities
 * are can be pushed into it, thus they become the topmost one. The
 * activities should be dynamically allocated, and the App will delete
 * them when they are no longer needed (they are popped from the stack).
 *
 * The normal use of the App class is creating an App object and pushing
 * the first activity onto its stack. This first activity is then receiving
 * the events, and is also allowed to create new activities. All timer,
 * keyboard and redraw events which are received by the owner of the app
 * object by some operating system or graphics library dependent way, must
 * be forwarded to the App using one of its event() methods. The app will
 * then send it to the topmost activity. The app also generates special
 * events for the activities when it manages them; for example, when an
 * activity becomes the topmost one or it is occluded by a new one, it is
 * notified.
 *
 * The activities are enabled to create Command objects and enqueue them
 * in the app. The app will execute these commands after any kind of event.
 *
 * In order to make an App object work, it must be assigned many helper
 * objects which do the OS and graphics library abstraction. These objects
 * are a Screen, a GameInputHandler, a FontManager and a PixbufFactory. The
 * activities inside the application are allowed - and required - to use
 * these objects to do their work.
 *
 * The App object also provides simple drawing methods not implemented anywhere
 * else, like clearing the screen, setting the color of the next text written,
 * or drawing a status line.
 * @todo Move the drawing commands in the App class to the Screen class?
 *
 * The App object can tell its owner when the last activity exited (and
 * therefore the app became defunct) or when one of the activities requested
 * a complete restart of the application. This is implemented using Command
 * objects, which can be assigned to such events.
 *
 * The App object provides functions for common tasks like selecting a file
 * or showing a piece of text. By default, these functions create the corresponding
 * activity. However, derived classes of the App class are allowed to override
 * these functions to specialize the user interface: for example, a GTK+
 * specialization can provide the builtin file selection dialog of GTK+ instead of
 * the one implemented by the SelectFileActivity class.
 */
class App : public PixmapStorage {
public:
    /**
     * These keycodes are an abstraction to the keycodes provided by the graphics
     * library. The owner of the App object should translate the pressing of these
     * keys to the codes, so the Activity objects can work with them, regardless of
     * the actual graphics library. */
    enum KeyCodeSpecialKey {
        Unknown = 0,
        Up = 1,
        Down = 2,
        Left = 3,
        Right = 4,
        PageUp = 5,
        PageDown = 6,
        Home = 7,
        End = 8,
        Tab = 9,

        F1 = 11,
        F2 = 12,
        F3 = 13,
        F4 = 14,
        F5 = 15,
        F6 = 16,
        F7 = 17,
        F8 = 18,
        F9 = 19,     ///< reserved for volume
        F10 = 20,    ///< reserved for gtk+ menu
        F11 = 21,    ///< reserved for fullscreen
        F12 = 22,    ///< reserved for fake restart

        BackSpace = 25,
        Enter = 26,
        Escape = 27,
    };

    /** Constructor.
     * After construction, the app should also be provided with a PixbufFactory,
     * a Screen, a GameInputHandler and a FontManager by simple assignment to the
     * corresponding pointer members. These will be deleted upon the destruction
     * of the app object. The app object might also be assigned a caveset, in case
     * the activities in it use it. */
    App(Screen &screenref);
    
    /** Destructor.
     * Deletes the screen, the gameinputhandler, the fontmanager and the pixbuffactory. */
    virtual ~App();
    
    /** Set a Command to be executed when the app sees that it finished its work.
     * This happens when there are no activities in it and also there are no commands
     * in its queue to execute. */
    void set_no_activity_command(std::unique_ptr<Command> command);
    
    /** Set the Command which is executed when the App is sent a quit event by calling
     * its App::quit_event() method. The task of the command object which is to be set
     * here is to decide what to do when the user closes the application. It might pop
     * all activities (therefore forcing the App to quit) or it might ask the user if
     * he wants to save his work. */
    void set_quit_event_command(std::unique_ptr<Command> command);
    
    /** Set the command to be executed when the App::request_restart() method is called.
     * Activities can call this method to request their running enviroment to restart,
     * for example because the sound card settings are changed or because a new
     * PixbufFactory is to be created. */
    void set_request_restart_command(std::unique_ptr<Command> command);
    
    /** Set the command to be executed on an App::start_editor() call. */
    void set_start_editor_command(std::unique_ptr<Command> command);
    
    /** The app stores a Pixmap (the background), so it is a PixmapStorage. */
    virtual void release_pixmaps();

    /* drawing */
    /** Clear the screen with a nice dark background. */
    void clear_screen();
    /** Set the color of the next text drawn. */
    void set_color(const GdColor &color);
    /** Set the title line, which is the topmost line on the screen. */
    void title_line(const char *text);
    /** Set the status line, which is the bottom line on the screen. */
    void status_line(const char *text);
    /** Draw a black window with a small frame. */
    void draw_window(int rx, int ry, int rw, int rh);
    /** Draw scrollbar. */
    void draw_scrollbar(int min, int current, int max);
    
    /* customizable ui features */
    /** Create a file selection dialog, and when the user accepts
     * the selection, parametrize the command with the name of the file and execute it.
     * By default, this creates a SelectFileActivity, but derived classes can override it.
     * @param title The title of the file selection dialog.
     * @param start_dir The directory in which the file selection should start.
     * @param glob A semicolon-separated list of file name globs to select the files to show, eg. *.bd;*.gds.
     * @param for_save If the purpose of the file selection is saving a file.
     * @param command_when_successful The Command to be executed when the file selection is successful. */
    virtual void select_file_and_do_command(const char *title, const char *start_dir, const char *glob, bool for_save, const char *defaultname, std::unique_ptr<Command1Param<std::string> > command_when_successful);
    
    /** Ask the user a simple yes or no question, and then execute the corresponding Command.
     * By default, this creates an AskYesNoActivity, but derived classes can override it.
     * @param question The text of the question.
     * @param yes_answer The text of the yes-answer.
     * @param no_answer The text of the no-answer.
     * @param command_when_yes The command to be executed if the user said yes. May be NULL.
     * @param command_when_no The command to be executed if the user said no. May be NULL. */
    virtual void ask_yesorno_and_do_command(char const *question, const char *yes_answer, char const *no_answer, std::unique_ptr<Command> command_when_yes, std::unique_ptr<Command> command_when_no = nullptr);
    
    /** Show a long text to the user. The text may contain line breaks and color setting
     * codes interpreted by the FontManager::blittext() routine. It is also wrapped to
     * fit the width of the screen.
     * By default, this creates a ShowTextActivity, but derived classes can override it.
     * @param title_line The title of the window.
     * @param text The text to be shown.
     * @param command_after_exit The Command to be executed after the activity is closed. May be omitted. */
    virtual void show_text_and_do_command(char const *title_line, std::string const &text, std::unique_ptr<Command> command_after_exit = nullptr);
    
    /** Show an about dialog with data form the About class.
     * By default, this creates a ShowTextActivity with the text, but derived classes can override it. */
    virtual void show_about_info();
    
    /** Ask the user to type one line of text, and when successful, call the Command parametrized with the text.
     * @param title_line The title of the window.
     * @param default_text The default value of the text box.
     * @param command_when_successful The command of one string parameter to be parametrized with the line
     *        typed and executed, if the user accepts the input. */
    virtual void input_text_and_do_command(char const *title_line, char const *default_text, std::unique_ptr<Command1Param<std::string> > command_when_successful);
    
    /** This method is to be called by Activity object to tell the App if a game is running.
     * By default, it does nothing, however derived classes may override it. */
    virtual void game_active(bool active) {}
    
    /** Show the settings screen for the array of settable parameters.
     * @param settings Array of settable parameters, delimited with an item with a NULL name. */
    virtual void show_settings(Setting *settings);
    
    /** Show a short message in a small window.
     * @param primary The message to show - first part.
     * @param secondary The message to show - second part, additional info.
     * @param command_after_exit A command to execute when the user acknowledged the text. May be omitted. */
    virtual void show_message(std::string const &primary, std::string const &secondary = "", std::unique_ptr<Command> command_after_exit = nullptr);
    
    /** Show help text.
     * @param help_text The strings of the help text. See the helpdata struct. */
    virtual void show_help(helpdata const help_text[]);


    /* events */
    /** See Activity::timer_event(). */
    void timer_event(int ms_elapsed);
    /** See Activity::timer2_event(). */
    void timer2_event();
    /**
     * To be called from the running environment when the user presses a key.
     * The keypresses are preprocessed - not all keypresses will get through to the
     * topmost activity. Also see Activity::keypress_event().
     * @param keycode A unicode character code, or some special key (see Activity::KeyCodeSpecialKey)
     * @param gfxlib_keycode Graphics library (sdl/gtk/etc) specific keycode
     */
    void keypress_event(Activity::KeyCode keycode, int gfxlib_keycode);
    /** See Activity::redraw_event(). */
    void redraw_event(bool full);
    /** To be called when the user closes the window of the game. The quit_event_command
     * set via the set_quit_event_command() will be executed. */
    void quit_event();
    
    /**
     * Pass the event to the topmost activity.
     */
    void textinput_event(char *appendtext) {
        if (have_activity())
            topmost_activity().textinput_event(appendtext);
    }
    
    /**
     * Pass the event to the topmost activity.
     */
    void textediting_event(char *overwritetext) {
        if (have_activity())
            topmost_activity().textediting_event(overwritetext);
    }

    /* handling activities and commands */
    /** Push a newly allocated activity on the top of the activity stack.
     * Before pushing, the old topmost activity is sent an Activity::hidden_event(),
     * and the new activity will be sent an Activity::push_event and an Activity::shown_event(),
     * and finally an Activity::redraw_event(). When popped, the activity will be automatically deleted.
     * @param the_activity The Activity to push. */
    void push_activity(std::unique_ptr<Activity>);
    
    /** Pop the topmost activity. Before popping, a hidden_event() is sent. */
    void pop_activity();
    
    /** Pop all activities. Can be used when restarting the application from the
     * title screen. The no activities command is not necessarily executed, because
     * there may be some commands left in the queue. */
    void pop_all_activities();
    
    /** Put a command to be executed after processing the event in the queue.
     * The commands will be executed in the order of queueing. Command objects
     * are handled via smart pointers; if a pointer to a newly allocated Command
     * is given as the parameter, it will be automatically deleted.
     * @param the_command The command to be executed later. */
    void enqueue_command(std::unique_ptr<Command> the_command);
    
    /** An Activity may call this to trigger executing the restart Command. */
    void request_restart();
    
    /** An Activity may call this to trigger executing the editor Command. */
    void start_editor();
    
    /** Return true if the topmost activity queued a redraw. */
    bool redraw_queued() const;

    /* supporting objects */
    /** The FontManager to be used for drawing texts. Will be deleted by ~App.
     * Should use the pixbuf_factory assigned to the App. */
    FontManager *font_manager;
    
    /** The Screen to draw on. Will be deleted by ~App. */
    Screen *screen;
    
    /** The CaveSet. For Activity objects. */
    CaveSet *caveset;
    
    /** Joystick & keyboard input, for the Activity objects. Will be deleted by ~App. */
    GameInputHandler *gameinput;

private:
    /** The image used by App::clear_screen(). */
    std::unique_ptr<Pixmap> background_image;
    
    /** The stack of the Activity objects handled. */
    std::stack<std::unique_ptr<Activity>> running_activities;
    
    /** Check if we have an activity. */
    bool have_activity() const {
        return !running_activities.empty();
    }
    
    /** The commands to be executed after the processing of the events. */
    std::queue<std::unique_ptr<Command>> command_queue;
    
    /** The command to be executed when all activities have quit. */
    std::unique_ptr<Command> no_activity_command;
    
    /** The command triggered by App::quit_event(). */
    std::unique_ptr<Command> quit_event_command;
    
    /** The command triggered by App::request_restart(). */
    std::unique_ptr<Command> request_restart_command;
    
    /** The command triggered by App::start_editor(). */
    std::unique_ptr<Command> start_editor_command;

protected:
    /** Process the commands in the queue. After all commands, check if
     * there are no activities left; if so, call the no_activity_command.
     * To be used by the App and descendants. */
    void process_commands();
    
    /** Returns the topmost activity. This is the one processing events. */
    Activity & topmost_activity() const {
        return *running_activities.top();
    }
};

#endif
