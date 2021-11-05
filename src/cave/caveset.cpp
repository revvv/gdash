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

#include <glib/gi18n.h>

#include <cstdio>
#include <fstream>
#include "cave/caveset.hpp"
#include "misc/logger.hpp"
#include "misc/autogfreeptr.hpp"
#include "cave/caverendered.hpp"
#include "fileops/bdcffsave.hpp"

/* list of possible extensions which can be opened */
const char *gd_caveset_extensions[] = {"*.gds", "*.bd", "*.bdr", "*.brc", "*.vsf", "*.mem", NULL};


PropertyDescription const CaveSet::descriptor[] = {
    /* default data */
    {"", GD_TAB, 0, N_("Caveset data")},
    {"Name", GD_TYPE_STRING, 0, N_("Name"), GetterBase::create_new(&CaveSet::name), N_("Name of the game")},
    {"Description", GD_TYPE_STRING, 0, N_("Description"), GetterBase::create_new(&CaveSet::description), N_("Some words about the game")},
    {"Author", GD_TYPE_STRING, 0, N_("Author"), GetterBase::create_new(&CaveSet::author), N_("Name of author")},
    {"Date", GD_TYPE_STRING, 0, N_("Date"), GetterBase::create_new(&CaveSet::date), N_("Date of creation")},
    {"WWW", GD_TYPE_STRING, 0, N_("WWW"), GetterBase::create_new(&CaveSet::www), N_("Web page or e-mail address")},
    {"Difficulty", GD_TYPE_STRING, 0, N_("Difficulty"), GetterBase::create_new(&CaveSet::difficulty), N_("Difficulty (informative)")},

    {"Charset", GD_TYPE_STRING, 0, N_("Character set"), GetterBase::create_new(&CaveSet::charset), N_("Theme used for displaying the game.")},
    {"Fontset", GD_TYPE_STRING, 0, N_("Font set"), GetterBase::create_new(&CaveSet::fontset), N_("Font used during the game.")},

    {"Lives", GD_TYPE_INT, 0, N_("Initial lives"), GetterBase::create_new(&CaveSet::initial_lives), N_("Number of lives you get at game start."), 3, 99},
    {"Lives", GD_TYPE_INT, 0, N_("Maximum lives"), GetterBase::create_new(&CaveSet::maximum_lives), N_("Maximum number of lives you can have by collecting bonus points."), 3, 99},
    {"BonusLife", GD_TYPE_INT, 0, N_("Bonus life score"), GetterBase::create_new(&CaveSet::bonus_life_score), N_("Number of points to collect for a bonus life."), 100, 5000},

    {"", GD_TAB, 0, N_("Story")},
    {"Story", GD_TYPE_LONGSTRING, 0, NULL, GetterBase::create_new(&CaveSet::story), N_("Long description of the game.")},
    {"", GD_TAB, 0, N_("Remark")},
    {"Remark", GD_TYPE_LONGSTRING, 0, NULL, GetterBase::create_new(&CaveSet::remark), N_("Remark (informative).")},

    {"TitleScreen", GD_TYPE_LONGSTRING, GD_DONT_SHOW_IN_EDITOR, N_("Title screen"), GetterBase::create_new(&CaveSet::title_screen), N_("Title screen image")},
    {"TitleScreenScroll", GD_TYPE_LONGSTRING, GD_DONT_SHOW_IN_EDITOR, N_("Title screen, scrolling"), GetterBase::create_new(&CaveSet::title_screen_scroll), N_("Scrolling background for title screen image")},

    {NULL},
};


/* calculates an adler checksum, for which it uses all
   elements of all cave-rendereds. */
unsigned CaveSet::checksum() const {
    unsigned a = 1, b = 0;
    for (unsigned int i = 0; i < caves.size(); ++i) {
        CaveRendered rendered(caves[i], 0, 0);  /* level=1, seed=0 */
        gd_cave_adler_checksum_more(rendered, a, b);
    }
    return (b << 16) + a;
}



/********************************************************************************
 *
 * Misc caveset functions
 *
 */

/* return index of first selectable cave */
int CaveSet::first_selectable_cave_index() const {
    for (unsigned int i = 0; i < caves.size(); ++i) {
        if (caves[i].selectable)
            return i;
    }

    gd_warning("no selectable cave in caveset!");
    /* and return the first one. */
    return 0;
}


void CaveSet::set_name_from_filename(const char *filename) {
    /* make up a caveset name from the filename. */
    AutoGFreePtr<char> name_str(g_path_get_basename(filename));
    /* convert underscores to spaces, remove extension */
    char *c;
    while ((c = strchr(name_str, '_')) != NULL)
        * c = ' ';
    if ((c = strrchr(name_str, '.')) != NULL)
        * c = 0;
    name = name_str;
}


/// Save caveset in BDCFF to the file.
/// @param filename The name of the file to write to.
/// @return true, if successful; false, if error.
void CaveSet::save_to_file(const char *filename) {
    std::ofstream outfile;
    outfile.open(filename);
    if (!outfile)
        throw std::runtime_error(_("Could not open file for writing."));
    std::list<std::string> saved;
    save_to_bdcff(*this, saved);
    for (std::list<std::string>::const_iterator it = saved.begin(); it != saved.end(); ++it)
        outfile << *it << std::endl;
    outfile.close();
    if (!outfile)
        throw std::runtime_error(_("Error writing to file."));
    /* remember savename and that now it is not edited */
    this->filename = filename;
    this->edited = false;
}


/// Check if there are any replays in the caveset.
/// @return True, if at least one of the caves has a replay.
bool CaveSet::has_replays() const {
    /* for all caves */
    for (unsigned int i = 0; i < caves.size(); ++i) {
        if (!caves[i].replays.empty())
            return true;
    }

    /* no replays at all */
    return false;
}


/// Check if there are any caves which have different difficulty levels.
/// @return True, if at least one of the caves has different difficulty levels set.
bool CaveSet::has_levels() const {
    /* for all caves */
    for (unsigned int i = 0; i < caves.size(); ++i) {
        if (caves[i].has_levels())
            return true;
    }
    /* no levels at all */
    return false;
}
