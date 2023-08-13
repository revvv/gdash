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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "gtk/gtkui.hpp"
#include "gfx/cellrenderer.hpp"
#include "gtk/gtkuisettings.hpp"
#include "gtk/gtkpixbuffactory.hpp"
#include "settings.hpp"
#include "misc/printf.hpp"
#include "misc/autogfreeptr.hpp"
#include "framework/thememanager.hpp"
#ifdef HAVE_SDL
    #include "framework/shadermanager.hpp"
#endif

#define GDASH_KEYSIM_WHAT_FOR "gdash-keysim-what-for"
#define GDASH_RESTAT_BOOL "gdash-restart-bool"


/* return a list of image gtk_image_filter's. */
/* they have floating reference. */
/* the list is to be freed by the caller. */
static GList *image_load_filters()
{
    GSList *formats = gdk_pixbuf_get_formats();
    GList *filters = NULL;    /* new list of filters */

    GtkFileFilter *all_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(all_filter, _("All image files"));

    /* iterate the list of formats given by gdk. create file filters for each. */
    for (GSList *iter = formats; iter!=NULL; iter=iter->next) {
        GdkPixbufFormat *frm = (GdkPixbufFormat *) iter->data;
        if (gdk_pixbuf_format_is_disabled(frm))
            continue;
        GtkFileFilter *filter=gtk_file_filter_new();
        gtk_file_filter_set_name(filter, gdk_pixbuf_format_get_description(frm));
        char **extensions=gdk_pixbuf_format_get_extensions(frm);
        for (int i=0; extensions[i]!=NULL; i++) {
            std::string pattern = Printf("*.%s", extensions[i]);
            gtk_file_filter_add_pattern(filter, pattern.c_str());
            gtk_file_filter_add_pattern(all_filter, pattern.c_str());
        }
        g_strfreev(extensions);
        filters = g_list_append(filters, filter);
    }
    g_slist_free(formats);

    /* add "all image files" filter */
    filters = g_list_prepend(filters, all_filter);

    return filters;
}

/* file open dialog, with filters for image types gdk-pixbuf recognizes. */
char *
gd_select_image_file(const char *title, GtkWidget *parent)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(title, GTK_WINDOW(parent), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

    /* obtain list of image filters, and add all to the window */
    GList *filters = image_load_filters();
    for (GList *iter = filters; iter!=NULL; iter=iter->next)
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(iter->data));
    g_list_free(filters);

    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    char *filename = NULL;
    if (result == GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(dialog));
    gtk_widget_destroy (dialog);
    
    return filename;
}

static void add_theme_cb(GtkWidget *dialog, GtkWidget *themecombo, std::vector<std::string> & themes) {
    char *filename = gd_select_image_file(_("Add Theme from Image File"), dialog);
    
    if (filename == NULL)
        return;

    GTKPixbufFactory gpf;
    bool ok = CellRenderer::is_image_ok_for_theme(gpf, filename);
    if (ok) {
        /* make up new filename */
        AutoGFreePtr<char> basename(g_path_get_basename(filename));
        AutoGFreePtr<char> new_filename(g_build_path(G_DIR_SEPARATOR_S, gd_user_config_dir.c_str(), (char*) basename, NULL));

        /* if file not exists, or exists BUT overwrite allowed */
        if (!g_file_test(new_filename, G_FILE_TEST_EXISTS) || gd_question_yesno(_("Overwrite file?"), new_filename)) {
            /* copy theme to user config directory */
            try {
                GError *error = NULL;
                GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
                if (error)
                    throw error;
                gdk_pixbuf_save(pixbuf, new_filename, "png", &error, "compression", "9", NULL);
                if (error)
                    throw error;
                g_object_unref(pixbuf);
                AutoGFreePtr<char> thm(g_filename_display_basename(new_filename));
                if (strrchr(thm, '.'))    /* remove extension */
                    *strrchr(thm, '.') = '\0';
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(themecombo), thm);
                themes.push_back(std::string(new_filename));
                gd_infomessage(_("The new theme is installed."), thm);
            } catch (GError *error) {
                gd_errormessage(error->message, NULL);
                g_error_free(error);
            }
        }
    }
    else
        gd_errormessage(_("The selected image cannot be used as a GDash theme."), NULL);
    g_free(filename);
}


gboolean SettingsWindow::keysim_button_keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    g_assert(event->type == GDK_KEY_PRESS); /* must be true. */
    gtk_dialog_response(GTK_DIALOG(widget), event->keyval);
    return TRUE;    /* and say that we processed the key. */
}


