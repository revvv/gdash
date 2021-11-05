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

#include <glib.h>
#include <cstring>
#include <cstdio>
#include <fstream>

#include "misc/autogfreeptr.hpp"
#include "misc/logger.hpp"
#include "cave/caveset.hpp"
#include "fileops/highscore.hpp"
#include "fileops/bdcffsave.hpp"
#include "fileops/bdcffload.hpp"
#include "fileops/bdcffhelper.hpp"
#include "settings.hpp"


/* make up a filename for the current caveset, to save highscores in. */
/* returns the file name; owned by the function (no need to free()) */
static std::string filename_for_cave_highscores(CaveSet const & caveset) {
    AutoGFreePtr<char> canon(g_strdup(caveset.name == "" ? "highscore-" : caveset.name.c_str()));
    /* allowed chars in the highscore file name; others are replaced with _ */
    g_strcanon(canon, "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", '_');
    AutoGFreePtr<char> fname(g_strdup_printf("%08x-%s.stat", caveset.checksum(), (char*) canon));
    AutoGFreePtr<char> outfile(g_build_path(G_DIR_SEPARATOR_S, gd_user_config_dir.c_str(), (char*) fname, NULL));
    return (char*) outfile;
}


/** Save highscores and playing stat of the current caveset to the configuration directory. */
void save_highscore(CaveSet const & caveset) {
    std::list<std::string> out;
    CaveStored defaultcave;     /* for the reflective comparison */

    /* caveset: only highscore */
    out.push_back(Printf("; Caveset: %s", caveset.name));
    out.push_back(Printf("Index=%d", -1));
    for (unsigned int i = 0; i < caveset.highscore.size(); i++)
        out.push_back(BdcffFormat("Highscore") << caveset.highscore[i].score << caveset.highscore[i].name);
    out.push_back("");
    
    /* for all caves: stat & highscore */
    for (unsigned int i = 0; i < caveset.caves.size(); ++i) {
        CaveStored const &cave = caveset.caves[i];
        out.push_back(Printf("; Cave: %s", cave.name));
        out.push_back(Printf("Index=%d", i));
        for (unsigned int i = 0; i < cave.highscore.size(); i++)
            out.push_back(BdcffFormat("Highscore") << cave.highscore[i].score << cave.highscore[i].name);
        save_properties(out, cave, defaultcave, 0, CaveStored::cave_statistics_data);
        out.push_back("");
    }

    /* write to file */
    std::ofstream outfile;
    outfile.open(filename_for_cave_highscores(caveset).c_str());
    for (std::list<std::string>::const_iterator it = out.begin(); it != out.end(); ++it)
        outfile << *it << std::endl;
    outfile.close();
}


/** Load highscores from a file saved in the configuration directory. */
bool load_highscore(CaveSet & caveset) {
    /* try to open file. */
    std::ifstream infile;
    infile.open(filename_for_cave_highscores(caveset).c_str());
    if (!infile.is_open())
        return false;
    
    /* if opened */
    std::string line;
    std::istringstream is;
    int caveindex = -1;
    while (getline(infile, line)) {
        if (line == "" || line[0] == ';')
            continue;

        AttribParam ap(line);
        if (ap.attrib == "Index") {
            std::istringstream is(ap.param);
            is >> caveindex;
        }
        else if (ap.attrib == "Highscore") {
            /* here we abuse an attribparam object to split the score from the name, which are like <score><SPACE><name> */
            AttribParam scorename(ap.param, ' ');
            std::istringstream is(scorename.attrib);
            int score;
            is >> score;
            if (caveindex == -1)
                caveset.highscore.add(scorename.param, score);
            else
                caveset.caves[caveindex].highscore.add(scorename.param, score);
        }
        else {
            if (!struct_set_property(caveset.caves[caveindex], ap.attrib, ap.param, 0, CaveStored::cave_statistics_data)) {
                gd_debug("No such property: %s", ap.attrib);
            }
        }
    }
    infile.close();
    return true;
}
