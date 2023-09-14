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

#include "config.h"

#include <memory>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include "gtk/gtkgameinputhandler.hpp"
#include "gtk/gtkmainwindow.hpp"
#include "gtk/gtkpixbuffactory.hpp"
#include "gtk/gtkscreen.hpp"
#include "gtk/gtkapp.hpp"
#include "framework/commands.hpp"
#include "framework/replaymenuactivity.hpp"
#include "framework/gameactivity.hpp"
#include "gtk/gtkui.hpp"
#include "settings.hpp"
#include "cave/caveset.hpp"
#include "cave/gamecontrol.hpp"
#include "misc/logger.hpp"
#include "misc/helptext.hpp"
#include "misc/autogfreeptr.hpp"


class GdMainWindow {
private:
    gulong focus_handler, keypress_handler, keyrelease_handler;
public:
    /* gtk ui part */
    GtkWidget *window, *drawing_area;

    /* gdash part */
    GTKApp *app;
    PixbufFactory *pbf;
    GTKScreen *screen;

    /* for the thread */
    GThread *timer_thread;
    bool quit_thread;
    int timer_events;
    int interval_msec;

    GtkWidget *menubar;
    CaveSet *caveset;
    bool fullscreen;
    NextAction &na;

    static gboolean main_window_set_fullscreen_idle_func(gpointer data);
    void main_window_set_fullscreen();

    static gboolean delete_event(GtkWidget *widget, GdkEventAny *event, gpointer data);
    static void help_cb(GtkWidget *widget, gpointer data);
    static void game_help_cb(GtkWidget *widget, gpointer data);
    static void volume_cb(GtkWidget *widget, gpointer data);
    static void preferences_cb(GtkWidget *widget, gpointer data);
    static void keyboard_preferences_cb(GtkWidget *widget, gpointer data);
    static void quit_cb(GtkWidget *widget, gpointer data);
    static void end_game_cb(GtkWidget *widget, gpointer data);
    static void random_colors_cb(GtkWidget *widget, gpointer data);
    static void take_snapshot_cb(GtkWidget *widget, gpointer data);
    static void revert_to_snapshot_cb(GtkWidget *widget, gpointer data);
    static void restart_level_cb(GtkWidget *widget, gpointer data);
    static void about_cb(GtkWidget *widget, gpointer data);
    static void open_caveset_cb(GtkWidget *widget, gpointer data);
    static void open_caveset_dir_cb(GtkWidget *widget, gpointer data);
    static void save_caveset_as_cb(GtkWidget *widget, gpointer data);
    static void save_caveset_cb(GtkWidget *widget, gpointer data);
    static void highscore_cb(GtkWidget *widget, gpointer data);
    static void statistics_cb(GtkWidget *widget, gpointer data);
    static void show_errors_cb(GtkWidget *widget, gpointer data);
    static void cave_editor_cb(GtkWidget *widget, gpointer data);
    static void recent_chooser_activated_cb(GtkRecentChooser *chooser, gpointer data);
    static void toggle_fullscreen_cb(GtkWidget *widget, gpointer data);
    static void pause_game_cb(GtkWidget *widget, gpointer data);
    static void show_replays_cb(GtkWidget *widget, gpointer data);
    static void cave_info_cb(GtkWidget *widget, gpointer data);

    static gboolean timing_event_idle_func(gpointer data);
    static gpointer timing_thread(gpointer data);

    GdMainWindow(bool add_menu, NextAction &na);
    ~GdMainWindow();
};


