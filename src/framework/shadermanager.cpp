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

#include "misc/logger.hpp"
#include "misc/autogfreeptr.hpp"
#include "framework/shadermanager.hpp"
#include "settings.hpp"


static void add_dir_to_shaders(std::vector<std::string> &shaders, const char *directory_name) {
    gd_debug("checking dir %s for shaders", directory_name);

    GDir *dir = g_dir_open(directory_name, 0, NULL);
    /* silently ignore unable-to-open directories */
    if (!dir)
        return;
    char const *name;
    while ((name = g_dir_read_name(dir)) != NULL) {
        AutoGFreePtr<char> filename(g_build_filename(directory_name, name, NULL));
        AutoGFreePtr<char> lower(g_ascii_strdown(filename, -1));
        if ((g_str_has_suffix(lower, ".shader") && g_file_test(filename, G_FILE_TEST_IS_REGULAR))) {
            gd_debug("found shader %s", filename);
            shaders.push_back((char*) filename);
        }
    }
    g_dir_close(dir);
}


/* will create a list of file names which can be used as themes. */
/* the first item will be an empty string to represent the default, built-in theme. */
void load_shaders_list(std::vector<std::string> &shaders, int &shadernum) {
    shaders.clear();
    
    shaders.push_back("");    /* this symbolizes the empty shader */
    for (unsigned i = 0; i < gd_shaders_dirs.size(); ++i)
        add_dir_to_shaders(shaders, gd_shaders_dirs[i].c_str());

    /* find the current shader. */
    std::vector<std::string>::iterator it =
        find(shaders.begin(), shaders.end(), gd_shader);
    /* if the current shader is not in the list, put it into the list */
    if (it == shaders.end()) {
        shaders.push_back(gd_shader);
        shadernum = shaders.size() - 1;
    } else {
        shadernum = it - shaders.begin();
    }
}
