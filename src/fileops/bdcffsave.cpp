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

#include <cstring>

#include "fileops/bdcffsave.hpp"
#include "cave/cavestored.hpp"
#include "cave/caveset.hpp"
#include "fileops/bdcffhelper.hpp"
#include "cave/elementproperties.hpp"
#include "misc/autogfreeptr.hpp"


/// @file fileops/bdcffsave.cpp
/// BDCFF save functions


/// write highscore to a bdcff file
static void write_highscore_func(std::list<std::string> &out, HighScoreTable const &scores) {
    for (unsigned int i = 0; i < scores.size(); i++)
        out.push_back(BdcffFormat() << scores[i].score << scores[i].name);
}


/// Save properties of a reflective object in bdcff format.
/// Used to save caves, cavesets, replays.
/// @param out The list of output strings to append to.
/// @param str The reflective object.
/// @param str_def Another reflective object, which is of the same type. Default values are taken from that,
///                 i.e. if a property in str has the same value as in str_def, it is not saved.
/// @param ratio The cave size, for ratio types. Set to cave->w*cave->h when calling.
/// @todo rename
void save_properties(std::list<std::string> &out, Reflective const &str, Reflective const &str_def, int ratio, PropertyDescription const *prop_desc) {
    bool should_write = false;
    const char *identifier = NULL;
    BdcffFormat line;

    /* for all properties */
    for (unsigned i = 0; prop_desc[i].identifier != NULL; i++) {
        std::unique_ptr<GetterBase> const &prop = prop_desc[i].prop;

        // used only by the gui, nothing to do
        if (prop_desc[i].type == GD_TAB || prop_desc[i].type == GD_LABEL)
            continue;
        // skip these
        if (prop_desc[i].flags & GD_DONT_SAVE)
            continue;
        // if it is a string, write as one line. do not even write identifier if no string, as default is empty.
        if (prop_desc[i].type == GD_TYPE_STRING) {
            if (str.get<GdString>(prop) != "")
                out.push_back(BdcffFormat(prop_desc[i].identifier) << str.get<GdString>(prop));
            continue;
        }
        // long string - also as one line. escape newlines.
        if (prop_desc[i].type == GD_TYPE_LONGSTRING) {
            if (str.get<GdString>(prop) != "") {
                AutoGFreePtr<char> escaped(g_strescape(str.get<GdString>(prop).c_str(), NULL));
                out.push_back(BdcffFormat(prop_desc[i].identifier) << escaped);
            }
            continue;
        }
        // effects are also stored in a different fashion.
        if (prop_desc[i].type == GD_TYPE_EFFECT) {
            if (str.get<GdElement>(prop) != str_def.get<GdElement>(prop))
                out.push_back(BdcffFormat("Effect") << prop_desc[i].identifier << str.get<GdElement>(prop));
            continue;
        }

        // and now process tags which have to be treated normally.

        // if identifier differs from the previous, write out the line collected, and start a new one
        if (!identifier || strcmp(prop_desc[i].identifier, identifier) != 0) {
            // write lines only which carry information other than the default settings
            if (should_write)
                out.push_back(line);

            line = BdcffFormat(prop_desc[i].identifier);
            should_write = false;

            // remember identifier
            identifier = prop_desc[i].identifier;
        }

        // if we always save this identifier, remember now
        if (prop_desc[i].flags & GD_ALWAYS_SAVE)
            should_write = true;

        switch (prop_desc[i].type) {
            case GD_TYPE_BOOLEAN:
                line << str.get<GdBool>(prop);
                if (str.get<GdBool>(prop) != str_def.get<GdBool>(prop))
                    should_write = true;
                break;
            case GD_TYPE_INT:
                if (prop_desc[i].flags & GD_BDCFF_RATIO_TO_CAVE_SIZE)
                    line << (str.get<GdInt>(prop) / (double)ratio); /* save as ratio! */
                else
                    line << str.get<GdInt>(prop);                   /* save as normal int */
                if (str.get<GdInt>(prop) != str_def.get<GdInt>(prop))
                    should_write = true;
                break;
            case GD_TYPE_INT_LEVELS:
                for (unsigned j = 0; j < prop->count; j++) {
                    if (prop_desc[i].flags & GD_BDCFF_RATIO_TO_CAVE_SIZE)
                        line << (str.get<GdIntLevels>(prop)[j] / (double)ratio); /* save as ratio! */
                    else
                        line << str.get<GdIntLevels>(prop)[j];                   /* save as normal int */
                    if (str.get<GdIntLevels>(prop)[j] != str_def.get<GdIntLevels>(prop)[j])
                        should_write = true;
                }
                break;
            case GD_TYPE_PROBABILITY:
                line << str.get<GdProbability>(prop);
                if (str.get<GdProbability>(prop) != str_def.get<GdProbability>(prop))
                    should_write = true;
                break;
            case GD_TYPE_PROBABILITY_LEVELS:
                for (unsigned j = 0; j < prop->count; j++) {
                    line << str.get<GdProbabilityLevels>(prop)[j];
                    if (str.get<GdProbabilityLevels>(prop)[j] != str_def.get<GdProbabilityLevels>(prop)[j])
                        should_write = true;
                }
                break;
            case GD_TYPE_ELEMENT:
                line << str.get<GdElement>(prop);
                if (str.get<GdElement>(prop) != str_def.get<GdElement>(prop))
                    should_write = true;
                break;
            case GD_TYPE_COLOR:
                line << str.get<GdColor>(prop);
                should_write = true;
                break;
            case GD_TYPE_DIRECTION:
                line << str.get<GdDirection>(prop);
                if (str.get<GdDirection>(prop) != str_def.get<GdDirection>(prop))
                    should_write = true;
                break;
            case GD_TYPE_SCHEDULING:
                line << str.get<GdScheduling>(prop);
                if (str.get<GdScheduling>(prop) != str_def.get<GdScheduling>(prop))
                    should_write = true;
                break;
            case GD_TAB:
            case GD_LABEL:
            case GD_TYPE_EFFECT:            /* handled above */
            case GD_TYPE_STRING:            /* handled above */
            case GD_TYPE_LONGSTRING:        /* handled above */
            case GD_TYPE_COORDINATE:        /* currently not needed */
            case GD_TYPE_BOOLEAN_LEVELS:    /* currently not needed */
                g_assert_not_reached();
                break;
        }
    }
    /* write remaining data */
    if (should_write)
        out.push_back(line);
}