/* Menu UI */
static GtkActionEntry action_entries_normal[] = {
    {"PlayMenu", NULL, N_("Play")},
    {"FileMenu", NULL, N_("File")},
    {"HelpMenu", NULL, N_("Help")},
    {"Quit", GTK_STOCK_QUIT, NULL, "", NULL, G_CALLBACK(GdMainWindow::quit_cb)},
    {"Errors", GTK_STOCK_DIALOG_ERROR, N_("_Error console"), NULL, NULL, G_CALLBACK(GdMainWindow::show_errors_cb)},
    {"About", GTK_STOCK_ABOUT, NULL, NULL, NULL, G_CALLBACK(GdMainWindow::about_cb)},
    {"Help", GTK_STOCK_HELP, NULL, "", NULL, G_CALLBACK(GdMainWindow::help_cb)},
    {"GameHelp", GTK_STOCK_HELP, N_("Game help"), "", NULL, G_CALLBACK(GdMainWindow::game_help_cb)},
    {"CaveInfo", GTK_STOCK_DIALOG_INFO, N_("Caveset _information"), NULL, N_("Show information about the game and its caves"), G_CALLBACK(GdMainWindow::cave_info_cb)},
    {"GamePreferences", GTK_STOCK_PREFERENCES, NULL, NULL, NULL, G_CALLBACK(GdMainWindow::preferences_cb)},
    {"KeyboardPreferences", GD_ICON_KEYBOARD, N_("Keyboard settings"), NULL, N_("Set keyboard settings"), G_CALLBACK(GdMainWindow::keyboard_preferences_cb)},
#ifdef HAVE_SDL
    {"Volume", GTK_STOCK_PREFERENCES, N_("_Sound volume"), "F9", NULL, G_CALLBACK(GdMainWindow::volume_cb)},
#endif
    {"CaveEditor", GD_ICON_CAVE_EDITOR, N_("Cave _editor"), NULL, NULL, G_CALLBACK(GdMainWindow::cave_editor_cb)},
    {"OpenFile", GTK_STOCK_OPEN, NULL, "", NULL, G_CALLBACK(GdMainWindow::open_caveset_cb)},
    {"LoadRecent", GTK_STOCK_DIALOG_INFO, N_("Open _recent")},
    {"OpenCavesDir", GTK_STOCK_CDROM, N_("O_pen shipped"), NULL, NULL, G_CALLBACK(GdMainWindow::open_caveset_dir_cb)},
    {"SaveFile", GTK_STOCK_SAVE, NULL, "", NULL, G_CALLBACK(GdMainWindow::save_caveset_cb)},
    {"SaveAsFile", GTK_STOCK_SAVE_AS, NULL, NULL, NULL, G_CALLBACK(GdMainWindow::save_caveset_as_cb)},
    {"HighScore", GD_ICON_AWARD, N_("Hi_ghscores"), NULL, NULL, G_CALLBACK(GdMainWindow::highscore_cb)},
    {"PlayStatistics", GD_ICON_STATISTICS, N_("Game play statistics"), NULL, N_("Shows statistics of playing caves: times played, times played successfully etc."), G_CALLBACK(GdMainWindow::statistics_cb)},
    {"ShowReplays", GD_ICON_REPLAY, N_("Show _replays"), NULL, N_("List replays which are recorded for caves in this caveset"), G_CALLBACK(GdMainWindow::show_replays_cb)},
    {"FullScreen", GTK_STOCK_FULLSCREEN, NULL, "F11", N_("Fullscreen mode"), G_CALLBACK(GdMainWindow::toggle_fullscreen_cb)},
};

static GtkActionEntry action_entries_game[] = {
    {"TakeSnapshot", GD_ICON_SNAPSHOT, N_("_Take snapshot"), NULL, NULL, G_CALLBACK(GdMainWindow::take_snapshot_cb)},
    {"RevertToSnapshot", GTK_STOCK_UNDO, N_("_Revert to snapshot"), NULL, NULL, G_CALLBACK(GdMainWindow::revert_to_snapshot_cb)},
    {"RandomColors", GTK_STOCK_SELECT_COLOR, N_("Random _colors"), NULL, NULL, G_CALLBACK(GdMainWindow::random_colors_cb)},
    {"RestartLevel", GD_ICON_RESTART_LEVEL, N_("Re_start level"), NULL, N_("Restart current level"), G_CALLBACK(GdMainWindow::restart_level_cb)},
    {"PauseGame", GTK_STOCK_MEDIA_PAUSE, N_("_Pause game"), NULL, N_("Restart current level"), G_CALLBACK(GdMainWindow::pause_game_cb)},
    {"EndGame", GTK_STOCK_STOP, N_("_End game"), NULL, N_("End current game"), G_CALLBACK(GdMainWindow::end_game_cb)},
};


