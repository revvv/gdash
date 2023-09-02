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
#include <cassert>
#include <cmath>
#include <numeric>

#include "cave/gamerender.hpp"

#include "cave/caverendered.hpp"
#include "cave/caveset.hpp"
#include "misc/util.hpp"
#include "input/gameinputhandler.hpp"
#include "gfx/pixbuf.hpp"
#include "gfx/screen.hpp"
#include "gfx/pixbufmanip.hpp"
#include "gfx/cellrenderer.hpp"
#include "gfx/fontmanager.hpp"
#include "cave/gamecontrol.hpp"
#include "cave/elementproperties.hpp"
#include "settings.hpp"

const char **gd_status_bar_colors_get_names() {
    static const char *types[] = {
        "C64 BD",
        "1stB",
        "CrLi",
        "Final BD",
        "Atari BD",
        NULL
    };
    return types;
}

GameRenderer::GameRenderer(Screen &screen, CellRenderer &cells, FontManager &font_manager, GameControl &game)
    :   PixmapStorage(screen),
        screen(screen),
        cells(cells),
        font_manager(font_manager),
        game(game),
        play_area_w(0), play_area_h(0),
        statusbar_height(0), statusbar_y1(0), statusbar_y2(0), statusbar_mid(0),
        out_of_window(false), show_replay_sign(true),
        scroll_x(0), scroll_y(0),
        scroll_desired_x(0), scroll_desired_y(0),
        millisecs_game(0),
        animcycle(0),
        must_draw_cave(false), must_clear_screen(false), must_draw_status(false), must_draw_story(false),
        status_bar_fast(false),
        status_bar_alternate(false),
        status_bar_paused(false) {
}


void GameRenderer::release_pixmaps() {
    story.background.release();
}


void GameRenderer::set_show_replay_sign(bool srs) {
    show_replay_sign = srs;
}


/* just set current viewport to upper left. */
void GameRenderer::scroll_to_origin() {
    scroll_x = 0;
    scroll_y = 0;
    scroll_speed_x = 0;
    scroll_speed_y = 0;
    scroll_speed_normal = -1.0;
    scroll_speeds_during_uncover.clear();
}

/**
 * Function which can do the x or the y scrolling for the cave.
 *
 * @param logical_size Logical pixel size of cave, usually larger than the screen.
 * @param physical_size Visible part of cave in pixels. (remember: player_x-x1!)
 * @param center the coordinates to scroll to.
 * @param exact scroll exactly (no hystheresis)
 * @param current the variable to be changed
 * @param desired the function stores its data here.
 * @param currspeed variable to store the speed
*/
bool GameRenderer::cave_scroll(int logical_size, int physical_size, int center, bool exact, double &current, int &desired, double & currspeed) {
    bool changed = false;
    
    int const cell_size = cells.get_cell_size();
    /* start the scrolling when reaches 3 cells at the screen edge */
    int const start = physical_size / 2 - cell_size * 3;
    /* scroll so that the player is at the center; the allowed difference is this */
    int const to = cell_size;

    /* if cave size smaller than the screen, no scrolling req'd */
    if (logical_size < physical_size) {
        desired = 0;
        if (current != 0) {
            current = 0;
            changed = true;
        }
        return changed;
    }

    int max = logical_size - physical_size;
    if (max < 0)
        max = 0;
    if (logical_size <= physical_size + cell_size) {
        /* if cave size is only a slightly larger than the screen, also no scrolling. */
        /* scroll to the middle of the cave */
        desired = max / 2;
    } else {
        if (!exact) {
            /* hystheresis function.
             * when scrolling left, always go a bit less left than player being at the middle.
             * when scrolling right, always go a bit less to the right. */
            if (current < center - start)
                desired = center - to;
            if (current > center + start)
                desired = center + to;
        } else {
            /* if exact scrolling, just go exactly to the center. */
            desired = center;
        }
    }
    desired = CLAMP(desired, 0, max);
    
    double const round_to = gd_fine_scroll ? 0.5 : 1;
    /* if no speed is selected, or speed is somehow zero, set to normal speed */
    if (currspeed <= 0)
        currspeed = scroll_speed_normal;
    /* if lagging to much, speed up. with hystheresis. */
    /* if closer to the scroll destination, slow down again. with hystheresis. */
    if (ABS(current - center) > physical_size / 2 - cell_size)
        currspeed = scroll_speed_normal + round_to;
    if (ABS(current - center) <= to + 2 * cell_size)
        currspeed = scroll_speed_normal;

    /* do the scroll */
    if (current < desired) {
        current += currspeed;
        if (current > desired)
            current = desired;
        changed = true;
    }
    if (current > desired) {
        current -= currspeed;
        if (current < desired)
            current = desired;
        changed = true;
    }

    return changed;
}


