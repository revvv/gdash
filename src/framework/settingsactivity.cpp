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

#include <cstring>
#include <glib/gi18n.h>

#include "framework/settingsactivity.hpp"
#include "framework/commands.hpp"
#include "framework/app.hpp"
#include "settings.hpp"
#include "cave/colors.hpp"
#include "gfx/screen.hpp"
#include "gfx/fontmanager.hpp"
#include "input/gameinputhandler.hpp"
#include "gfx/cellrenderer.hpp"
#include "misc/logger.hpp"
#include "misc/util.hpp"
#include "misc/autogfreeptr.hpp"
#include "framework/thememanager.hpp"
#ifdef HAVE_SDL
    #include "framework/shadermanager.hpp"
#endif


class SelectKeyActivity: public Activity {
public:
    SelectKeyActivity(App *app, char const *title, char const *action, int *pkeycode)
        : Activity(app),
          title(title), action(action), pkeycode(pkeycode) {
    }
    virtual void keypress_event(KeyCode keycode, int gfxlib_keycode);
    virtual void redraw_event(bool full) const;

private:
    std::string title, action;
    int *pkeycode;
};


void SelectKeyActivity::keypress_event(KeyCode keycode, int gfxlib_keycode) {
    *pkeycode = gfxlib_keycode;
    app->enqueue_command(std::make_unique<PopActivityCommand>(app));
}


void SelectKeyActivity::redraw_event(bool full) const {
    int height = 6 * app->font_manager->get_line_height();
    int y1 = (app->screen->get_height() - height) / 2; /* middle of the screen */
    int cx = 2 * app->font_manager->get_font_width_narrow(), cy = y1, cw = app->screen->get_width() - 2 * cx, ch = height;

    app->draw_window(cx, cy, cw, ch);
    app->screen->set_clip_rect(cx, cy, cw, ch);
    app->set_color(GD_GDASH_WHITE);
    app->font_manager->blittext_n(-1, y1 + app->font_manager->get_line_height(), title.c_str());
    app->font_manager->blittext_n(-1, y1 + 3 * app->font_manager->get_line_height(), action.c_str());
    app->screen->remove_clip_rect();

    app->screen->drawing_finished();
}


class ThemeSelectedCommand: public Command1Param<std::string> {
public:
    ThemeSelectedCommand(App *app, SettingsActivity *activity)
        :
        Command1Param<std::string>(app),
        activity(activity),
        filename(p1) {
    }

private:
    SettingsActivity *activity;
    std::string &filename;
    virtual void execute() {
        Logger l;
        if (CellRenderer::is_image_ok_for_theme(app->screen->pixbuf_factory, filename.c_str())) {
            /* copy theme to user config directory */
            std::string new_name = filename_for_new_theme(filename.c_str());
            install_theme(filename.c_str(), new_name.c_str());
            activity->load_themes();
        } else {
            /* if file is not ok as a theme, error was logged */
        }

        if (!l.empty()) {
            app->show_message(_("Cannot install theme!"), l.get_messages_in_one_string());
            l.clear();
        }
    }
};


void SettingsActivity::load_themes() {
    gd_settings_array_unprepare(settings, TypeTheme);
    load_themes_list(app->screen->pixbuf_factory, themes, themenum);
    gd_settings_array_prepare(settings, TypeTheme, themes, &themenum);
}


SettingsActivity::SettingsActivity(App *app, Setting *settings_data)
    : Activity(app) {
    /* copy the pointer into the member variable */
    settings = settings_data;
    for (numsettings = 0; settings[numsettings].name != NULL; ++numsettings)
        ;
    numpages = settings[numsettings - 1].page + 1;  /* number of pages: take it from the last setting */

    load_themes();
#ifdef HAVE_SDL
    load_shaders_list(shaders, shadernum);
    gd_settings_array_prepare(settings, TypeShader, shaders, &shadernum);
#endif

    /* check the settings, calculate sizes etc. */
    yd = app->font_manager->get_line_height();
    y1.resize(numpages);
    for (unsigned page = 0; page < numpages; page++) {
        int num = 0;
        for (unsigned n = 0; n < numsettings; n++)
            if (settings[n].page == page)
                num++;
        y1[page] = (app->screen->get_height() - num * yd) / 2;
    }
    current = 1;    /* 0th is presumably a page identifier */
    restart = false;
    /* check if a theme settings is included */
    have_theme = false;
    for (unsigned n = 0; n < numsettings; n++)
        if (settings[n].type == TypeTheme)
            have_theme = true;
}


