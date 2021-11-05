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
#ifndef EDITORWIDGETS_HPP_INCLUDED
#define EDITORWIDGETS_HPP_INCLUDED

#include "config.h"

#include <gtk/gtk.h>

#include "cave/cavetypes.hpp"

class GdColor;

GtkWidget *gd_color_combo_new(const GdColor &color);
void gd_color_combo_set(GtkComboBox *combo, const GdColor &color);
const GdColor &gd_color_combo_get_color(GtkWidget *widget);

void gd_element_button_set_dialog_title(GtkWidget *button, const char *title);
GdElementEnum gd_element_button_get(GtkWidget *button);
GtkWidget *gd_element_button_new(GdElementEnum initial_element, const char *special_title);
void gd_element_button_set(GtkWidget *button, GdElementEnum element);
void gd_element_button_update_pixbuf(GtkWidget *button);

GtkWidget *gd_direction_combo_new(GdDirectionEnum initial);
GdDirectionEnum gd_direction_combo_get_direction(GtkWidget *combo);

GtkWidget *gd_scheduling_combo_new(GdSchedulingEnum initial);
GdSchedulingEnum gd_scheduling_combo_get_scheduling(GtkWidget *combo);


#endif