/** Scrolls to the player during game play.

* scrolling is a bit complicated. different caves have different speeds, and
* also the game rendering can be run at different speeds (depending on the
* refresh rate of the user's display). so
* first a pixel/frame scrolling speed is calculated using the measured delay
* and the cave speed.
* 
* then this pixel speed is rounded to an integer or a half value. this is needed
* because values like 1.57 pixel/frame cause jerky scrolling (the pixel value
* of the pixmaps blitted must be rounded to an integer, of course). so when
* not using fine scrolling (slow refresh rate), it is rounded to an integer.
* for high refresh rates (fine scrolling), rounding to 0.5 is also acceptable.
* 
* this rounding goes downwards, with the floor() function. scrolling must be slower
* than the calculated ideal value, or it would be faster than the speed of
* player running. and if the scrolling were faster, sometimes it would stop and
* then start over again.
* 
* however, when using scrolling that is slower than the player running, it is
* possible that it will lag, and then the player will run out of the screen. to
* handle this, the cave_scroll() function has a hystheresis function. when the
* player approaches the edge of the screen, scrolling switches to a faster speed
* (so in that case, the rounding is done upwards).
* 
* the cave speed can also change (it can slow down, when an amoeba grows, or speed
* up, when the player eats many diamonds). when this happens, the scrolling speed
* is not immediately changed, but also through a hystheresis function.

 * @param ms The number of milliseconds elapsed
 * @param exact_scroll Whether to scroll to the exact position, or allow hystheresis.
 * @return true, if player is not visible, ie. it is out of the visible size in the drawing area.
 */
bool GameRenderer::scroll(int ms, bool exact_scroll) {
    assert(play_area_w > 0);
    assert(play_area_h > 0);

    int cell_size = cells.get_cell_size();

    int player_x = game.played_cave->player_x - game.played_cave->x1; /* cell coordinates of player */
    int player_y = game.played_cave->player_y - game.played_cave->y1;
    int visible_x = (game.played_cave->x2 - game.played_cave->x1 + 1) * cell_size; /* pixel size of visible part of the cave (may be smaller in intermissions) */
    int visible_y = (game.played_cave->y2 - game.played_cave->y1 + 1) * cell_size;

    /* calculate speed for current frame */
    double current_speed_calculated = (double)cell_size * ms / game.played_cave->speed;
    /* if uncovering, save to the array, and select the average instead. */
    if (game.is_uncovering()) {
        scroll_speeds_during_uncover.push_back(current_speed_calculated);
        current_speed_calculated =
            std::accumulate(scroll_speeds_during_uncover.begin(), scroll_speeds_during_uncover.end(), 0.0)
            / scroll_speeds_during_uncover.size();
    }
    /* round to the nearest feasible value, which may be an even number, or maybe a half for fine scrolling */
    double const round_to = gd_fine_scroll ? 0.5 : 1;
    double current_speed_rounded = floor(current_speed_calculated / round_to) * round_to;
    /* if not yet calculated, now use it. */
    if (scroll_speed_normal < 0)
        scroll_speed_normal = current_speed_rounded;
    /* change the speed only if differs too much from the previous value (need the next step).
     * this does the hystheresis for the cave speed change. */
    if (current_speed_calculated >= scroll_speed_normal + round_to || current_speed_calculated <= scroll_speed_normal)
        scroll_speed_normal = current_speed_rounded;
    /* if so slow that it seems to be zero, select the lowest possible value */
    if (scroll_speed_normal == 0)
        scroll_speed_normal = round_to;
    
    /* and now scroll. */
    bool scrolled = false;
    if (cave_scroll(visible_x, play_area_w, player_x * cell_size + cell_size / 2 - play_area_w / 2,
        exact_scroll, scroll_x, scroll_desired_x, scroll_speed_x))
        scrolled = true;
    if (cave_scroll(visible_y, play_area_h, player_y * cell_size + cell_size / 2 - play_area_h / 2,
        exact_scroll, scroll_y, scroll_desired_y, scroll_speed_y))
        scrolled = true;

    /* if scrolling, we should update entire screen. */
    if (scrolled && !game.gfx_buffer.empty()) {
        for (int y = 0; y < game.played_cave->h; y++)
            for (int x = 0; x < game.played_cave->w; x++)
                game.gfx_buffer(x, y) |= GD_REDRAW;
    }

    /* check if active player is visible at the moment. */
    bool out_of_window = false;
    /* check if active player is outside drawing area. if yes, we should wait for scrolling.
     * but only if scrolling happened at all! */
    if (scrolled) {
        if ((player_x * cell_size) < scroll_x || (player_x * cell_size + cell_size - 1) > scroll_x + play_area_w)
            /* but only do the wait, if the player SHOULD BE visible, ie. he is inside the defined visible area of the cave */
            if (game.played_cave->player_x >= game.played_cave->x1 && game.played_cave->player_x <= game.played_cave->x2)
                out_of_window = true;
        if ((player_y * cell_size) < scroll_y || (player_y * cell_size + cell_size - 1) > scroll_y + play_area_h)
            /* but only do the wait, if the player SHOULD BE visible, ie. he is inside the defined visible area of the cave */
            if (game.played_cave->player_y >= game.played_cave->y1 && game.played_cave->player_y <= game.played_cave->y2)
                out_of_window = true;
    }
    
    scroll_ms = ms;

    /* if not yet born, we treat as visible. so cave will run. the user is unable to control an unborn player, so this is the right behaviour. */
    if (game.played_cave->player_state == GD_PL_NOT_YET)
        return false;
    return out_of_window;
}


