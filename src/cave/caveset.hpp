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
#ifndef CAVESET_HPP_INCLUDED
#define CAVESET_HPP_INCLUDED

#include "config.h"

#include <vector>

#include "cave/cavetypes.hpp"
#include "cave/helper/cavehighscore.hpp"
#include "cave/helper/reflective.hpp"
#include "cave/cavestored.hpp"

/// @ingroup Cave
class CaveSet : public Reflective {
public:
    /* variables */
    GdString name;                  ///< Name of caveset
    GdString description;           ///< Some words about the caveset
    GdString author;                ///< Author
    GdString difficulty;            ///< difficulty of the caveset, for info purposes
    GdString www;                   ///< link to author's webpage
    GdString date;                  ///< date of creation

    GdString story;                 ///< story for the caves
    GdString remark;                ///< notes about the game

    GdString title_screen;          ///< base64-encoded title screen image
    GdString title_screen_scroll;   ///< scrolling background for title screen image

    GdString charset;
    GdString fontset;

    GdInt initial_lives = 3;        ///< initial lives at game start; 3 is BDCFF default
    GdInt maximum_lives = 9;        ///< maximum lives; 9 is BDCFF default
    GdInt bonus_life_score = 500;   ///< bonus life / number of points; 500 is BDCFF default
    HighScoreTable highscore;       ///< highscore table for caveset

    GdBool edited;                  ///< changed since last save
    GdString filename;              ///< Loaded from / save to this file
    GdInt last_selected_cave;       ///< If running a game, the index of the selected gave is stored here
    GdInt last_selected_level;      ///< If running a game, the level of the selected gave is stored here

    std::vector<CaveStored> caves;

    void save_to_file(const char *filename);
    void set_name_from_filename(const char *filename);

    bool has_caves() const {
        return !caves.empty();
    }
    bool has_levels() const;
    bool has_replays() const;
    int first_selectable_cave_index() const;
    unsigned checksum() const;

// for reflective
public:
    static PropertyDescription const descriptor[];
    virtual PropertyDescription const *get_description_array() const {
        return descriptor;
    }
};

extern const char *gd_caveset_extensions[];

#endif                          /* _CAVESET_H */
