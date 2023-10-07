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
#ifndef GAMECONTROL_HPP_INCLUDED
#define GAMECONTROL_HPP_INCLUDED

#include "config.h"

#include <memory>
#include "cave/helper/cavemap.hpp"
#include "cave/cavetypes.hpp"
#include "cave/caverendered.hpp"
#include "cave/helper/cavereplay.hpp"

// forward declarations
class CaveSet;
class CaveStored;
class GameInputHandler;

/// @ingroup Cave
/**
 * The GameControl class controls the flow of a game: iterating the cave,
 * loading the caves, adding score, managing number of lives etc.
 * It loads a cave from the caveset, passes movement keypresses
 * to the cave iterate routine, controls uncover and cover animations,
 * saves replays, manages highscore.
 *
 * While it is not the responsibility of this class to draw the cave to
 * the screen, it manages two maps; one map (gfx_buffer) contains the cell
 * indexes, which are essentially the cave, and the other one (covered)
 * is a map of booleans which show which cell is covered and which is not
 * (after loading and before a new cave). These maps are in the GameControl
 * class so it can easily manage them when loading a cave.
 *
 * The GameControl class has members which store the cave number, the pointer
 * to the currently played cave and so on. When playing a game, a new
 * GameControl object is created, and it is given a caveset with a start cave
 * number, or a rendered cave with or without a replay, or a stored cave
 * to test from the editor.
 *
 * The object controls rendereding the caves if needed, and also has a gfx_buffer
 * structure, which is a map with the same size that of the cave, and stores
 * the pixbuf indexes of the elements. It also controls recording replays.
 * Manages counting lives, manages bonus scores.
 *
 * To run the whole process, the main_int() function has to be called regurarly.
 * Usually it is called at 25hz or 50hz, depending on user interface settings.
 * Its first parameter is a milliseconds value, in which the caller tells the
 * GameControl, how many milliseconds have passed since the last call (40ms or
 * 20ms, respectively). The object then decides what to do - it may load the
 * cave, it may iterate the cave, it may count bonus points. The return value
 * of this function has information valuable to the caller; for example, loading
 * the cave is signaled (new cave size, new cave colors), also it is signaled
 * when the status bars have to be updated.
 *
 * Internally the GameControl has a state counter, which starts from its lowest value
 * (LOAD_CAVE), and is or is not incremented by the main_int() call. The
 * life cycle of a singe cave is as follows:
 *  - LOAD_CAVE: to load the cave from the caveset (for replays, tests, nothing)
 *  - SHOW_STORY: showing the story of the cave (only for games)
 *  - (SHOW_STORY_WAIT: waiting for the user to press fire)
 *  - (UNCOVER_START: the first frame of the uncover animation)
 *  - UNCOVER: uncovering the cave (70 frames total)
 *  - (UNCOVER_ALL: last frame of uncover animation)
 *  - CAVE_RUNNING: play.
 *  - CHECK_BONUS_TIME: when the cave is finished, bonus time is added to the points,
 *      depending on remaining cave time.
 *  - WAIT_BEFORE_COVER: a 4s wait before covering the cave, after counting bonus
 *      points.
 *  - COVER_START: first frame of covering animation (8 frames total)
 *  - COVER_ALL: last frame of covering animation
 */
class GameControl {
public:
    /// Type of a GameControl.
    enum Type {
        TYPE_NORMAL,            ///< Normal game from a caveset, with cave number and level number.
        TYPE_SNAPSHOT,          ///< Playing from a snapshot (continuing).
        TYPE_TEST,              ///< Testing a cave from the editor.
        TYPE_REPLAY,            ///< Playing a replay.
        TYPE_CONTINUE_REPLAY,   ///< During the replay, the user took control of the cave.
    };

