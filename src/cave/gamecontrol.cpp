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

#include <glib.h>
#include <glib/gi18n.h>

#include "cave/gamecontrol.hpp"
#include "cave/cavestored.hpp"
#include "cave/caverendered.hpp"
#include "cave/caveset.hpp"
#include "sound/sound.hpp"
#include "misc/util.hpp"
#include "input/gameinputhandler.hpp"
#include "settings.hpp"



/* prepare cave, gfx buffer */
#define GAME_INT_LOAD_CAVE -73
/* show description/note of cave. */
#define GAME_INT_SHOW_STORY -72
/* waiting fire button after showing the story. */
#define GAME_INT_SHOW_STORY_WAIT -71
/* start uncovering */
#define GAME_INT_START_UNCOVER -70
/* ...70 frames until full uncover... */
#define GAME_INT_UNCOVER_ALL -1
/* normal running state. */
#define GAME_INT_CAVE_RUNNING 0
/* add points for remaining time */
#define GAME_INT_CHECK_BONUS_TIME 1
/* ...2..99 = wait and do nothing, after adding time */
#define GAME_INT_WAIT_BEFORE_COVER 2
/* start covering */
#define GAME_INT_COVER_START 100
/* ... 8 frames of cover animation */
#define GAME_INT_COVER_ALL 108


std::unique_ptr<CaveRendered> GameControl::snapshot_cave;   ///< Saved snapshot


/// The default constructor which fills members with some initial values.
GameControl::GameControl(Type type) :
    type(type),
    player_score(0),
    player_lives(0),
    caveset(NULL),
    played_cave(),
    original_cave(NULL),
    bonus_life_flash(0),
    statusbartype(status_bar_none),
    statusbarsince(0),
    story_shown(false),
    caveset_has_levels(false),
    cave_num(0),
    level_num(0),
    milliseconds_game(0),
    state_counter(GAME_INT_LOAD_CAVE) {
}

/// Create a full game from the caveset.
/// @returns A newly allocated GameControl.
std::unique_ptr<GameControl> GameControl::new_normal(CaveSet *caveset, std::string player_name, int cave, int level) {
    auto g = std::make_unique<GameControl>(TYPE_NORMAL);
    g->caveset = caveset;
    g->player_name = player_name;
    g->cave_num = cave;
    g->level_num = level;
    g->caveset_has_levels = caveset->has_levels();
    g->player_lives = g->caveset->initial_lives;
    g->story_shown = false;

    return g;
}

/// Create a new game of a single snapshot.
/// @returns A newly allocated GameControl.
std::unique_ptr<GameControl> GameControl::new_snapshot() {
    if (snapshot_cave.get() == NULL)
        throw std::logic_error("no snapshot");
    
    auto g = std::make_unique<GameControl>(TYPE_SNAPSHOT);
    g->played_cave = std::make_unique<CaveRendered>(*snapshot_cave);

    return g;
}

/// Create a new game to test in the editor.
/// @returns A newly allocated GameControl.
std::unique_ptr<GameControl> GameControl::new_test(CaveStored *cave, int level) {
    auto g = std::make_unique<GameControl>(TYPE_TEST);
    g->original_cave = cave;
    g->level_num = level;

    return g;
}

/// Create a cave replay.
/// @returns A newly allocated GameControl.
std::unique_ptr<GameControl> GameControl::new_replay(CaveSet *caveset, CaveStored *cave, CaveReplay *replay) {
    auto g = std::make_unique<GameControl>(TYPE_REPLAY);
    g->caveset = caveset;
    g->original_cave = cave;
    g->replay_from = replay;

    return g;
}


/// Add a bonus life (if live number is not more than maximum number of lives.
/// Only feasible for normal games, not for replays or tests.
/// @param inform_user If set to true, will also show bonus life flash. Sometimes this
///      is not desired (after completing an intermission, for example).
void GameControl::add_bonus_life(bool inform_user) {
    /* only inform about bonus life when playing a game */
    /* or when testing the cave (so the user can see that a bonus life can be earned in that cave */
    if (type == TYPE_NORMAL || type == TYPE_TEST)
        if (inform_user) {
            gd_sound_play_bonus_life();
            bonus_life_flash = 4000;
        }

    /* really increment number of lifes? only in a real game, nowhere else. */
    if (type == TYPE_NORMAL && player_lives < caveset->maximum_lives)
        /* only add a life, if lives is >0.  lives==0 is a test run or a snapshot, no bonus life then. */
        /* also, obey max number of bonus lives. */
        player_lives++;
}

