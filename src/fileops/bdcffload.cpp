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

#include "fileops/bdcffload.hpp"

#include "misc/logger.hpp"
#include "fileops/bdcffhelper.hpp"
#include "cave/caveset.hpp"
#include "cave/helper/cavereplay.hpp"
#include "cave/cavestored.hpp"
#include "cave/caverendered.hpp"
#include "misc/printf.hpp"
#include "misc/util.hpp"
#include "misc/autogfreeptr.hpp"
#include "cave/elementproperties.hpp"
#include "settings.hpp"

/// @todo engine types should be moved somewhere else?
#include "fileops/c64import.hpp"  /* c64import defines the engine types */
#include "cave/object/caveobjectfillrect.hpp" /* bdcff intermission hack - adding a cavefillrect */


/// @todo remove
bool struct_set_property(Reflective &str, const std::string &attrib, const std::string &param, int ratio, PropertyDescription const *prop_desc) {
    int paramindex = 0;

    char **params = g_strsplit_set(param.c_str(), " ", -1);
    int paramcount = g_strv_length(params);
    bool identifier_found = false;

    /* check all known tags. do not exit this loop if identifier_found==true...
       as there are more lines in the array which have the same identifier. */
    bool was_string = false;
    for (unsigned i = 0; prop_desc[i].identifier != NULL; i++)
        if (gd_str_ascii_caseequal(prop_desc[i].identifier, attrib)) {
            /* found the identifier */
            identifier_found = true;
            std::unique_ptr<GetterBase> const &prop = prop_desc[i].prop;
            if (prop_desc[i].type == GD_TYPE_STRING) {
                /* strings are treated different, as occupy the whole length of the line */
                str.get<GdString>(prop) = param;
                was_string = true;  /* remember this to skip checking the number of parameters at the end of the function */
                continue;
            }

            if (prop_desc[i].type == GD_TYPE_LONGSTRING) {
                AutoGFreePtr<char> compressed(g_strcompress(param.c_str()));
                str.get<GdString>(prop) = compressed;
                was_string = true;  /* remember this to skip checking the number of parameters at the end of the function */
                continue;
            }

            /* not a string, so use scanf calls */
            /* try to read as many words, as there are elements in this property (array) */
            /* ALSO, if no more parameters to process, exit loop */
            for (unsigned j = 0; j < prop->count && params[paramindex] != NULL; j++) {
                bool success = false;

                switch (prop_desc[i].type) {
                    case GD_TYPE_BOOLEAN:
                        success = read_from_string(params[paramindex], str.get<GdBool>(prop));
                        /* if we are processing an array, fill other values with these. if there are other values specified, those will be overwritten. */
                        break;
                    case GD_TYPE_INT:
                        if (prop_desc[i].flags & GD_BDCFF_RATIO_TO_CAVE_SIZE)
                            success = read_from_string(params[paramindex], str.get<GdInt>(prop), ratio); /* saved as double, ratio to cave size */
                        else
                            success = read_from_string(params[paramindex], str.get<GdInt>(prop));
                        break;
                    case GD_TYPE_INT_LEVELS:
                        if (prop_desc[i].flags & GD_BDCFF_RATIO_TO_CAVE_SIZE)
                            success = read_from_string(params[paramindex], str.get<GdIntLevels>(prop)[j], ratio); /* saved as double, ratio to cave size */
                        else
                            success = read_from_string(params[paramindex], str.get<GdIntLevels>(prop)[j]);
                        if (success) /* copy to other if array */
                            for (unsigned k = j + 1; k < prop->count; k++)
                                str.get<GdIntLevels>(prop)[k] = str.get<GdIntLevels>(prop)[j];
                        break;
                    case GD_TYPE_PROBABILITY:
                        success = read_from_string(params[paramindex], str.get<GdProbability>(prop));
                        break;
                    case GD_TYPE_PROBABILITY_LEVELS:
                        success = read_from_string(params[paramindex], str.get<GdProbabilityLevels>(prop)[j]);
                        if (success) /* copy to other if array */
                            for (unsigned k = j + 1; k < prop->count; k++)
                                str.get<GdProbabilityLevels>(prop)[k] = str.get<GdProbabilityLevels>(prop)[j];
                        break;
                    case GD_TYPE_ELEMENT:
                        success = read_from_string(params[paramindex], str.get<GdElement>(prop));
                        break;
                    case GD_TYPE_DIRECTION:
                        success = read_from_string(params[paramindex], str.get<GdDirection>(prop));
                        break;
                    case GD_TYPE_SCHEDULING:
                        success = read_from_string(params[paramindex], str.get<GdScheduling>(prop));
                        break;

                    case GD_TYPE_LONGSTRING:    /* processed above */
                    case GD_TYPE_STRING:        /* processed above */
                    case GD_TYPE_COLOR:         /* processed elsewhere */
                    case GD_TYPE_EFFECT:        /* processed elsewhere */
                    case GD_TYPE_COORDINATE:    /* caves do not have */
                    case GD_TYPE_BOOLEAN_LEVELS:    /* caves do not have */
                    case GD_TAB:                /* ui */
                    case GD_LABEL:              /* ui */
                        g_assert_not_reached();
                        break;
                }

                if (success)
                    paramindex++;   /* go to next parameter to process */
                else
                    gd_warning("invalid parameter '%s' for attribute %s", params[paramindex], attrib);
            }
        }
    /* if we found the identifier, but still could not process all parameters... */
    /* of course, not for strings, as the whole line is the string */
    if (identifier_found && !was_string && paramindex < paramcount)
        gd_message("excess parameters for attribute '%s': '%s'", attrib, params[paramindex]);
    g_strfreev(params);

    return identifier_found;
}


