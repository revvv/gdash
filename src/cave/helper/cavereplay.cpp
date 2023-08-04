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

#include <vector>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cassert>

#include "cave/helper/cavereplay.hpp"
#include "cave/cavebase.hpp"


/* entries. */
/* type given for each element */
PropertyDescription const CaveReplay::descriptor[] = {
    /* default data */
    {"Level", GD_TYPE_INT, GD_ALWAYS_SAVE, NULL, GetterBase::create_new(&CaveReplay::level)},
    {"RandomSeed", GD_TYPE_INT, GD_ALWAYS_SAVE, NULL, GetterBase::create_new(&CaveReplay::seed)},
    {"Player", GD_TYPE_STRING, 0, NULL, GetterBase::create_new(&CaveReplay::player_name)},
    {"Date", GD_TYPE_STRING, 0, NULL, GetterBase::create_new(&CaveReplay::date)},
    {"Comment", GD_TYPE_STRING, 0, NULL, GetterBase::create_new(&CaveReplay::comment)},
    {"RecordedWith", GD_TYPE_STRING, 0, NULL, GetterBase::create_new(&CaveReplay::recorded_with)},
    {"Score", GD_TYPE_INT, 0, NULL, GetterBase::create_new(&CaveReplay::score)},
    {"Duration", GD_TYPE_INT, 0, NULL, GetterBase::create_new(&CaveReplay::duration)},
    {"Success", GD_TYPE_BOOLEAN, 0, NULL, GetterBase::create_new(&CaveReplay::success)},
    {"CheckSum", GD_TYPE_INT, 0, NULL, GetterBase::create_new(&CaveReplay::checksum)},
    {NULL}  /* end of array */
};

CaveReplay::CaveReplay() :
    current_playing_pos(0),
    level(1),
    seed(0),
    score(0),
    duration(0),
    success(false),
    checksum(0),
    wrong_checksum(false),
    saved(false) {
}


/* store movement in a replay */
void CaveReplay::store_movement(GdDirectionEnum player_move, bool player_fire, bool suicide) {
    assert(player_move == (player_move & REPLAY_MOVE_MASK));
    movements.push_back((player_move) | (player_fire ? REPLAY_FIRE_MASK : 0) | (suicide ? REPLAY_SUICIDE_MASK : 0));
}

/* get next available movement from a replay; store variables to player_move, player_fire, suicide */
/* return true if successful */
bool CaveReplay::get_next_movement(GdDirectionEnum &player_move, bool &player_fire, bool &suicide) {
    /* if no more available movements */
    if (current_playing_pos >= movements.size())
        return false;

    movement data = movements[current_playing_pos++];

    suicide = (data & REPLAY_SUICIDE_MASK) != 0;
    player_fire = (data & REPLAY_FIRE_MASK) != 0;
    player_move = (GdDirectionEnum)(data & REPLAY_MOVE_MASK);

    return true;
}

void CaveReplay::rewind() {
    current_playing_pos = 0;
}

bool CaveReplay::load_one_from_bdcff(const std::string &str) {
    bool up, down, left, right;
    bool fire, suicide;
    int num = -1;

    fire = suicide = up = down = left = right = false;
    for (size_t i = 0; i < str.length(); i++)
        switch (str[i]) {
            case 'U':
                fire = true;
            case 'u':
                up = true;
                break;

            case 'D':
                fire = true;
            case 'd':
                down = true;
                break;

            case 'L':
                fire = true;
            case 'l':
                left = true;
                break;

            case 'R':
                fire = true;
            case 'r':
                right = true;
                break;

            case 'F':
                fire = true;
                break;

            case 'k':
                suicide = true;
                break;

            case '.':
                /* do nothing, as all other movements are false */
                break;

            case 'c':
            case 'C':
                /* bdcff 'combined' flags. do nothing. */
                break;

            default:
                if (isdigit(str[i])) {
                    if (num == -1)
                        sscanf(str.c_str() + i, "%d", &num);
                }
                break;
        }
    GdDirectionEnum dir = gd_direction_from_keypress(up, down, left, right);
    size_t count = 1;
    if (num != -1)
        count = num;
    for (size_t i = 0; i < count; i++)
        store_movement(dir, fire, suicide);

    return true;
}

bool CaveReplay::load_from_bdcff(std::string const &str) {
    std::istringstream is(str);
    std::string one;
    bool result = true;
    while (is >> one)
        result = result && load_one_from_bdcff(one);
    return result;
}


const char *CaveReplay::direction_to_bdcff(GdDirectionEnum mov) {
    switch (mov) {
        /* not moving */
        case MV_STILL:
            return ".";
        /* directions */
        case MV_UP:
            return "u";
        case MV_UP_RIGHT:
            return "ur";
        case MV_RIGHT:
            return "r";
        case MV_DOWN_RIGHT:
            return "dr";
        case MV_DOWN:
            return "d";
        case MV_DOWN_LEFT:
            return "dl";
        case MV_LEFT:
            return "l";
        case MV_UP_LEFT:
            return "ul";
        default:
            assert(false);
    }
}

/* same as above; pressing fire will be a capital letter. */
const char *CaveReplay::direction_fire_to_bdcff(GdDirectionEnum dir, bool fire) {
    static char mov[10];

    strcpy(mov, direction_to_bdcff(dir));
    if (fire) {
        /* uppercase all letters */
        for (int i = 0; mov[i] != '\0'; i++)
            mov[i] = toupper(mov[i]);
    }

    return mov;
}

std::string CaveReplay::movements_to_bdcff() const {
    std::string str;

    for (unsigned pos = 0; pos < movements.size(); pos++) {
        int num = 1;

        /* if this is not the first movement, append a space. */
        if (!str.empty())
            str += ' ';

        /* if same byte appears, count number of occurrences - something like an rle compression. */
        /* be sure not to cross the array boundaries */
        while (pos < movements.size() - 1 && movements[pos] == movements[pos + 1]) {
            pos++;
            num++;
        }
        movement data = movements[pos];
        if (data & REPLAY_SUICIDE_MASK)
            str += "k";
        else if ((data & REPLAY_FIRE_MASK) && ((data & REPLAY_MOVE_MASK) == MV_STILL))
            str += "F";
        else
            str += direction_fire_to_bdcff(GdDirectionEnum(data & REPLAY_MOVE_MASK), (data & REPLAY_FIRE_MASK) != 0);
        if (num != 1) {
            std::ostringstream s;

            s << num;
            str += s.str();
        }
    }

    return str;
}
