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
#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include "config.h"

#include <string>
#include <vector>

/// find file, looking in user directory and install directory
/// @return full path of file if found, "" if not
std::string gd_find_data_file(const std::string &filename, const std::vector<std::string>& dirs);

/// create a string form a char*, and free it with g_free
std::string gd_tostring_free(char *str);

/// @brief Wrap a text to lines of specified maximum width.
/// Honors original newlines as well.
/// @param input The UTF-8 text to wrap. May contain UTF-8 characters, and GD_COLOR_SETCOLOR characters.
/// @param width The maximum length of lines.
/// @return The vector of lines.
std::vector<std::string> gd_wrap_text(const char *input, int width);

/// @brief Returns current date in 2008-12-04 format.
std::string gd_get_current_date();

/// @brief Returns current date in 2008-12-04 12:34 format.
std::string gd_get_current_date_time();

/// clamp integer into range
int gd_clamp(int val, int min, int max);

/// clamp double into range
double gd_clamp_double(double val, double min, double max);

/// remove leading and trailing spaces from s
void gd_strchomp(std::string &s);

/// compare two strings, ignoring case for ascii characters
int gd_str_ascii_casecmp(const std::string &s1, const std::string &s2);

bool gd_str_ascii_caseequal(const std::string &s1, const std::string &s2);

bool gd_str_equal(const char *a, const char *b);

bool gd_str_ascii_prefix(const std::string &str, const std::string &prefix);

#endif