SettingsActivity::~SettingsActivity() {
    gd_theme = themes[themenum];
    gd_settings_array_unprepare(settings, TypeTheme);
#ifdef HAVE_SDL
    gd_shader = shaders[shadernum];
    gd_settings_array_unprepare(settings, TypeShader);
#endif
    if (restart)
        app->request_restart();
}


void SettingsActivity::keypress_event(KeyCode keycode, int gfxlib_keycode) {
    unsigned page = settings[current].page;
    switch (keycode) {
        case App::Up:
            current = gd_clamp(current - 1, 0, numsettings - 1);
            while (current > 0 && settings[current].type == TypePage)
                current--;
            break;
        case App::Down:
            current = gd_clamp(current + 1, 0, numsettings - 1);
            break;
        case App::PageUp:
            if (page > 0)
                while (settings[current].page == page)
                    current--;    /* decrement until previous page is found */
            break;
        case App::PageDown:
            if (page < numpages - 1)
                while (settings[current].page == page)
                    current++;    /* increment until previous page is found */
            break;

            /* CHANGE SETTINGS */
        case App::Left:    /* key left */
            switch (settings[current].type) {
                case TypePage:
                    break;
                case TypeBoolean:
                    *(bool *)settings[current].var = false;
                    break;
                case TypeInteger:
                    *(int *)settings[current].var = gd_clamp(*(int *)settings[current].var - 1, settings[current].min, settings[current].max);
                    break;
                case TypePercent:
                    *(int *)settings[current].var = gd_clamp(*(int *)settings[current].var - 5, 0, 100);
                    break;
                case TypeShader:
                case TypeTheme:
                case TypeStringv:
                    *(int *)settings[current].var = gd_clamp(*(int *)settings[current].var - 1, 0, g_strv_length((char **) settings[current].stringv) - 1);
                    break;
                case TypeKey:
                    break;
                case TypeDouble:
                    *(double *)settings[current].var = gd_clamp_double(*(double *)settings[current].var - DOUBLE_STEP, settings[current].min, settings[current].max);
                    break;
            }
            if (settings[current].restart)
                restart = true;
            break;

        case App::Right:    /* key right */
            switch (settings[current].type) {
                case TypePage:
                    break;
                case TypeBoolean:
                    *(bool *)settings[current].var = true;
                    break;
                case TypeInteger:
                    *(int *)settings[current].var = gd_clamp(*(int *)settings[current].var + 1, settings[current].min, settings[current].max);
                    break;
                case TypePercent:
                    *(int *)settings[current].var = gd_clamp(*(int *)settings[current].var + 5, 0, 100);
                    break;
                case TypeTheme:
                case TypeStringv:
                case TypeShader:
                    *(int *)settings[current].var = gd_clamp(*(int *)settings[current].var + 1, 0, g_strv_length((char **) settings[current].stringv) - 1);
                    break;
                case TypeKey:
                    break;
                case TypeDouble:
                    *(double *)settings[current].var = gd_clamp_double(*(double *)settings[current].var + DOUBLE_STEP, settings[current].min, settings[current].max);
                    break;
            }
            if (settings[current].restart)
                restart = true;
            break;

        case ' ':
        case App::Enter:
            switch (settings[current].type) {
                case TypePage:
                    break;
                case TypeBoolean:
                    *(bool *)settings[current].var = !*(bool *)settings[current].var;
                    break;
                case TypeInteger:
                    *(int *)settings[current].var = (*(int *)settings[current].var + 1 - settings[current].min)
                        % (settings[current].max-settings[current].min+1) + settings[current].min;
                    break;
                case TypePercent:
                    *(int *)settings[current].var = gd_clamp(*(int *)settings[current].var + 5, 0, 100);
                    break;
                case TypeShader:
                case TypeTheme:
                case TypeStringv:
                    *(int *)settings[current].var = (*(int *)settings[current].var + 1) % g_strv_length((char **) settings[current].stringv);
                    break;
                case TypeKey:
                    // TRANSLATORS: 35 chars max
                    app->enqueue_command(std::make_unique<PushActivityCommand>(app, std::make_unique<SelectKeyActivity>(app, _("Select key for action"), settings[current].name, (int *) settings[current].var)));
                    break;
                case TypeDouble:
                    if (*(double *)settings[current].var >= settings[current].max)
                        *(double *)settings[current].var = settings[current].min;
                    else
                        *(double *)settings[current].var = gd_clamp_double(*(double *)settings[current].var + DOUBLE_STEP, settings[current].min, settings[current].max);
                    break;
            }
            if (settings[current].restart)
                restart = true;
            break;

        case 't':
        case 'T':
            // TRANSLATORS: 40 chars max
            app->select_file_and_do_command(_("Select Image for Theme"), g_get_home_dir(), "*.bmp;*.png", false, "", std::make_unique<ThemeSelectedCommand>(app, this));
            break;

        case 'h':
        case 'H':
        case '?':
            if (settings[current].description)
                app->show_message(_(settings[current].description));
            break;

        case App::Escape:    /* finished options menu */
            app->enqueue_command(std::make_unique<PopActivityCommand>(app));
            break;
    }

    if (settings[current].type == TypePage)
        current++;
    queue_redraw();
}