static bool struct_set_own_property(Reflective &str, const std::string &attrib, const std::string &param, int ratio) {
    return struct_set_property(str, attrib, param, ratio, str.get_description_array());
}


static bool replay_process_tag(CaveReplay &replay, const std::string &attrib, const std::string &param) {
    bool identifier_found = false;

    /* movements */
    if (gd_str_ascii_caseequal(attrib, "Movements")) {
        identifier_found = true;
        bool correct = replay.load_from_bdcff(param);
        if (!correct)
            gd_warning("Error in replay data");
    } else
        /* any other tag */
        identifier_found = struct_set_own_property(replay, attrib, param, 0); /* 0: for ratio types; not used */

    /* the func returns true if the identifier is to be removed */
    return identifier_found;
}


static bool cave_process_tags_func(CaveStored &cave, const std::string &attrib, const std::string &param) {
    char **params = g_strsplit_set(param.c_str(), " ", -1);
    /// @todo make an autoptr-like class for this stuff
    struct Finally {
        char **params;
        Finally(char **params) : params(params) {}
        ~Finally() { g_strfreev(params); }
    } finally(params);
    int paramcount = g_strv_length(params);

    /* compatibility with old snapexplosions flag */
    if (gd_str_ascii_caseequal(attrib, "SnapExplosions")) {
        GdBool b;
        if (read_from_string(param, b)) {
            if (b)  // was "true" -> snapping explosions
                cave.snap_element = O_EXPLODE_1;
            else    // was "false" -> normal space snapping
                cave.snap_element = O_SPACE;
        } else
            gd_warning("invalid param for '%s': '%s'", attrib, param);
        return true;
    }
    
    /* compatibility with old bd1scheduling flag */
    if (gd_str_ascii_caseequal(attrib, "BD1Scheduling")) {
        GdBool b;
        if (read_from_string(param, b)) {
            if (b)
                if (cave.scheduling == GD_SCHEDULING_PLCK)
                    cave.scheduling = GD_SCHEDULING_BD1;
        } else
            gd_warning("invalid param for '%s': '%s'", attrib, param);
        return true;
    }
    
    /* bdcff engine flag */
    if (gd_str_ascii_caseequal(attrib, "Engine")) {
        GdEngine e;
        if (read_from_string(param, e))
            C64Import::cave_set_engine_defaults(cave, e);
        else
            gd_warning("invalid param for '%s': '%s'", attrib, param);
        return true;
    }
    
    /* compatibility with old AmoebaProperties flag */
    if (gd_str_ascii_caseequal(attrib, "AmoebaProperties")) {
        GdElement elem1 = O_STONE, elem2 = O_DIAMOND;
        bool success = read_from_string(params[0], elem1) && read_from_string(params[1], elem2);
        if (success) {
            cave.amoeba_too_big_effect = elem1;
            cave.amoeba_enclosed_effect = elem2;
        } else
            gd_warning("invalid param for '%s': '%s'", attrib, param);
        return true;
    }
    
    /* colors attribute is a mess, have to process explicitly */
    if (gd_str_ascii_caseequal(attrib, "Colors")) {
        /* Colors=[border background] foreground1 foreground2 foreground3 [amoeba slime] */
        bool ok = true;
        GdColor cb, c0, c1, c2, c3, c4, c5;

        if (paramcount == 3) {
            // only color1,2,3
            cb = GdColor::from_c64(0); // border - black
            c0 = GdColor::from_c64(0); // background - black
            ok = ok && read_from_string(params[0], c1);
            ok = ok && read_from_string(params[1], c2);
            ok = ok && read_from_string(params[2], c3);
            c4 = c3;  // amoeba
            c5 = c1;  // slime
        } else if (paramcount == 5) {
            /* bg,color0,1,2,3 */
            ok = ok && read_from_string(params[0], cb);
            ok = ok && read_from_string(params[1], c0);
            ok = ok && read_from_string(params[2], c1);
            ok = ok && read_from_string(params[3], c2);
            ok = ok && read_from_string(params[4], c3);
            c4 = c3;  // amoeba
            c5 = c1;  // slime
        } else if (paramcount == 7) {
            // bg,color0,1,2,3,amoeba,slime
            ok = ok && read_from_string(params[0], cb);
            ok = ok && read_from_string(params[1], c0);
            ok = ok && read_from_string(params[2], c1);
            ok = ok && read_from_string(params[3], c2);
            ok = ok && read_from_string(params[4], c3);
            ok = ok && read_from_string(params[5], c4); // amoeba
            ok = ok && read_from_string(params[6], c5); // slime
        } else {
            ok = false;
        }

        if (ok) {
            cave.colorb = cb;
            cave.color0 = c0;
            cave.color1 = c1;
            cave.color2 = c2;
            cave.color3 = c3;
            cave.color4 = c4;
            cave.color5 = c5;
        } else {
            gd_message("invalid param for '%s': '%s'", attrib, param);
        }
        
        return true;
    }
    
    /* effects are also handled in an ugly way in bdcff */
    if (gd_str_ascii_caseequal(attrib, "Effect")) {
        /* an effect command has two parameters */
        if (paramcount == 2) {
            bool success = false;
            PropertyDescription const *descriptor = cave.get_description_array();

            int i;
            for (i = 0; descriptor[i].identifier != NULL; i++) {
                /* we have to search for this effect */
                if (descriptor[i].type == GD_TYPE_EFFECT && gd_str_ascii_caseequal(params[0], descriptor[i].identifier)) {
                    /* found identifier */
                    success = read_from_string(params[1], cave.get<GdElement>(descriptor[i].prop));
                    if (success)
                        cave.get<GdElement>(descriptor[i].prop) = nonscanned_pair(cave.get<GdElement>(descriptor[i].prop));
                    break;
                }
            }
            /* if we didn't find first element name */
            if (descriptor[i].identifier == NULL) {
                /* for compatibility with tim stridmann's memorydump->bdcff converter... .... ... */
                if (gd_str_ascii_caseequal(params[0], "BOUNCING_BOULDER")) {
                    success = read_from_string(params[1], cave.stone_bouncing_effect);
                    if (success)
                        cave.stone_bouncing_effect = nonscanned_pair(cave.stone_bouncing_effect);
                } else if (gd_str_ascii_caseequal(params[0], "EXPLOSION3S")) {
                    success = read_from_string(params[1], cave.explosion_3_effect);
                    if (success)
                        cave.explosion_3_effect = nonscanned_pair(cave.explosion_3_effect);
                }
                /* falling with one l... */
                else if (gd_str_ascii_caseequal(params[0], "STARTING_FALING_DIAMOND")) {
                    success = read_from_string(params[1], cave.diamond_falling_effect);
                    if (success)
                        cave.diamond_falling_effect = nonscanned_pair(cave.diamond_falling_effect);
                }
                /* dirt lookslike */
                else if (gd_str_ascii_caseequal(params[0], "DIRT"))
                    success = read_from_string(params[1], cave.dirt_looks_like);
                else if (gd_str_ascii_caseequal(params[0], "HEXPANDING_WALL") && gd_str_ascii_caseequal(params[1], "STEEL_HEXPANDING_WALL")) {
                    success = read_from_string(params[1], cave.expanding_wall_looks_like);
                } else {
                    /* didn't find at all */
                    gd_warning("invalid effect name '%s'", params[0]);
                    success = true;     // to ignore
                }

                if (!success)
                    gd_warning("cannot read element name '%s'", params[1]);
            }
        } else
            gd_warning("invalid effect specification '%s'", param);
        return true;
    }
    
    /* otherwise process normally */
    return struct_set_own_property(cave, attrib, param, cave.w * cave.h);
}

