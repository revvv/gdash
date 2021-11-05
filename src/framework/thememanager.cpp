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
#include <algorithm>

#include "framework/thememanager.hpp"
#include "misc/logger.hpp"
#include "misc/autogfreeptr.hpp"
#include "gfx/pixbuffactory.hpp"
#include "gfx/cellrenderer.hpp"
#include "settings.hpp"

static void add_file_to_themes(PixbufFactory &pf, std::vector<std::string> &themes, const char *filename) {
    g_assert(filename != NULL);

    /* if file name already in themes list, remove. */
    for (unsigned i = 0; i < themes.size(); i++)
        if (themes[i] != "" && themes[i] == filename) {
            themes.erase(themes.begin() + i);
            i--;
        }

    if (CellRenderer::is_image_ok_for_theme(pf, filename))
        themes.push_back(filename);
}

static void add_dir_to_themes(PixbufFactory &pf, std::vector<std::string> &themes, const char *directory_name) {
    Logger l(true);       /* do not report error messages */

    GDir *dir = g_dir_open(directory_name, 0, NULL);
    /* silently ignore unable-to-open directories */
    if (!dir)
        return;
    char const *name;
    while ((name = g_dir_read_name(dir)) != NULL) {
        AutoGFreePtr<char> filename(g_build_filename(directory_name, name, NULL));
        AutoGFreePtr<char> lower(g_ascii_strdown(filename, -1));
        /* we only allow bmp and png files. converted to lowercase. */
        if ((g_str_has_suffix(lower, ".bmp") || g_str_has_suffix(lower, ".png")) && g_file_test(filename, G_FILE_TEST_IS_REGULAR))
            /* try to add the file. */
            add_file_to_themes(pf, themes, filename);
    }
    g_dir_close(dir);
}


void load_themes_list(PixbufFactory &pf, std::vector<std::string> &themes, int &themenum) {
    /* will create a list of file names which can be used as themes. */
    /* the first item will be an empty string to represent the default, built-in theme. */
    themes.clear();
    themes.push_back("");    /* this symbolizes the default theme */
    for (unsigned i = 0; i < gd_themes_dirs.size(); ++i)
        add_dir_to_themes(pf, themes, gd_themes_dirs[i].c_str());
    
    /* find the current theme */
    std::vector<std::string>::iterator it = find(themes.begin(), themes.end(), gd_theme);
    /* if its not in the list, add. also set the index */
    if (it == themes.end()) {
        add_file_to_themes(pf, themes, gd_theme.c_str());
        themenum = themes.size() - 1;
    } else {
        themenum = it - themes.begin();
    }
}


std::string filename_for_new_theme(const char *theme_to_install) {
    // create a new filename, with all lowercase letters
    AutoGFreePtr<char> basename(g_path_get_basename(theme_to_install));
    AutoGFreePtr<char> utf8(g_filename_to_utf8(basename, -1, NULL, NULL, NULL));
    AutoGFreePtr<char> lowercase(g_utf8_strdown(utf8, -1));
    AutoGFreePtr<char> lowercase_sys(g_filename_from_utf8(lowercase, -1, NULL, NULL, NULL));
    // first dir in themes_dirs is the themes dir of the user
    AutoGFreePtr<char> new_filename(g_build_path(G_DIR_SEPARATOR_S, gd_themes_dirs[0].c_str(), (char*) lowercase_sys, NULL));
    return (char *) new_filename;
}


bool install_theme(const char *from, const char *to) {
    // first dir in themes_dirs is the themes dir of the user
    g_mkdir_with_parents(gd_themes_dirs[0].c_str(), 0700);
    GError *error = NULL;
    gchar *contents = NULL;
    gsize length;
    gboolean result = g_file_get_contents(from, &contents, &length, &error) && g_file_set_contents(to, contents, length, &error);
    if (error) {
        gd_critical(error->message);
        g_error_free(error);
    }
    g_free(contents);
    return result;
}
