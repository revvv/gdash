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

#include <sstream>
#include "cave/object/caveobject.hpp"
#include "cave/helper/namevaluepair.hpp"
#include "cave/helper/polymorphic.hpp"

/* for factory */
#include "cave/object/caveobjectboundaryfill.hpp"
#include "cave/object/caveobjectcopypaste.hpp"
#include "cave/object/caveobjectfillrect.hpp"
#include "cave/object/caveobjectfloodfill.hpp"
#include "cave/object/caveobjectjoin.hpp"
#include "cave/object/caveobjectline.hpp"
#include "cave/object/caveobjectmaze.hpp"
#include "cave/object/caveobjectpoint.hpp"
#include "cave/object/caveobjectrandomfill.hpp"
#include "cave/object/caveobjectraster.hpp"
#include "cave/object/caveobjectrectangle.hpp"


/// Check if the object is visible on all levels.
/// @return True, if visible.
bool CaveObject::is_seen_on_all() const {
    for (unsigned i = 0; i < 5; ++i)
        if (!seen_on[i])
            return false;       // if invisible on any, not seen on all.
    return true;
}

/// Check if the object is invisible - not visible on any level.
/// @return True, if fully invisible.
bool CaveObject::is_invisible() const {
    for (unsigned i = 0; i < 5; i++)
        if (seen_on[i])
            return false;       // if seen on any, not invisible.
    return true;
}

/// Enable object on all levels.
void CaveObject::enable_on_all() {
    for (unsigned i = 0; i < 5; ++i)
        seen_on[i] = true;
}

/// Disable object on all levels.
void CaveObject::disable_on_all() {
    for (unsigned i = 0; i < 5; ++i)
        seen_on[i] = false;
}

namespace {
    NameValuePair<Polymorphic<CaveObject>> object_prototypes = {
        {"Point", CavePoint()},
        {"Line", CaveLine()},
        {"Rectangle", CaveRectangle()},
        {"FillRect", CaveFillRect()},
        {"Raster", CaveRaster()},
        {"Join", CaveJoin()},
        {"Add", CaveJoin()},
        {"AddBackward", CaveJoin()},
        {"BoundaryFill", CaveBoundaryFill()},
        {"FloodFill", CaveFloodFill()},
        {"Maze", CaveMaze()},
        {"CopyPaste", CaveCopyPaste()},
        {"RandomFill", CaveRandomFill()},
        {"RandomFillC64", CaveRandomFill()},
    };
}

std::unique_ptr<CaveObject> CaveObject::create_from_bdcff(const std::string &str) {
    std::string::size_type f = str.find('=');
    if (f == std::string::npos)
        return NULL;
    try {
        std::string type = str.substr(0, f);
        std::istringstream is(str.substr(f + 1));
        return object_prototypes.lookup_name(type).get().clone_from_bdcff(type, is);
    } catch (std::exception &e) {
        return NULL;
    }
}