static void save_own_properties(std::list<std::string> &out, Reflective const &str, Reflective const &str_def, int ratio) {
    save_properties(out, str, str_def, ratio, str.get_description_array());
}


/* remove a line from the list of strings. */
/* the prefix should be a property; add an equal sign! so properties which have names like
   "slime" and "slimeproperties" won't match each other. */
static void cave_properties_remove(std::list<std::string> &out, const char *attrib) {
    out.remove_if(HasAttrib(attrib));
}


static BdcffFile::BdcffSection save_replay_func(CaveReplay const &replay) {
    BdcffFile::BdcffSection out;
    CaveReplay default_values;                          // an empty replay to store default values
    save_own_properties(out, replay, default_values, 0);    // 0 is for ratio, here it is not used
    out.push_back(BdcffFormat("Movements") << replay.movements_to_bdcff());

    return out;
}


/// Output properties of a CaveStored to a BdcffFile::CaveInfo structure.
/// Saves everything; properties, map, objects.
static BdcffFile::CaveInfo caveset_save_cave_func(CaveStored &cave) {
    BdcffFile::CaveInfo out;

    write_highscore_func(out.highscore, cave.highscore);

    // first add the properties to the list.
    // later, some are deleted (slime permeability, for example) - this is needed because of the inconsistencies of the bdcff.
    CaveStored default_values;
    save_own_properties(out.properties, cave, default_values, cave.w * cave.h);

    // here come properties which are handled explicitly. these cannot be handled easily above,
    // as they have some special meaning. for example, slime_permeability=x sets permeability to
    // x, and sets predictable to false. bdcff format is simply inconsistent in these aspects.

    // slime permeability is always set explicitly, as it also sets predictability.
    // both have the ALWAYS_SAVE flags, so now they are in the array regardless of their values.
    if (cave.slime_predictable)
        // if slime is predictable, remove permeab. flag, as that would imply unpredictable slime.
        cave_properties_remove(out.properties, "SlimePermeability");
    else
        // if slime is UNpredictable, remove permeabc64 flag, as that would imply predictable slime.
        cave_properties_remove(out.properties, "SlimePermeabilityC64");

    // save unknown tags as they are. somewhat hackish - writes a string with multi-lines.
    if (cave.unknown_tags != "")
        out.properties.push_back(cave.unknown_tags);

    // is cave has a map
    if (!cave.map.empty()) {
        // save map
        for (int y = 0; y < cave.h; ++y) {
            std::string line;
            line.reserve(cave.w);
            for (int x = 0; x < cave.w; ++x) {
                // check if character is non-zero; the ...save() should have assigned a character to every element
                // the gd_element_properties[...].character_new is created by the caller.
                g_assert(gd_element_properties[cave.map(x, y)].character_new != 0);
                line.push_back(gd_element_properties[cave.map(x, y)].character_new);
            }
            out.map.push_back(std::move(line));
        }
    }

    // save drawing objects
    for (auto it = cave.objects.cbegin(); it != cave.objects.cend(); ++it) {
        CaveObject const & obj = *it;

        // not for all levels?
        if (!obj.is_seen_on_all()) {
            std::string line = "[Level=";
            bool once = false;  // will be true if already written one number
            for (int i = 0; i < 5; i++) {
                if (obj.seen_on[i]) {
                    if (once)   // if written at least one number so far, we need a comma
                        line += ',';
                    line += char('1' + i); // level number, ascii character 1, 2, 3, 4 or 5
                    once = true;
                }
            }
            line += ']';
            out.objects.push_back(line);
        }
        out.objects.push_back(obj.get_bdcff());
        // again, not for all? then save closing tag, too
        if (!obj.is_seen_on_all())
            out.objects.push_back("[/Level]");
    }

    // save replays
    for (auto r_it = cave.replays.cbegin(); r_it != cave.replays.cend(); ++r_it)
        if (r_it->saved)
            out.replays.push_back(save_replay_func(*r_it));

    return out;
}


