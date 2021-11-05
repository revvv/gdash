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
#ifndef SOUND_HPP_INCLUDED
#define SOUND_HPP_INCLUDED

#include "config.h"

#include "cave/caverendered.hpp"

gboolean gd_sound_init(unsigned int bufsize = 44100 / 25);
void gd_sound_close();

void gd_sound_off();
void gd_sound_play_sounds(SoundWithPos const &sound1, SoundWithPos const &sound2, SoundWithPos const &sound3);
void gd_sound_play_bonus_life();

void gd_music_play_random();
void gd_music_stop();

void gd_sound_set_music_volume(int percent);
void gd_sound_set_chunk_volumes(int percent);
void gd_sound_set_music_volume();
void gd_sound_set_chunk_volumes();

#endif