void SettingsActivity::redraw_event(bool full) const {
    app->clear_screen();
    // TRANSLATORS: 40 chars max (and it has title line capitalization in English)
    app->title_line(_("GDash Options"));
    // TRANSLATORS: 40 chars max. 'Change' means to change the setting in the options window
    app->status_line(_("Space: change   H: help   Esc: exit"));
    app->set_color(GD_GDASH_GRAY1);
    // TRANSLATORS: 40 chars max
    if (have_theme)
        app->font_manager->blittext_n(-1, app->screen->get_height() - 2 * app->font_manager->get_line_height(), _("Press T to install a new theme."));

    /* show settings */
    unsigned page = settings[current].page;
    unsigned linenum = 0;
    for (unsigned n = 0; n < numsettings; n++) {
        if (settings[n].page == page) {
            std::string value;
            switch (settings[n].type) {
                case TypePage:
                    break;
                case TypeBoolean:
                    value = *(bool *)settings[n].var ? _("yes") : _("no");
                    break;
                case TypeInteger:
                    value = Printf("%d", *(int *)settings[n].var);
                    break;
                case TypePercent:
                    value = Printf("%d%%", *(int *)settings[n].var);
                    break;
                case TypeTheme:
                case TypeShader:
                case TypeStringv:
                    value = _(settings[n].stringv[*(int *)settings[n].var]);
                    break;
                case TypeKey:
                    value = app->gameinput->get_key_name_from_keycode(*(guint *)settings[n].var);
                    break;
                case TypeDouble:
                    value = Printf("%f", *(double *)settings[n].var);
                    break;
            }

            int y = y1[page] + linenum * yd;
            if (settings[n].type != TypePage) {
                int x = 4 * app->font_manager->get_font_width_narrow();
                app->font_manager->blittext_n(x, y, "%c%s  %c%s", current == n ? GD_COLOR_INDEX_YELLOW : GD_COLOR_INDEX_LIGHTBLUE, _(settings[n].name), GD_COLOR_INDEX_GREEN, value);
            } else {
                int x = 2 * app->font_manager->get_font_width_narrow();
                app->font_manager->blittext_n(x, y, "%c%s", GD_COLOR_INDEX_WHITE, _(settings[n].name));
            }
            linenum++;
        }
    }
    app->draw_scrollbar(0, current, numsettings-1);
    app->screen->drawing_finished();
}