void GameRenderer::drawcave() const {
    int cell_size = cells.get_cell_size();

    /* on-screen clipping rectangle */
    screen.set_clip_rect(0, statusbar_height, play_area_w, play_area_h);

    int scroll_y_aligned;
    if (screen.get_pal_emulation())
        scroll_y_aligned = int(scroll_y) / 2 * 2;      /* make it even (dividable by two) */
    else
        scroll_y_aligned = int(scroll_y);

    /* if the cave is smaller than the play area, add some pixels to make it centered */
    int xplus, yplus;
    int cave_pixel_w = (game.played_cave->x2 - game.played_cave->x1 + 1) * cell_size;
    int cave_pixel_h = (game.played_cave->y2 - game.played_cave->y1 + 1) * cell_size;
    if (play_area_w > cave_pixel_w)
        xplus = (play_area_w - cave_pixel_w) / 2;
    else
        xplus = 0;
    if (play_area_h > cave_pixel_h)
        yplus = (play_area_h - cave_pixel_h) / 2;
    else
        yplus = 0;

    if (gd_full_cave_view && game.played_cave->intermission)
        yplus = 0; // align top

    /* if using particle effects, draw the background, as particles might have moved "out" of it.
     * we should only do this if the cave is smaller than the screen! that we well know from the xplus
     * and yplus variables set above. */
    if (must_clear_screen || (gd_particle_effects && (xplus != 0 || yplus != 0))) {
        /* fill screen with status bar background color - particle effects might have gone "out" of the cave */
        screen.fill(cols.background);
    }

    /* here we draw all cells to be redrawn. the in-cell clipping will be done by the graphics
     * engine, we only clip full cells. */
    /* the x and y coordinates are cave physical coordinates.
     * xd and yd are relative to the visible area. */
    int x, y, xd, yd;
    for (y = game.played_cave->y1, yd = 0; y <= game.played_cave->y2; y++, yd++) {
        int ys = yplus - scroll_y_aligned + statusbar_height + yd * cell_size;
        for (x = game.played_cave->x1, xd = 0; x <= game.played_cave->x2; x++, xd++) {
            if (game.gfx_buffer(x, y) & GD_REDRAW) {    /* if it needs to be redrawn */
                // calculate on-screen coordinates
                int xs = xplus - scroll_x + xd * cell_size;
                int dr = game.gfx_buffer(x, y) & ~GD_REDRAW;
                screen.blit(cells.cell(dr), xs, ys);
                game.gfx_buffer(x, y) = dr;   /* now that we drew it */
            }
        }
    }

    /* now draw the particles */
    if (gd_particle_effects) {
        int xs = xplus - scroll_x - game.played_cave->x1 * cell_size;
        int ys = yplus + statusbar_height - scroll_y_aligned - game.played_cave->y1 * cell_size;
        std::list<ParticleSet>::const_iterator it;
        for (it = game.played_cave->particles.begin(); it != game.played_cave->particles.end(); ++it)
            screen.draw_particle_set(xs, ys, *it);
    }

    /* if using particle effects, the whole cave needs to be redrawn later. */
    if (gd_particle_effects) {
        /* remember to redraw the whole cave */
        for (int y = game.played_cave->y1; y <= game.played_cave->y2; y++)
            for (int x = game.played_cave->x1; x <= game.played_cave->x2; x++)
                game.gfx_buffer(x, y) |= GD_REDRAW;
    }

    /* writing the scrolling parameters to the screen */
    if (gd_show_fps) {
        std::string s = Printf("ms=%2d fps=%2d sm=%4.2f sx=%4.2f sy=%4.2f", scroll_ms, 1000/scroll_ms, scroll_speed_normal, scroll_speed_x, scroll_speed_y);
        font_manager.blittext_n(1, screen.get_height()-font_manager.get_line_height()+1, GD_GDASH_BLACK, s.c_str());
        font_manager.blittext_n(0, screen.get_height()-font_manager.get_line_height(), GD_GDASH_WHITE, s.c_str());
    }

    /* restore clipping to whole screen */
    screen.remove_clip_rect();
}


void GameRenderer::set_random_colors() {
    if (game.played_cave.get() == NULL)
        return;
    gd_cave_set_random_colors(*game.played_cave, GdColor::Type(gd_preferred_palette));
    set_colors_from_cave();
    draw(true);
}


