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
#ifndef CAVEREPLAY_HPP_INCLUDED
#define CAVEREPLAY_HPP_INCLUDED

#include "config.h"

#include <string>
#include "cave/cavetypes.hpp"
#include "cave/helper/reflective.hpp"

class CaveReplay : public Reflective {
private:
    static const char *direction_to_bdcff(GdDirectionEnum mov);
    static const char *direction_fire_to_bdcff(GdDirectionEnum dir, bool fire);
    bool load_one_from_bdcff(const std::string &str);
    typedef unsigned char movement;
    std::vector<movement> movements;
    unsigned int current_playing_pos;
    enum {
        REPLAY_MOVE_MASK = 0x0f,
        REPLAY_FIRE_MASK = 0x10,
        REPLAY_SUICIDE_MASK = 0x20,
    };

public:
    /* reflective */
    static PropertyDescription const descriptor[];
    virtual PropertyDescription const *get_description_array() const {
        return descriptor;
    }

    CaveReplay();

    /* i/o */
    std::string movements_to_bdcff() const;
    bool load_from_bdcff(const std::string &str);
    void store_movement(GdDirectionEnum player_move, bool player_fire, bool suicide);
    bool get_next_movement(GdDirectionEnum &player_move, bool &player_fire, bool &suicide);
    void rewind();
    unsigned int length() const {
        return movements.size();
    }

    GdInt level;            ///< replay for level n
    GdInt seed;                ///< seed the cave is to be rendered with
    GdString recorded_with;    ///< application name and version, which was used to create this replay

    GdString player_name;    ///< name of player in this replay
    GdString date;            ///< date when this replay was recorded
    GdString comment;        ///< comments on the replay

    GdInt score;            ///< score collected in this replay
    GdInt duration;            ///< seconds duration of replay
    GdBool success;            ///< true, if the player was successful
    GdInt checksum;            ///< checksum of rendered cave

    GdBool wrong_checksum;    ///< this replay's checksum is calculated, and it came out to be a mismatch with the cave
    GdBool saved;            ///< whether this replay is to be saved in the game
};

#endif    /* CAVEREPLAY_H */
