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

#ifndef PIXMAPSTORAGE_HPP_INCLUDED
#define PIXMAPSTORAGE_HPP_INCLUDED

#include "config.h"

#include "gfx/screen.hpp"

/// @ingroup Graphics
/// A class which stores rendered Pixmap objects can be notified by a Screen object
/// when the resolution or the video mode change. The Screen object does this by calling
/// the PixmapStorage::release_pixmaps() function of the storage object. The storage object
/// should be able to recreate its Pixmap objects, i.e. it should be able to recover after
/// a resolution change.
class PixmapStorage {
private:
    Screen &screen;
public:
    PixmapStorage(Screen &screen) : screen(screen) {
        screen.register_pixmap_storage(this);
    }
    virtual ~PixmapStorage() {
        screen.unregister_pixmap_storage(this);
    }
    
    PixmapStorage(PixmapStorage const &) = delete;
    PixmapStorage(PixmapStorage &&) = delete;
    PixmapStorage& operator=(PixmapStorage const &) = delete;
    PixmapStorage& operator=(PixmapStorage &&) = delete;
    
    /// Notifying the PixmapStorage object that its Pixmap objects are to be deleted.
    /// This function is called just before the resolution change.
    virtual void release_pixmaps() = 0;
};

#endif