static const char *ui_info =
    "<ui>"
    "<menubar name='MenuBar'>"
    "<menu action='FileMenu'>"
    "<separator/>"
    "<menuitem action='OpenFile'/>"
    "<menuitem action='LoadRecent'/>"
    "<menuitem action='OpenCavesDir'/>"
    "<separator/>"
    "<menuitem action='SaveFile'/>"
    "<menuitem action='SaveAsFile'/>"
    "<separator/>"
    "<menuitem action='CaveInfo'/>"
    "<menuitem action='HighScore'/>"
    "<menuitem action='PlayStatistics'/>"
    "<menuitem action='ShowReplays'/>"
    "<menuitem action='CaveEditor'/>"
    "<separator/>"
    "<menuitem action='Quit'/>"
    "</menu>"
    "<menu action='PlayMenu'>"
    "<menuitem action='PauseGame'/>"
    "<menuitem action='TakeSnapshot'/>"
    "<menuitem action='RevertToSnapshot'/>"
    "<menuitem action='RestartLevel'/>"
    "<menuitem action='EndGame'/>"
    "<menuitem action='RandomColors'/>"
    "<separator/>"
#ifdef HAVE_SDL
    "<menuitem action='Volume'/>"
#endif
    "<menuitem action='FullScreen'/>"
    "<menuitem action='KeyboardPreferences'/>"
    "<menuitem action='GamePreferences'/>"
    "</menu>"
    "<menu action='HelpMenu'>"
    "<menuitem action='Help'/>"
    "<menuitem action='GameHelp'/>"
    "<separator/>"
    "<menuitem action='Errors'/>"
    "<menuitem action='About'/>"
    "</menu>"
    "</menubar>"
    "</ui>";


class SetNextActionAndGtkQuitCommand : public Command {
public:
    SetNextActionAndGtkQuitCommand(App *app, NextAction &na, NextAction to_what): Command(app), na(na), to_what(to_what) {}
private:
    virtual void execute() {
        na = to_what;
        gtk_main_quit();
    }
    NextAction &na;
    NextAction to_what;
};


gboolean GdMainWindow::main_window_set_fullscreen_idle_func(gpointer data) {
    gtk_window_fullscreen(GTK_WINDOW(data));
    return FALSE;  /* do not call again */
}


static GdkRGBA default_background_color;


static void backup_background_color(GtkWidget *window) {
    GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(window));
    gtk_style_context_get_background_color(context, GTK_STATE_FLAG_NORMAL, &default_background_color);
    const char* color_str = gdk_rgba_to_string(&default_background_color);
    gd_debug("GTK default background color: %s", color_str);
}


static void restore_background_color(GtkWidget *window) {
    if (default_background_color.red == 0
            && default_background_color.green == 0
            && default_background_color.blue == 0
            && default_background_color.alpha == 0) {
        // there is no color backup
        gd_warning("GTK default background color: restore failed");
        return;
    }
    gtk_widget_override_background_color(GTK_WIDGET(window), GTK_STATE_FLAG_NORMAL, &default_background_color);
}


static void set_black_background_color(GtkWidget *window) {
    GdkRGBA color;
    gdk_rgba_parse(&color, "#000000"); // black
    gtk_widget_override_background_color(GTK_WIDGET(window), GTK_STATE_FLAG_NORMAL, &color);
}


/* set or unset fullscreen if necessary */
/* hack: gtk-win32 does not correctly handle fullscreen & removing widgets.
   so we put fullscreening call into a low priority idle function, which will be called
   after all window resizing & the like did take place. */
