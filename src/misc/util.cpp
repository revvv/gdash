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
#include <cstdlib>
#include <cstring>

#include "cave/colors.hpp"
#include "misc/logger.hpp"

#include "misc/util.hpp"

std::string gd_tostring_free(char *str) {
    std::string ret;

    if (str) {
        ret = str;
        g_free(str);
    }

    return ret;
}

static std::string find_file_try_path(const char *path, const char *filename) {
    std::string ret;
    char *result;

    // create path
    result = g_build_path(G_DIR_SEPARATOR_S, path, filename, NULL);

    // check if exists
    if (g_file_test(result, G_FILE_TEST_EXISTS))
        ret = result;
    g_free(result);

    return ret;
}


/* tries to find a file in the gdash installation and returns a path */
std::string gd_find_data_file(const std::string &filename, const std::vector<std::string>& dirs) {
    for (unsigned i = 0; i < dirs.size(); ++i) {
        std::string result = find_file_try_path(dirs[i].c_str(), filename.c_str());
        if (result != "")
            return result;
    }

    return "";
}

int gd_clamp(int val, int min, int max) {
    g_assert(min <= max);

    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

double gd_clamp_double(double val, double min, double max) {
    g_assert(min <= max);

    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

/* return current date in 2008-12-04 format */
std::string gd_get_current_date() {
    char dats[128];

    GDate *dat = g_date_new();
    g_date_set_time_t(dat, time(NULL));
    g_date_strftime(dats, sizeof(dats), "%Y-%m-%d", dat);
    g_date_free(dat);

    return dats;
}


std::string gd_get_current_date_time() {
    char dats[128];

    GDate *dat = g_date_new();
    g_date_set_time_t(dat, time(0));
    g_date_strftime(dats, sizeof(dats), "%Y-%m-%d %H:%I", dat);
    g_date_free(dat);

    return dats;
}

/* remove leading and trailing spaces from string */
void gd_strchomp(std::string &s) {
    while (s.length() > 0 && s[0] == ' ')
        s.erase(0, 1);
    while (s.length() > 0 && s[s.length() - 1] == ' ')
        s.erase(s.length() - 1, 1);
}

int gd_str_ascii_casecmp(const std::string &s1, const std::string &s2) {
    int s1len = s1.length();
    int s2len = s2.length();

    if (s1len == 0 && s2len == 0)
        return 0;
    if (s1len == 0 && s2len != 0)
        return -1;
    if (s1len != 0 && s2len == 0)
        return 1;

    /* compare characters */
    int i = 0;
    while (i < s1len && i < s2len) {
        int c1 = s1[i];
        if (c1 >= 'A' && c1 <= 'Z')
            c1 = c1 - 'A' + 'a'; /* convert to lowercase, but only ascii characters */

        int c2 = s2[i];
        if (c2 >= 'A' && c2 <= 'Z')
            c2 = c2 - 'A' + 'a';

        if (c1 != c2)
            return (c1 - c2);
        i++;
    }
    /* ... one of the strings (or both of them) are ended. */
    if (s2len < s1len)  /* if s1 is longer, it is "larger". (s2 is the prefix of s1.) */
        return -1;
    if (s2len > s1len)  /* if s2 is longer, it is "larger". (s1 is the prefix of s2.) */
        return 1;
    return 0;
}

bool gd_str_ascii_caseequal(const std::string &s1, const std::string &s2) {
    return gd_str_ascii_casecmp(s1, s2) == 0;
}

bool gd_str_equal(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

bool gd_str_ascii_prefix(const std::string &str, const std::string &prefix) {
    return gd_str_ascii_casecmp(str.substr(0, prefix.length()), prefix) == 0;
}


std::vector<std::string> gd_wrap_text(const char *input, int width) {
    std::vector<std::string> retlines;

    std::istringstream is_lines(input);
    std::string thisline;
    while (getline(is_lines, thisline)) {
        gunichar *inputtext = g_utf8_to_ucs4(thisline.c_str(), -1, NULL, NULL, NULL);
        std::vector<gunichar> one_line, one_word;
        int wordlen = 0, linelen = 0;

        int i = 0;
        while (inputtext[i] != 0) {
            switch (inputtext[i]) {
                case ' ':
                    if (!one_word.empty()) {
                        /* cannot fit, must start new line? */
                        if (linelen + wordlen > width) {
                            gchar *utf8 = g_ucs4_to_utf8(&one_line[0], one_line.size(), NULL, NULL, NULL);
                            retlines.push_back(utf8);
                            g_free(utf8);
                            one_line.clear();
                            linelen = 0;
                        }
                        one_line.insert(one_line.end(), one_word.begin(), one_word.end());
                    }
                    one_word.clear();
                    linelen += wordlen;
                    wordlen = 0;
                    one_line.push_back(' ');
                    linelen++;
                    break;
                case GD_COLOR_SETCOLOR: /* the special markup character symbolizing color change */
                    one_word.push_back(inputtext[i]);
                    ++i;
                    one_word.push_back(inputtext[i]);
                    /* wordlen is deliberately not increased here */
                    break;
                default:
                    one_word.push_back(inputtext[i]);
                    wordlen++;
                    break;
            }
            i++;
        }
        g_free(inputtext);
        if (!one_word.empty()) {
            /* cannot fit, must start new line? */
            if (linelen + wordlen > width) {
                gchar *utf8 = g_ucs4_to_utf8(&one_line[0], one_line.size(), NULL, NULL, NULL);
                retlines.push_back(utf8);
                g_free(utf8);
                one_line.clear();
                linelen = 0;
            }
            one_line.insert(one_line.end(), one_word.begin(), one_word.end());
        }
        /* add the rest of the line */
        gchar *utf8 = g_ucs4_to_utf8(&one_line[0], one_line.size(), NULL, NULL, NULL);
        retlines.push_back(utf8);
        g_free(utf8);
    }

    return retlines;
}
