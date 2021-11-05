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

#ifdef HAVE_SDL
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_mixer.h>
#endif

#include <glib.h>
#include <cmath>
#include "settings.hpp"
#include "cave/helper/cavesound.hpp"
#include "cave/caverendered.hpp"
#include "misc/logger.hpp"
#include "misc/util.hpp"

#include "sound/sound.hpp"

/**
    @file sound/sound.cpp

    SDL Sound routines.

    The C64 sound chip (the SID) had 3 channels. Boulder Dash used all 3 of them.

    Different channels were used for different sounds.

    - Channel 1: Small sounds, ie. diamonds falling, boulders rolling.

    - Channel 2: Walking; also time running out sound.

    - Channel 3: amoeba sound, magic wall sound,
        cave cover & uncover sound, and the crack sound (gate open)

    Sounds have precedence over each other. Ie. the crack sound is given precedence
    over other sounds (amoeba, for example.)
    Different channels also behave differently. Channel 2 sounds are not stopped, ie.
    walking can not be heard, until the explosion sound is finished playing completely.
    Explosions are always restarted, though. This is controlled by the array defined
    in cave.c.
    Channel 1 sounds are always stopped, if a new sound is requested.

 */


#ifdef HAVE_SDL
static Mix_Chunk *sounds[GD_S_MAX];
static bool mixer_started = false;
static GdSound snd_playing[5];
static Mix_Music *music = NULL;

static int music_volume = MIX_MAX_VOLUME;
#endif

#ifdef HAVE_SDL
static GdSound sound_playing(int channel) {
    if (!mixer_started || !gd_sound_enabled)
        return GD_S_NONE;

    return snd_playing[channel];
}
#endif

void gd_sound_set_music_volume(int percent) {
#ifdef HAVE_SDL
    if (!mixer_started || !gd_sound_enabled)
        return;

    music_volume = MIX_MAX_VOLUME * percent / 100;
    Mix_VolumeMusic(music_volume);
#endif
}

void gd_sound_set_music_volume() {
#ifdef HAVE_SDL
    gd_sound_set_music_volume(gd_sound_music_volume_percent);
#endif
}

void gd_sound_set_chunk_volumes(int percent) {
#ifdef HAVE_SDL
    if (!mixer_started || !gd_sound_enabled)
        return;

    Mix_Volume(-1, MIX_MAX_VOLUME * percent / 100);
#endif
}

void gd_sound_set_chunk_volumes() {
#ifdef HAVE_SDL
    gd_sound_set_chunk_volumes(gd_sound_chunks_volume_percent);
#endif
}

#ifdef HAVE_SDL
static void loadsound(GdSound which, const char *filename) {
    g_assert(!gd_sound_is_fake(which));
    g_assert(mixer_started);
    /* make sure sound isn't already loaded */
    if (sounds[which] != NULL)
        Mix_FreeChunk(sounds[which]);

    std::string full_filename = gd_find_data_file(filename, gd_sound_dirs);
    if (full_filename == "") {
        /* if cannot find file, exit now */
        gd_message("%s: no such sound file", filename);
        return;
    }

    sounds[which] = Mix_LoadWAV(full_filename.c_str());
    if (sounds[which] == NULL)
        gd_message("%s: %s", filename, Mix_GetError());
}
#endif


#ifdef HAVE_SDL
/* this is called by sdlmixer for an sdlmixer channel. so no magic with channel_x_alter. */
static void channel_done(int channel) {
    snd_playing[channel] = GD_S_NONE;
}
#endif

#ifdef HAVE_SDL
static void halt_channel(int channel) {
    Mix_FadeOutChannel(channel, 40);
}
#endif

#ifdef HAVE_SDL
static void set_channel_panning(int channel, int dx, int dy) {
    if (gd_sound_stereo) {
        int left = gd_clamp(128 - dx * 2, 0, 255);
        int distance = gd_clamp(sqrt(dx * dx + dy * dy) * 2, 0, 255);
        Mix_SetPanning(channel, left, 255 - left);
        Mix_SetDistance(channel, distance);
    }
}
#endif

