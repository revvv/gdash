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

#include <iomanip>
#include <stdexcept>
#include "fileops/bdcffhelper.hpp"
#include "misc/printf.hpp"

/// Constructor: split string given by the separator given to attrib and param.
/// @param str The string to split.
/// @param separator Separator between attrib and param; default is =.
AttribParam::AttribParam(const std::string &str, char separator) {
    size_t equal = str.find(separator);
    if (equal == std::string::npos)
        throw std::runtime_error(Printf("No separator in line: '%s'", str));
    attrib = str.substr(0, equal);
    param = str.substr(equal + 1);
}


/// Create a new formatter.
/// @param F The name of the output string; for example
///         give it "Point" if intending to write a line like "Point=1 2 DIRT"
BdcffFormat::BdcffFormat(const std::string &f)
    :   name(f),
        firstparam(true) {
    os << std::setprecision(4) << std::fixed;
}

/// Get the output string.
/// @return The converted string.
std::string BdcffFormat::str() const {
    if (name.empty())
        return os.str();
    else
        return name + '=' + os.str();
}