    /// Returned by the main_int() function, this state variable stores important information for
    /// the caller, who controls drawing the cave on the screen.
    enum State {
        STATE_CAVE_LOADED,      ///< The new cave is loaded.
        STATE_SHOW_STORY,       ///< When this is received, the story should be shown to the user. First frame.
        STATE_SHOW_STORY_WAIT,  ///< When the story is shown, and has to be redrawn.
        STATE_FIRST_FRAME,      ///< First frame - screen should be ready for drawing.
        STATE_NOTHING,          ///< Nothing special. Just draw the cave.
        STATE_STOP,             ///< Finished; the GameControl object can be destroyed.
        STATE_GAME_OVER,        ///< Finished, but this is a game over, so highscore can be recorded by the caller for the game.
    };

    /// Status bar state. So the renderer knows which status bar to show.
    enum StatusBarState {
        status_bar_none,        ///< Do not drow status bar.
        status_bar_uncover,     ///< Uncover - show name of cave and game.
        status_bar_game,        ///< In game, with points and other stuff.
        status_bar_game_over,   ///< Game over status.
    };

    /// Named constructor.
    static std::unique_ptr<GameControl> new_normal(CaveSet *caveset, std::string _player_name, int _cave, int _level);
    /// Named constructor.
    static std::unique_ptr<GameControl> new_snapshot();
    /// Named constructor.
    static std::unique_ptr<GameControl> new_test(CaveStored *cave, int level);
    /// Named constructor.
    static std::unique_ptr<GameControl> new_replay(CaveSet *caveset, CaveStored *cave, CaveReplay *replay);
    
    /// Default constructor - only used internally by the named constructors.
    GameControl(Type type);

    /* functions to work on */
    bool save_snapshot() const;
    bool load_snapshot();
    State main_int(GameInputHandler *inputhandler, bool allow_iterate);
    bool is_uncovering() const;

    /* public variables */
    Type type;

    std::string player_name;    ///< Name of player
    int player_score;           ///< Score of player
    int player_lives;           ///< Remaining lives of player

    CaveSet *caveset;           ///< Caveset used to load next cave in normal games.
    std::unique_ptr<CaveRendered> played_cave;  ///< Rendered version of the cave. This is the iterated one
    CaveStored *original_cave;  ///< original cave from caveset. Used to record highscore, as it is associated with the original cave in the caveset.

    int bonus_life_flash;       ///< flashing for bonus life
    StatusBarState statusbartype;
    int statusbarsince;               ///< The number of milliseconds since the last status bar change. If zero, just changed.

    CaveMap<int> gfx_buffer;      ///< contains the indexes to the cells; created by *start_level, deleted by *stop_game
    CaveMap<bool> covered;        ///< a map which stores which cells of the cave are still covered

    int replay_no_more_movements;
    bool story_shown;           ///< variable to remember if the story for a particular cave is to be shown.
    bool caveset_has_levels;    ///< set to true in the constructor if the caveset has difficulty levels

private:
    std::unique_ptr<CaveReplay> replay_record;
    CaveReplay *replay_from;
    unsigned int cave_num;      ///< actual playing cave number
    unsigned int level_num;     ///< actual playing level
    int cave_score;             ///< score collected in this cave
    int milliseconds_game;      ///< here we remember, how many milliseconds have passed since we last iterated the cave
    int state_counter;          ///< counter used to control the game flow, rendering of caves
    
    static std::unique_ptr<CaveRendered> snapshot_cave;   ///< Saved snapshot

    void add_bonus_life(bool inform_user);
    void increment_score(int increment);
    void select_next_level_indexes();

    void set_status_bar_state(StatusBarState s);

    /* internal functions for the different states */
    void load_cave();
    void unload_cave();
    State show_story();
    void start_uncover();
    void uncover_animation();
    void uncover_all();
    State iterate_cave(GameInputHandler *inputhandler);
    State wait_before_cover();
    void check_bonus_score();
    void check_bonus_score_fast();
    void cover_animation();
    State finished_covering();
};

#endif