/// Increment score of the player.
/// Store in the game score, in player score, and also in the replay, if any.
/// If a bonus life is got, show bonus life flash.
void GameControl::increment_score(int increment) {
    /* remember original score */
    int prev = 0;
    if (caveset)
        prev = player_score / caveset->bonus_life_score;
    /* add score */
    player_score += increment;
    cave_score += increment;
    if (replay_record.get())  /* also record to replay */
        replay_record->score += increment;
    if (caveset && player_score / caveset->bonus_life_score > prev)
        add_bonus_life(true);   /* if score crossed bonus_life_score point boundary, player won a bonus life */
}


/// Load the cave.
/// The role of this function greatly depends on the GameControl type.
/// The cave might be loaded from a caveset, it might be just a copy of a CaveRendered given.
void GameControl::load_cave() {
    guint32 seed;

    /* delete gfx buffer */
    gfx_buffer.remove();
    covered.remove();

    snapshot_cave = NULL;

    /* load the cave */
    cave_score = 0;
    switch (type) {
        case TYPE_NORMAL:
            /* specified cave from memory; render the cave with a randomly selected seed */
            original_cave = &caveset->caves[cave_num];
            /* for playing: seed=random */
            seed = g_random_int_range(0, GD_CAVE_SEED_MAX);
            played_cave = std::make_unique<CaveRendered>(*original_cave, level_num, seed);
            played_cave->setup_for_game();
            if (played_cave->intermission && played_cave->intermission_instantlife)
                add_bonus_life(false);

            /* create replay */
            replay_record = std::make_unique<CaveReplay>();
            replay_record->level = played_cave->rendered_on + 1; /* compatibility with bdcff - level=1 is written in file */
            replay_record->seed = played_cave->render_seed;
            replay_record->checksum = gd_cave_adler_checksum(*played_cave); /* calculate a checksum for this cave */
            replay_record->recorded_with = PACKAGE_STRING;                  // name of gdash and version
            replay_record->player_name = player_name;
            replay_record->date = gd_get_current_date_time();
            break;

        case TYPE_TEST:
            seed = g_random_int_range(0, GD_CAVE_SEED_MAX);
            played_cave = std::make_unique<CaveRendered>(*original_cave, level_num, seed);
            played_cave->setup_for_game();
            break;

        case TYPE_SNAPSHOT:
            /* if a snapshot or test is requested, that one... just create a copy */
            /* copy already created in new_game, so nothing to do here. */
            g_assert(played_cave.get() != NULL);
            g_assert(original_cave == NULL);
            break;

        case TYPE_REPLAY:
            g_assert(replay_from != NULL);
            g_assert(played_cave.get() == NULL);

            replay_record.release();
            replay_from->rewind();
            replay_no_more_movements = 0;

            /* -1 is because level=1 is in bdcff for level 1, and internally we number levels from 0 */
            played_cave = std::make_unique<CaveRendered>(*original_cave, replay_from->level - 1, replay_from->seed);
            played_cave->setup_for_game();
            break;

        case TYPE_CONTINUE_REPLAY:
            g_assert_not_reached();
            break;
    }

    milliseconds_game = 0;      /* set game timer to zero, too */
    state_counter = GAME_INT_SHOW_STORY;
}


/// Saves a snapshot of the currently played cave to the static snapshot variable.
/// @return true, if successful
bool GameControl::save_snapshot() const {
    if (played_cave.get() == NULL)
        return false;

    snapshot_cave = std::make_unique<CaveRendered>(*played_cave);
    return true;
}


/// Replaces the current game with a snapshot game from the saved snapshot cave.
/// @return true, if successful
bool GameControl::load_snapshot() {
    if (snapshot_cave.get() == NULL)
        return false;

    /* overwrite this object with a snapshot game */
    //*this = GameControl(TYPE_SNAPSHOT); // triggers uncover mosaic and as side effect entering outbox will end the game instead of loading next cave!
    played_cave = std::make_unique<CaveRendered>(*snapshot_cave);

    /* success */
    return true;
}


bool GameControl::is_uncovering() const {
    return state_counter > GAME_INT_START_UNCOVER && state_counter < GAME_INT_UNCOVER_ALL;
}