/// process a given cave property (by its name) - and do nothing, if no such property exists.
/// this function helps processing some cave tags in advance.
/// @param cave The cave to process the tag for.
/// @param lines The list of lines to find the attrib in.
/// @param name The name of the attribute to find.
/// @return true, if the property is found. If found, it is also processed and removed.
static bool cave_process_specific_tag(CaveStored &cave, BdcffFile::BdcffSection &lines, const std::string &name) {
    auto it = find_if(lines.begin(), lines.end(), HasAttrib(name));
    bool found = it != lines.end();
    if (found) {
        try {
            AttribParam ap(*it);        // split into attrib and param
            cave_process_tags_func(cave, ap.attrib, ap.param);
        } catch (std::exception &e) {
            gd_warning("Cannot parse: %s", *it);
        }
        lines.erase(it);            // erase after processing
    }
    return found;
}

/// Process properties for the cave, and set cave parameters according to it.
/// Some cave properties must be read in correct order, because bdcff is not a well designed format.
/// For example, the name is processed first, to be able to show all error messages with the cave name context.
/// Then the engine tag is processed - well, because bdcff sucks.
/// Then the size - to make sure ratios are read correctly - bdcff sucks.
static void cave_process_all_tags(CaveStored &cave, BdcffFile::BdcffSection &lines) {
    // first check cave name, so we can report errors correctly (saying that CaveStored xy: error foobar)
    cave_process_specific_tag(cave, lines, "Name");
    SetLoggerContextForFunction scf((cave.name == "") ? Printf("<unnamed cave>") : (Printf("Cave '%s'", cave.name)));

    // process lame engine tag first so its settings may be overwritten later. fail.
    cave_process_specific_tag(cave, lines, "Engine");
    // check if this is an intermission, so we can set to cavesize or intermissionsize - another epic fail
    cave_process_specific_tag(cave, lines, "Intermission");
    // process size at the beginning... as ratio types depend on this. bdcff design fail.
    cave_process_specific_tag(cave, lines, "Size");

    // these properties have values, but also make some implications.
    if (cave_process_specific_tag(cave, lines, "SlimePermeability"))
        cave.slime_predictable = false;
    if (cave_process_specific_tag(cave, lines, "SlimePermeabilityC64"))
        cave.slime_predictable = true;

    // these set scheduling type. framedelay takes precedence, if both exist. so we check it AFTER checking CaveDelay..
    if (cave_process_specific_tag(cave, lines, "CaveDelay")) {
        // only set scheduling type, when it is not the gdash-default.
        // this allows setting cavescheduling=bd1 in the [game] section, for example.
        // in that case, this one will not overwrite it.
        // bdcff fail, fail, fail.
        if (cave.scheduling == GD_SCHEDULING_MILLISECONDS)
            cave.scheduling = GD_SCHEDULING_PLCK;
    }
    if (cave_process_specific_tag(cave, lines, "FrameTime")) {
        // but if the cave has a frametime setting, always switch to milliseconds.
        // bdcff says that we should select the better scheduling if we support both.
        cave.scheduling = GD_SCHEDULING_MILLISECONDS;
    }

    // process remaining tags - most of them do not require special care.
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        try {
            AttribParam ap(*it);
            if (!cave_process_tags_func(cave, ap.attrib, ap.param)) {
                gd_message("unknown tag '%s'", ap.attrib);
                cave.unknown_tags += *it;
                cave.unknown_tags += '\n';
            }
        } catch (std::exception &e) {
            gd_warning("Cannot parse line: %s", *it);
        }
    }
}