void GdMainWindow::main_window_set_fullscreen() {
    if (fullscreen) {
        backup_background_color(window);
        set_black_background_color(window);
        gtk_widget_hide(menubar);
        g_idle_add_full(G_PRIORITY_LOW, (GSourceFunc) main_window_set_fullscreen_idle_func, window, NULL);
    } else {
        restore_background_color(window);
        gtk_window_unfullscreen(GTK_WINDOW(window));
        gtk_widget_show(menubar);
    }
}


gboolean GdMainWindow::delete_event(GtkWidget *widget, GdkEventAny *event, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->quit_event();
    return TRUE;
}


void GdMainWindow::cave_editor_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->start_editor();
}


void GdMainWindow::help_cb(GtkWidget *widget, gpointer data) {
    show_help_window(titlehelp, static_cast<GdMainWindow *>(data)->window);
}


void GdMainWindow::game_help_cb(GtkWidget *widget, gpointer data) {
    show_help_window(gamehelp, static_cast<GdMainWindow *>(data)->window);
}


void GdMainWindow::preferences_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->show_settings(gd_get_game_settings_array());
}


void GdMainWindow::keyboard_preferences_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->show_settings(gd_get_keyboard_settings_array(main_window->app->gameinput));
}


void GdMainWindow::volume_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->keypress_event(App::F9, 0);
}


void GdMainWindow::quit_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->quit_event();
}


void GdMainWindow::end_game_cb(GtkWidget *widget, gpointer data) {
    /* synthetic keypress */
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->keypress_event(GameActivity::EndGameKey, 0);
}


void GdMainWindow::take_snapshot_cb(GtkWidget *widget, gpointer data) {
    /* synthetic keypress */
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->keypress_event(GameActivity::TakeSnapshotKey, 0);
}


void GdMainWindow::revert_to_snapshot_cb(GtkWidget *widget, gpointer data) {
    /* synthetic keypress */
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->keypress_event(GameActivity::RevertToSnapshotKey, 0);
}


void GdMainWindow::random_colors_cb(GtkWidget *widget, gpointer data) {
    /* synthetic keypress */
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->keypress_event(GameActivity::RandomColorKey, 0);
}


void GdMainWindow::restart_level_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->gameinput->set_restart();
}


void GdMainWindow::about_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->show_about_info();
}

void GdMainWindow::open_caveset_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->enqueue_command(std::make_unique<SelectFileToLoadIfDiscardableCommand>(main_window->app, gd_last_folder));
}


void GdMainWindow::open_caveset_dir_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->enqueue_command(std::make_unique<SelectFileToLoadIfDiscardableCommand>(main_window->app, gd_system_caves_dir));
}


void GdMainWindow::save_caveset_as_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->enqueue_command(std::make_unique<SaveFileAsCommand>(main_window->app));
}


void GdMainWindow::save_caveset_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->enqueue_command(std::make_unique<SaveFileCommand>(main_window->app));
}


void GdMainWindow::highscore_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->enqueue_command(std::make_unique<ShowHighScoreCommand>(main_window->app, nullptr, -1));
}


void GdMainWindow::statistics_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->enqueue_command(std::make_unique<ShowStatisticsCommand>(main_window->app));
}


void GdMainWindow::show_errors_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->enqueue_command(std::make_unique<ShowErrorsCommand>(main_window->app, Logger::get_active_logger()));
}


/* called from the menu when a recent file is activated. */
void GdMainWindow::recent_chooser_activated_cb(GtkRecentChooser *chooser, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);

    GtkRecentInfo *current = gtk_recent_chooser_get_current_item(chooser);
    /* we do not support non-local files */
    if (!gtk_recent_info_is_local(current)) {
        AutoGFreePtr<char> display_name(gtk_recent_info_get_uri_display(current));
        gd_errormessage(_("GDash cannot load file from a network link."), display_name);
    } else {
        AutoGFreePtr<char> filename_utf8(gtk_recent_info_get_uri_display(current));
        AutoGFreePtr<char> filename(g_filename_from_utf8(filename_utf8, -1, NULL, NULL, NULL));
        main_window->app->enqueue_command(std::make_unique<AskIfChangesDiscardedCommand>(main_window->app, std::make_unique<OpenFileCommand>(main_window->app, (char*) filename)));
    }
    gtk_recent_info_unref(current);
}


