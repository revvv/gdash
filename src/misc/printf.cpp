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
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <glib.h>

#include "cave/cavetypes.hpp"

#include "misc/printf.hpp"

namespace {

    /// All conversion specifiers which are recognized by the class in the format string, eg. printf %s, %d, %c etc.
    const char *conv_specifiers = "sdiucfgx";
    /// Conversion modifiers supported.
    const char *flag_characters = "0-+lhm";


    /**
     * @brief This function html-markups a string.
     * It will change <, >, &, and \n characters into &lt; &gt; &amp; and <br>.
     */
    std::string html_markup_text(const std::string &of_what) {
        std::string result;
        result.reserve(of_what.size());

        for (unsigned i = 0; i < of_what.size(); ++i) {
            switch (of_what[i]) {
                case '<':
                    result += "&lt;";
                    break;
                case '>':
                    result += "&gt;";
                    break;
                case '&':
                    result += "&amp;";
                    break;
                case '\n':
                    result += "\n<br>\n";
                    break;
                default:
                    result += of_what[i];
                    break;
            }
        }

        return result;
    }


    /// Pads a piece of text on the left or on the right with the specified padding char
    std::string pad_text(std::string what, int width, char pad, bool left) {
        int length = g_utf8_strlen(what.c_str(), -1);
        if (width - length <= 0)
            return std::move(what);     // no place to pad
        std::string pads(width - length, pad);
        if (left)
            return what + pads;
        else
            return pads + what;
    }

}   // namespace

void Printf::parse_format_string(std::string format) {
    size_t pos, nextpos = 0;
    
    // search for the next conversion specifier.
    // the position is stored in pos.
    // if a %% is found, it is replaced with %, and the search is continued.
    while ((pos = format.find('%', nextpos)) != std::string::npos) {
        if (pos + 1 == format.length())
            throw std::runtime_error("unterminated conversion specifier at the end of the string");
        /* just a percent sign? */
        if (format[pos + 1] == '%') {
            format.erase(pos, 1);
            nextpos = pos + 1;
            continue;
        }

        /* this is a conversion specifier. */
        size_t last = format.find_first_of(conv_specifiers, pos + 1);
        if (last == std::string::npos)
            throw std::runtime_error("unterminated conversion specifier");

        // ok we found something like %-5s. get the conversion type (s), and parse the manipulator (-5).
        Conversion c;
        c.pos = pos;
        c.conv = format[last];
        c.html_markup = false;
        c.width = 0;
        c.precision = -1;
        c.left = false;
        c.pad = ' ';
        c.showpos = false;
        // parse the manipulator
        std::string manip = format.substr(pos + 1, last - pos - 1);
        while (!manip.empty() && strchr(flag_characters, manip[0]) != NULL) {
            switch (manip[0]) {
                case '-':
                    c.left = true;
                    break;
                case '0':
                    c.pad = '0';
                    break;
                case '+':
                    c.showpos = true;
                    break;
                case 'l':
                case 'h':
                    // do nothing; for compatibility with printf;
                    break;
                case 'm':
                    c.html_markup = true;
                    break;
                default:
                    throw std::logic_error("unknown flag");
            }
            manip.erase(0, 1);  // erase processed flag from the string
        }
        if (!manip.empty()) {
            std::istringstream is(manip);
            is >> c.width;
            is.clear(); // clear error state, as we might not have had a width specifier
            char ch;
            if (is >> ch) {
                is >> c.precision;
                if (!is)
                    throw std::runtime_error("invalid precision");
            }
        }
        
        conversions.push_back(c);
        // now delete the conversion specifier from the string.
        format.erase(pos, last - pos + 1);
        nextpos = pos;
    }
    
    this->format = format;
}

/// This function finds the next conversion specifier in the format string,
/// and sets the ostringstream accordingly.
/// @param os The ostringstream to setup according to the next found conversion specifier.
/// @param pos The position, which is the char position of the original conversion specifier.
std::ostringstream Printf::create_ostream(Conversion const & conversion) const {
    std::ostringstream os;
    if (conversion.conv == 'x')
        os << std::hex;
    if (conversion.showpos)
        os << std::showpos;
    if (conversion.precision >= 0) {
        os.precision(conversion.precision);
        os << std::fixed;
    }
    return os;
}


/// This function puts the contents of the ostringstream to the
/// string at the given position.
/// @param os The ostringstream, which should already contain the data formatted.
/// @param pos The position to insert the contents of the string at.
void Printf::insert_converted(std::string str, Conversion const & conversion) {
    // pad it and html-markup it
    if (conversion.width != 0)
        str = pad_text(std::move(str), conversion.width, conversion.pad, conversion.left);
    if (conversion.html_markup)
        str = html_markup_text(str);

    // add inserted_chars to the originally calculated position - as
    // before this conversion, the already finished conversions added that much characters before the current position
    format.insert(conversion.pos + inserted_chars, str);
    
    // and remember the successive position
    inserted_chars += str.length();
}