static bool add_highscore(HighScoreTable &hs, const std::string &name, const std::string &score) {
    std::istringstream is(score);
    int sc;

    /* parse score */
    if (!(is >> sc))
        return false;
    if (name == "")
        return false;
    hs.add(name, sc);

    return true;
}

static BdcffFile parse_bdcff_sections(const char *file_contents) {
    BdcffFile file;
    std::istringstream is(file_contents);
    enum ReadState {
        Start,          ///< should be nothing here.
        Bdcff,          ///< inside [bdcff], eg. version=0.5
        BdcffMapCodes,  ///< inside [bdcff] [mapcodes] - not "standard", but gdash made files like this
        Game,           ///< inside [game], eg. author=foo
        GameHighScore,  ///< trivial
        GameMapCodes,   ///< eg. x=STEELWALL
        Cave,           ///< a cave, eg. name=Cave A
        CaveSReplay,    ///< replay section for a cave
        CaveDemo,       ///< old styled demo (replay), just movements, no random data & the like
        CaveHighScore,  ///< highscores for a cave
        CaveObjects,    ///< objects for a cave
        CaveMap         ///< map-encoded cave
    } state;

    std::string line;
    state = Start;
    bool bailout = false;
    for (int lineno = 1; !bailout && getline(is, line); lineno++) {
        SetLoggerContextForFunction scf(Printf("Line %d", lineno));

        size_t found_r;
        while ((found_r = line.find('\r')) != std::string::npos)
            line.erase(found_r, 1);     /* remove windows-nightmare \r-s */
        if (line.empty())
            continue;                   /* skip empty lines */

        if (state != CaveMap && line[0] == ';')
            continue;                   /* just skip comments. be aware that map lines may start with a semicolon... */

        /* STARTING WITH A BRACKET [ IS A SECTION */
        if (line[0] == '[') {
            if (gd_str_ascii_caseequal(line, "[BDCFF]")) {
                if (state != Start) {
                    gd_critical("first section should be [BDCFF]. Bailing out!");
                    bailout = true;
                }
                state = Bdcff;
            } else if (gd_str_ascii_caseequal(line, "[/BDCFF]")) {
                state = Start;
            } else if (gd_str_ascii_caseequal(line, "[game]")) {
                if (state != Bdcff)
                    gd_warning("[game] should be inside [BDCFF]");
                state = Game;
            } else if (gd_str_ascii_caseequal(line, "[/game]")) {
                if (state != Game)
                    gd_warning("[/game] not in [game] section");
            } else if (gd_str_ascii_caseequal(line, "[mapcodes]")) {
                switch (state) {
                    case Game:
                        state = GameMapCodes;
                        break;
                    case Bdcff:
                        state = BdcffMapCodes;
                        break;
                    default:
                        gd_warning("[mapcodes] allowed only in [game] section");
                        state = BdcffMapCodes;
                        break;
                }
            } else if (gd_str_ascii_caseequal(line, "[/mapcodes]")) {
                switch (state) {
                    case GameMapCodes:
                        state = Game;
                        break;
                    case BdcffMapCodes:
                        state = Bdcff;
                        break;
                    default:
                        gd_warning("[/mapcodes] not after [mapcodes]");
                        state = Game;
                }
            } else if (gd_str_ascii_caseequal(line, "[cave]")) {
                if (state != Game)
                    gd_warning("[cave] allowed only in [game] section");
                state = Cave;
                file.caves.push_back(BdcffFile::CaveInfo());    /* new empty space for a cave */
            } else if (gd_str_ascii_caseequal(line, "[/cave]")) {
                if (state != Cave)
                    gd_warning("[/cave] tag without starting [cave]");
                state = Game;
            } else if (gd_str_ascii_caseequal(line, "[map]")) {
                if (state != Cave)
                    gd_warning("[map] section only allowed inside [cave]");
                else    /* else: do not enter map reading when not in a cave! */
                    state = CaveMap;
            } else if (gd_str_ascii_caseequal(line, "[/map]")) {
                if (state != CaveMap)
                    gd_warning("[/map] tag without starting [map]");
                state = Cave;
            } else if (gd_str_ascii_caseequal(line, "[highscore]")) {
                /* can be inside game or cave */
                if (state == Game)
                    state = GameHighScore;
                else if (state == Cave)
                    state = CaveHighScore;
                else {
                    gd_critical("[highscore] section only allowed inside [game] and [cave]. This confuses the parser, bailing out!");
                    bailout = true;
                }
            } else if (gd_str_ascii_caseequal(line, "[/highscore]")) {
                if (state == GameHighScore)
                    state = Game;
                else if (state == CaveHighScore)
                    state = Cave;
                else {
                    gd_critical("[/highscore] only allowed after starting [highscore]. This confuses the parser, bailing out!");
                    bailout = true;
                }
            } else if (gd_str_ascii_caseequal(line, "[objects]")) {
                if (state != Cave)
                    gd_warning("[objects] tag only allowed in [cave]");
                if (file.caves.empty()) {
                    gd_warning("[replay] tag does not belong to any cave!");
                    file.caves.push_back(BdcffFile::CaveInfo());
                }
                state = CaveObjects;
            } else if (gd_str_ascii_caseequal(line, "[/objects]")) {
                if (state != CaveObjects)
                    gd_warning("[/objects] tag without starting [objects] tag");
                state = Cave;
            } else if (gd_str_ascii_caseequal(line, "[demo]")) {
                if (state != Cave)
                    gd_warning("[demo] tag only allowed in [cave]");
                if (file.caves.empty()) {
                    gd_warning("[demo] tag does not belong to any cave!");
                    file.caves.push_back(BdcffFile::CaveInfo());
                }
                state = CaveDemo;
                file.caves.back().demo.push_back("");   /* push an empty string, lines will be added */
            } else if (gd_str_ascii_caseequal(line, "[/demo]")) {
                if (state != CaveDemo)
                    gd_warning("[/demo] tag without starting [demo] tag");
                state = Cave;
            } else if (gd_str_ascii_caseequal(line, "[replay]")) {
                if (state != Cave)
                    gd_warning("[replay] tag only allowed in [cave]");
                if (file.caves.empty()) {
                    gd_warning("[replay] tag does not belong to any cave!");
                    file.caves.push_back(BdcffFile::CaveInfo());
                }
                state = CaveSReplay;
                file.caves.back().replays.push_back(BdcffFile::BdcffSection());
            } else if (gd_str_ascii_caseequal(line, "[/replay]")) {
                if (state != CaveSReplay)
                    gd_warning("[/replay] tag without starting [replay] tag");
                state = Cave;
            }
            /* GOSH i hate bdcff */
            else if (gd_str_ascii_prefix(line, "[level=")) {
                /* dump this thing in the object list. */
                if (state != CaveObjects)
                    gd_message("[level] tag only allowed inside [objects] section. Ignored.");
                else
                    file.caves.back().objects.push_back(line);
            } else if (gd_str_ascii_caseequal(line, "[/level]")) {
                /* dump this thing in the object list. */
                if (state != CaveObjects)
                    gd_message("[/level] tag only allowed inside [objects] section. Ignored.");
                else
                    file.caves.back().objects.push_back(line);
            } else
                gd_warning("unknown section: \"%s\"", line);

            continue;
        }

        /* OK, processed the section tags. */
        /* now copy the line to the correct part of the BdcffFile object. */

        /* first, check if we are at a map line. no stripping of spaces then! */
        if (state == CaveMap) {
            file.caves.back().map.push_back(line);
            continue;
        }

        /* if not a map, we may strip spaces. do it here. */
        gd_strchomp(line);

        switch (state) {
            case Start: /* should be nothing here. */
                gd_critical("nothing allowed outside [BDCFF]: %s", line);
                bailout = true;
                break;

            case Bdcff: /* inside [bdcff], eg. version=0.5 */
                file.bdcff.push_back(line);
                break;

            case Game:  /* inside [game], eg. author=foo */
                file.caveset_properties.push_back(line);
                break;

            case GameHighScore: /* trivial */
                file.highscore.push_back(line);
                break;

            case BdcffMapCodes:
            case GameMapCodes:  /* eg. x=STEELWALL */
                file.mapcodes.push_back(line);
                break;

            case Cave:          /* a cave, eg. name=Cave A */
                file.caves.back().properties.push_back(line);
                break;

            case CaveSReplay:    /* replay for a cave */
                file.caves.back().replays.back().push_back(line);
                break;

            case CaveDemo:      /* old styled demo (replay), just movements, no random data & the like */
                /* does not contain anything to check for! */
                file.caves.back().demo.back() += line + ' ';
                break;

            case CaveHighScore: /* highscores for a cave */
                file.caves.back().highscore.push_back(line);
                break;

            case CaveObjects:   /* objects for a cave */
                file.caves.back().objects.push_back(line);
                break;

            case CaveMap:
                /* should already have handled it above */
                g_assert_not_reached();
                break;
        }
    }

    if (bailout)
        throw std::runtime_error("Error parsing BDCFF input");

    return file;
}