void GdMainWindow::toggle_fullscreen_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->fullscreen = !main_window->fullscreen;
    main_window->main_window_set_fullscreen();
}


void GdMainWindow::pause_game_cb(GtkWidget *widget, gpointer data) {
    /* synthetic keypress */
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->keypress_event(GameActivity::PauseKey, 0);
}


void GdMainWindow::show_replays_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    App *app = main_window->app;
    app->enqueue_command(std::make_unique<RestartWithTitleScreenCommand>(app));
    app->enqueue_command(std::make_unique<PushActivityCommand>(app, std::make_unique<ReplayMenuActivity>(app)));
}


void GdMainWindow::cave_info_cb(GtkWidget *widget, gpointer data) {
    GdMainWindow *main_window = static_cast<GdMainWindow *>(data);
    main_window->app->enqueue_command(std::make_unique<ShowCaveInfoCommand>(main_window->app));
}


static Activity::KeyCode activity_keycode_from_gdk_keyval(guint keyval) {
    switch (keyval) {
        case GDK_KEY_Up:
            return App::Up;
        case GDK_KEY_Down:
            return App::Down;
        case GDK_KEY_Left:
            return App::Left;
        case GDK_KEY_Right:
            return App::Right;
        case GDK_KEY_Page_Up:
            return App::PageUp;
        case GDK_KEY_Page_Down:
            return App::PageDown;
        case GDK_KEY_Home:
            return App::Home;
        case GDK_KEY_End:
            return App::End;
        case GDK_KEY_F1:
            return App::F1;
        case GDK_KEY_F2:
            return App::F2;
        case GDK_KEY_F3:
            return App::F3;
        case GDK_KEY_F4:
            return App::F4;
        case GDK_KEY_F5:
            return App::F5;
        case GDK_KEY_F6:
            return App::F6;
        case GDK_KEY_F7:
            return App::F7;
        case GDK_KEY_F8:
            return App::F8;
        case GDK_KEY_F9:
            return App::F9;
        case GDK_KEY_BackSpace:
            return App::BackSpace;
        case GDK_KEY_Return:
            return App::Enter;
        case GDK_KEY_Tab:
            return App::Tab;
        case GDK_KEY_Escape:
            return App::Escape;

        default:
            return gdk_keyval_to_unicode(keyval);
    }
}


/* keypress and key release. pass it to the app framework. */
static gboolean main_window_keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    GTKApp *the_app = static_cast<GTKApp *>(data);
    bool press = event->type == GDK_KEY_PRESS; /* true for press, false for release */

    if (press) {
        Activity::KeyCode keycode = activity_keycode_from_gdk_keyval(event->keyval);
        the_app->keypress_event(keycode, event->keyval);
    } else {
        the_app->gameinput->keyrelease(event->keyval);
    }

    return FALSE;   /* TODO what to do here? */
}


/* focus leaves game play window. remember that keys are not pressed!
    as we don't receive key released events along with focus out. */
static gboolean main_window_focus_out_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
    GTKApp *the_app = static_cast<GTKApp *>(data);
    the_app->gameinput->clear_all_keypresses();
    return FALSE;
}


/* this function is an idle func which will run in the main thread.
 * the main thread has the applicaton, and also it is allowed to use gtk and the other libs. */
