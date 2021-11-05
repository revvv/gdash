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

#include <cmath>
#include <memory>

#include "sdl/sdlabstractscreen.hpp"

#include "cave/colors.hpp"
#include "gfx/pixbuffactory.hpp"
#include "sdl/sdlpixbuf.hpp"
#include "sdl/sdlscreen.hpp"
#include "cave/particle.hpp"
#include "mainwindow.hpp"
#include "settings.hpp"


int SDLPixmap::get_width() const {
    return surface->w;
}


int SDLPixmap::get_height() const {
    return surface->h;
}


void SDLAbstractScreen::fill_rect(int x, int y, int w, int h, const GdColor &c) {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    unsigned char r, g, b;
    c.get_rgb(r, g, b);
    SDL_FillRect(surface.get(), &dst, SDL_MapRGB(surface->format, r, g, b));
}

void SDLAbstractScreen::blit(Pixmap const &src, int dx, int dy) const {
    SDL_Surface *from = static_cast<SDLPixmap const &>(src).surface.get();
    SDL_Rect dstr;
    dstr.x = dx;
    dstr.y = dy;
    /* the clipping in sdl_blitsurface is very fast, so we do not do any pre-clipping here */
    SDL_BlitSurface(from, NULL, surface.get(), &dstr);
}


void SDLAbstractScreen::set_clip_rect(int x1, int y1, int w, int h) {
    /* on-screen clipping rectangle */
    SDL_Rect cliprect;
    cliprect.x = x1;
    cliprect.y = y1;
    cliprect.w = w;
    cliprect.h = h;
    SDL_SetClipRect(surface.get(), &cliprect);
}


void SDLAbstractScreen::remove_clip_rect() {
    SDL_SetClipRect(surface.get(), NULL);
}


/**
 * \brief Draw a horizontal line with alpha blending.
 * The surface must be locked if needed.
 * \param dst The surface to draw on.
 * \param x1 X coordinate of the start point (upper left) of the line.
 * \param x2 X coordinate of the end point (upper left) of the line.
 * \param y Y coordinate of the line.
 * \param color The color value of the line to draw (0xRRGGBBAA).
*/
static void hlineColor(SDL_Surface *dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color, bool pal_emu) {
    {
        /* Get clipping boundary and check visibility of hline */
        Sint16 left = dst->clip_rect.x;
        Sint16 right = dst->clip_rect.x + dst->clip_rect.w - 1;
        Sint16 top = dst->clip_rect.y;
        Sint16 bottom = dst->clip_rect.y + dst->clip_rect.h - 1;
        if (x2 < left || x1 > right || y < top || y > bottom)
            return;
        /* Clip */
        if (x1 < left)
            x1 = left;
        if (x2 > right)
            x2 = right;
    }

    SDL_PixelFormat *format = dst->format;

    Uint8 cR = (color >> 24) & 0xFF,
          cG = (color >> 16) & 0xFF,
          cB = (color >> 8) & 0xFF,
          cA = (color >> 0) & 0xFF;
    /* if we are doing software pal emu, here shade the particles as well */
    if (pal_emu && y % 2 == 1)
        cA = cA * gd_pal_emu_scanline_shade / 100;

    Uint32 Rmask = format->Rmask;
    Uint32 Gmask = format->Gmask;
    Uint32 Bmask = format->Bmask;
    Uint32 Rshift = format->Rshift;
    Uint32 Gshift = format->Gshift;
    Uint32 Bshift = format->Bshift;
    Uint32 Rloss = format->Rloss;
    Uint32 Gloss = format->Gloss;
    Uint32 Bloss = format->Bloss;

    switch (format->BytesPerPixel) {
        case 1: {
            /* 8-bpp */
            SDL_Palette *palette = format->palette;
            SDL_Color *colors = palette->colors;

            Uint8 *row = (Uint8 *) dst->pixels + y * dst->pitch;
            for (Sint32 x = x1; x <= x2; x++) {
                Uint8 *pixel = row + x;

                Sint32 R = colors[*pixel].r;
                Sint32 G = colors[*pixel].g;
                Sint32 B = colors[*pixel].b;

                R = R + ((cR - R) * cA >> 8);
                G = G + ((cG - G) * cA >> 8);
                B = B + ((cB - B) * cA >> 8);

                *pixel = SDL_MapRGB(format, R, G, B);
            }
        }
        break;

        case 2: {
            /* 15-bpp or 16-bpp */
            Uint16 *row = (Uint16 *) dst->pixels + y * dst->pitch / 2;
            for (Sint32 x = x1; x <= x2; x++) {
                Uint16 *pixel = row + x;

                Sint32 R = (*pixel & Rmask) >> Rshift << Rloss;
                Sint32 G = (*pixel & Gmask) >> Gshift << Gloss;
                Sint32 B = (*pixel & Bmask) >> Bshift << Bloss;

                R = R + ((cR - R) * cA >> 8);
                G = G + ((cG - G) * cA >> 8);
                B = B + ((cB - B) * cA >> 8);

                *pixel = R >> Rloss << Rshift | G >> Gloss << Gshift | B >> Bloss << Bshift;
            }
        }
        break;

        case 3: {
            /* 24-bpp */
            Uint32 bitoff = ~(Rmask | Gmask | Bmask);
            for (Sint32 x = x1; x <= x2; x++) {
                Uint32 *pixel = (Uint32 *)((Uint8 *) dst->pixels + y * dst->pitch + x * 3);

                Sint32 R = (*pixel & Rmask) >> Rshift;
                Sint32 G = (*pixel & Gmask) >> Gshift;
                Sint32 B = (*pixel & Bmask) >> Bshift;

                R = R + ((cR - R) * cA >> 8);
                G = G + ((cG - G) * cA >> 8);
                B = B + ((cB - B) * cA >> 8);

                *pixel = (*pixel & bitoff) | R << Rshift | G << Gshift | B << Bshift;
            }
        }
        break;

        case 4: {
            /* 32-bpp */
            Uint32 bitoff = ~(Rmask | Gmask | Bmask);   // to retain alpha
            Uint32 *row = (Uint32 *) dst->pixels + y * dst->pitch / 4;
            for (Sint32 x = x1; x <= x2; x++) {
                Uint32 *pixel = row + x;

                Uint8 R = (*pixel & Rmask) >> Rshift;
                Uint8 G = (*pixel & Gmask) >> Gshift;
                Uint8 B = (*pixel & Bmask) >> Bshift;

                R = R + ((cR - R) * cA >> 8);
                G = G + ((cG - G) * cA >> 8);
                B = B + ((cB - B) * cA >> 8);

                *pixel = (*pixel & bitoff) | R << Rshift | G << Gshift | B << Bshift;
            }
        }
        break;
    }
}


