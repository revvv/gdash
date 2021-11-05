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

#include <glib/gi18n.h>

#include "framework/volumeactivity.hpp"
#include "framework/commands.hpp"
#include "framework/app.hpp"
#include "gfx/fontmanager.hpp"
#include "gfx/screen.hpp"
#include "misc/util.hpp"
#include "sound/sound.hpp"
#include "settings.hpp"


VolumeActivity::VolumeActivity(App *app)
    : Activity(app), wait_ms(WaitBeforeDisappearMs) {
}


void VolumeActivity::redraw_event(bool full) const {
    int height = 6 * app->font_manager->get_line_height();
    int y1 = (app->screen->get_height() - height) / 2; /* middle of the screen */
    int cx = 2 * app->font_manager->get_font_width_narrow(), cy = y1, cw = app->screen->get_width() - 2 * cx, ch = height;

    app->draw_window(cx, cy, cw, ch);
    app->screen->set_clip_rect(cx, cy, cw, ch);
    app->set_color(GD_GDASH_WHITE);
    // TRANSLATORS: sound volume
    app->font_manager->blittext_n(-1, y1 + app->font_manager->get_line_height(), _("Volume"));

    std::string name, value;

    // TRANSLATORS: sound volume during game. the space at the end makes the text of the same length with the other string.
    name = _("Cave volume ");
    value = "";
    for (int i = 0; i < 100; i += 10) /* each char is 10% */
        value += (i < gd_sound_chunks_volume_percent) ? GD_FULL_BOX_CHAR : GD_UNCHECKED_BOX_CHAR;
    app->font_manager->blittext_n(-1, y1 + 3 * app->font_manager->get_line_height(), "%c%s  %c %c%d %c%c", GD_COLOR_INDEX_LIGHTBLUE, name, GD_LEFT_CHAR, GD_COLOR_INDEX_GREEN, value, GD_COLOR_INDEX_LIGHTBLUE, GD_RIGHT_CHAR);

    // TRANSLATORS: title screen music volume. the space at the end makes the text of the same length with the other string.
    name = _("Music volume");
    value = "";
    for (int i = 0; i < 100; i += 10) /* each char is 10% */
        value += (i < gd_sound_music_volume_percent) ? GD_FULL_BOX_CHAR : GD_UNCHECKED_BOX_CHAR;
    app->font_manager->blittext_n(-1, y1 + 4 * app->font_manager->get_line_height(), "%c%s  %c %c%d %c%c", GD_COLOR_INDEX_LIGHTBLUE, name, GD_DOWN_CHAR, GD_COLOR_INDEX_GREEN, value, GD_COLOR_INDEX_LIGHTBLUE, GD_UP_CHAR);

    app->screen->remove_clip_rect();

    app->screen->drawing_finished();
}


void VolumeActivity::keypress_event(KeyCode keycode, int gfxlib_keycode) {
    switch (keycode) {
        case App::Left:
            gd_sound_chunks_volume_percent = gd_clamp(gd_sound_chunks_volume_percent - 10, 0, 100);
            gd_sound_set_chunk_volumes();
            queue_redraw();
            wait_ms = WaitBeforeDisappearMs;
            break;
        case App::Right:
            gd_sound_chunks_volume_percent = gd_clamp(gd_sound_chunks_volume_percent + 10, 0, 100);
            gd_sound_set_chunk_volumes();
            queue_redraw();
            wait_ms = WaitBeforeDisappearMs;
            break;
        case App::Down:
            gd_sound_music_volume_percent = gd_clamp(gd_sound_music_volume_percent - 10, 0, 100);
            gd_sound_set_music_volume();
            queue_redraw();
            wait_ms = WaitBeforeDisappearMs;
            break;
        case App::Up:
            gd_sound_music_volume_percent = gd_clamp(gd_sound_music_volume_percent + 10, 0, 100);
            gd_sound_set_music_volume();
            queue_redraw();
            wait_ms = WaitBeforeDisappearMs;
            break;
        case App::Escape:
            /* do this so the next event will remove it */
            wait_ms = 0;
            break;
    }
}


void VolumeActivity::timer_event(int ms_elapsed) {
    wait_ms -= ms_elapsed;
    if (wait_ms <= 0) {
        app->enqueue_command(std::make_unique<PopActivityCommand>(app));
    }
}