/// For games, calculate the next cave number and level number.
/// Called from main_int(), in a normal game, when the cave
/// is successfully finished (or not successfully, but it is an
/// intermission).
/// This does NOT load the new cave, just calculates its number.
void GameControl::select_next_level_indexes() {
    cave_num++;         /* next cave */

    /* if no more caves at this level, back to first one */
    if (cave_num >= caveset->caves.size()) {
        cave_num = 0;
        if (caveset_has_levels) {
            /* but now more difficult */
            level_num++;
            /* if level 5 finished, back to first cave, same difficulty (original game behaviour) */
            if (level_num > 4)
                level_num = 4;
        }
    }

    /* show story. */
    /* if the user fails to solve the cave, the story will not be shown again */
    story_shown = false;
}


void GameControl::set_status_bar_state(StatusBarState s) {
    statusbartype = s;
    statusbarsince = 0;
}


/// Show story for a cave, if it has not been already shown.
GameControl::State GameControl::show_story() {
    /* if we have a story, and user settings permit showing that etc */
    if (gd_show_story && !story_shown && type == TYPE_NORMAL && original_cave->story != "") {
        played_cave->clear_sounds();            /* to stop cover sound from previous cave; there should be no cover sound when the user reads the story */
        gd_sound_play_sounds(played_cave->sound1, played_cave->sound2, played_cave->sound3);
        state_counter = GAME_INT_SHOW_STORY_WAIT;
        set_status_bar_state(status_bar_none);
        story_shown = true;   /* so the story will not be shown again, for example when the cave is not solved and played again. */
        return STATE_SHOW_STORY;
    } else {
        /* if no story */
        state_counter = GAME_INT_START_UNCOVER;
        return STATE_NOTHING;
    }
}


/// Start uncover animation. Create gfx buffers.
void GameControl::start_uncover() {
    /* create gfx buffer. fill with "invalid" */
    gfx_buffer.set_size(played_cave->w, played_cave->h, -1);
    /* cover all cells of cave */
    covered.set_size(played_cave->w, played_cave->h, true);

    /* to play cover sound */
    played_cave->clear_sounds();
    played_cave->sound_play(GD_S_COVER, played_cave->player_x, played_cave->player_y);
    gd_sound_play_sounds(played_cave->sound1, played_cave->sound2, played_cave->sound3);

    /* advance to next state */
    state_counter += 1;
}


/// One frame of the uncover animation.
void GameControl::uncover_animation() {
    /* original game uncovered one cell per line each frame.
     * we have different cave sizes, so uncover width*height/40 random cells each frame. (original was width=40).
     * this way the uncovering is the same speed also for intermissions. */
    for (int j = 0; j < played_cave->w * played_cave->h / 40; j++)
        covered(g_random_int_range(0, played_cave->w), g_random_int_range(0, played_cave->h)) = false;
    state_counter += 1; /* as we did something, advance the counter. */
}


/// Uncover all cells of cave, and switch in to the normal cave running state.
void GameControl::uncover_all() {
    covered.fill(false);

    /* to stop uncover sound. */
    played_cave->clear_sounds();
    gd_sound_play_sounds(played_cave->sound1, played_cave->sound2, played_cave->sound3);

    state_counter = GAME_INT_CAVE_RUNNING;
}


/// One frame of the cover animation.
void GameControl::cover_animation() {
    /* covering eight times faster than uncovering. */
    for (int j = 0; j < played_cave->w * played_cave->h * 8 / 40; j++)
        covered(g_random_int_range(0, played_cave->w), g_random_int_range(0, played_cave->h)) = true;

    state_counter += 1; /* as we did something, advance the counter. */
}


