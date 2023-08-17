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

#include <cmath>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "framework/gameactivity.hpp"
#include "framework/commands.hpp"
#include "gtk/gtkapp.hpp"
#include "settings.hpp"
#include "gtk/gtkscreen.hpp"
#include "gtk/gtkgameinputhandler.hpp"
#include "gtk/gtkpixbuffactory.hpp"
#include "gtk/gtkui.hpp"
#include "gtk/gtkuisettings.hpp"
#include "gfx/fontmanager.hpp"
#include "misc/autogfreeptr.hpp"
#include "misc/logger.hpp"


static double calculate_full_cave_scaling_factor_for_monitor() {
    GdkRectangle monitor = {0};
    gdk_monitor_get_workarea(gdk_display_get_primary_monitor(gdk_display_get_default()), &monitor);
    gd_debug("GTK screen size: %u x %u", monitor.width, monitor.height);
    int w = GAME_RENDERER_SCREEN_SIZE_MAX_X * 16;
    int h = GAME_RENDERER_SCREEN_SIZE_MAX_Y * 16;
    gd_debug("Cave size: %u x %u", w, h);
    double ratioW = (double) monitor.width / (double) w;
    double ratioH = (double) monitor.height / (double) h;
    double ratio = std::min(ratioW, ratioH);
    // ratio = x * DOUBLE_STEP + y
    double x = std::round(ratio / DOUBLE_STEP);
    double r = x * DOUBLE_STEP;
    if (r > ratio)
        r -= DOUBLE_STEP;
    int newWidth = (int) (w * r);
    int newHeight = (int) (h * r);
    gd_debug("GTK upscaled full cave size: %u x %u ratio=%f -> %f", newWidth, newHeight, ratio, r);
    gd_full_cave_scaling_factor = r;
    return r;
}


GTKApp::GTKApp(GTKScreen &screenref, GtkWidget *toplevel, GtkActionGroup *actions_game)
    :
    App(screenref),
    toplevel(toplevel),
    actions_game(actions_game) {
    if (gd_full_cave_view)
       screen->set_properties(calculate_full_cave_scaling_factor_for_monitor(), GdScalingType(gd_cell_scale_type_game), gd_pal_emulation_game);
    else
        screen->set_properties(gd_cell_scale_factor_game, GdScalingType(gd_cell_scale_type_game), gd_pal_emulation_game);
    gameinput = new GTKGameInputHandler;    /* deleted by the base class dtor */
    font_manager = new FontManager(*screen, "");    /* deleted by the base class dtor */
    game_active(false);
}


void GTKApp::select_file_and_do_command(const char *title, const char *start_dir, const char *glob, bool for_save, const char *defaultname, std::unique_ptr<Command1Param<std::string> > command_when_successful) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new(title, GTK_WINDOW(toplevel),
                        for_save ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN,
                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
    if (for_save)
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    if (defaultname && !g_str_equal(defaultname, ""))
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), defaultname);

    /* add file filter based on the glob given */
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, glob);
    char **globs = g_strsplit_set(glob, ";", -1);
    for (int i = 0; globs[i] != NULL; i++)
        gtk_file_filter_add_pattern(filter, globs[i]);
    g_strfreev(globs);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    /* if shipped with a directory name, show that directory by default */
    if (start_dir && !g_str_equal(start_dir, ""))
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), start_dir);

    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        /* give the filename to the command */
        AutoGFreePtr<char> filename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
        command_when_successful->set_param1(std::string(filename));
        enqueue_command(std::move(command_when_successful));
    }
    gtk_widget_destroy(dialog);
}


void GTKApp::ask_yesorno_and_do_command(char const *question, const char *yes_answer, char const *no_answer, std::unique_ptr<Command> command_when_yes, std::unique_ptr<Command> command_when_no) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(toplevel), GtkDialogFlags(0),
                        GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s", question);

    GtkWidget *buttonno = gtk_button_new_with_mnemonic(no_answer);
    gtk_button_set_image(GTK_BUTTON(buttonno), gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_BUTTON));
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), buttonno, GTK_RESPONSE_NO);

    GtkWidget *buttonyes = gtk_button_new_with_mnemonic(yes_answer);
    gtk_button_set_image(GTK_BUTTON(buttonyes), gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_BUTTON));
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), buttonyes, GTK_RESPONSE_YES);

    gtk_widget_show_all(dialog);
    bool yes = gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES;
    gtk_widget_destroy(dialog);

    if (yes)
        enqueue_command(std::move(command_when_yes));
    else
        enqueue_command(std::move(command_when_no));
}


void GTKApp::show_about_info() {
    gd_show_about_info();
}


void GTKApp::input_text_and_do_command(char const *title_line, char const *default_text, std::unique_ptr<Command1Param<std::string> > command_when_successful) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title_line, GTK_WINDOW(toplevel),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry), default_text);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content_area), entry, FALSE, FALSE, 6);

    gtk_widget_show_all(dialog);
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        command_when_successful->set_param1(gtk_entry_get_text(GTK_ENTRY(entry)));
        enqueue_command(std::move(command_when_successful));
    }
    gtk_widget_destroy(dialog);
}


void GTKApp::game_active(bool active) {
    if (actions_game != NULL)
        gtk_action_group_set_sensitive(actions_game, active);
}


void GTKApp::show_settings(Setting *settings) {
    bool restart_reqd = SettingsWindow::do_settings_dialog(settings, screen->pixbuf_factory);
    if (restart_reqd)
        request_restart();
}


/** remove GD_COLOR_SETCOLOR "markup" */
static std::string remove_gdash_markup(std::string const & text) {
    std::string text_stripped;
    for (unsigned i = 0; i != text.length(); ++i) {
        if (text[i] != GD_COLOR_SETCOLOR)
            text_stripped += text[i];
        else
            ++i;
    }
    return text_stripped;
}


void GTKApp::show_message(std::string const &primary, std::string const &secondary, std::unique_ptr<Command> command_after_exit) {
    gd_infomessage(remove_gdash_markup(primary).c_str(), remove_gdash_markup(secondary).c_str());
    enqueue_command(std::move(command_after_exit));
}

void GTKApp::show_help(helpdata const help_text[]) {
    show_help_window(help_text, (GtkWidget *) guess_active_toplevel());
}