gboolean GdMainWindow::timing_event_idle_func(gpointer data) {
    GdMainWindow *win = static_cast<GdMainWindow *>(data);
    /* the number of events received since the last processing.
     * if the computer is slow, or the window is moved by the user with the mouse,
     * then this may be more than one. */
    int timer_events_received = win->timer_events;
    /* process the event; but only do it once, even if the counter is lagging. */
    if (timer_events_received > 0) {
        /* decrease. (maybe set to zero) */
        g_atomic_int_add(&win->timer_events, -timer_events_received);

        /* before calling the timer event, process pending gtk events. */
        /* if the computer is slow, and we are lagging behind, this helps keyboard
         * events to get to the app inside. */
        #ifdef G_OS_WIN32
        int i = 0;
        #endif
        while (gtk_events_pending()) {
            gtk_main_iteration();
            #ifdef G_OS_WIN32
            i++;
            if (i > 10000) {
                // WORKAROUND for https://gitlab.gnome.org/GNOME/gtk/-/issues/5688
                gd_warning("WORKAROUND: frozen window detected");
                break;
            }
            #endif
        }
        win->app->timer_event(win->interval_msec);

        /* now maybe redrawing. */
        if (win->app->redraw_queued()) {
            win->app->redraw_event(win->app->screen->must_redraw_all_before_flip());
        }
        if (win->app->screen->is_drawn()) {
            /* only flip if drawn something. */
            win->app->screen->do_the_flip();
        }
    }
    return FALSE;
}


/* this function will run in its own thread, and add the main int as an idle func in every 20 ms */
gpointer GdMainWindow::timing_thread(gpointer data) {
    GdMainWindow *win = static_cast<GdMainWindow *>(data);

    /* wait before we first call it */
    g_usleep(win->interval_msec * 1000);
    while (!win->quit_thread) {
        /* add processing as an idle func. no need to lock the main loop context, as glib does
         * that automatically. the idle func will run in the main thread, which is allowed to
         * access gtk+ and all the other libs. */
        g_atomic_int_inc(&win->timer_events);
        g_idle_add_full(G_PRIORITY_LOW, timing_event_idle_func, win, NULL);
        g_usleep(win->interval_msec * 1000);
    }
    /* when thread is quit, set timer events counter to zero, so the remaining idle
     * func will do nothing, even if it is already scheduled. */
    g_atomic_int_set(&win->timer_events, 0);
    return NULL;
}


GdMainWindow::GdMainWindow(bool add_menu, NextAction &na)
    : na(na) {
    this->fullscreen = false;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 320, 200);
    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(GdMainWindow::delete_event), this);

    /* vertical box */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    /* menu */
    GtkActionGroup *actions_game;
    if (add_menu) {
        GtkActionGroup *actions_normal = gtk_action_group_new("actions_normal");
        gtk_action_group_set_translation_domain(actions_normal, PACKAGE);
        gtk_action_group_add_actions(actions_normal, action_entries_normal, G_N_ELEMENTS(action_entries_normal), this);

        actions_game = gtk_action_group_new("actions_game");
        gtk_action_group_set_translation_domain(actions_game, PACKAGE);
        gtk_action_group_add_actions(actions_game, action_entries_game, G_N_ELEMENTS(action_entries_game), this);

        /* build the ui */
        GtkUIManager *ui = gtk_ui_manager_new();
        gtk_ui_manager_insert_action_group(ui, actions_normal, 0);
        gtk_ui_manager_insert_action_group(ui, actions_game, 0);
        gtk_window_add_accel_group(GTK_WINDOW(window), gtk_ui_manager_get_accel_group(ui));
        gtk_ui_manager_add_ui_from_string(ui, ui_info, -1, NULL);
        menubar = gtk_ui_manager_get_widget(ui, "/MenuBar");
        gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

        /* recent file chooser */
        GtkWidget *recent_chooser = gtk_recent_chooser_menu_new();
        GtkRecentFilter *recent_filter = gtk_recent_filter_new();
        /* gdash file extensions */
        for (int i = 0; gd_caveset_extensions[i] != NULL; i++)
            gtk_recent_filter_add_pattern(recent_filter, gd_caveset_extensions[i]);
        gtk_recent_chooser_add_filter(GTK_RECENT_CHOOSER(recent_chooser), recent_filter);
        gtk_recent_chooser_set_local_only(GTK_RECENT_CHOOSER(recent_chooser), TRUE);
        gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER(recent_chooser), 10);
        gtk_recent_chooser_set_sort_type(GTK_RECENT_CHOOSER(recent_chooser), GTK_RECENT_SORT_MRU);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(gtk_ui_manager_get_widget(ui, "/MenuBar/FileMenu/LoadRecent")), recent_chooser);
        g_signal_connect(G_OBJECT(recent_chooser), "item-activated", G_CALLBACK(GdMainWindow::recent_chooser_activated_cb), this);

        g_object_unref(ui);
    } else {
        actions_game = NULL;
    }

    /* drawing area for game */
    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_halign(drawing_area, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(drawing_area, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    gtk_window_present(GTK_WINDOW(window));

    /* initialize the app framework */
    pbf = new GTKPixbufFactory;
    screen = new GTKScreen(*pbf, drawing_area);
    app = new GTKApp(*screen, window, actions_game);
    /* attach events */
    focus_handler = g_signal_connect(G_OBJECT(window), "focus_out_event", G_CALLBACK(main_window_focus_out_event), app);
    keypress_handler = g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(main_window_keypress_event), app);
    keyrelease_handler = g_signal_connect(G_OBJECT(window), "key_release_event", G_CALLBACK(main_window_keypress_event), app);

    /* install timer. create a thread which will install idle funcs. */
    quit_thread = false;
    timer_events = 0;
    interval_msec = gd_fine_scroll ? 20 : 40; /* no fine scrolling supported. */
#ifndef G_THREADS_ENABLED
    #error Thread support in Glib must be enabled
#endif
    timer_thread = g_thread_new("timerfunc", timing_thread, this);
}


