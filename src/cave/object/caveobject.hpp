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
#ifndef CAVEOBJECT_HPP_INCLUDED
#define CAVEOBJECT_HPP_INCLUDED

#include "config.h"

#include <memory>

#include "cave/cavetypes.hpp"
#include "cave/helper/reflective.hpp"

class CaveRendered;

/// Cave object abstract base class, from which all cave objects inherit.
class CaveObject : public Reflective {
public:
    /// The type of the object.
    /// This is an enumeration of all possible object types.
    enum Type {
        GD_POINT,               ///< single point of object1
        GD_LINE,                ///< line from (1) to (2) of object1
        GD_RECTANGLE,           ///< rectangle with corners (1) and (2) of object1
        GD_FILLED_RECTANGLE,    ///< rectangle with corners (1) and (2) of object1, filled with object2
        GD_RASTER,              ///< aligned plots
        GD_JOIN,                ///< every object1 has an object2 next to it, relative (dx,dy)
        GD_FLOODFILL_REPLACE,   ///< fill by replacing
        GD_FLOODFILL_BORDER,    ///< fill to another element, a border
        GD_MAZE,                ///< maze
        GD_MAZE_UNICURSAL,      ///< unicursal maze
        GD_MAZE_BRAID,          ///< braid maze
        GD_RANDOM_FILL,         ///< random fill
        GD_COPY_PASTE,          ///< copy & paste with optional mirror and flip
    };

    /// Levels on which this object is visible on.
    GdBoolLevels seen_on;

    bool is_seen_on_all() const;
    bool is_invisible() const;
    void enable_on_all();
    void disable_on_all();

    /// By default, the object is seen on all levels.
    CaveObject() {
        enable_on_all();
    }

    /// Virtual destructor.
    virtual ~CaveObject() = default;
    
    /// Get type enum of object.
    virtual Type get_type() const = 0;

    /// Clone object - create an exact copy.
    /// This will be the virtual constructor.
    virtual std::unique_ptr<CaveObject> clone() const = 0;

    /// Draw the object on a specified level, in the specified cave. All derived objects implement this.
    /// @param cave The mapped (for game) cave to draw on
    virtual void draw(CaveRendered &cave, int order_idx) const = 0;

    /// Get BDCFF description of object. All derived objects must implement.
    virtual std::string get_bdcff() const = 0;

    /// Create a new object, and load properties from an istream.
    /// As sometimes the object name in the bdcff file also stores information about
    /// an object (for example, AddBackwards, we must also pass the name.
    /// Implementations should return NULL, if a read error occurs.
    virtual std::unique_ptr<CaveObject> clone_from_bdcff(const std::string &name, std::istream &is) const = 0;

    /// Object factory.
    static std::unique_ptr<CaveObject> create_from_bdcff(const std::string &str);

    /* for the editor */
    /// Move an object after creating it in the editor; dragging the mouse at the moment when the
    /// button is still pressed.
    /// @param current The coordinate clicked.
    /// @param displacement The movement vector after the last click.
    virtual void create_drag(Coordinate current, Coordinate displacement) = 0;

    /// This function is called when a single object is moved in the editor by drag&drop.
    /// @param current The coordinate clicked.
    /// @param displacement The movement vector after the last click.
    virtual void move(Coordinate current, Coordinate displacement) = 0;

    /// This function is called when multiple objects are dragged in the editor.
    /// @param displacement The movement vector.
    virtual void move(Coordinate displacement) = 0;

    /// Get a short string which shows the coordinates of the object in the object list of the editor.
    virtual std::string get_coordinates_text() const = 0;

    /// Get an element which is very characteristic of the object - to show in the object list.
    /// If there is none, this returns O_NONE (for example a copy&paste object).
    virtual GdElementEnum get_characteristic_element() const = 0;

    /// Get a string which describes the object in one sentence. It is translated to the current language.
    virtual std::string get_description_markup() const = 0;
    
    /// If this coordinate is to be marked with an x in the editor.
    virtual bool mark_coordinate(Coordinate c) const {
        return false;
    }
};

#endif