bool GameRenderer::drawstatus_firstline(bool in_game) const {
    bool first_line = false; /* will be set to true, if we draw in the next few code lines. so the y coordinate of the second status line can be decided. */

    /* if playing a replay, tell the user! */
    switch (game.type) {
        case GameControl::TYPE_REPLAY:
            if (show_replay_sign) {
                // TRANSLATORS: the translated string must be at most 20 characters long
                font_manager.blittext(-1, statusbar_y1, GD_GDASH_YELLOW, _("PLAYING REPLAY"));
                first_line = true;
            } else if (gd_show_name_of_game && !in_game) {
                /* if showing the name of the cave... */
                int len = g_utf8_strlen(game.caveset->name.c_str(), -1);
                if (screen.get_width() / font_manager.get_font_width_wide() >= len) /* if have place for double-width font */
                    font_manager.blittext(-1, statusbar_y1, cols.default_color, game.caveset->name.c_str());
                else
                    font_manager.blittext_n(-1, statusbar_y1, cols.default_color, game.caveset->name.c_str());
                first_line = true;
            }
            break;
        case GameControl::TYPE_CONTINUE_REPLAY:
            if (show_replay_sign) {
                // TRANSLATORS: the translated string must be at most 20 characters long
                font_manager.blittext(-1, statusbar_y1, GD_GDASH_YELLOW, _("CONTINUING REPLAY"));
                first_line = true;
            }
            break;
        case GameControl::TYPE_SNAPSHOT:
            if (show_replay_sign) {
                // TRANSLATORS: the translated string must be at most 20 characters long
                font_manager.blittext(-1, statusbar_y1, GD_GDASH_YELLOW, _("PLAYING SNAPSHOT"));
                first_line = true;
            }
            break;
        case GameControl::TYPE_TEST:
            if (show_replay_sign) {
                // TRANSLATORS: the translated string must be at most 20 characters long
                font_manager.blittext(-1, statusbar_y1, GD_GDASH_YELLOW, _("TESTING CAVE"));
                first_line = true;
            }
            break;
        case GameControl::TYPE_NORMAL:
            /* normal game - but if not really playing */
            if (!in_game) {
                /* also inform about intermission, but not if playing a replay. also the replay saver should not show it! f */
                if (game.played_cave->intermission) {
                    // TRANSLATORS: the translated string must be at most 20 characters long
                    font_manager.blittext(-1, statusbar_y1, cols.default_color, _("ONE LIFE EXTRA"));
                    first_line = true;
                } else if (gd_show_name_of_game) {
                    /* if not an intermission, we may show the name of the game (caveset) */
                    /* if showing the name of the cave... */
                    int len = g_utf8_strlen(game.caveset->name.c_str(), -1);
                    if (screen.get_width() / font_manager.get_font_width_wide() >= len) /* if have place for double-width font */
                        font_manager.blittext(-1, statusbar_y1, cols.default_color, game.caveset->name.c_str());
                    else
                        font_manager.blittext_n(-1, statusbar_y1, cols.default_color, game.caveset->name.c_str());
                    first_line = true;
                }
            }
            break;
    }

    return first_line;
}


void GameRenderer::drawstatus_uncover() const {
    bool first_line = drawstatus_firstline(false);

    int cavename_y = first_line ? statusbar_y2 : statusbar_mid;
    /* "xy players, cave ab/3" */
    std::string str;
    if (game.type == GameControl::TYPE_NORMAL) {
        if (game.caveset_has_levels)
            str = Printf("%d%c, %s/%d", game.player_lives, GD_PLAYER_CHAR, game.played_cave->name, int(game.played_cave->rendered_on + 1));
        else
            str = Printf("%d%c, %s", game.player_lives, GD_PLAYER_CHAR, game.played_cave->name);
    } else
        /* if not a normal game, do not show number of remaining lives */
        str = Printf("%s/%d", game.played_cave->name, int(game.played_cave->rendered_on + 1));
    int len = g_utf8_strlen(str.c_str(), -1);
    if (screen.get_width() / font_manager.get_font_width_wide() >= len) /* if have place for double-width font */
        font_manager.blittext(-1, cavename_y, cols.default_color, str.c_str());
    else
        font_manager.blittext_n(-1, cavename_y, cols.default_color, str.c_str());
}


static char gravity_char(GdDirectionEnum dir) {
    switch (dir) {
        case MV_DOWN:
            return GD_DOWN_CHAR;
        case MV_LEFT:
            return GD_LEFT_CHAR;
        case MV_RIGHT:
            return GD_RIGHT_CHAR;
        case MV_UP:
            return GD_UP_CHAR;
        default:
            return '?';
    }
}


