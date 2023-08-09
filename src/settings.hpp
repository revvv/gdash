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
#ifndef SETTINGS_HPP_INCLUDED
#define SETTINGS_HPP_INCLUDED

#include "config.h"

#include <glib.h>
#include <string>
#include <vector>

class GameInputHandler;

/* universal settings */
extern int gd_language;
extern std::string gd_username;
extern std::string gd_theme;
extern bool gd_no_invisible_outbox;
extern bool gd_all_caves_selectable;
extern bool gd_import_as_all_caves_selectable;
extern bool gd_use_bdcff_highscore;
extern int gd_pal_emu_scanline_shade;
extern bool gd_fine_scroll;
extern bool gd_particle_effects;
extern bool gd_full_cave_view;
extern bool gd_show_story;
extern bool gd_show_name_of_game;
extern int gd_status_bar_colors;
extern bool gd_alternate_vertical_animation;

/* palette settings */
extern int gd_c64_palette;
extern int gd_c64dtv_palette;
extern int gd_atari_palette;
extern int gd_preferred_palette;


/* GTK settings */

/* editor settings */
extern bool gd_game_view;    /* show animated cells instead of arrows & ... */
extern bool gd_colored_objects;    /* show objects with different color */
extern bool gd_show_object_list;    /* show object list */
extern int gd_editor_window_width;    /* window size */
extern int gd_editor_window_height;    /* window size */
extern bool gd_fast_uncover_in_test;

/* preferences */
extern bool gd_show_preview;

/* graphics */
extern int gd_graphics_engine;
extern bool gd_fullscreen;
extern double gd_cell_scale_factor_game;
extern int gd_cell_scale_type_game;
extern bool gd_pal_emulation_game;
extern double gd_cell_scale_factor_editor;
extern int gd_cell_scale_type_editor;
extern bool gd_pal_emulation_editor;

/* keyboard */
#ifdef HAVE_GTK
extern int gd_gtk_key_left;
extern int gd_gtk_key_right;
extern int gd_gtk_key_up;
extern int gd_gtk_key_down;
extern int gd_gtk_key_fire_1;
extern int gd_gtk_key_fire_2;
extern int gd_gtk_key_suicide;
extern int gd_gtk_key_fast_forward;
extern int gd_gtk_key_status_bar;
extern int gd_gtk_key_restart_level;
#endif

/* html output option */
extern char *gd_html_stylesheet_filename;
extern char *gd_html_favicon_filename;



/* SDL settings */

#ifdef HAVE_SDL
extern int gd_sdl_key_left;
extern int gd_sdl_key_right;
extern int gd_sdl_key_up;
extern int gd_sdl_key_down;
extern int gd_sdl_key_fire_1;
extern int gd_sdl_key_fire_2;
extern int gd_sdl_key_suicide;
extern int gd_sdl_key_fast_forward;
extern int gd_sdl_key_status_bar;
extern int gd_sdl_key_restart_level;
#endif


/* SOUND settings */

#ifdef HAVE_SDL
extern bool gd_sound_enabled;
extern bool gd_sound_16bit_mixing;
extern bool gd_sound_44khz_mixing;
extern bool gd_sound_stereo;
extern bool gd_classic_sound;
extern int gd_sound_chunks_volume_percent;
extern int gd_sound_music_volume_percent;

extern std::string gd_shader;
extern int shader_pal_radial_distortion;
extern int shader_pal_chroma_to_luma_strength;
extern int shader_pal_luma_to_chroma_strength;
extern int shader_pal_scanline_shade_luma;
extern int shader_pal_phosphor_shade;
extern int shader_pal_random_scanline_displace;
extern int shader_pal_random_y;
extern int shader_pal_random_uv;
extern int shader_pal_luma_x_blur;
extern int shader_pal_chroma_x_blur;
extern int shader_pal_chroma_y_blur;
#endif    /* if gd_sound */


/* command line parameters */
extern int gd_param_license;
extern char **gd_param_cavenames;
extern gboolean gd_param_debug;
extern gboolean gd_param_load_default_settings;

/* gdash directories */
extern std::string gd_user_config_dir;
extern std::string gd_system_data_dir;
extern std::string gd_system_caves_dir;
extern std::string gd_system_music_dir;

extern std::vector<std::string> gd_sound_dirs, gd_themes_dirs, gd_fonts_dirs, gd_shaders_dirs;

extern const char *gd_languages_names[];

/* init settings (directories), and load language files */
void gd_settings_init();
void gd_settings_init_dirs();
void gd_settings_set_locale();
void gd_settings_init_translation();

/* settings loading and saving */
void gd_save_settings();
void gd_load_settings();

/* command line arguments parsing */
GOptionContext *gd_option_context_new();



/* for settings */
enum SettingType {
    TypePage,
    TypeBoolean,
    TypeInteger,
    TypeTheme,
    TypeShader,
    TypePercent,
    TypeStringv,
    TypeKey,
    TypeDouble,
};
class Setting {
public:
    SettingType type;
    const char *name;
    void *var;
    bool restart;     // a setting which requires a restart
    const char **stringv;
    char const *description;
    int min, max;     // for integers
    std::string *stringvar; // for the theme and the scaler

    unsigned page;    // generated inside the game
};

Setting *gd_get_game_settings_array();
Setting *gd_get_keyboard_settings_array(GameInputHandler *gih);
void gd_settings_array_prepare(Setting *settings, SettingType which,
                               std::vector<std::string> const & strings, int *var);
void gd_settings_array_unprepare(Setting *settings, SettingType which);

#endif
