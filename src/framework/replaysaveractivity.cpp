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

/* the replay saver thing only works in the sdl version */
#ifdef HAVE_SDL

#include <SDL2/SDL_mixer.h>
#include <glib/gi18n.h>

#include "framework/replaysaveractivity.hpp"
#include "framework/app.hpp"
#include "framework/commands.hpp"
#include "sdl/IMG_savepng.hpp"
#include "sound/sound.hpp"
#include "cave/gamecontrol.hpp"
#include "cave/titleanimation.hpp"
#include "cave/caveset.hpp"
#include "misc/logger.hpp"
#include "settings.hpp"
#include "sdl/sdlpixbuf.hpp"

class SDLInmemoryPixmap: public Pixmap {
protected:
    std::unique_ptr<SDL_Surface, Deleter<SDL_Surface, SDL_FreeSurface>> surface;

public:
    SDLInmemoryPixmap(SDL_Surface *surface_) : surface(surface_) {}

    virtual int get_width() const override {
        return surface->w;
    }
    virtual int get_height() const override {
        return surface->h;
    }
};


std::unique_ptr<Pixmap> SDLInmemoryScreen::create_pixmap_from_pixbuf(Pixbuf const &pb, bool keep_alpha) const {
    SDL_Surface *to_copy = static_cast<SDLPixbuf const &>(pb).get_surface();
    SDL_Surface *newsurface = SDL_CreateRGBSurface(0, to_copy->w, to_copy->h, 32,
                              surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
    SDL_BlitSurface(to_copy, NULL, newsurface, NULL);
    return std::make_unique<SDLInmemoryPixmap>(newsurface);
}


void SDLInmemoryScreen::set_title(char const *) {
    /* do nothing, not a real window */
}


void SDLInmemoryScreen::configure_size() {
    surface.reset(SDL_CreateRGBSurface(0, w, h, 32, Pixbuf::rmask, Pixbuf::gmask, Pixbuf::bmask, Pixbuf::amask));
}


void SDLInmemoryScreen::save(char const *filename) {
    IMG_SavePNG(filename, surface.get(), 2);        // 2 = not too much compression, but a bit faster than the default
}


Pixbuf const *SDLInmemoryScreen::create_pixbuf_screenshot() const {
    SDL_Surface *sub = SDL_CreateRGBSurfaceFrom(surface->pixels,
                       w, h, 32, surface->pitch, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, 0);
    return new SDLPixbuf(sub);
}


ReplaySaverActivity::ReplaySaverActivity(App *app, CaveStored *cave, CaveReplay *replay, std::string const &filename_prefix)
    :
    Activity(app),
    filename_prefix(filename_prefix),
    game(GameControl::new_replay(app->caveset, cave, replay)),
    pf(),
    pm(pf),
    fm(pm, ""),
    cellrenderer(pm, gd_theme),
    gamerenderer(pm, cellrenderer, fm, *game) {
    int cell_size = cellrenderer.get_cell_size();
    pm.set_size(cell_size * gd_view_width, cell_size * (gd_view_height + 1), false);
    gamerenderer.screen_initialized();
    gamerenderer.set_show_replay_sign(false);
    std::string wav_filename = filename_prefix + ".wav";
    wavfile = fopen(wav_filename.c_str(), "wb");
    if (!wavfile) {
        gd_critical("Cannot open %s for sound output", wav_filename);
        app->enqueue_command(std::make_unique<PopActivityCommand>(app));
        return;
    }
    fseek(wavfile, 44, SEEK_SET);    /* 44bytes offset: start of data in a wav file */
    wavlen = 0;
    frame = 0;
    /* save settings and install own settings */
    saved_gd_show_name_of_game = gd_show_name_of_game;
    gd_show_name_of_game = true;
}


void ReplaySaverActivity::shown_event() {
    std::vector<std::unique_ptr<Pixmap>> animation = get_title_animation_pixmap(app->caveset->title_screen, app->caveset->title_screen_scroll, true, pm, pf);
    pm.blit(*animation[0], 0, 0);
    gd_music_stop();

    /* enable own timer and sound saver */
    install_own_mixer();
}


ReplaySaverActivity::~ReplaySaverActivity() {
    Mix_SetPostMix(NULL, NULL);    /* remove wav file saver */
    uninstall_own_mixer();
    gd_sound_off();

    /* write wav header, as now we now its final size. */
    fseek(wavfile, 0, SEEK_SET);
    Uint32 out32;
    Uint16 out16;

    int i = 0;
    i += fwrite("RIFF", 1, 4, wavfile);  /* "RIFF" */
    out32 = GUINT32_TO_LE(wavlen + 36);
    i += fwrite(&out32, 1, 4, wavfile);  /* 4 + 8+subchunk1size + 8+subchunk2size */
    i += fwrite("WAVE", 1, 4, wavfile);  /* "WAVE" */

    i += fwrite("fmt ", 1, 4, wavfile); /* "fmt " */
    out32 = GUINT32_TO_LE(16);
    i += fwrite(&out32, 1, 4, wavfile); /* fmt chunk size=16 bytes */
    out16 = GUINT16_TO_LE(1);
    i += fwrite(&out16, 1, 2, wavfile); /* 1=pcm */
    out16 = GUINT16_TO_LE(channels);
    i += fwrite(&out16, 1, 2, wavfile);
    out32 = GUINT32_TO_LE(frequency);
    i += fwrite(&out32, 1, 4, wavfile);
    out32 = GUINT32_TO_LE(frequency * bits / 8 * channels);
    i += fwrite(&out32, 1, 4, wavfile); /* byterate */
    out16 = GUINT16_TO_LE(bits / 8 * channels);
    i += fwrite(&out16, 1, 2, wavfile); /* blockalign */
    out16 = GUINT16_TO_LE(bits);
    i += fwrite(&out16, 1, 2, wavfile); /* bitspersample */

    i += fwrite("data", 1, 4, wavfile); /* "data" */
    out32 = GUINT32_TO_LE(wavlen);
    i += fwrite(&out32, 1, 4, wavfile);  /* actual data length */
    fclose(wavfile);

    if (i != 44)
        gd_critical("Could not write wav header to file!");

    std::string message = Printf(_("Saved %d video frames and %dMiB of audio data to %s_*.png and %s.wav."), frame + 1, wavlen / 1048576, filename_prefix, filename_prefix);
    app->show_message(_("Replay Saved"), message);

    // restore settings
    gd_show_name_of_game = saved_gd_show_name_of_game;
    gd_sound_set_music_volume();
    gd_sound_set_chunk_volumes();
    gd_music_play_random();
}


void ReplaySaverActivity::install_own_mixer() {
    gd_sound_close();

    /* we setup mixing and other parameters for our own needs. */
    saved_gd_sdl_sound = gd_sound_enabled;
    saved_gd_sdl_44khz_mixing = gd_sound_44khz_mixing;
    saved_gd_sdl_16bit_mixing = gd_sound_16bit_mixing;
    saved_gd_sound_stereo = gd_sound_stereo;
    const char *driver = g_getenv("SDL_AUDIODRIVER");
    if (driver)
        saved_driver = driver;

    /* and after tweaking settings to our own need, we can init sdl. */
    gd_sound_enabled = true;
    gd_sound_44khz_mixing = true;
    gd_sound_16bit_mixing = true;
    gd_sound_stereo = true;
    // select the dummy driver, as it accepts any format, and we have control
    g_setenv("SDL_AUDIODRIVER", "dummy", TRUE);
    gd_sound_init(44100 / 25);  /* so the buffer will be 1/25th of a second. and we get our favourite 25hz interrupt from the mixer. */

    /* query audio format from sdl */
    Uint16 format;
    Mix_QuerySpec(&frequency, &format, &channels);
    if (frequency != 44100)      /* something must be really going wrong. */
        gd_critical("Cannot initialize mixer to 44100Hz mixing. The replay saver will not work correctly!");

    switch (format) {
        case AUDIO_U8:
        case AUDIO_S8:
            bits = 8;
            break;
        case AUDIO_U16LSB:
        case AUDIO_S16LSB:
        case AUDIO_U16MSB:
        case AUDIO_S16MSB:
            bits = 16;
            break;
        default:
            g_assert_not_reached();
    }
    Mix_SetPostMix(mixfunc, this);
}


void ReplaySaverActivity::uninstall_own_mixer() {
    gd_sound_close();

    // restore settings
    gd_sound_enabled = saved_gd_sdl_sound;
    gd_sound_44khz_mixing = saved_gd_sdl_44khz_mixing;
    gd_sound_16bit_mixing = saved_gd_sdl_16bit_mixing;
    gd_sound_stereo = saved_gd_sound_stereo;

    if (saved_driver != "")
        g_setenv("SDL_AUDIODRIVER", saved_driver.c_str(), TRUE);
    else
        g_unsetenv("SDL_AUDIODRIVER");

    gd_sound_init();
}


/* this function saves the wav file, and also does the timing! */
void ReplaySaverActivity::mixfunc(void *udata, Uint8 *stream, int len) {
    ReplaySaverActivity *rs = static_cast<ReplaySaverActivity *>(udata);

    if (!rs->wavfile)
        return;
    if (fwrite(stream, 1, len, rs->wavfile) != size_t(len))
        gd_critical("Cannot write to wav file!");

    // push a user event -> to do the timing
    SDL_Event ev;
    ev.type = SDL_USEREVENT + 1;
    SDL_PushEvent(&ev);

    rs->wavlen += len;
}

#include <typeinfo>

void ReplaySaverActivity::redraw_event(bool full) const {
    app->clear_screen();

    // TRANSLATORS: Title line capitalization in English
    app->title_line(_("Saving Replay"));
    app->status_line(_("Please wait"));

    // show it to the user.
    // it is not in displayformat, but a small image - not that slow to draw.
    // center coordinates
    int x = (app->screen->get_width() - pm.get_width()) / 2;
    int y = (app->screen->get_height() - pm.get_height()) / 2;
    Pixbuf const *pb = pm.create_pixbuf_screenshot();
    app->screen->blit_pixbuf(*pb, x, y, false);
    delete pb;

    app->screen->drawing_finished();
}


void ReplaySaverActivity::timer2_event() {
    /* iterate and see what happened */
    /* give no gameinputhandler to the renderer */
    GameRenderer::State state = gamerenderer.main_int(40, false, NULL);
    gamerenderer.draw(pm.must_redraw_all_before_flip());
    pm.do_the_flip();
    switch (state) {
        case GameRenderer::Nothing:
            break;

        case GameRenderer::Stop:        /* game stopped, this could be a replay or a snapshot */
        case GameRenderer::GameOver:    /* game over should not happen for a replay, but no problem */
            app->enqueue_command(std::make_unique<PopActivityCommand>(app));
            break;
    }

    /* before incrementing frame number, check if to save the frame to disk. */
    pm.save(Printf("%s_%08d.png", filename_prefix, frame).c_str());

    queue_redraw();
    frame++;
}

#endif /* IFDEF HAVE_SDL */