CaveSet load_from_bdcff(const char *contents) {
    // this may throw, but we do not catch
    BdcffFile file = parse_bdcff_sections(contents);

    /* this cave will store the default properties, specified in the [game] section for caves. */
    /* especially the pain-in-the-ass engine tag. */
    CaveStored default_cave;

    CharToElementTable ctet;
    std::string version_read = "0.32";  /* assume version to be 0.32, also when the file does not specify it explicitly */

    /* PROCESS BDCFF PROPERTIES */
    for (auto it = file.bdcff.begin(); it != file.bdcff.end(); ++it) {
        AttribParam ap(*it);

        if (gd_str_ascii_caseequal(ap.attrib, "Version"))
            version_read = ap.param;
        else if (gd_str_ascii_caseequal(ap.attrib, "Engine")) {
            // invalid but we accept
            cave_process_tags_func(default_cave, ap.attrib, ap.param);
            gd_message("Invalid BDCFF: Engine= belongs in the [game] section!");
        } else
            gd_warning("Invalid attribute: %s", ap.attrib);
    }

    CaveSet cs;

    /* PROCESS CAVESET PROPERTIES */
    for (auto it = file.caveset_properties.begin(); it != file.caveset_properties.end(); ++it) {
        AttribParam ap(*it);

        if (gd_str_ascii_caseequal(ap.attrib, "Caves"))
            continue; /* BDCFF files sometimes state how many caves they have; we ignore this field. */
        if (gd_str_ascii_caseequal(ap.attrib, "Levels"))
            continue; /* BDCFF files sometimes state how many caves they have; we ignore this field. */

        /* try to interpret property for caveset */
        if (!struct_set_own_property(cs, ap.attrib, ap.param, 0))
            /* if not applicable, use it for the default cave */
            if (!cave_process_tags_func(default_cave, ap.attrib, ap.param))
                /* if not applicable for that, it is invalid. */
                gd_warning("Invalid attribute: %s", ap.attrib);
    }


    /* PROCESS CAVESET PROPERTIES */
    for (auto it = file.caveset_properties.begin(); it != file.caveset_properties.end(); ++it) {
        AttribParam ap(*it);

        if (gd_str_ascii_caseequal(ap.attrib, "Caves"))
            continue; /* BDCFF files sometimes state how many caves they have; we ignore this field. */
        if (gd_str_ascii_caseequal(ap.attrib, "Levels"))
            continue; /* BDCFF files sometimes state how many caves they have; we ignore this field. */

        /* try to interpret property for caveset */
        if (!struct_set_own_property(cs, ap.attrib, ap.param, 0))
            /* if not applicable, use it for the default cave */
            if (!struct_set_own_property(default_cave, ap.attrib, ap.param, default_cave.w * default_cave.h))
                /* if not applicable for that, it is invalid. */
                gd_warning("Invalid attribute: %s", ap.attrib);
    }

    /* PROCESS CAVESET HIGHSCORE */
    /* if not using bdcff highscore, simply ignore it. */
    if (gd_use_bdcff_highscore) {
        for (auto it = file.highscore.begin(); it != file.highscore.end(); ++it) {
            /* stored as <score> <space> <name> */
            try {
                AttribParam ap(*it, ' ');
                if (!add_highscore(cs.highscore, ap.param, ap.attrib))
                    gd_message("Invalid highscore: '%s'", *it);
            } catch (std::exception &e) {
                gd_message("Invalid highscore line: '%s'", *it);
            }
        }
    }

    /* PROCESS CAVESET MAPCODES */
    for (auto it = file.mapcodes.begin(); it != file.mapcodes.end(); ++it) {
        AttribParam ap(*it);

        if (gd_str_ascii_caseequal(ap.attrib, "Length")) {
            if (ap.param != "1")
                gd_critical("Only one-character map codes are currently supported!");
        } else {
            GdElement elem;
            if (read_from_string(ap.param, elem))
                ctet.set(ap.attrib[0], elem);
            else
                gd_warning("Unknown element name for map char: '%s'", ap.attrib[0]);
        }
    }

    /* PROCESS CAVES */
    /* xxx const iterator cannot be used */
    for (std::list<BdcffFile::CaveInfo>::iterator it = file.caves.begin(); it != file.caves.end(); ++it) {
        CaveStored cave = default_cave;

        cave_process_all_tags(cave, it->properties);

        /* process cave highscore. if not using bdcff highscore, simply ignore. */
        if (gd_use_bdcff_highscore) {
            for (auto hit = it->highscore.begin(); hit != it->highscore.end(); ++hit) {
                /* stored as <score> <space> <name> */
                try {
                    AttribParam ap(*hit, ' ');
                    if (!add_highscore(cave.highscore, ap.param, ap.attrib))
                        gd_message("Invalid highscore: '%s'", *hit);
                } catch (std::exception &e) {
                    gd_message("Invalid highscore line: '%s'", *hit);
                }
            }
        }

        /* at the end, when read all tags (especially the size= tag) */
        /* process map, if any. */
        /* only report if map read is bigger than size= specified. */
        /* some old bdcff files use smaller intermissions than the one specified. */
        if (!it->map.empty()) {
            /* yes, we have a map. */
            /* create map and fill with initial border, in case that map strings are shorter or somewhat */
            cave.map.set_size(cave.w, cave.h, cave.initial_border);

            if (int(it->map.size()) != cave.height())
                gd_warning("map error: cave height=%d (%d visible), map height=%u", cave.height(), cave.y2 - cave.y1 + 1, it->map.size());

            int y = 0;
            for (auto mit = it->map.begin(); mit != it->map.end(); ++mit) {
                int linelen = mit->size();

                for (int x = 0; x < std::min(linelen, signed(cave.w)); x++)
                    cave.map(x, y) = ctet.get((*mit)[x]);
                ++y;
                if (y >= cave.h)
                    break;
            }
        }

        /* process cave objects */
        GdBoolLevels levels;
        for (unsigned n = 0; n < 5; ++n)
            levels[n] = true;
        for (auto oit = it->objects.begin(); oit != it->objects.end(); ++oit) {
            // process [levels] tags for objects, or process objects.
            // [level] tags are badly designed in bdcff, as they are
            // not really "sections", but properties of objects.
            // yet, they are stored in sections. huge fail.
            if (*oit == "[/Level]") {
                for (unsigned n = 0; n < 5; ++n)
                    levels[n] = true;
            } else if (gd_str_ascii_prefix(*oit, "[Level=")) {
                std::istringstream is(oit->substr(oit->find('=') + 1));
                for (unsigned n = 0; n < 5; ++n)
                    levels[n] = false;
                int i;
                while (is >> i) {
                    if (i - 1 >= 0 && i - 1 < 5)
                        levels[i - 1] = true;
                    else {
                        gd_warning("Invalid [Levels=xxx] specification");
                        for (unsigned n = 0; n < 5; ++n)
                            levels[n] = true;
                        break;
                    }
                    char c;
                    is >> c; // read comma
                }
            } else {
                auto newobj = CaveObject::create_from_bdcff(*oit);
                if (newobj) {
                    for (unsigned n = 0; n < 5; ++n)
                        newobj->seen_on[n] = levels[n];
                    cave.objects.push_back(std::move(newobj));
                } else
                    gd_warning("invalid object specification: %s", *oit);
            }
        }

        /* process replays */
        for (auto rit = it->replays.begin(); rit != it->replays.end(); ++rit) {
            cave.replays.push_back(CaveReplay());       /* push an empty replay */
            CaveReplay &replay = cave.replays.back(); /* and work on that object */

            replay.saved = true; /* set "saved" flag, so this replay will be written when the caveset is saved again */
            /* and process its contents */
            for (auto lines_it = rit->begin(); lines_it != rit->end(); ++lines_it) {
                if (lines_it->find('=') != std::string::npos) {
                    AttribParam ap(*lines_it);
                    replay_process_tag(replay, ap.attrib, ap.param);
                } else
                    replay_process_tag(replay, "Movements", *lines_it); /* try to interpret it as a bdcff replay */
            }
        }

        /* process demos */
        for (auto dit = it->demo.begin(); dit != it->demo.end(); ++dit) {
            cave.replays.push_back(CaveReplay());       /* push an empty replay */
            CaveReplay &replay = cave.replays.back(); /* and work on that object */

            replay.saved = true; /* set "saved" flag, so this replay will be written when the caveset is saved again */
            replay.player_name = "???";
            replay_process_tag(replay, "Movements", *dit);  /* try to interpret it as a bdcff replay */
        }
        
        cs.caves.push_back(std::move(cave));   /* add new cave */
    }

    /* old bdcff files hack. explanation follows. */
    /* there were 40x22 caves in c64 bd, intermissions were also 40x22, but the visible */
    /* part was the upper left corner, 20x12. 40x22 caves are needed, as 20x12 caves would */
    /* look different (random cave elements needs the correct size.) */
    /* also, in older bdcff files, there is no size= tag. caves default to 40x22 and 20x12. */
    /* even the explicit drawrect and other drawing instructions, which did set up intermissions */
    /* to be 20x12, are deleted. very very bad decision. */
    /* here we try to detect and correct this. */
    if (version_read == "0.32") {
        gd_message("No BDCFF version, or 0.32. Using unspecified-intermission-size hack.");

        for (unsigned int i = 0; i < cs.caves.size(); ++i) {
            CaveStored &cav = cs.caves[i];

            /* only applies to intermissions */
            /* not applied to mapped caves, as maps are filled with initial border, if the map read is smaller */
            if (cav.intermission && cav.map.empty()) {
                /* we do not set the cave to 20x12, rather to 40x22 with 20x12 visible. */
                cav.w = 40;
                cav.h = 22;
                cav.x1 = 0;
                cav.y1 = 0;
                cav.x2 = 19;
                cav.y2 = 11;

                /* and cover the invisible area */
                cav.objects.push_back(CaveFillRect(Coordinate(0, 11), Coordinate(39, 21), cav.initial_border, cav.initial_border));
                cav.objects.push_back(CaveFillRect(Coordinate(19, 0), Coordinate(39, 21), cav.initial_border, cav.initial_border));
            }
        }
    }

    if (version_read != BDCFF_VERSION)
        gd_warning("BDCFF version %s, loaded caveset may have errors.", version_read);

    // check for replays which are problematic
    for (unsigned int i = 0; i < cs.caves.size(); ++i)
        gd_cave_check_replays(cs.caves[i], true, false, false);

    // return the created caveset.
    return cs;
}