/** Draw a diamond with the specified color, 0xRRGGBBAA.
 * The surface must be already locked if needed. */
static void filledDiamondColor(SDL_Surface *dst, Sint16 xc, Sint16 yc, Sint16 r, Uint32 color, bool software_pal_emulation) {
    if ((dst->clip_rect.w == 0) || (dst->clip_rect.h == 0))
        return;
    if (r < 0)
        return;

    Sint16 x2 = xc + r;
    Sint16 left = dst->clip_rect.x;
    if (x2 < left)
        return;
    Sint16 x1 = xc - r;
    Sint16 right = dst->clip_rect.x + dst->clip_rect.w - 1;
    if (x1 > right)
        return;
    Sint16 y2 = yc + r;
    Sint16 top = dst->clip_rect.y;
    if (y2 < top)
        return;
    Sint16 y1 = yc - r;
    Sint16 bottom = dst->clip_rect.y + dst->clip_rect.h - 1;
    if (y1 > bottom)
        return;

    /* Draw */
    hlineColor(dst, xc - r, xc + r, yc, color, software_pal_emulation);
    for (Sint16 f = 0; f < r; ++f) {
        hlineColor(dst, xc - f, xc + f, yc - (r - f), color, software_pal_emulation);
        hlineColor(dst, xc - f, xc + f, yc + (r - f), color, software_pal_emulation);
    }
}


void SDLAbstractScreen::draw_particle_set(int dx, int dy, ParticleSet const &ps) {
    unsigned char r, g, b;
    ps.color.get_rgb(r, g, b);
    Uint8 a = ps.life / 1000.0 * ps.opacity * 255;
    Uint32 color = r << 24 | g << 16 | b << 8 | a << 0;
    int size = ceil(ps.size);
    if (SDL_MUSTLOCK(surface.get()))
        if (SDL_LockSurface(surface.get()) < 0)
            return;
    bool software_pal_emulation = get_pal_emulation();
    for (ParticleSet::const_iterator it = ps.begin(); it != ps.end(); ++it) {
        filledDiamondColor(surface.get(), dx + it->px, dy + it->py, size, color, software_pal_emulation);
    }
    if (SDL_MUSTLOCK(surface.get()))
        SDL_UnlockSurface(surface.get());
}