void GameRenderer::drawstatus_game() const {
    if (game.played_cave->player_state == GD_PL_TIMEOUT
            && game.statusbarsince / 1000 % 4 == 0) {
        // TRANSLATORS: the translated string must be at most 20 characters long
        font_manager.blittext(-1, statusbar_mid, GD_GDASH_WHITE, _("OUT OF TIME"));
        return;
    }

    /* y position of status bar */
    bool first_line = drawstatus_firstline(true);

    int y = first_line ? statusbar_y2 : statusbar_mid;

    if (status_bar_alternate) {
        /* ALTERNATIVE STATUS BAR BY PRESSING SHIFT */
        /* this will output a total of 20 chars */
        int x = (screen.get_width() - 20 * font_manager.get_font_width_wide()) / 2;

        x = font_manager.blittext(x, y, cols.default_color, "%c%02d ", GD_PLAYER_CHAR, gd_clamp(game.player_lives, 0, 99)); /* max 99 in %2d */
        /* color numbers are not the same as key numbers! c3->k1, c2->k2, c1->k3 */
        /* this is how it was implemented in crdr7. */
        x = font_manager.blittext(x, y, game.played_cave->color3, "%c%1d ", GD_KEY_CHAR, gd_clamp(int(game.played_cave->key1), 0, 9)); /* max 9 in %1d */
        x = font_manager.blittext(x, y, game.played_cave->color2, "%c%1d ", GD_KEY_CHAR, gd_clamp(int(game.played_cave->key2), 0, 9));
        x = font_manager.blittext(x, y, game.played_cave->color1, "%c%1d ", GD_KEY_CHAR, gd_clamp(int(game.played_cave->key3), 0, 9));
        if (game.played_cave->gravity_will_change > 0) {
            x = font_manager.blittext(x, y, cols.default_color, "%c%02d ", gravity_char(game.played_cave->gravity_next_direction), gd_clamp(game.played_cave->time_visible(game.played_cave->gravity_will_change), 0, 99));
        } else {
            x = font_manager.blittext(x, y, cols.default_color, "%c%02d ", gravity_char(game.played_cave->gravity), 0);
        }
        x = font_manager.blittext(x, y, cols.diamond_collected, "%c%02d", GD_SKELETON_CHAR, gd_clamp(int(game.played_cave->skeletons_collected), 0, 99));
    } else {
        int scale = screen.get_pixmap_scale();
        /* NORMAL STATUS BAR */
        /* will draw 18 chars (*16 pixels) + 1+10+11+10 pixels inside. */
        /* the two spaces available between scores etc must be divided into
         * three "small" spaces. */
        int x = (screen.get_width() - 20 * font_manager.get_font_width_wide()) / 2;
        int time_secs;

        /* cave time is rounded _UP_ to seconds. so at the exact moment when it changes from
           2sec remaining to 1sec remaining, the player has exactly one second. when it changes
           to zero, it is the exact moment of timeout. */
        time_secs = game.played_cave->time_visible(game.played_cave->time);

        x += 1 * scale;
        if (status_bar_fast) {
            /* fast forward mode - show "FAST" */
            x = font_manager.blittext(x, y, cols.default_color, "%cFAST%c", GD_DIAMOND_CHAR, GD_DIAMOND_CHAR);
        } else {
            /* normal speed mode - show diamonds NEEDED <> VALUE */
            /* or if collected enough diamonds,   <><><> VALUE */
            if (game.played_cave->diamonds_needed > game.played_cave->diamonds_collected) {
                if (game.played_cave->diamonds_needed > 0)
                    x = font_manager.blittext(x, y, cols.diamond_needed, "%03d", game.played_cave->diamonds_needed);
                else
                    /* did not already count diamonds needed */
                    x = font_manager.blittext(x, y, cols.diamond_needed, "%c%c%c", GD_DIAMOND_CHAR, GD_DIAMOND_CHAR, GD_DIAMOND_CHAR);
            } else
                x = font_manager.blittext(x, y, cols.default_color, " %c%c", GD_DIAMOND_CHAR, GD_DIAMOND_CHAR);
            x = font_manager.blittext(x, y, cols.default_color, "%c", GD_DIAMOND_CHAR);
            x = font_manager.blittext(x, y, cols.diamond_value, "%02d", game.played_cave->diamond_value);
        }
        x += 10 * scale;
        x = font_manager.blittext(x, y, cols.diamond_collected, "%03d", game.played_cave->diamonds_collected);
        x += 11 * scale;
        x = font_manager.blittext(x, y, cols.default_color, "%03d", time_secs);
        x += 10 * scale;
        x = font_manager.blittext(x, y, cols.score, "%06d", game.player_score);
    }
}


