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

#include <algorithm>
#include "cave/helper/cavehighscore.hpp"

/// Check if the achieved score will be put on the list.
/// @param name The name of the player.
/// @param score The score to be checked.
/// @return true, if the score is a highscore, and can be put on the list.
bool HighScoreTable::is_highscore(const std::string &name, int score) const {

    /* do not add duplicates to the list */
    for (unsigned int i = 0; i < table.size(); i++) {
        if (name == table[i].name && score == table[i].score) {
            return false;
        }
    }

    /* if score is above zero AND bigger than the last one */
    if (score > 0 && (table.size() < GD_HIGHSCORE_NUM || score > table.back().score))
        return true;

    return false;
}

/// Adds a player with some score to the highscore table.
/// Returns the new rank.
/// @param name The name of the player.
/// @param score The score achieved.
/// @return The index in the table, or -1 if did not fit.
int HighScoreTable::add(const std::string &name, int score) {
    if (!is_highscore(name, score))
        return -1;

    /* add to the end */
    table.push_back(HighScore(name, score));
    std::sort(table.begin(), table.end(), [] (const HighScore &a, const HighScore &b) {
        return b.score < a.score;
    });
    /* if too big, remove the lowest ones (after sorting) */
    if (table.size() > GD_HIGHSCORE_NUM)
        table.resize(GD_HIGHSCORE_NUM);

    /* and find it so we can return an index */
    for (unsigned int i = 0; i < table.size(); i++)
        if (table[i].name == name && table[i].score == score)
            return i;

    return -1;
}
