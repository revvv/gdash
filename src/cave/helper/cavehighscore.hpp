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
#ifndef CAVEHIGHSCORE_HPP_INCLUDED
#define CAVEHIGHSCORE_HPP_INCLUDED

#include "config.h"

#include <string>
#include <vector>
#include <utility>

/// A structure which holds a player name and a high score.
/// The HighScoreTable will store these sorted.
struct HighScore {
    std::string name;
    int score = 0;
    HighScore() = default;
    HighScore(std::string name_, int score_) : name(std::move(name_)), score(score_) {}
};


/// A HighScoreTable for a cave or a caveset.
class HighScoreTable {
private:
    std::vector<HighScore> table;   ///< The table
    enum { GD_HIGHSCORE_NUM = 20 }; ///< Maximum size

public:
    /// Return nth entry
    HighScore & operator[](unsigned n) {
        return table.at(n);
    }
    HighScore const & operator[](unsigned n) const {
        return table.at(n);
    }
    /// Check if the table has at least one entry. @return True, if there is an entry.
    bool has_highscore() {
        return !table.empty();
    }
    void clear() {
        table.clear();
    }
    unsigned size() const {
        return table.size();
    }
    bool is_highscore(std::string const & name, int score) const;
    int add(std::string const & name, int score);
};

#endif