void SettingsWindow::keysim_button_clicked_cb(GtkWidget *button, gpointer data) {
    const char *what_for = (const char *)g_object_get_data(G_OBJECT(button), GDASH_KEYSIM_WHAT_FOR);
    int *keyval = (int *) data;

    /* dialog which has its keypress event connected to the handler above */
    // TRANSLATORS: Title text capitalization in English
    GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Select Key"), GTK_WINDOW(gtk_widget_get_toplevel(button)),
                        GTK_DIALOG_MODAL,
                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 6);
    GtkWidget *label = gd_label_new_leftaligned(Printf(_("Press key for action: <b>%s</b>"), what_for).c_str());
    gtk_container_add(GTK_CONTAINER(content_area), label);
    g_signal_connect(G_OBJECT(dialog), "key_press_event", G_CALLBACK(keysim_button_keypress_event), dialog);

    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gtk_widget_show_all(dialog);
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result >= 0) {
        /* if positive, it must be a keyval. gtk_response_cancel and gtk_response delete is negative. */
        *keyval = result;
        gtk_button_set_label(GTK_BUTTON(button), gdk_keyval_name(*keyval));
    }
    gtk_widget_destroy(dialog);
}


GtkWidget *SettingsWindow::gd_keysim_button(Setting *setting) {
    char const *what_for = setting->name;
    int *keyval = (int *) setting->var;
    g_assert(keyval != NULL);

    /* the button shows the current value in its name */
    GtkWidget *button = gtk_button_new_with_label(gdk_keyval_name(*keyval));
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(keysim_button_clicked_cb), keyval);
    g_object_set_data(G_OBJECT(button), GDASH_KEYSIM_WHAT_FOR, (gpointer) what_for);
    gtk_widget_set_tooltip_text(button, Printf(_("Click here to set the key for action: %s"), what_for).c_str());

    return button;
}


/* settings window */
void SettingsWindow::bool_toggle(GtkWidget *widget, gpointer data) {
    bool *restart_bool = (bool *)g_object_get_data(G_OBJECT(widget), GDASH_RESTAT_BOOL);
    Setting *setting = static_cast<Setting *>(data);
    bool *bl = (bool *) setting->var;

    *bl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (setting->restart && restart_bool)
        *restart_bool = true;
}


void SettingsWindow::int_change(GtkWidget *widget, gpointer data) {
    bool *restart_bool = (bool *)g_object_get_data(G_OBJECT(widget), GDASH_RESTAT_BOOL);
    Setting *setting = static_cast<Setting *>(data);
    int *value = (int *) setting->var;
    *value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
    if (setting->restart && restart_bool)
        *restart_bool = true;
}


void SettingsWindow::double_change(GtkWidget *widget, gpointer data) {
    bool *restart_bool = (bool *)g_object_get_data(G_OBJECT(widget), GDASH_RESTAT_BOOL);
    Setting *setting = static_cast<Setting *>(data);
    double *value = (double *) setting->var;
    *value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
    if (setting->restart && restart_bool)
        *restart_bool = true;
}


void SettingsWindow::stringv_change(GtkWidget *widget, gpointer data) {
    bool *restart_bool = (bool *)g_object_get_data(G_OBJECT(widget), GDASH_RESTAT_BOOL);
    Setting *setting = static_cast<Setting *>(data);
    int *ptr = (int *) setting->var;
    *ptr = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    /* if nothing selected (for some reason), set to zero. */
    if (*ptr == -1)
        *ptr = 0;
    if (setting->restart && restart_bool)
        *restart_bool = true;
}


void SettingsWindow::theme_change(GtkWidget *widget, gpointer data) {
    bool *restart_bool = (bool *)g_object_get_data(G_OBJECT(widget), GDASH_RESTAT_BOOL);
    int *ptr = (int *) data;
    *ptr = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    /* if nothing selected (for some reason), set to zero. */
    if (*ptr == -1)
        *ptr = 0;
    if (restart_bool)
        *restart_bool = true;
}


GtkWidget *SettingsWindow::combo_box_new_from_stringv(const char **str) {
    GtkWidget *combo = gtk_combo_box_text_new();
    for (unsigned i = 0; str[i] != NULL; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), _(str[i])); // also translate
    return combo;
}


GtkWidget *SettingsWindow::combo_box_new_from_themelist(std::vector<std::string> const &strings) {
    GtkWidget *combo = gtk_combo_box_text_new();
    for (unsigned i = 0; i != strings.size(); i++) {
        if (strings[i] == "")
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), _("[Default]"));
        else {
            AutoGFreePtr<char> thm(g_filename_display_basename(strings[i].c_str()));
            if (strrchr(thm, '.'))    /* remove extension */
                *strrchr(thm, '.') = '\0';
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), thm);
        }
    }
    return combo;
}



