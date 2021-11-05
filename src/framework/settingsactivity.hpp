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

#ifndef SETTINGSACTIVITY_HPP_INCLUDED
#define SETTINGSACTIVITY_HPP_INCLUDED

#include <vector>
#include <string>

#include "framework/activity.hpp"

class Command;
class Setting;

class SettingsActivity: public Activity {
public:
    SettingsActivity(App *app, Setting *settings_data);
    ~SettingsActivity();
    void load_themes();
    virtual void keypress_event(KeyCode keycode, int gfxlib_keycode);
    virtual void redraw_event(bool full) const;

private:
    Setting *settings;
    unsigned numsettings, numpages, current, yd;
    std::vector<int> y1;
    std::vector<std::string> themes, shaders;
    bool have_theme;
    int themenum;
#ifdef HAVE_SDL
    int shadernum;
#endif
    bool restart;
};

#endif
