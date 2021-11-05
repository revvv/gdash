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
#ifndef EDITORAUTOWIDGETS_HPP_INCLUDED
#define EDITORAUTOWIDGETS_HPP_INCLUDED

#include "config.h"

#include <gtk/gtk.h>

class Reflective;
struct PropertyDescription;

class EditorAutoUpdate {
  public:
    Reflective *r;                      ///< A reflective object to work on
    Reflective *def;                    ///< A reflective object, if any, which stores default value
    PropertyDescription const *descr;   ///< A pointer to the description of the property this is working on
    GtkWidget *widget;                  ///< A widget to show on the screen. May not be the widget which stores data!
    bool expand_vertically;             ///< For longstrings - fill window with widget.

    void update() const {
        if (update_cb)
            update_cb();
    }
    void reload() const {
        if (reload_cb)
            reload_cb(widget);
    }
    EditorAutoUpdate(Reflective *r, Reflective *def, PropertyDescription const *descr, void (*update_cb)());
    
    void maximize_widget(EditorAutoUpdate &maximized);
    
    /* EditorAutoUpdate objects cannot be moved or copied,
     * as Gtk signals registered use their "this" pointers. */
    EditorAutoUpdate(EditorAutoUpdate &&) = delete;
    EditorAutoUpdate(EditorAutoUpdate const &) = delete;
    EditorAutoUpdate & operator=(EditorAutoUpdate const &) = delete;
    EditorAutoUpdate & operator=(EditorAutoUpdate &) = delete;

private:
    void (*update_cb)();                ///< A function to call when update happens
    void (*reload_cb)(GtkWidget *);     ///< A function to call for the widget when values have to be reloaded from the Reflective object
};

#endif