/**
 * @return true, if a restart is required by some setting changed
 */
bool SettingsWindow::do_settings_dialog(Setting *settings, PixbufFactory &pf) {
    bool request_restart = false;

    // TRANSLATORS: Title text capitalization in English
    GtkWidget *dialog = gtk_dialog_new_with_buttons(_("GDash Preferences"), guess_active_toplevel(),
                        GTK_DIALOG_DESTROY_WITH_PARENT, NULL);

    /* notebook with tabs */
    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_set_border_width(GTK_CONTAINER(notebook), 6);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), notebook);

    GtkWidget *themecombo = NULL;
    std::vector<std::string> themes;
    int themenum;
    load_themes_list(pf, themes, themenum);
    gd_settings_array_prepare(settings, TypeTheme, themes, &themenum);
    
#ifdef HAVE_SDL
    std::vector<std::string> shaders;
    int shadernum;
    load_shaders_list(shaders, shadernum);
    gd_settings_array_prepare(settings, TypeShader, shaders, &shadernum);
#endif

    int row = 0;
    GtkWidget *grid = NULL;
    for (unsigned i = 0; settings[i].name != NULL; i++) {
        GtkWidget *widget = NULL;

        switch (settings[i].type) {
            case TypePage:
                grid = gtk_grid_new();
                gtk_container_set_border_width(GTK_CONTAINER(grid), 9);
                gtk_grid_set_row_spacing(GTK_GRID(grid), 9);
                gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
                gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid, gd_label_new_leftaligned(_(settings[i].name)));
                row = 0;
                break;

            case TypeBoolean:
                widget = gtk_check_button_new();
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), *(bool *)settings[i].var);
                gtk_grid_attach(GTK_GRID(grid), widget, 1, row, 1, 1);
                g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(SettingsWindow::bool_toggle), &settings[i]);
                break;

            case TypePercent:
                widget = gtk_spin_button_new_with_range(0, 100, 5);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), *(int *)settings[i].var);
                g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(SettingsWindow::int_change), &settings[i]);
                gtk_grid_attach(GTK_GRID(grid), widget, 1, row, 1, 1);
                break;

            case TypeInteger:
                widget = gtk_spin_button_new_with_range(settings[i].min, settings[i].max, 1);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), *(int *)settings[i].var);
                g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(SettingsWindow::int_change), &settings[i]);
                gtk_grid_attach(GTK_GRID(grid), widget, 1, row, 1, 1);
                break;

            case TypeTheme:
            case TypeStringv:
            case TypeShader:
                widget = SettingsWindow::combo_box_new_from_stringv(settings[i].stringv);
                gtk_combo_box_set_active(GTK_COMBO_BOX(widget), *(int *)settings[i].var);
                g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(SettingsWindow::stringv_change), &settings[i]);
                gtk_grid_attach(GTK_GRID(grid), widget, 1, row, 1, 1);
                break;

            case TypeKey:
                widget = gd_keysim_button(&settings[i]);
                gtk_grid_attach(GTK_GRID(grid), widget, 1, row, 1, 1);
                break;

            case TypeDouble:
                widget = gtk_spin_button_new_with_range(settings[i].min, settings[i].max, DOUBLE_STEP);
                gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 3);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), *(double *)settings[i].var);
                g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(SettingsWindow::double_change), &settings[i]);
                gtk_grid_attach(GTK_GRID(grid), widget, 1, row, 1, 1);
                break;
        }

        if (widget) {
            gtk_widget_set_hexpand(widget, TRUE);
            gtk_widget_set_tooltip_text(widget, _(settings[i].description));
            g_object_set_data(G_OBJECT(widget), GDASH_RESTAT_BOOL, &request_restart);
            GtkWidget *label = gd_label_new_leftaligned(_(settings[i].name));
            gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
            gtk_label_set_mnemonic_widget(GTK_LABEL(label), widget);
        }
        
        if (settings[i].type == TypeTheme)
            themecombo = widget;

        row ++;
    }
    
    constexpr int response_new_theme = 1;

    if (themecombo != NULL) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Add theme"), response_new_theme);
    }
    gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT);

    gtk_widget_show_all(dialog);
    while (true) {
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response == response_new_theme) {
            add_theme_cb(dialog, themecombo, themes);
        } else {
            break;
        }
    }
    gtk_widget_destroy(dialog);

    gd_theme = themes[themenum];
    gd_settings_array_unprepare(settings, TypeTheme);
#ifdef HAVE_SDL
    gd_shader = shaders[shadernum];
    gd_settings_array_unprepare(settings, TypeShader);
#endif

    return request_restart;
}