void GameRenderer::drawstatus() const {
    /* check if no status bar at all */
    if (game.statusbartype == GameControl::status_bar_none)
        return;

    /* clear the header bar */
    screen.fill_rect(0, 0, screen.get_width(), statusbar_height, cols.background);

    /* when paused, switch between "paused" status bar and normal */
    if (status_bar_paused && game.statusbarsince / 1000 % 4 == 0) {
        // TRANSLATORS: the translated string must be at most 20 characters long
        font_manager.blittext(-1, statusbar_mid, cols.default_color, _("SPACEBAR TO RESUME"));
        return;
    }

    switch (game.statusbartype) {
        case GameControl::status_bar_none:
            /* do nothing */
            break;
        case GameControl::status_bar_uncover:
            drawstatus_uncover();
            break;
        case GameControl::status_bar_game:
            drawstatus_game();
            break;
        case GameControl::status_bar_game_over:
            // TRANSLATORS: the translated string must be at most 20 characters long.
            // the c64 original had these spaces - you are allowed to do so.
            font_manager.blittext(-1, statusbar_mid, cols.default_color, _("G A M E   O V E R"));
            break;
    }
}


void GameRenderer::select_status_bar_colors() {
    GdColor(*color_indexer)(unsigned i);
    /* first, count the number of c64 colors the cave uses. */
    /* if it uses mostly c64 colors, we will use c64 colors for the status bar. */
    /* otherwise we will use gdash colors. */
    /* note that the atari original status bar color setting only uses the game colors. */
    int c64_col = 0;
    if (game.played_cave->color0.is_c64()) c64_col++;
    if (game.played_cave->color1.is_c64()) c64_col++;
    if (game.played_cave->color2.is_c64()) c64_col++;
    if (game.played_cave->color3.is_c64()) c64_col++;
    if (game.played_cave->color4.is_c64()) c64_col++;
    if (game.played_cave->color5.is_c64()) c64_col++;
    if (c64_col > 4)
        color_indexer = GdColor::from_c64;
    else
        color_indexer = GdColor::from_gdash_index;

    switch (gd_status_bar_colors) {
        case GD_STATUS_BAR_ORIGINAL:
            cols.background = color_indexer(GD_COLOR_INDEX_BLACK);
            cols.diamond_needed = color_indexer(GD_COLOR_INDEX_YELLOW);
            cols.diamond_collected = color_indexer(GD_COLOR_INDEX_YELLOW);
            cols.diamond_value = color_indexer(GD_COLOR_INDEX_WHITE);
            cols.score = color_indexer(GD_COLOR_INDEX_WHITE);
            cols.default_color = color_indexer(GD_COLOR_INDEX_WHITE);
            break;
        case GD_STATUS_BAR_1STB:
            cols.background = color_indexer(GD_COLOR_INDEX_BLACK);
            cols.diamond_needed = color_indexer(GD_COLOR_INDEX_YELLOW);
            cols.diamond_collected = color_indexer(GD_COLOR_INDEX_YELLOW);
            cols.score = color_indexer(GD_COLOR_INDEX_YELLOW);
            cols.diamond_value = color_indexer(GD_COLOR_INDEX_WHITE);
            cols.default_color = color_indexer(GD_COLOR_INDEX_WHITE);
            break;
        case GD_STATUS_BAR_CRLI:
            cols.background = color_indexer(GD_COLOR_INDEX_BLACK);
            cols.diamond_needed = color_indexer(GD_COLOR_INDEX_RED);
            cols.diamond_collected = color_indexer(GD_COLOR_INDEX_GREEN);
            cols.diamond_value = color_indexer(GD_COLOR_INDEX_CYAN);
            cols.score = color_indexer(GD_COLOR_INDEX_YELLOW);
            cols.default_color = color_indexer(GD_COLOR_INDEX_WHITE);
            break;
        case GD_STATUS_BAR_FINAL:
            cols.background = color_indexer(GD_COLOR_INDEX_BLACK);
            cols.diamond_needed = color_indexer(GD_COLOR_INDEX_RED);
            cols.diamond_collected = color_indexer(GD_COLOR_INDEX_GREEN);
            cols.diamond_value = color_indexer(GD_COLOR_INDEX_WHITE);
            cols.score = color_indexer(GD_COLOR_INDEX_WHITE);
            cols.default_color = color_indexer(GD_COLOR_INDEX_WHITE);
            break;
        case GD_STATUS_BAR_ATARI_ORIGINAL:
            cols.background = game.played_cave->color0;
            cols.diamond_needed = game.played_cave->color2;
            cols.diamond_collected = game.played_cave->color2;
            cols.diamond_value = game.played_cave->color3;
            cols.score = game.played_cave->color3;
            cols.default_color = game.played_cave->color3;
            break;
        default:
            g_assert_not_reached();
    }
}


