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

#ifndef GAMERENDERER_HPP_INCLUDED
#define GAMERENDERER_HPP_INCLUDED

#include "config.h"

#include <vector>
#include <string>
#include <memory>

#include "cave/colors.hpp"
#include "cave/cavetypes.hpp"
#include "gfx/pixmapstorage.hpp"

class Screen;
class CellRenderer;
class FontManager;
class GameControl;
class GameInputHandler;
class Pixbuf;
class Pixmap;

#define GAME_RENDERER_SCREEN_SIZE_X (20)
#define GAME_RENDERER_SCREEN_SIZE_Y (12.5)

enum GdStatusBarType {
    GD_STATUS_BAR_ORIGINAL,
    GD_STATUS_BAR_1STB,
    GD_STATUS_BAR_CRLI,
    GD_STATUS_BAR_FINAL,
    GD_STATUS_BAR_ATARI_ORIGINAL,
};
enum {
    // should be the last from the status bar type
    GD_STATUS_BAR_MAX = int(GD_STATUS_BAR_ATARI_ORIGINAL)
};
const char **gd_status_bar_colors_get_names();


/// @ingroup Cave
///
/// @brief This class manages drawing the cave and scrolling on the screen; it also
/// receives keypresses (controls) from the user and passes it to the game
/// controlling class GameControl.
///
/// The GameRenderer class can be used the following way:
///
/// - Creating an object, and assigning it a CellRenderer, a FontManager, a Screen
///   for the graphics stuff, and a GameControl object to control the game.
/// - The GameRenderer::main_int() function is to be called regurarly, and passed
///   the number of milliseconds elapsed and the user controls.
///
/// The object will then control the game and also draw it. And that's all.
/// Showing the story, scrolling the cave, drawing the status bar, enabling and
/// disabling game pause and everything is handled inside.
class GameRenderer : public PixmapStorage {
    Screen &screen;
    CellRenderer &cells;
    FontManager &font_manager;
    GameControl &game;

    int play_area_w;
    int play_area_h;
    int statusbar_height;
    int statusbar_y1;
    int statusbar_y2;
    int statusbar_mid;

    bool out_of_window;

    bool show_replay_sign;

    double scroll_x, scroll_y;
    double scroll_speed_x, scroll_speed_y;
    std::vector<double> scroll_speeds_during_uncover;
    double scroll_speed_normal;
    int scroll_ms;
    int scroll_desired_x, scroll_desired_y;

    int millisecs_game;
    int animcycle;              ///< animation frames, from 0 to 7, and then again 0

    mutable bool must_draw_cave, must_clear_screen, must_draw_status, must_draw_story;

    // the last set status bar in the game
    bool status_bar_fast, status_bar_alternate, status_bar_paused;


    // for showing the story
    struct StoryStuff {
        std::vector<std::string> wrapped_text;
        int scroll_y, max_y;
        unsigned linesavailable;
        std::unique_ptr<Pixmap> background;
    } mutable story;

    bool cave_scroll(int logical_size, int physical_size, int center, bool exact, double &current, int &desired, double & currspeed);
    bool scroll(int ms, bool exact_scroll);
    void scroll_to_origin();

    void drawstory() const;
    void drawcave() const;
    bool drawstatus_firstline(bool in_game) const;
    void drawstatus_uncover() const;
    void drawstatus_game() const;
    void drawstatus() const;

    void set_colors_from_cave();
    void select_status_bar_colors();

    struct StatusBarColors {
        GdColor background;
        GdColor diamond_needed;
        GdColor diamond_value;
        GdColor diamond_collected;
        GdColor score;
        GdColor default_color;
    };
    StatusBarColors cols;

public:
    GameRenderer(Screen &screen_, CellRenderer &cells_, FontManager &font_manager_, GameControl &game_);

    /// This enum shows how the game ended.
    enum State {
        Nothing,        ///< Nothing happened, the game still runs.
        Stop,           ///< The game is stopped.
        GameOver,       ///< The game is stopped, and highscore should be shown.
    };
    State main_int(int millisecs_elapsed, bool paused, GameInputHandler *gameinput);

    /**
     * One should call this function when the Screen is initialized,
     * and the GameRenderer can ask for its sizes to calculate status
     * bar alignment.
     */
    void screen_initialized();

    /**
     * Show replay sign means that for non-real-game game types,
     * the status bar can show the type ("PLAYING REPLAY" or "TESTING CAVE").
     * By default, it is turned on.
     */
    void set_show_replay_sign(bool srs);

    /**
     * The user can request some random colors if he does not like the
     * ones from the cave. */
    void set_random_colors();

    /**
     * Call this to do the drawing.
     * @param everything Redraw everything, or just the parts changed.
     */
    void draw(bool full) const;

    /** Implement PixbufStorage. */
    void release_pixmaps();
};


#endif
