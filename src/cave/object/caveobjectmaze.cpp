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

#include "cave/object/caveobjectmaze.hpp"
#include "cave/elementproperties.hpp"

#include <glib/gi18n.h>

#include "fileops/bdcffhelper.hpp"
#include "cave/caverendered.hpp"
#include "misc/printf.hpp"
#include "misc/logger.hpp"
#include "misc/util.hpp"

std::string CaveMaze::get_bdcff() const {
    BdcffFormat f("Maze");

    f << p1 << p2 << wall_width << path_width << horiz;
    f << seed[0] << seed[1] << seed[2] << seed[3] << seed[4] << wall_element << path_element;
    switch (maze_type) {
        case Perfect:
            f << "perfect";
            break;
        case Unicursal:
            f << "unicursal";
            break;
        case Braid:
            f << "braid";
            break;
        default:
            g_assert_not_reached();
    };
    return f;
}

std::unique_ptr<CaveObject> CaveMaze::clone_from_bdcff(const std::string &name, std::istream &is) const {
    Coordinate p1, p2;
    int ww, pw, horiz;
    int seed[5];
    GdElementEnum wall, path;
    std::string mazetype;
    CaveMaze::MazeType mazet;

    if (!(is >> p1 >> p2 >> ww >> pw >> horiz >> seed[0] >> seed[1] >> seed[2] >> seed[3] >> seed[4] >> wall >> path >> mazetype))
        return NULL;
    if (gd_str_ascii_caseequal(mazetype, "unicursal"))
        mazet = CaveMaze::Unicursal;
    else if (gd_str_ascii_caseequal(mazetype, "perfect"))
        mazet = CaveMaze::Perfect;
    else if (gd_str_ascii_caseequal(mazetype, "braid"))
        mazet = CaveMaze::Braid;
    else {
        gd_warning("unknown maze type: %s, defaulting to perfect", mazetype);
        mazet = CaveMaze::Perfect;
    }
    CaveMaze m(p1, p2, wall, path, mazet);
    m.set_horiz(horiz);
    m.set_widths(ww, pw);
    m.set_seed(seed[0], seed[1], seed[2], seed[3], seed[4]);

    return std::make_unique<CaveMaze>(std::move(m));
}

std::unique_ptr<CaveObject>CaveMaze::clone() const {
    return std::make_unique<CaveMaze>(*this);
};

/* create a maze in a bool map. */
/* recursive algorithm. */
void CaveMaze::mazegen(CaveMap<bool> &maze, RandomGenerator &rand, int x, int y, int horiz) {
    int dirmask = 15;

    maze(x, y) = true;
    while (dirmask != 0) {
        int dir;

        dir = rand.rand_int_range(0, 100) < horiz ? 2 : 0; /* horiz or vert */
        /* if no horizontal movement possible, choose vertical */
        if (dir == 2 && (dirmask & 12) == 0)
            dir = 0;
        else if (dir == 0 && (dirmask & 3) == 0) /* and vice versa */
            dir = 2;
        dir += rand.rand_int_range(0, 2);           /* dir */

        if (dirmask & (1 << dir)) {
            dirmask &= ~(1 << dir);

            switch (dir) {
                case 0: /* up */
                    if (y >= 2 && !maze(x, y - 2)) {
                        maze(x, y - 1) = true;
                        mazegen(maze, rand, x, y - 2, horiz);
                    }
                    break;
                case 1: /* down */
                    if (y < maze.height() - 2 && !maze(x, y + 2)) {
                        maze(x, y + 1) = true;
                        mazegen(maze, rand, x, y + 2, horiz);
                    }
                    break;
                case 2: /* left */
                    if (x >= 2 && !maze(x - 2, y)) {
                        maze(x - 1, y) = true;
                        mazegen(maze, rand, x - 2, y, horiz);
                    }
                    break;
                case 3: /* right */
                    if (x < maze.width() - 2 && !maze(x + 2, y)) {
                        maze(x + 1, y) = true;
                        mazegen(maze, rand, x + 2, y, horiz);
                    }
                    break;
                default:
                    g_assert_not_reached();
            }
        }
    }
}


