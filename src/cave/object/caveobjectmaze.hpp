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
#ifndef CAVEOBJECTMAZE_HPP_INCLUDED
#define CAVEOBJECTMAZE_HPP_INCLUDED

#include "config.h"

#include "cave/object/caveobjectrectangular.hpp"

/* forward declarations for maze object */
class RandomGenerator;
template <class T> class CaveMap;

/// Cave maze objects.
///
/// There are three types of them:
///     - perfect maze
///     - braid maze (with no dead ends)
///     - unicursal maze (which is a one long closed path).
/// This is stored in the maze_type member, but also the object type
/// is different for all three maze types.
///
/// Mazes have path and wall elements; and have path and wall widths.
/// They have random number seed values for all levels, so it is possible
/// to generate a different maze for all levels. If the seed
/// value is set to -1, a different maze is generated every time a cave is rendered.
/// The horizontal parameter is a value between 0 and 100; it determines how
/// much the maze generator algorithm prefers long horizontal paths. Default is 50.
///
/// If the size of the maze object is larger than the "common multiple" determined
/// by the wall and path width, the remaining part is filled with wall.
/// No border for the maze is drawn - if the user wants a border, he can use
/// a rectangle object.
class CaveMaze : public CaveRectangular {
public:
    enum MazeType {
        Perfect,
        Braid,
        Unicursal
    };

private:
    MazeType const maze_type;   ///< Type of the maze, perfect, braid, unicursal.
    GdInt wall_width;           ///< Width of the walls in the maze (in cells); >=1
    GdInt path_width;           ///< Width of the paths in the maze; >=1
    GdElement wall_element;     ///< Walls are made of this element
    GdElement path_element;     ///< Paths are made of this.
    GdInt horiz;                ///< 0..100, with greater numbers, horizontal paths are preferred
    GdIntLevels seed;           ///< Seed values for difficulty levels.

    static void mazegen(CaveMap<bool> &maze, RandomGenerator &rand, int x, int y, int horiz);
    static void braidmaze(CaveMap<bool> &maze, RandomGenerator &rand);
    static void unicursalmaze(CaveMap<bool> &maze, int &w, int &h);

public:
    CaveMaze(Coordinate _p1, Coordinate _p2, GdElementEnum _wall, GdElementEnum _path, MazeType _type);
    CaveMaze(): maze_type(Perfect) {}
    Type get_type() const { 
        if (maze_type == Braid) return GD_MAZE_BRAID;
        if (maze_type == Unicursal) return GD_MAZE_UNICURSAL;
        return GD_MAZE;
    }
    virtual std::unique_ptr<CaveObject> clone() const;
    virtual void draw(CaveRendered &cave, int order_idx) const;
    void set_horiz(int _horiz) {
        horiz = _horiz;
    }
    void set_widths(int wall, int path);
    void set_seed(int s1, int s2, int s3, int s4, int s5);
    virtual std::string get_bdcff() const;
    virtual std::unique_ptr<CaveObject> clone_from_bdcff(const std::string &name, std::istream &is) const;

private:
    static PropertyDescription const descriptor[];

public:
    virtual PropertyDescription const *get_description_array() const {
        return descriptor;
    }
    virtual GdElementEnum get_characteristic_element() const;
    virtual std::string get_description_markup() const;
};


#endif