#ifdef HAVE_SDL
static void play_sound(int channel, SoundWithPos sound) {
    /* channel 1 and channel 4 are used alternating */
    /* channel 2 and channel 5 are used alternating */
    static const GdSound diamond_sounds[] = {
        GD_S_DIAMOND_1,
        GD_S_DIAMOND_2,
        GD_S_DIAMOND_3,
        GD_S_DIAMOND_4,
        GD_S_DIAMOND_5,
        GD_S_DIAMOND_6,
        GD_S_DIAMOND_7,
        GD_S_DIAMOND_8,
    };

    /* if the user likes classic sounds */
    if (gd_classic_sound)
        if (!gd_sound_is_classic(sound.sound))
            sound.sound = gd_sound_classic_equivalent(sound.sound);
    /* no sound is possible, as not all sounds have classic equivalent */
    if (sound.sound == GD_S_NONE)
        return;

    /* change diamond falling random to a selected diamond falling sound. */
    if (sound.sound == GD_S_DIAMOND_RANDOM)
        sound.sound = diamond_sounds[g_random_int_range(0, G_N_ELEMENTS(diamond_sounds))];

    /* at this point, fake sounds should have been changed to normal sounds */
    g_assert(!gd_sound_is_fake(sound.sound));

    /* now play it. */
    Mix_PlayChannel(channel, sounds[sound.sound], gd_sound_is_looped(sound.sound) ? -1 : 0);
    Mix_Volume(channel, MIX_MAX_VOLUME * gd_sound_chunks_volume_percent / 100);
    set_channel_panning(channel, sound.dx, sound.dy);
    snd_playing[channel] = sound.sound;
}
#endif


gboolean gd_sound_init(unsigned int bufsize) {
#ifdef HAVE_SDL
    g_assert(!mixer_started);
    mixer_started = false;

    for (unsigned i = 0; i < G_N_ELEMENTS(snd_playing); i++)
        snd_playing[i] = GD_S_NONE;

    // if sound turned off by user, return now
    if (!gd_sound_enabled)
        return TRUE;

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        gd_message(SDL_GetError());
        return FALSE;
    }
    if (Mix_OpenAudio(gd_sound_44khz_mixing ? 44100 : 22050, gd_sound_16bit_mixing ? AUDIO_S16 : AUDIO_U8, 2, bufsize) == -1) {
        gd_message(Mix_GetError());
        return FALSE;
    }
    mixer_started = true;
    /* add callback when a sound stops playing */
    Mix_ChannelFinished(channel_done);

    for (unsigned i = 0; i < GD_S_MAX; i++)
        if (gd_sound_get_filename(GdSound(i)) != NULL)
            loadsound(GdSound(i), gd_sound_get_filename(GdSound(i)));

    return TRUE;
#else
    /* if compiled without sound support, return TRUE, "sound init successful" */
    return TRUE;
#endif
}

void gd_sound_close() {
#ifdef HAVE_SDL
    if (!mixer_started)
        return;

    gd_sound_off();
    Mix_CloseAudio();
    for (unsigned i = 0; i < GD_S_MAX; i++)
        if (sounds[i] != 0) {
            Mix_FreeChunk(sounds[i]);
            sounds[i] = NULL;
        }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    mixer_started = false;
#endif
}

void gd_sound_off() {
#ifdef HAVE_SDL
    if (!mixer_started)
        return;

    /* stop all sounds. */
    for (unsigned i = 0; i < G_N_ELEMENTS(snd_playing); i++)
        halt_channel(i);
#endif
}

void gd_sound_play_bonus_life() {
#ifdef HAVE_SDL
    if (!mixer_started || !gd_sound_enabled)
        return;

    play_sound(gd_sound_get_channel(GD_S_BONUS_LIFE), SoundWithPos(GD_S_BONUS_LIFE, 0, 0));
#endif
}