void GameRenderer::set_colors_from_cave() {
    /* select colors, prepare drawing etc. */
    cells.select_pixbuf_colors(game.played_cave->color0, game.played_cave->color1, game.played_cave->color2, game.played_cave->color3, game.played_cave->color4, game.played_cave->color5);
    /* select status bar colors here, as some depend on actual cave colors */
    select_status_bar_colors();

    game.played_cave->dirt_particle_color = average_nonblack_colors_in_pixbuf(cells.cell_pixbuf(abs(gd_element_properties[game.played_cave->dirt_looks_like].image_game)));
    game.played_cave->dirt_2_particle_color = average_nonblack_colors_in_pixbuf(cells.cell_pixbuf(abs(gd_element_properties[O_DIRT2].image_game)));
    game.played_cave->stone_particle_color = average_nonblack_colors_in_pixbuf(cells.cell_pixbuf(abs(gd_element_properties[O_STONE].image_game)));
    game.played_cave->mega_stone_particle_color = average_nonblack_colors_in_pixbuf(cells.cell_pixbuf(abs(gd_element_properties[O_MEGA_STONE].image_game)));
    game.played_cave->diamond_particle_color = lightest_color_in_pixbuf(cells.cell_pixbuf(abs(gd_element_properties[O_DIAMOND].image_game)));
    game.played_cave->explosion_particle_color = lightest_color_in_pixbuf(cells.cell_pixbuf(abs(gd_element_properties[O_EXPLODE_1].image_game)));
    game.played_cave->magic_wall_particle_color = average_nonblack_colors_in_pixbuf(cells.cell_pixbuf(abs(gd_element_properties[O_MAGIC_WALL].image_game)));
    game.played_cave->expanding_wall_particle_color = average_nonblack_colors_in_pixbuf(cells.cell_pixbuf(abs(gd_element_properties[game.played_cave->expanding_wall_looks_like].image_game)));
    game.played_cave->expanding_steel_wall_particle_color = average_nonblack_colors_in_pixbuf(cells.cell_pixbuf(abs(gd_element_properties[O_EXPANDING_STEEL_WALL].image_game)));
    game.played_cave->lava_particle_color = average_nonblack_colors_in_pixbuf(cells.cell_pixbuf(abs(gd_element_properties[O_LAVA].image_game)));
}


void GameRenderer::drawstory() const {
    // create dark background
    if (story.background.get() == NULL) {
        // create the pixbuf for it
        int w = screen.get_width() / screen.get_pixmap_scale(),
            h = screen.get_height() / screen.get_pixmap_scale();
        std::unique_ptr<Pixbuf> background_pixbuf(screen.pixbuf_factory.create(w, h));
        GdElementEnum bgcells[8] = { O_STONE, O_DIAMOND, O_BRICK, O_DIRT, O_SPACE, O_SPACE, O_DIRT, O_SPACE };
        int cs = cells.get_cell_pixbuf_size();
        for (int y = 0; y < h; y += cs)
            for (int x = 0; x < w; x += cs)
                cells.cell_pixbuf(abs(gd_element_properties[bgcells[g_random_int_range(0, 7)]].image_game)).copy(*background_pixbuf, x, y);
        std::unique_ptr<Pixbuf> dark_background_pixbuf(screen.pixbuf_factory.create_composite_color(*background_pixbuf, GdColor::from_rgb(0, 0, 0), 256 * 4 / 5));
        // this one should be the size of the screen again
        story.background = screen.create_scaled_pixmap_from_pixbuf(*dark_background_pixbuf, false);
    }
    screen.blit(*story.background, 0, 0);

    // title line, status line
    font_manager.blittext_n(-1, 0, GD_GDASH_GRAY2, game.played_cave->name.c_str());
    // TRANSLATORS: the translated string must be at most 40 characters long
    font_manager.blittext_n(-1, screen.get_height() - font_manager.get_font_height(), GD_GDASH_GRAY2, _("UP, DOWN: MOVE    FIRE: CONTINUE"));

    // text
    for (unsigned l = 0; l < story.linesavailable && story.scroll_y + l < story.wrapped_text.size(); ++l)
        font_manager.blittext_n(font_manager.get_font_width_narrow() * 2,
                                l * font_manager.get_line_height() + font_manager.get_line_height() * 3, GD_GDASH_WHITE, story.wrapped_text[story.scroll_y + l].c_str());

    // up & down arrow
    if (story.scroll_y < story.max_y)
        font_manager.blittext_n(screen.get_width() - font_manager.get_font_width_narrow(),
                                screen.get_height() - 3 * font_manager.get_line_height(), GD_GDASH_GRAY2, Printf("%c", GD_DOWN_CHAR).c_str());
    if (story.scroll_y > 0)
        font_manager.blittext_n(screen.get_width() - font_manager.get_font_width_narrow(),
                                font_manager.get_line_height() * 2, GD_GDASH_GRAY2, Printf("%c",GD_UP_CHAR).c_str());
}


