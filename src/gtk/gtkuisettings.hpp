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

#ifndef GTKUISETTINGS_HPP_INCLUDED
#define GTKUISETTINGS_HPP_INCLUDED

#include "config.h"

#include <gtk/gtk.h>

class Setting;
class PixbufFactory;

/**
 * A collection of functions which implement a settings dialog in GTK+.
 * A Setting array can be given, and the window will be automatically
 * generated and run modally.
 */
class SettingsWindow {
private:
    static void bool_toggle(GtkWidget *widget, gpointer data);
    static void int_change(GtkWidget *widget, gpointer data);
    static void double_change(GtkWidget *widget, gpointer data);
    static void stringv_change(GtkWidget *widget, gpointer data);
    static void theme_change(GtkWidget *widget, gpointer data);
    static GtkWidget *combo_box_new_from_stringv(const char **str);
    static GtkWidget *combo_box_new_from_themelist(std::vector<std::string> const &strings);
    static gboolean keysim_button_keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer data);
    static void keysim_button_clicked_cb(GtkWidget *button, gpointer data);
    static GtkWidget *gd_keysim_button(Setting *setting);
public:
    static bool do_settings_dialog(Setting *settings, PixbufFactory &pf);
};

#endif