void gd_sound_play_sounds(SoundWithPos const &sound1, SoundWithPos const &sound2, SoundWithPos const &sound3) {
#ifdef HAVE_SDL
    if (!mixer_started || !gd_sound_enabled)
        return;

    /* CHANNEL 1 is for small sounds */
    if (sound1.sound != GD_S_NONE) {
        /* start new sound */
        play_sound(1, sound1);
    } else {
        /* only interrupt looped sounds. non-looped sounds will go away automatically. */
        if (gd_sound_is_looped(sound_playing(1)))
            halt_channel(1);
    }

    /* CHANNEL 2 is for walking, explosions */
    /* if no sound requested, do nothing. */
    if (sound2.sound != GD_S_NONE) {
        /* always start if not currently playing a sound. */
        /* always start if forced. */
        /* otherwise, start if higher precedence than previous. */
        if (sound_playing(2) == GD_S_NONE
            || gd_sound_force_start(sound2.sound)
            || gd_sound_get_precedence(sound2.sound) > gd_sound_get_precedence(sound_playing(2)))
            play_sound(2, sound2);
    } else {
        /* only interrupt looped sounds. non-looped sounds will go away automatically. */
        if (gd_sound_is_looped(sound_playing(2)))
            halt_channel(2);
    }

    /* CHANNEL 3 is for crack sound, amoeba and magic wall. */
    if (sound3.sound != GD_S_NONE) {
        /* if requests a non-looped sound, play that immediately. that can be a crack sound, gravity change, new life, ... */
        if (!gd_sound_is_looped(sound3.sound))
            play_sound(3, sound3);
        else {
            /* if the sound is looped, play it, but only if != previous one. if they are equal,
               the sound is looped, and already playing, no need to touch it. */
            /* also, do not interrupt the previous sound, if it is non-looped. later calls of this function will probably
               contain the same sound3, and then it will be set. */
            /* but if its looped, set the mixing properties, as those might have changed. */
            if (sound_playing(3) == GD_S_NONE || (sound3.sound != sound_playing(3) && gd_sound_is_looped(sound_playing(3))))
                play_sound(3, sound3);
            else if (sound3.sound == sound_playing(3) && gd_sound_is_looped(sound_playing(3))) {
                set_channel_panning(3, sound3.dx, sound3.dy);
            }
        }
    } else {
        /* sound3=none, so interrupt sound requested. */
        /* only interrupt looped sounds. non-looped sounds will go away automatically. */
        if (gd_sound_is_looped(sound_playing(3)))
            halt_channel(3);
    }
#endif
}


void gd_music_play_random() {
#ifdef HAVE_SDL
    static std::vector<char *> music_filenames;
    static bool music_dir_read = false;

    if (!mixer_started || !gd_sound_enabled)
        return;

    /* if already playing, do nothing */
    if (Mix_PlayingMusic())
        return;

    Mix_FreeMusic(music);
    music = NULL;

    if (!music_dir_read) {
        const char *name;    /* name of file */
        const char *opened;  /* directory we use */
        GDir *musicdir;

        music_dir_read = true;

        opened = gd_system_music_dir.c_str();
        musicdir = g_dir_open(opened, 0, NULL);
        if (!musicdir) {
            /* for testing: open "music" dir in current directory. */
            opened = "music";
            musicdir = g_dir_open(opened, 0, NULL);
        }
        if (!musicdir)
            return;            /* if still cannot open, return now */

        while ((name = g_dir_read_name(musicdir))) {
            if (g_str_has_suffix(name, ".ogg"))
                music_filenames.push_back(g_build_filename(opened, name, NULL));
        }
        g_dir_close(musicdir);
    }

    /* if loaded dir contents, but no file found */
    if (music_filenames.empty())
        return;

    music = Mix_LoadMUS(music_filenames[g_random_int_range(0, music_filenames.size())]);
    if (!music)
        gd_message(SDL_GetError());
    if (music) {
        Mix_VolumeMusic(music_volume);
        Mix_PlayMusic(music, -1);
    }
#endif
}

void gd_music_stop() {
#ifdef HAVE_SDL
    if (!mixer_started)
        return;

    if (Mix_PlayingMusic())
        Mix_FadeOutMusic(120);
#endif
}