/// Add a group of properties to a bdcff file.
/// A new line is started, then the group name is written like [group].
/// Then the properties, and the closing [/group].
/// Only creates group, if it won't be empty.
/// @param name The name of the group in te bdcff file.
/// @param in The list of properties to save to the file. Will be cleared after this.
/// @param out The list of strings, which will be the bdcff file. Writes to this list.
static void add_group(std::string name, std::list<std::string> &in, std::list<std::string> &out) {
    if (!in.empty()) {
        out.push_back("");
        out.push_back("[" + name + "]");
        // Move all lines from the list 'in' to the end of list 'out'
        out.splice(out.end(), in);
        out.push_back("[/" + name + "]");
    }
}

/// Save caveset in BDCFF format to a list of strings.
void save_to_bdcff(CaveSet &caveset, std::list<std::string> &out) {

    BdcffFile outfile;

    outfile.bdcff.push_back(BdcffFormat("Version") << BDCFF_VERSION);

    /* check if we need an own mapcode table ------ */
    /* copy original characters to character_new fields; new elements will be added to that one */
    /* check all caves */
    bool write_mapcodes = false;
    CharToElementTable ctet;            // create a new table
    for (unsigned int i = 0; i < O_MAX; i++)
        gd_element_properties[i].character_new = gd_element_properties[i].character;
    for (unsigned int i = 0; i < caveset.caves.size(); i++) {
        CaveStored &cave = caveset.caves[i];

        // if they have a map (random elements+object based maps do not need characters)
        if (!cave.map.empty()) {
            // check every element of map
            for (int y = 0; y < cave.h; ++y)
                for (int x = 0; x < cave.w; ++x) {
                    GdElementEnum e = cave.map(x, y);
                    if (gd_element_properties[e].character_new == 0) {
                        write_mapcodes = true;
                        gd_element_properties[e].character_new = ctet.find_place_for(e);
                    }
                }
        }
    }
    // this flag was set above if we need to write mapcodes
    if (write_mapcodes) {
        outfile.mapcodes.push_back(BdcffFormat("Length") << 1);
        for (unsigned int i = 0; i < O_MAX; i++) {
            // if no character assigned by specification BUT (AND) we assigned one
            if (gd_element_properties[i].character == 0 && gd_element_properties[i].character_new != 0)
                // write something like ".=DIRT".
                outfile.mapcodes.push_back(BdcffFormat(std::string(1, gd_element_properties[i].character_new)) << gd_element_properties[i].filename);
        }
    }

    // caveset data
    write_highscore_func(outfile.highscore, caveset.highscore);
    CaveSet default_caveset;  // temporary object holds default values
    save_own_properties(outfile.caveset_properties, caveset, default_caveset, 0);
    outfile.caveset_properties.push_back(BdcffFormat("Levels") << 5);

    // caves data
    for (unsigned int i = 0; i < caveset.caves.size(); ++i)
        outfile.caves.push_back(caveset_save_cave_func(caveset.caves[i]));

    // now convert it to an output file.
    // move all strings created to the output list of strings in sections.
    out.push_back("[BDCFF]");
    out.splice(out.end(), outfile.bdcff);           // bdcff version string
    add_group("mapcodes", outfile.mapcodes, out);

    out.push_back("");
    out.push_back("[game]");
    out.splice(out.end(), outfile.caveset_properties);      // game (caveset) properties)
    add_group("highscore", outfile.highscore, out);         // game highscores

    // data of caves
    for (auto it = outfile.caves.begin(); it != outfile.caves.end(); ++it) {
        out.push_back("");
        out.push_back("[cave]");
        out.splice(out.end(), it->properties);  // cave properties
        add_group("map", it->map, out);
        add_group("objects", it->objects, out);
        add_group("highscore", it->highscore, out);
        // each replay has its own group
        for (auto rit = it->replays.begin(); rit != it->replays.end(); ++rit)
            add_group("replay", *rit, out);
        out.push_back("[/cave]");
    }

    out.push_back("[/game]");
    out.push_back("[/BDCFF]");
}