/// Iterate the cave.
/// @param fast_forward If set to true, cave will be iterated at 25fps, regardless of cave speed calculated by the cave.
/// @param player_move The direction of move, by keypresses.
/// @param fire If the user pressed the fire.
/// @param suicide If the user pressed the suicide button.
/// @param restart If the user requested a cave restart.
GameControl::State GameControl::iterate_cave(GameInputHandler *inputhandler) {
    State return_state;
    bool fast_forward = false, fire = false, suicide = false, restart = false;
    GdDirectionEnum player_move = MV_STILL;

    if (inputhandler != NULL) {
        fast_forward = inputhandler->fast_forward;
        fire = inputhandler->fire1() || inputhandler->fire2();
        suicide = inputhandler->suicide;
        restart = inputhandler->restart();
        player_move = gd_direction_from_keypress(inputhandler->up(), inputhandler->down(), inputhandler->left(), inputhandler->right());
    }

    int irl_cavespeed;      // caveset in real life. if fast_forward is enabled, we ignore the speed of the cave (but it thinks it runs at normal speed)
    if (!fast_forward)
        irl_cavespeed = played_cave->speed;   /* cave speed in ms, like 175ms/frame */
    else
        irl_cavespeed = 40;                   /* if fast forward, ignore cave speed, and go as 25 iterations/sec */

    /* if we are playing a replay, but the user intervents, continue as a snapshot. */
    /* do not trigger this for fire, as it would not be too intuitive. */
    if (type == TYPE_REPLAY)
        if (player_move != MV_STILL) {
            type = TYPE_CONTINUE_REPLAY;
            replay_from = NULL;
        }

    /* ANYTHING EXCEPT A TIMEOUT, WE ITERATE THE CAVE */
    /* iterate cave */
    return_state = STATE_NOTHING; /* normally nothing happes. but if we iterate, this might change. */
    milliseconds_game += 40;

    /* decide if cave will be iterated. */
    if (played_cave->player_state != GD_PL_TIMEOUT && milliseconds_game >= irl_cavespeed) {
        // ok, cave has to be iterated.

        /* IF PLAYING FROM REPLAY, OVERWRITE KEYPRESS VARIABLES FROM REPLAY */
        if (type == TYPE_REPLAY) {
            /* if the user does touch the keyboard, we immediately exit replay, and he can continue playing */
            bool result = replay_from->get_next_movement(player_move, fire, suicide);
            /* if could not get move from snapshot, continue from keyboard input. */
            if (!result)
                replay_no_more_movements++;
            /* if no more available movements, and the user does not do anything, we cover cave and stop game. */
            if (replay_no_more_movements > 15)
                state_counter = GAME_INT_COVER_START;
        }

        milliseconds_game -= irl_cavespeed;
        // if recording the replay, now store the movement
        if (replay_record.get() != NULL)
            replay_record->store_movement(player_move, fire, suicide);

        /* cave iterate gives us a new player move, which might have diagonal movements removed */
        played_cave->iterate(player_move, fire, suicide);
        if (played_cave->score)
            increment_score(played_cave->score);
        return_state = STATE_NOTHING;
        /* as we iterated, the score and the like could have been changed.
         * but only do this if the player is not hatched yet (ie only after cave start signal) */
        if (played_cave->hatched)
            set_status_bar_state(status_bar_game);

        gd_sound_play_sounds(played_cave->sound1, played_cave->sound2, played_cave->sound3);
    }

    if (played_cave->player_state == GD_PL_EXITED) {
        // if recording the replay, now store the movement
        if (replay_record.get() != NULL)
            replay_record->success = true;
        /* start adding points for remaining time */
        state_counter = GAME_INT_CHECK_BONUS_TIME;
        played_cave->clear_sounds();
        played_cave->sound_play(GD_S_FINISHED, played_cave->player_x, played_cave->player_y); /* play cave finished sound */
        gd_sound_play_sounds(played_cave->sound1, played_cave->sound2, played_cave->sound3);
    }

    /* player died and user presses fire -> try again */
    /* time out and user presses fire -> try again */
    /* user requests cave restart (when stuck under stones) -> try again */
    if (((played_cave->player_state == GD_PL_DIED || played_cave->player_state == GD_PL_TIMEOUT) && fire) || restart) {
        if (!played_cave->intermission && player_lives > 0) // normal cave, died -> lives decreased
            player_lives--;
        if (type == TYPE_NORMAL && player_lives == 0) {
            /* wait some time - this is a game over */
            state_counter = GAME_INT_WAIT_BEFORE_COVER;
        } else {
            /* start cover animation immediately */
            state_counter = GAME_INT_COVER_START;
        }
    }

    return return_state;
}

/// After adding bonus points, we wait some time before starting to cover.
/// This is the FIRST frame... so we check for game over and maybe jump there.
/// If no more lives, game is over.
GameControl::State GameControl::wait_before_cover() {
    state_counter += 1; /* advance counter */
    if (type == TYPE_NORMAL && player_lives == 0) {
        set_status_bar_state(status_bar_game_over);
    }
    return STATE_NOTHING;
}

