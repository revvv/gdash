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

#ifndef PRINTF_HPP_INCLUDED
#define PRINTF_HPP_INCLUDED

#include "config.h"

#include <string>
#include <sstream>
#include <vector>

/**
 * A class which is able to process format strings like misc/printf.
 *
 * Usage:
 * std::string s = Printf("Hello, %s! %-5d", "world", 9);
 * The format string is passed in the constructor. Also data to be processed.
 *
 * The conversion specifiers work the same way as they did in misc/printf.
 * The % character introduces a conversion, after which manipulators
 * can be given; the conversion specifier is terminated with a character
 * which specifies the type of the conversion.
 *
 * The % operator is a template, so any variable can be fed, which has
 * a standard << operator outputting it to an ostream. Giving the correct
 * type of the variable is therefore not important; Printf("%d", "hello")
 * will work. The main purpose of the format characters is to terminate the conversion
 * specifier, and to make the format strings compatible with those of printf. Sometimes they slightly
 * modify the conversion, eg. %x will print hexadecimal. %ms will print
 * a html-markup string, i.e. Printf("%ms", "i<5") will have "i&lt;5".
 *
 * Conversions can be modified by giving a width or a width.precision.
 * The modifiers 0 (to print zero padded, %03d), + (to always show sign, %+5d)
 * and - (to print left aligned, %-9s) are supported. Width calculation is
 * UTF-8-aware.
 */
class Printf {
  private:
    /// The format string, which may already have the results of some conversions.
    std::string format;
    /// Characters inserted so far - during the conversion. To know where to insert the next string.
    size_t inserted_chars = 0;

    struct Conversion {
        size_t pos;         ///< Position to insert the converted string at (+inserted_chars)
        char conv;          ///< conversion type
        int width;          ///< width. zero if no padding, positive is left padding, negative if right padding
        int precision;      ///< decimal places
        bool left;          ///< left pad
        char pad;           ///< padding char
        bool showpos;       ///< show positive sign, eg. +5
        bool html_markup;   ///< Must do HTML conversion (> to &gt; etc.) for this one
    };
    std::vector<Conversion> conversions;
    size_t next_conversion = 0;

    void parse_format_string(std::string format);
    std::ostringstream create_ostream(Conversion const & conversion) const;
    void insert_converted(std::string os, Conversion const & conversion);

    /// The % operator feeds the next data item into the string.
    /// The function replaces the next specified conversion.
    /// @param data The parameter to convert to string in the specified format.
    template <class TIP>
    void feed_one(TIP const & data) {
        Conversion &conversion = conversions.at(next_conversion);
        std::ostringstream os = create_ostream(conversion);
        os << data;
        insert_converted(std::move(os.str()), conversion);
        ++next_conversion;
    }
    
    template <typename HEAD, typename ... TAIL>
    void feed_args(HEAD const & head, TAIL const & ... tail) {
        feed_one(head);
        feed_args(tail...);
    }

    /// Base case for feed_args
    void feed_args() {
    }

  public:
    /// Printf object constructor.
    /// @param format The format string.
    template <typename ... ARGS>
    Printf(std::string const & format_, ARGS const & ... args) {
        parse_format_string(format_);
        feed_args(args...);
    }
    
    /// Convert result to std::string.
    operator std::string const &() const {
        return format;
    }
    
    /// std::string-like c_str().
    char const *c_str() const {
        return format.c_str();
    }
};

#endif

