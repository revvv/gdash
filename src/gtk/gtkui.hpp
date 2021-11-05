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
#ifndef GTKUI_HPP_INCLUDED
#define GTKUI_HPP_INCLUDED

#include "config.h"

#include <gtk/gtk.h>

class CaveStored;
class CaveSet;
class Logger;
class Setting;
class PixbufFactory;
struct helpdata;

#define GD_ICON_CAVE_EDITOR "cave-editor"
#define GD_ICON_EDITOR_MOVE "editor-move"
#define GD_ICON_EDITOR_JOIN "editor-add-join"
#define GD_ICON_EDITOR_POINT "editor-add-point"
#define GD_ICON_EDITOR_FREEHAND "editor-add-freehand"
#define GD_ICON_EDITOR_LINE "editor-add-line"
#define GD_ICON_EDITOR_RECTANGLE "editor-add-rectangle"
#define GD_ICON_EDITOR_FILLRECT "editor-add-filled-rectangle"
#define GD_ICON_EDITOR_RASTER "editor-add-raster"
#define GD_ICON_EDITOR_FILL_BORDER "editor-add-fill-border"
#define GD_ICON_EDITOR_FILL_REPLACE "editor-add-fill-replace"
#define GD_ICON_EDITOR_MAZE "editor-add-maze"
#define GD_ICON_EDITOR_MAZE_UNI "editor-add-maze-uni"
#define GD_ICON_EDITOR_MAZE_BRAID "editor-add-maze-braid"
#define GD_ICON_SNAPSHOT "snapshot"
#define GD_ICON_RESTART_LEVEL "restart-level"
#define GD_ICON_RANDOM_FILL "random-fill"
#define GD_ICON_REMOVE_MAP "remove-map"
#define GD_ICON_FLATTEN_CAVE "flatten-cave"
#define GD_ICON_AWARD "icon-award"
#define GD_ICON_TO_TOP "icon-to-top"
#define GD_ICON_TO_BOTTOM "icon-to-botton"
#define GD_ICON_OBJECT_ON_ALL "icon-object-on-all"
#define GD_ICON_OBJECT_NOT_ON_ALL "icon-object-not-on-all"
#define GD_ICON_OBJECT_NOT_ON_CURRENT "icon-object-not-on-current"
#define GD_ICON_REPLAY "icon-replay"
#define GD_ICON_KEYBOARD "icon-keyboard"
#define GD_ICON_IMAGE "icon-image"
#define GD_ICON_STATISTICS "icon-statistics"

GdkPixbuf *gd_pixbuf_for_builtin_theme();
GdkPixbuf *gd_icon();
void gd_register_stock_icons();

GtkWindow *guess_active_toplevel();

void gd_show_errors(Logger &l, const char *title, bool always_show = false);

void gd_save_caveset_as(CaveSet &caveset);
void gd_save_caveset(CaveSet &caveset);
void gd_open_caveset(const char *directory, CaveSet &caveset);

bool gd_question_yesno(const char *primary, const char *secondary);
bool gd_discard_changes(CaveSet const &caveset);

void gd_warningmessage(const char *primary, const char *secondary);
void gd_errormessage(const char *primary, const char *secondary);
void gd_infomessage(const char *primary, const char *secondary);

void gd_show_about_info();

GtkWidget *gd_label_new_leftaligned(const char *markup);
GtkWidget *gd_label_new_centered(const char *markup);
GtkWidget *gd_label_new_rightaligned(const char *markup);
void gd_dialog_add_hint(GtkDialog *dialog, const char *hint);

char *gd_select_image_file(const char *title);

GtkWindow *guess_active_toplevel();

void show_help_window(const helpdata help_text[], GtkWidget *parent);

#endif