/// Add some bonus scores.
/// This may advance the state, or not - depending on if there is still cave time left.
void GameControl::check_bonus_score() {
    /* player exited, but some time remained. now count bonus points. */
    /* if time remaining, bonus points are added. do not start animation yet. */
    if (played_cave->time > 0) {
        // cave time is mult'd by timing factor, which is 1000 or 1200, depending on cave pal setting.
        // we do not really care this here, but have to multiply.
        if (played_cave->time > 60 * played_cave->timing_factor) { /* if much time (>60s) remained, fast counter :) */
            played_cave->time -= 9 * played_cave->timing_factor; /* decrement by nine each frame, so it also looks like a fast counter. 9 is 8+1! */
            increment_score(played_cave->timevalue * 9);
        } else {
            played_cave->time -= played_cave->timing_factor;    /* subtract number of "milliseconds" - nothing to do with gameplay->millisecs! */
            increment_score(played_cave->timevalue);            /* higher levels get more bonus points per second remained */
        }
        // maybe we we substracted too much (remaining time was fraction of a second)
        if (played_cave->time < 0)
            played_cave->time = 0;
    } else
        /* if no more points, start waiting a bit, and later start covering. */
        state_counter = GAME_INT_WAIT_BEFORE_COVER;

    /* play bonus sound - which is the same as the seconds sound, when only <10 seconds left */
    played_cave->set_seconds_sound();
    gd_sound_play_sounds(played_cave->sound1, played_cave->sound2, played_cave->sound3);
    /* number of lives changed -> status bar changed */
    set_status_bar_state(status_bar_game);
}

/// Forget all stuff for the current cave.
/// Maybe push the recording into the replays etc.
void GameControl::unload_cave() {
    gfx_buffer.remove();
    covered.remove();

    /* if this is a normal game: */
    if (type == TYPE_NORMAL) {
        // if the replay was successful, or it has some length which makes sense, add it to the cave.
        if (replay_record->success || replay_record->length() >= 16)
            original_cave->replays.push_back(*replay_record);
        replay_record.release();

        // if the cave was successful, manage bonus life and points
        if (played_cave->player_state == GD_PL_EXITED) {
            // one life extra for completing intermission
            if (played_cave->intermission && played_cave->intermission_rewardlife)
                add_bonus_life(false);
            // we also have added points for remaining time -> now check for highscore
            original_cave->highscore.add(player_name, cave_score);
        }

        /* put into game statistics */
        ++original_cave->stat_level_played[level_num];
        if (played_cave->player_state == GD_PL_EXITED) {
            ++original_cave->stat_level_played_successfully[level_num];
        }
        if (played_cave->diamonds_collected > original_cave->stat_level_most_diamonds[level_num])
            original_cave->stat_level_most_diamonds[level_num] = played_cave->diamonds_collected;
        if (cave_score > original_cave->stat_level_highest_score[level_num])
            original_cave->stat_level_highest_score[level_num] = cave_score;
        if (played_cave->player_state == GD_PL_EXITED) {
            /* use timing in seconds, but use cave seconds (maybe pal seconds = 1200ms) */
            /* only record time, if it was successfull (otherwise we would record 1sec deaths :D) */
            int time = played_cave->time_elapsed / played_cave->timing_factor;
            if (original_cave->stat_level_best_time[level_num] == 0
                || time < original_cave->stat_level_best_time[level_num])
                original_cave->stat_level_best_time[level_num] = time;
        }
        original_cave = NULL;
    }

    played_cave.reset();
}

/// After finishing the cave cover animation, decide what to do next.
/// For a normal game, manage replays, and select next level.
/// For testing, do nothing, as the cave will be reloaded.
/// For other types, signal GameControl finish.
GameControl::State GameControl::finished_covering() {
    State return_state;

    /* if this is a normal game: */
    if (type == TYPE_NORMAL) {
        /* if successful or this was an intermission, advance to next level.
         * for intermissions, there is only one chance given, so always go to next. */
        if (played_cave->player_state == GD_PL_EXITED || played_cave->intermission)
            select_next_level_indexes();
        /* maybe game over? */
        if (player_lives > 0)
            return_state = STATE_NOTHING;
        else
            return_state = STATE_GAME_OVER;
    } else if (type == TYPE_TEST) {
        /* if testing, start again. do nothing, cave will be reloaded. */
        return_state = STATE_NOTHING;
    } else
        /* for snapshots and replays and the like, this is the end. */
        return_state = STATE_STOP;

    /* load next cave on next call. */
    state_counter = GAME_INT_LOAD_CAVE;
    return return_state;
}