void
CaveMaze::braidmaze(CaveMap<bool> &maze, RandomGenerator &rand) {
    int w = maze.width();
    int h = maze.height();

    for (int y = 0; y < h; y += 2)
        for (int x = 0; x < w; x += 2) {
            int closed = 0, dirs = 0;
            int closed_dirs[4];

            /* if it is the edge of the map, OR no path carved, then we can't go in that direction. */
            if (x < 1 || !maze(x - 1, y)) {
                closed++;   /* closed from this side. */
                if (x > 0)
                    closed_dirs[dirs++] = MV_LEFT;  /* if not the edge, we might open this wall (carve a path) to remove a dead end */
            }
            /* other 3 directions similar */
            if (y < 1 || !maze(x, y - 1)) {
                closed++;
                if (y > 0)
                    closed_dirs[dirs++] = MV_UP;
            }
            if (x >= w - 1 || !maze(x + 1, y)) {
                closed++;
                if (x < w - 1)
                    closed_dirs[dirs++] = MV_RIGHT;
            }
            if (y >= h - 1 || !maze(x, y + 1)) {
                closed++;
                if (y < h - 1)
                    closed_dirs[dirs++] = MV_DOWN;
            }

            /* if closed from 3 sides, then it is a dead end. also check dirs!=0, that might fail for a 1x1 maze :) */
            if (closed == 3 && dirs != 0) {
                /* make up a random direction, and open in that direction, so dead end is removed */
                int dir = closed_dirs[rand.rand_int_range(0, dirs)];

                switch (dir) {
                    case MV_LEFT:
                        maze(x - 1, y) = true;
                        break;
                    case MV_UP:
                        maze(x, y - 1) = true;
                        break;
                    case MV_RIGHT:
                        maze(x + 1, y) = true;
                        break;
                    case MV_DOWN:
                        maze(x, y + 1) = true;
                        break;
                }
            }
        }
}


void CaveMaze::unicursalmaze(CaveMap<bool> &maze, int &w, int &h) {
    /* convert to unicursal maze */
    /* original:
        xxx x
          x x
        xxxxx

        unicursal:
        xxxxxxx xxx
        x     x x x
        xxxxx x x x
            x x x x
        xxxxx xxx x
        x         x
        xxxxxxxxxxx
    */
    CaveMap<bool> unicursal(w * 2 + 1, h * 2 + 1, false);

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            if (maze(x, y)) {
                unicursal(x * 2, y * 2) = true;
                unicursal(x * 2 + 2, y * 2) = true;
                unicursal(x * 2, y * 2 + 2) = true;
                unicursal(x * 2 + 2, y * 2 + 2) = true;

                if (x < 1 || !maze(x - 1, y)) unicursal(x * 2, y * 2 + 1) = true;
                if (y < 1 || !maze(x, y - 1)) unicursal(x * 2 + 1, y * 2) = true;
                if (x >= w - 1 || !maze(x + 1, y)) unicursal(x * 2 + 2, y * 2 + 1) = true;
                if (y >= h - 1 || !maze(x, y + 1)) unicursal(x * 2 + 1, y * 2 + 2) = true;
            }
        }

    /* change to new maze - the unicursal maze */
    /* copy this, as this will be drawn */
    maze = unicursal;
    h = h * 2 - 1;
    w = w * 2 - 1;
}


CaveMaze::CaveMaze(Coordinate _p1, Coordinate _p2, GdElementEnum _wall, GdElementEnum _path, MazeType _type)
    :   CaveRectangular(_p1, _p2),
        maze_type(_type),
        wall_width(1),
        path_width(1),
        wall_element(_wall),
        path_element(_path),
        horiz(50) {
    for (int j = 0; j < 5; j++)
        seed[j] = -1;
}

/// Set wall and path width.
void CaveMaze::set_widths(int wall, int path) {
    wall_width = wall;
    path_width = path;
}

/// Set seed values.
void CaveMaze::set_seed(int s1, int s2, int s3, int s4, int s5) {
    seed[0] = s1;
    seed[1] = s2;
    seed[2] = s3;
    seed[3] = s4;
    seed[4] = s5;
}

/*
 * This draws all kinds of mazes.
 *
 * Steps:
 * - draw a normal maze with walls and paths of 1 cell in width.
 * - if needed, convert it to braid maze or unicursal maze.
 * - resize the maze to the given path and wall width.
 */
