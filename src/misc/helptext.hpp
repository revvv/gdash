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
#ifndef HELPTEXT_HPP_INCLUDED
#define HELPTEXT_HPP_INCLUDED

#include "config.h"

#include "cave/cavetypes.hpp"

/**
 * This is one paragraph in the help text.
 */
struct helpdata {
    /** GTK stock icon to draw, or null. */
    const char *stock_id;
    /** Heading (title of paragraph) to show, or null. */
    const char *heading;
    /** A key name on the keyboard, or NULL. */
    const char *keyname;
    /** GDash element to draw, or O_NONE. */
    GdElementEnum element;
    /** Text. */
    const char *description;
};

#define HELP_LAST_LINE "HELP_LAST_LINE"

extern helpdata const titlehelp[];
extern helpdata const gamehelp[];
extern helpdata const replayhelp[];
extern helpdata const editorhelp[];

#endif
