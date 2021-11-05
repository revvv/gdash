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

#ifndef CAVEMAP_HPP_INCLUDED
#define CAVEMAP_HPP_INCLUDED

#include "config.h"

#include <stdexcept>
#include <algorithm>
#include <utility>
#include <vector>

class CaveMapFuncs {
protected:
    CaveMapFuncs() = default;
public:
    /* types */
    enum WrapType {
        RangeCheck,
        Perfect,
        LineShift
    };
    
    static void range_check_coords(int w, int h, int &x, int &y) {
        if (x < 0 || y < 0 || x >= w || y >= h)
            throw std::out_of_range("CaveMap::getrangecheck");
    }

    /* functions which process coordinates to be "wrapped" */
    static void perfect_wrap_coords(int w, int h, int &x, int &y) {
        y = (y + h) % h;
        x = (x + w) % w;
    }
    
    /* this lineshifting does not fix the y coordinates. if out of bounds, it is left that way. */
    /* if such an object appeared in the c64 game, well, it was a buffer overrun - ONLY TO BE USED WHEN DRAWING THE CAVE */
    static void lineshift_wrap_coords_only_x(int w, int h, int &x, int &y) {
        if (x < 0) {
            y -= -x / w + 1;
            x = x % w + w;
        } else if (x >= w) {
            y += x / w;
            x = x % w;
        }
        /* here do not change y to be >=0 and <= h-1 */
    }
    
    /* fix y coordinate, too: TO BE USED WHEN PLAYING THE CAVE */
    static void lineshift_wrap_coords_both(int w, int h, int &x, int &y) {
        lineshift_wrap_coords_only_x(w, h, x, y);
        y = (y + h) % h;
    }
};


template <typename T>
class CaveMap: public CaveMapFuncs {
private:
    /**
     * Zero-overhead wrapper class for the contained T object.
     * This allows us to use a bool, prevent std::vector<bool> specialization from being instantiated.
     */
    struct BoxedT {
        T boxed_t = T();
    };
    
    int w = 0, h = 0;
    std::vector<BoxedT> data;
    CaveMapFuncs::WrapType wrap_type = CaveMapFuncs::RangeCheck;

public:
    CaveMap() = default;
    CaveMap(int w, int h, const T &initial = T())
        : w(w), h(h), data(w * h, BoxedT{initial}) {
    }
    void set_size(int new_w, int new_h, const T &def = T());
    void resize(int new_w, int new_h, const T &def = T());
    void remove() {
        data.clear();
        w = h = 0;
    }
    void fill(const T &value) {
        std::fill(data.begin(), data.end(), BoxedT{value});
    }
    bool empty() const {
        return w == 0 || h == 0;
    }
    int width() const {
        return w;
    }
    int height() const {
        return h;
    }

    void set_wrap_type(CaveMapFuncs::WrapType t) {
        wrap_type = t;
    }
    
    T & operator()(int x, int y) {
        switch (wrap_type) {
            case CaveMap<T>::RangeCheck:
                CaveMapFuncs::range_check_coords(w, h, x, y);
                break;
            case CaveMap<T>::Perfect:
                CaveMapFuncs::perfect_wrap_coords(w, h, x, y);
                break;
            case CaveMap<T>::LineShift:
                CaveMapFuncs::lineshift_wrap_coords_both(w, h, x, y);
                break;
        }
        return data[y * w + x].boxed_t;
    }
    
    const T & operator()(int x, int y) const {
        return const_cast<CaveMap<T>&>(*this)(x, y);    /* const cast, but return const& */
    }
};


/* set size of map; fill all with def */
template <typename T>
void CaveMap<T>::set_size(int new_w, int new_h, const T &def) {
    /* resize only if size is really new; otherwise only fill */
    if (new_w != w || new_h != h) {
        w = new_w;
        h = new_h;
        data = std::vector<BoxedT>(w * h, BoxedT{def});
    } else {
        fill(def);
    }
}


/* resize map to new size; new parts are filled with def */
template <typename T>
void CaveMap<T>::resize(int new_w, int new_h, const T &def) {
    int orig_w = w, orig_h = h;
    if (new_w == orig_w && new_h == orig_h) /* same size - do nothing */
        return;

    /* new array */
    std::vector<BoxedT> new_data(new_w * new_h, BoxedT{def});

    /* copy useful data from original */
    for (int y = 0; y < std::min(orig_h, new_h); y++)
        for (int x = 0; x < std::min(orig_w, new_w); x++)
            new_data[y * new_w + x] = data[y * orig_w + x];

    /* remember new sizes */
    w = new_w;
    h = new_h;
    data = std::move(new_data);
}

#endif