/// The main_int function, which controls the whole working of a GameControl.
/// It must be called at 25fps - 40 times a second. Then the cave will run in real time.
/// @param inputhandler The GameInputHandler which has user input.
/// @param allow_iterate If the game is paused by the user, this prevents the cave from iterating. But the cells are still animated!
/// @param fast_forward If set to true, cave will be iterated for every call (at 25fps), regardless of cave speed calculated by the cave itself.
GameControl::State GameControl::main_int(GameInputHandler *inputhandler, bool allow_iterate) {
    State return_state;

    if (bonus_life_flash > 0) {  /* bonus life flash - milliseconds */
        bonus_life_flash -= 40;
        if (bonus_life_flash < 0)
            bonus_life_flash = 0;
    }
    statusbarsince += 40;   /* milliseconds */

    if (state_counter < GAME_INT_LOAD_CAVE) {
        /* cannot be less than uncover start. */
        g_assert_not_reached();
    } else if (state_counter == GAME_INT_LOAD_CAVE) {
        /* do our tasks associated with loading a new cave. */
        /* the caller will not know about this yet */
        load_cave();
        set_status_bar_state(status_bar_none);
        return_state = STATE_CAVE_LOADED;
    } else if (state_counter == GAME_INT_SHOW_STORY) {
        /* for normal game, every cave can have a long string of description/story. show that. */
        return_state = show_story();
    } else if (state_counter == GAME_INT_SHOW_STORY_WAIT) {
        /* if we are showing the story, we are waiting for the user to press fire. nothing else */
        /* if user presses fire (or maybe esc), proceed with loading the cave. */
        if (inputhandler != NULL && (inputhandler->fire1() || inputhandler->restart()))
            state_counter = GAME_INT_START_UNCOVER;
        return_state = STATE_SHOW_STORY_WAIT;
    } else if (state_counter == GAME_INT_START_UNCOVER) {
        /* the very beginning. this will be the first cave frame drawn by the caller. */
        start_uncover();
        /* very important: tell the caller that we loaded a new cave. */
        /* size of the cave might be new, colors might be new, and so on. */
        set_status_bar_state(status_bar_uncover);
        return_state = STATE_FIRST_FRAME;
    } else if (state_counter < GAME_INT_UNCOVER_ALL) {
        /* uncover animation */
        uncover_animation();
        /* if fast uncover animation in test requested, do 3 more times */
        if (type == TYPE_TEST && gd_fast_uncover_in_test)
            for (unsigned i = 0; i < 3 && state_counter < GAME_INT_UNCOVER_ALL; ++i)
                uncover_animation();
        return_state = STATE_NOTHING;
    } else if (state_counter == GAME_INT_UNCOVER_ALL) {
        /* time to uncover the whole cave. */
        uncover_all();
        return_state = STATE_NOTHING;
    } else if (state_counter == GAME_INT_CAVE_RUNNING) {
        /* normal. */
        if (allow_iterate)
            return_state = iterate_cave(inputhandler);
        else
            return_state = STATE_NOTHING;
    } else if (state_counter == GAME_INT_CHECK_BONUS_TIME) {
        /* before covering, we check for time bonus score */
        check_bonus_score();
        return_state = STATE_NOTHING;
    } else if (state_counter == GAME_INT_WAIT_BEFORE_COVER) {
        /* after adding bonus points, we wait some time before starting to cover. this is the FIRST frame... so we check for game over and maybe jump there */
        /* if no more lives, game is over. */
        return_state = wait_before_cover();
    } else if (state_counter > GAME_INT_WAIT_BEFORE_COVER && state_counter < GAME_INT_COVER_START) {
        /* after adding bonus points, we wait some time before starting to cover. ... and the other frames. */
        /* here we do nothing, but wait */
        state_counter += 1;     /* 40ms elapsed, advance counter */
        return_state = STATE_NOTHING;
    } else if (state_counter == GAME_INT_COVER_START) {
        /* starting to cover. start cover sound. */
        played_cave->clear_sounds();
        played_cave->sound_play(GD_S_COVER, played_cave->player_x, played_cave->player_y);
        /* to play cover sound */
        gd_sound_play_sounds(played_cave->sound1, played_cave->sound2, played_cave->sound3);

        state_counter += 1;
        return_state = STATE_NOTHING;
    } else if (state_counter > GAME_INT_COVER_START && state_counter < GAME_INT_COVER_ALL) {
        /* covering. */
        cover_animation();
        return_state = STATE_NOTHING;
    } else if (state_counter == GAME_INT_COVER_ALL) {
        /* cover all */
        covered.fill(true);

        state_counter += 1;
        return_state = STATE_NOTHING;
    } else {
        return_state = finished_covering();
        unload_cave();
    }

    return return_state;
}
