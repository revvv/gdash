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

#include "framework/gameactivity.hpp"
#include "framework/commands.hpp"
#include "gfx/screen.hpp"
#include "cave/caveset.hpp"
#include "cave/gamecontrol.hpp"
#include "settings.hpp"
#include "sound/sound.hpp"
#include "input/gameinputhandler.hpp"
#include "misc/helptext.hpp"
#include "settings.hpp"


/* amoeba state to string */
static const char *
amoeba_state_string(AmoebaState a) {
    switch (a) {
        case GD_AM_SLEEPING:
            return _("sleeping");          /* sleeping - not yet let out. */
        case GD_AM_AWAKE:
            return _("awake");                /* living, growing */
        case GD_AM_TOO_BIG:
            return _("too big");            /* grown too big, will convert to stones */
        case GD_AM_ENCLOSED:
            return _("enclosed");          /* enclosed, will convert to diamonds */
    }
    return _("unknown");
}

/* amoeba state to string */
static const char *
magic_wall_state_string(MagicWallState m) {
    switch (m) {
        case GD_MW_DORMANT:
            return _("dormant");
        case GD_MW_ACTIVE:
            return _("active");
        case GD_MW_EXPIRED:
            return _("expired");
    }
    return _("unknown");
}


static std::string info_and_variables_of_cave(CaveStored const *cavestored, CaveRendered const *cave) {
    std::string s;

    if (cavestored != NULL) {
        s += caveinfo_text(*cavestored);
        s += "\n";
    }

    s += Printf(_("%cCave internals:\n"), GD_COLOR_INDEX_WHITE);
    s += Printf(_("%cSpeed %c%dms\n%cAmoeba 1 %c%ds, %s\n%cAmoeba 2 %c%ds, %s\n%cMagic wall %c%ds, %s\n"
                   "%cExpanding wall %c%s\n%cCreatures %c%ds, %s\n%cGravity %c%s\n"
                   "%cKill player %c%s\n%cSweet eaten %c%s\n%cDiamond key %c%s\n%cDiamonds collected %c%d"),
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, cave->speed,
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, cave->time_visible(cave->amoeba_time), amoeba_state_string(cave->amoeba_state),
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, cave->time_visible(cave->amoeba_2_time), amoeba_state_string(cave->amoeba_2_state),
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, cave->time_visible(cave->magic_wall_time), magic_wall_state_string(cave->magic_wall_state),
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, (cave->expanding_wall_changed ? _("vertical") : _("horizontal")),
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, cave->time_visible(cave->creatures_direction_will_change), (cave->creatures_backwards ? _("backwards") : _("forwards")),
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, visible_name(cave->gravity_disabled ? GdDirection(MV_STILL) : cave->gravity),
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, (cave->kill_player ? _("yes") : _("no")),
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, (cave->sweet_eaten ? _("yes") : _("no")),
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, (cave->diamond_key_collected ? _("yes") : _("no")),
                GD_COLOR_INDEX_YELLOW, GD_COLOR_INDEX_LIGHTBLUE, cave->diamonds_collected);

    return s;
}


GameActivity::GameActivity(App *app, std::unique_ptr<GameControl> thegame)
    : Activity(app),
      game(std::move(thegame)),
      cellrenderer(*app->screen, gd_theme),
      gamerenderer(*app->screen, cellrenderer, *app->font_manager, *game),
      exit_game(false),
      show_highscore(false),
      paused(false)
{
}


GameActivity::~GameActivity() {
    gd_sound_off();
}


void GameActivity::shown_event() {
    app->game_active(true);
    int cell_size = cellrenderer.get_cell_size();
    if (gd_full_cave_view)
        app->screen->set_size(cell_size * GAME_RENDERER_SCREEN_SIZE_MAX_X, cell_size * GAME_RENDERER_SCREEN_SIZE_MAX_Y, gd_fullscreen);
    else
        app->screen->set_size(cell_size * GAME_RENDERER_SCREEN_SIZE_X, cell_size * GAME_RENDERER_SCREEN_SIZE_Y, gd_fullscreen);
    gamerenderer.screen_initialized();
    gd_music_stop();
    /* get the "restart" state here, so it goes back to false.
     * this is required because other activities that might get on top of the
     * gameactivity can use the escape key as an exit key, and after the activity
     * exists, this activity might think that the cave is to be restarted. */
    app->gameinput->restart();
}


void GameActivity::hidden_event() {
    app->game_active(false);
}


void GameActivity::redraw_event(bool full) const {
    gamerenderer.draw(full);
}


void GameActivity::keypress_event(KeyCode keycode, int gfxlib_keycode) {
    switch (keycode) {
        case EndGameKey:
            exit_game = true;
            break;
        case PauseKey:
            paused = !paused;
            if (paused)
                gd_sound_off();
            break;
        case RandomColorKey:
            gamerenderer.set_random_colors();
            break;
        case TakeSnapshotKey:
            if (game->save_snapshot())
                app->show_message(_("Snapshot taken."));
            else
                app->show_message(_("No cave loaded, no snapshot taken."));
            break;
        case RevertToSnapshotKey:
            if (game->load_snapshot())
                ; /* ok, and user has of course noticed */
            else
                app->show_message(_("No snapshot saved."));
            break;
        case CaveVariablesKey:
            gd_sound_off();
            app->show_text_and_do_command(_("Cave Information"), info_and_variables_of_cave(game->original_cave, game->played_cave.get()));
            break;
        case 'h':
        case 'H':
            gd_sound_off();
            app->show_help(gamehelp);
            break;
    }
}


void GameActivity::timer_event(int ms_elapsed) {
    GameRenderer::State state = gamerenderer.main_int(ms_elapsed, paused, app->gameinput);
    queue_redraw();

    /* state of game, returned by gd_game_main_int */
    switch (state) {
        case GameRenderer::Nothing:
            break;

        case GameRenderer::Stop:        /* game stopped, this could be a replay or a snapshot */
            exit_game = true;
            break;

        case GameRenderer::GameOver:    /* normal game stopped, may jump to highscore later. */
            exit_game = true;
            show_highscore = true;
            break;
    }

    if (exit_game) {
        /* pop the game activity. */
        app->enqueue_command(std::make_unique<PopActivityCommand>(app));
        /* if showing highscore, enqueue a command to show it. */
        /* there might be a command to be run after the game. if there is no highscore,
         * the game object will execute it, when popping. if there is a highscoresactivity
         * object, the command to be run is passed to it. */
        if (show_highscore && game->caveset->highscore.is_highscore(game->player_score)) {
            /* enter to highscore table */
            int rank = game->caveset->highscore.add(game->player_name, game->player_score);
            app->enqueue_command(std::make_unique<ShowHighScoreCommand>(app, nullptr, rank));
        } else {
            /* no high score */
        }
    }
}