void GameRenderer::draw(bool full) const {
    // if cave exists and colors are selected, it means that the cave was drawn
    if (!game.gfx_buffer.empty()) {
        // if everything must be redrawn, clear the screen and remember that
        // all cave cells must be drawn
        if (full) {
            must_clear_screen = true;
            for (int y = game.played_cave->y1; y <= game.played_cave->y2; y++)
                for (int x = game.played_cave->x1; x <= game.played_cave->x2; x++)
                    game.gfx_buffer(x, y) |= GD_REDRAW;
        }
        if (full || must_draw_cave)
            drawcave();
        if (full || must_draw_status) {
            drawstatus();
        }

        must_clear_screen = false;
        must_draw_cave = false;
        must_draw_status = false;
    }
    // if story is not empty, redraw that
    else if (!story.wrapped_text.empty()) {
        if (full || must_draw_story)
            drawstory();
        must_draw_story = false;
    }
    screen.drawing_finished();
}


void GameRenderer::screen_initialized() {
    /* check the screen size, and calculate status bar alignment */
    play_area_w = screen.get_width();
    statusbar_height = font_manager.get_font_height() * 2 + 1 * screen.get_pixmap_scale();
    play_area_h = screen.get_height() - statusbar_height;
    statusbar_y1 = 0;
    statusbar_y2 = font_manager.get_font_height();
    statusbar_mid = (statusbar_height - font_manager.get_font_height()) / 2;
    /* for story */
    story.linesavailable = screen.get_height() / font_manager.get_line_height() - 6;
}

/** For removing old particles (a predicate) */
static bool old_particle(ParticleSet const &ps) {
    return ps.life < 0;
}


GameRenderer::State GameRenderer::main_int(int millisecs_elapsed, bool paused, GameInputHandler *inputhandler) {
    GameControl::State state = GameControl::STATE_NOTHING;

    /* remember for the status bar drawer */
    status_bar_alternate = inputhandler != NULL ? inputhandler->alternate_status : false;
    status_bar_fast = inputhandler != NULL ? inputhandler->fast_forward : false;
    if (status_bar_paused != paused) {
        game.statusbarsince = 0;
        must_draw_status = true;
    }
    status_bar_paused = paused;

    millisecs_game += millisecs_elapsed;
    while (millisecs_game >= 40) {
        millisecs_game -= 40;

        /* tell the interrupt "40 ms has passed" - the cave will move. */
        state = game.main_int(inputhandler, !paused && !out_of_window);
        animcycle = (animcycle + 1) % 8;
        must_draw_cave = true;
        must_draw_status = true;

        /* check state of game */
        switch (state) {
            case GameControl::STATE_CAVE_LOADED:
                set_colors_from_cave();
                scroll_to_origin();
                break;

            case GameControl::STATE_SHOW_STORY:
                story.wrapped_text = gd_wrap_text(game.played_cave->story.c_str(), screen.get_width() / font_manager.get_font_width_narrow() - 4);
                story.scroll_y = 0;
                if (story.wrapped_text.size() < story.linesavailable)
                    story.max_y = 0;
                else
                    story.max_y = story.wrapped_text.size() - story.linesavailable;
                must_draw_story = true;
                break;

            case GameControl::STATE_SHOW_STORY_WAIT:
                if (inputhandler != NULL && inputhandler->down() && story.scroll_y < story.max_y) {
                    story.scroll_y++;
                    must_draw_story = true;
                }
                if (inputhandler != NULL && inputhandler->up() && story.scroll_y > 0) {
                    story.scroll_y--;
                    must_draw_story = true;
                }
                break;

            case GameControl::STATE_FIRST_FRAME:
                story.wrapped_text.clear();
                story.background.release();
                must_clear_screen = true;
                break;

            case GameControl::STATE_NOTHING:
            case GameControl::STATE_STOP:
            case GameControl::STATE_GAME_OVER:
                break;
        }
    }

    if (!game.gfx_buffer.empty()) {
        /* do the scrolling. */
        /* scroll exactly, if player is not yet alive. */
        /* remember the "player out of window" for next iteration. */
        /* the scrolling routine invalidates the game gfx cells if needed. */
        out_of_window = scroll(millisecs_elapsed, game.played_cave->player_state == GD_PL_NOT_YET);

        /* move the particles */
        std::list<ParticleSet>::iterator it;
        for (it = game.played_cave->particles.begin(); it != game.played_cave->particles.end(); ++it) {
            if (it->is_new)
                it->normalize(cells.get_cell_size());
            it->move(millisecs_elapsed);
        }
        game.played_cave->particles.remove_if(old_particle);

        /* always render the cave to the gfx buffer; however it may do nothing if animcycle was not changed. */
        game.played_cave->draw_indexes(game.gfx_buffer, game.covered, game.bonus_life_flash > 0, animcycle, gd_no_invisible_outbox);

        /* draw the cave. */
        must_draw_cave = true;
    }

    switch (state) {
        case GameControl::STATE_STOP:
            return Stop;
        case GameControl::STATE_GAME_OVER:
            return GameOver;
        default:
            return Nothing;
    }
}