GdMainWindow::~GdMainWindow() {
    /* quit thread */
    quit_thread = true;
    g_thread_join(timer_thread);

    /* disconnect any signals and timeout handlers */
    g_signal_handler_disconnect(G_OBJECT(window), focus_handler);
    g_signal_handler_disconnect(G_OBJECT(window), keypress_handler);
    g_signal_handler_disconnect(G_OBJECT(window), keyrelease_handler);
    while (g_idle_remove_by_data(app))
        ; /* remove all */

    /* process all pending events before and after deleting the window. (there might be some generated by the destroying) */
    /* if the app is destroyed, there is noone to draw on the screen.
     * so destroy the window, and then the supporting screen/pixbuffactory objects. */
    while (gtk_events_pending())
        gtk_main_iteration();
    gtk_widget_destroy(window);
    while (gtk_events_pending())
        gtk_main_iteration();
    screen->set_title(NULL); // workaround: mark screen as destroyed

    delete app;
    delete screen;
    delete pbf;
}


void gd_main_window_gtk_run(CaveSet *caveset, NextAction &na) {
    GdMainWindow main_window(true, na);

    /* configure the app */
    main_window.app->caveset = caveset;
    main_window.app->push_activity(std::make_unique<TitleScreenActivity>(main_window.app));
    main_window.app->set_no_activity_command(std::make_unique<SetNextActionAndGtkQuitCommand>(main_window.app, na, Quit));
    main_window.app->set_quit_event_command(std::make_unique<AskIfChangesDiscardedCommand>(main_window.app, std::make_unique<PopAllActivitiesCommand>(main_window.app)));
    main_window.app->set_request_restart_command(std::make_unique<SetNextActionAndGtkQuitCommand>(main_window.app, na, Restart));
    main_window.app->set_start_editor_command(std::make_unique<SetNextActionAndGtkQuitCommand>(main_window.app, na, StartEditor));

    /* and run */
    gtk_main();
}


void gd_main_window_gtk_run_a_game(std::unique_ptr<GameControl> game) {
    NextAction na = StartTitle;      // because the funcs below need one to work with
    GdMainWindow main_window(false, na);

    /* configure */
    main_window.app->set_quit_event_command(std::make_unique<SetNextActionAndGtkQuitCommand>(main_window.app, na, Quit));
    main_window.app->set_no_activity_command(std::make_unique<SetNextActionAndGtkQuitCommand>(main_window.app, na, Quit));
    main_window.app->push_activity(std::make_unique<GameActivity>(main_window.app, std::move(game)));

    /* and run */
    gtk_main();
}