void CaveMaze::draw(CaveRendered &cave, int order_idx) const {
    /* change coordinates if not in correct order */
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    if (y1 > y2)
        std::swap(y1, y2);
    if (x1 > x2)
        std::swap(x1, x2);
    int wall = this->wall_width;
    if (wall < 1)
        wall = 1;
    int path = this->path_width;
    if (path < 1)
        path = 1;

    /* calculate the width and height of the maze.
        n=number of passages, path=path width, wall=wall width, maze=maze width.
       if given the number of passages, the width of the maze is:

       n*path+(n-1)*wall=maze
       n*path+n*wall-wall=maze
       n*(path+wall)=maze+wall
       n=(maze+wall)/(path+wall)
     */
    /* number of passages for each side */
    int w = (x2 - x1 + 1 + wall) / (path + wall);
    int h = (y2 - y1 + 1 + wall) / (path + wall);
    /* and we calculate the size of the internal map */
    if (maze_type == Unicursal) {
        /* for unicursal maze, width and height must be mod2=0, and we will convert to paths&walls later */
        w = w / 2 * 2;
        h = h / 2 * 2;
    } else {
        /* for normal maze */
        w = 2 * (w - 1) + 1;
        h = 2 * (h - 1) + 1;
    }

    RandomGenerator rand;
    if (seed[cave.rendered_on] == -1)
        rand.set_seed(cave.random.rand_int());
    else
        rand.set_seed(seed[cave.rendered_on]);

    /* start generation only if map is big enough. otherwise the application would crash, as the editor
     * places maze objects during mouse click&drag with no size! */

    /* draw maze. */
    CaveMap<bool> map(w, h, false);
    if (w >= 1 && h >= 1)
        mazegen(map, rand, 0, 0, horiz);
    /* if braid maze, turn the above to a braid one. */
    if (maze_type == Braid)
        braidmaze(map, rand);
    /* if unicursal, turn it into an unicursal one. w and h is changed! */
    if (w >= 1 && h >= 1 && maze_type == Unicursal)
        unicursalmaze(map, w, h);

    /* copy map to cave with correct elements and size */
    /* now copy the map into the cave. the copying works like this...
       pwpwp
       xxxxx p
       x x   w
       x xxx p
       x     w
       xxxxx p
       columns and rows denoted with "p" are to be drawn with path width, the others with wall width. */
    int yk = y1;
    for (int y = 0; y < h; y++) {
        for (int i = 0; i < (y % 2 == 0 ? path : wall); i++) {
            int xk = x1;
            for (int x = 0; x < w; x++)
                for (int j = 0; j < (x % 2 == 0 ? path : wall); j++)
                    cave.store_rc(xk++, yk, map(x, y) ? path_element : wall_element, order_idx);

            /* if width is smaller than requested, fill with wall */
            for (int x = xk; x <= x2; x++)
                cave.store_rc(x, yk, wall_element, order_idx);

            yk++;
        }
    }
    /* if height is smaller than requested, fill with wall */
    for (int y = yk; y <= y2; y++)
        for (int x = x1; x <= x2; x++)
            cave.store_rc(x, y, wall_element, order_idx);
}

PropertyDescription const CaveMaze::descriptor[] = {
    {"", GD_TAB, 0, N_("Maze")},
    {"", GD_TYPE_BOOLEAN_LEVELS, 0, N_("Levels"), GetterBase::create_new(&CaveMaze::seen_on), N_("Levels on which this object is visible.")},
    {"", GD_TYPE_INT_LEVELS, 0, N_("Seed"), GetterBase::create_new(&CaveMaze::seed), N_("The random seed value for the random number generator. If it is -1, the cave is different every time played. If not, the same pattern is always recreated."), -1, GD_CAVE_SEED_MAX},
    {"", GD_TYPE_COORDINATE, 0, N_("Start corner"), GetterBase::create_new(&CaveMaze::p1), N_("Specifies one of the corners of the object."), 0, 127},
    {"", GD_TYPE_COORDINATE, 0, N_("End corner"), GetterBase::create_new(&CaveMaze::p2), N_("Specifies one of the corners of the object."), 0, 127},
    {"", GD_TYPE_INT, 0, N_("Wall width"), GetterBase::create_new(&CaveMaze::wall_width), NULL, 1, 40},
    {"", GD_TYPE_INT, 0, N_("Path width"), GetterBase::create_new(&CaveMaze::path_width), NULL, 1, 40},
    {"", GD_TYPE_ELEMENT, 0, N_("Wall element"), GetterBase::create_new(&CaveMaze::wall_element), N_("Walls of the maze are drawn using this element.")},
    {"", GD_TYPE_ELEMENT, 0, N_("Path element"), GetterBase::create_new(&CaveMaze::path_element), N_("This element sets the paths in the maze.")},
    {"", GD_TYPE_INT, 0, N_("Horizontal %"), GetterBase::create_new(&CaveMaze::horiz), N_("This number controls, how much the algorithm will prefer horizontal routes."), 0, 100},
    {NULL},
};

std::string CaveMaze::get_description_markup() const {
    return Printf(_("Maze from %d,%d to %d,%d, wall <b>%ms</b>, path <b>%ms</b>"), p1.x, p1.y, p2.x, p2.y, visible_name_lowercase(wall_element), visible_name_lowercase(path_element));
}

GdElementEnum CaveMaze::get_characteristic_element() const {
    return path_element;
}
