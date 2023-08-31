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

#include <glib/gi18n.h>
#include <glib.h>
#include <map>

#include "misc/logger.hpp"
#include "misc/autogfreeptr.hpp"
#include "cave/colors.hpp"
#include "misc/util.hpp"
#include "cave/gamerender.hpp"
#include "gfx/pixbuffactory.hpp"
#include "input/gameinputhandler.hpp"
#include "mainwindow.hpp"

#include "settings.hpp"

#ifdef HAVE_SDL
#include <SDL.h>
#endif

#ifdef HAVE_GTK
#include <gdk/gdkkeysyms.h>
#endif


#define SETTINGS_INI_FILE "gdash.ini"
#define SETTINGS_GDASH_GROUP "GDash"

/* possible languages. */
/* they are not translated, so every language name is show in that language itself. */
const char *gd_languages_names[] = {N_("System default"), "English", "Deutsch", "Magyar", NULL};
/* this should correspond to the above one. */
static const char *languages_for_env[] = { NULL, "en", "de", "hu" };

#ifdef G_OS_WIN32
/* locale names used in windows. */
static const char *language_locale_default[] = { "", NULL, };
static const char *language_locale_en[] = { "English", NULL, };
static const char *language_locale_de[] = { "German", NULL, };
static const char *language_locale_hu[] = { "Hungarian", NULL, };
#else
/* locale names used in unix. */
/* anyone, a better solution for this? */
static const char *language_locale_default[] =
{ "", NULL, };
static const char *language_locale_en[] =
{ "en_US.UTF-8", "en_US.UTF8", "en_US.ISO8859-15", "en_US.ISO8859-1", "en_US.US-ASCII", "en_US", "en", NULL, };
static const char *language_locale_de[] = {
    "de_DE.UTF-8", "de_DE.UTF8", "de_DE.ISO8859-15", "de_DE.ISO8859-1", "de_DE",
    "de_AT.UTF-8", "de_AT.UTF8", "de_AT.ISO8859-15", "de_AT.ISO8859-1", "de_AT",
    "de_CH.UTF-8", "de_CH.UTF8", "de_CH.ISO8859-15", "de_CH.ISO8859-1", "de_CH",
    "de", NULL,
};
static const char *language_locale_hu[] =
{ "hu_HU.UTF-8", "hu_HU.UTF8", "hu_HU.ISO8859-2", "hu_HU", "hu", NULL, };
#endif    /* ifdef g_os_win32 else */

/* put the locales to be tried in an array - same for windows and unix */
static const char **language_locale[] = {
    language_locale_default,
    language_locale_en,
    language_locale_de,
    language_locale_hu,
};


static std::map<char const *, int *> settings_integers;
static std::map<char const *, bool *> settings_bools;
static std::map<char const *, std::string *> settings_strings;
static std::map<char const *, double *> settings_doubles;


/* universal settings */
std::string gd_username;
std::string gd_theme;
bool gd_no_invisible_outbox = false;
bool gd_all_caves_selectable = false;
bool gd_import_as_all_caves_selectable = false;
bool gd_use_bdcff_highscore = false;
int gd_pal_emu_scanline_shade = 80;
bool gd_fine_scroll = true;
bool gd_particle_effects = true;
bool gd_full_cave_view = false;
double gd_full_cave_scaling_factor = 2.0;
bool gd_open_gl_center;
bool gd_show_story = true;
bool gd_show_name_of_game = true;
int gd_status_bar_colors = GD_STATUS_BAR_ORIGINAL;
bool gd_alternate_vertical_animation = true;

/* palette settings */
int gd_c64_palette = 0;
int gd_c64dtv_palette = 0;
int gd_atari_palette = 0;
int gd_preferred_palette = GdColor::TypeRGB;
/* editor settings */
bool gd_game_view = true;  /* show animated cells instead of arrows & ... */
bool gd_colored_objects = true;  /* show objects with different color */
bool gd_show_object_list = true;  /* show object list */
int gd_editor_window_width = 800;  /* window size */
int gd_editor_window_height = 520;  /* window size */
bool gd_fast_uncover_in_test = true;

/* preferences */
int gd_language = 0;
bool gd_show_preview = true;

/* graphics */
int gd_graphics_engine = 0; /* whichever is the first one supported */
bool gd_fullscreen = false;
double gd_cell_scale_factor_game = 2.0;
int gd_cell_scale_type_game = GD_SCALING_NEAREST;
bool gd_pal_emulation_game = false;
double gd_cell_scale_factor_editor = 1.0;
int gd_cell_scale_type_editor = GD_SCALING_NEAREST;
bool gd_pal_emulation_editor = false;

/* html output option */
/* CURRENTLY ONLY FROM THE COMMAND LINE */
char *gd_html_stylesheet_filename = NULL;
char *gd_html_favicon_filename = NULL;

/* GTK keyboard settings */
#ifdef HAVE_GTK    /* only if having gtk */
int gd_gtk_key_left = GDK_KEY_Left;
int gd_gtk_key_right = GDK_KEY_Right;
int gd_gtk_key_up = GDK_KEY_Up;
int gd_gtk_key_down = GDK_KEY_Down;
int gd_gtk_key_fire_1 = GDK_KEY_Control_L;
int gd_gtk_key_fire_2 = GDK_KEY_Control_R;
int gd_gtk_key_suicide = GDK_KEY_s;
int gd_gtk_key_fast_forward = GDK_KEY_f;
int gd_gtk_key_status_bar = GDK_KEY_Shift_L;
int gd_gtk_key_restart_level = GDK_KEY_Escape;
#endif    /* only if having gtk */

/* SDL settings */
#ifdef HAVE_SDL
int gd_sdl_key_left = SDLK_LEFT;
int gd_sdl_key_right = SDLK_RIGHT;
int gd_sdl_key_up = SDLK_UP;
int gd_sdl_key_down = SDLK_DOWN;
int gd_sdl_key_fire_1 = SDLK_LCTRL;
int gd_sdl_key_fire_2 = SDLK_RCTRL;
int gd_sdl_key_suicide = SDLK_s;
int gd_sdl_key_fast_forward = SDLK_f;
int gd_sdl_key_status_bar = SDLK_LSHIFT;
int gd_sdl_key_restart_level = SDLK_ESCAPE;
std::string gd_shader;
int shader_pal_radial_distortion = 10;
int shader_pal_random_scanline_displace = 50;
int shader_pal_luma_x_blur = 40;
int shader_pal_chroma_x_blur = 50;
int shader_pal_chroma_y_blur = 50;
int shader_pal_chroma_to_luma_strength = 15;
int shader_pal_luma_to_chroma_strength = 15;
int shader_pal_random_y = 5;
int shader_pal_random_uv = 10;
int shader_pal_scanline_shade_luma = 90;
int shader_pal_phosphor_shade = 90;
#endif    /* use_sdl */

/* sound settings */
#ifdef HAVE_SDL
bool gd_sound_enabled = true;
bool gd_sound_16bit_mixing = true;
bool gd_sound_44khz_mixing = true;
bool gd_sound_stereo = true;
bool gd_classic_sound = false;
int gd_sound_chunks_volume_percent = 50;
int gd_sound_music_volume_percent = 50;
#endif    /* if gd_sound */


/* some directories the game uses */
std::string gd_user_config_dir;
std::string gd_system_data_dir;
std::string gd_system_caves_dir;
std::string gd_system_music_dir;

std::vector<std::string> gd_sound_dirs, gd_themes_dirs, gd_fonts_dirs, gd_shaders_dirs;

/* command line parameters */
int gd_param_license = 0;
char **gd_param_cavenames = NULL;
gboolean gd_param_debug = FALSE;
gboolean gd_param_load_default_settings = FALSE;



static void set_page_numbers(Setting *settings) {
    int page = -1;
    for (size_t i = 0; settings[i].name != NULL; ++i) {
        if (settings[i].type == TypePage)
            page++;
        settings[i].page = page;
    }
}


Setting *gd_get_game_settings_array() {
    static Setting settings_static[] = {
        { TypePage, N_("Game") },
        { TypeStringv, N_("Language"), &gd_language, false, gd_languages_names, N_("The language of the application. Changing this setting requires a restart!") },
        { TypeBoolean, N_("All caves selectable"), &gd_all_caves_selectable, false, NULL, N_("All caves and intermissions can be selected at game start.") },
        { TypeBoolean, N_("Import as all selectable"), &gd_import_as_all_caves_selectable, false, NULL, N_("Original, C64 games are imported not with A, E, I, M caves selectable, but all caves (ABCD, EFGH... excluding intermissions). This does not affect BDCFF caves.") },
        { TypeBoolean, N_("Use BDCFF highscore"), &gd_use_bdcff_highscore, false, NULL, N_("Use BDCFF highscores. GDash saves highscores in its own configuration directory and also in the *.bd files. However, it prefers loading them from the configuration directory; as the *.bd files might be read-only. You can enable this setting to let GDash load them from the *.bd files.") },
        { TypeBoolean, N_("Show story"), &gd_show_story, false, NULL, N_("If the cave has a story, it will be shown when the cave is first started.") },
        { TypeBoolean, N_("Game name at uncover"), &gd_show_name_of_game, false, NULL, N_("Show the name of the game when uncovering a cave.") },
        { TypeBoolean, N_("No invisible outbox"), &gd_no_invisible_outbox, false, NULL, N_("Show invisible outboxes as visible (blinking) ones.") },

        { TypePage, N_("Theme and colors") },
        { TypeTheme,   N_("Theme"), NULL, false, NULL, N_("Graphics theme used inside the game."), 0, 0, NULL },
        { TypeStringv, N_("Status bar colors"), &gd_status_bar_colors, false, gd_status_bar_colors_get_names(), N_("Preferred status bar color scheme. Only affects the colors, not the status bar layout.") },
        { TypeStringv, N_("  C64 palette"), &gd_c64_palette, false, GdColor::get_c64_palette_names(), N_("The color palette for games imported from C64 files.") },
        { TypeStringv, N_("  C64DTV palette"), &gd_c64dtv_palette, false, GdColor::get_c64dtv_palette_names(), N_("The color palette for imported C64 DTV games.") },
        { TypeStringv, N_("  Atari palette"), &gd_atari_palette, false, GdColor::get_atari_palette_names(), N_("The color palette for imported Atari games.") },
        { TypeStringv, N_("  Preferred palette"), &gd_preferred_palette, false, GdColor::get_palette_types_names(), N_("New caves and random colored caves use this palette.") },
        { TypeBoolean, N_("Alternate vertical animation"), &gd_alternate_vertical_animation, true, NULL, N_("Alternate vertical animation for the player.") },

        { TypePage, N_("Game graphics") },
        // TRANSLATORS: here "engine" = "graphics engine"
        { TypeStringv, N_("Engine"), &gd_graphics_engine, true, gd_graphics_engine_names, N_("Graphics engine which used for drawing.") },
        { TypeDouble, N_("Scaling factor"), &gd_cell_scale_factor_game, true, NULL, N_("Scaling size."), 1, 8 },
        { TypeStringv, N_("  Scaling type"), &gd_cell_scale_type_game, true, gd_scaling_names, N_("Software scaling method used. This setting is only effective for the GTK+ and the SDL engines. If you use the OpenGL engine, you can configure its scaling method by selecting a shader.") },
        { TypeBoolean, N_("  Software PAL emu"), &gd_pal_emulation_game, true, NULL, N_("Use PAL emulated graphics, i.e. lines are striped, and colors are distorted like on a TV. Only effective for the GTK+ and the SDL engines.") },
        { TypePercent, N_("  PAL scanline shade"), &gd_pal_emu_scanline_shade, true, NULL, N_("Darker rows for PAL emulation. Only effective for the GTK+ and the SDL engines.") },
        { TypeBoolean, N_("Fine scrolling"), &gd_fine_scroll, true, NULL, N_("If fine scrolling is turned off, scrolling and cave animation is limited to a lower frame rate, and consumes much less CPU. On some hardware, it might actually look better than fine scrolling. Not all graphics engines support fine scrolling.") },
        { TypeBoolean, N_("Particle effects"), &gd_particle_effects, true, NULL, N_("Particle effects during play. This requires a lot of CPU power.") },
        { TypeBoolean, N_("Full cave view"), &gd_full_cave_view, true, NULL, N_("Show the whole cave on the screen without scrolling.") },
        { TypeBoolean, N_("OpenGL: Center full screen"), &gd_open_gl_center, true, NULL, N_("If this option is activated, the display fills the entire screen under OpenGL. To preserve the aspect ratio it is recommended to increase the scaling factor.") },

#ifdef HAVE_SDL
        { TypePage, N_("OpenGL settings") },
        { TypeShader,  N_("Shader"), NULL, true, NULL, N_("The shader which adds a graphical effect to the screen. Only effective if the OpenGL engine is used. If your video card is older, it might not support shaders. The GDash TV shader can be configured with the settings below."), 0, 0, &gd_shader },
        { TypePercent, N_("Radial distortion"), &shader_pal_radial_distortion, false, NULL, N_("With radial distortion, the screen won't be flat, but it will look like as if it's projected on a sphere.") },
        { TypePercent, N_("Random scanline displacement"), &shader_pal_random_scanline_displace, false, NULL, N_("Increasing this setting causes the the image to be unstable horizontally.") },
        { TypePercent, N_("Luma X blur"), &shader_pal_luma_x_blur, false, NULL, N_("Horizontal blur of luminosity.") },
        { TypePercent, N_("Chroma X blur"), &shader_pal_chroma_x_blur, false, NULL, N_("Slight horizontal blur of colors.") },
        { TypePercent, N_("Chroma Y blur"), &shader_pal_chroma_y_blur, false, NULL, N_("Vertical blur of colors, to imitate a PAL TV signal.") },
        { TypePercent, N_("Chroma to luma"), &shader_pal_chroma_to_luma_strength, false, NULL, N_("Sudden changes in colors can cause patterns to appear on the image.") },
        { TypePercent, N_("Luma to chroma"), &shader_pal_luma_to_chroma_strength, false, NULL, N_("Sudden changes in luminosity can cause false colors to appear on the image.") },
        { TypePercent, N_("Random Y"), &shader_pal_random_y, false, NULL, N_("Noise level of luminosity.") },
        { TypePercent, N_("Random UV"), &shader_pal_random_uv, false, NULL, N_("Noise level of colors.") },
        { TypePercent, N_("Scanline shade"), &shader_pal_scanline_shade_luma, false, NULL, N_("Darkened horizontal rows to emulate a TV screen.") },
        { TypePercent, N_("Phosphor shade"), &shader_pal_phosphor_shade, false, NULL, N_("Red, green and blue subpixels of a TV screen can be emulated.") },
#endif        

#ifdef HAVE_GTK
        { TypePage, N_("Editor settings") },
        { TypeDouble, N_("Scaling factor"), &gd_cell_scale_factor_editor, true, NULL, N_("Scaling size."), 1, 8 },
        { TypeStringv, N_("  Scaling type"), &gd_cell_scale_type_editor, true, gd_scaling_names, N_("Scaling method.") },
        { TypeBoolean, N_("  Software PAL emu"), &gd_pal_emulation_editor, true, NULL, N_("Use PAL emulated graphics, i.e. lines are striped, and colors are distorted like on a TV.") },
        { TypeBoolean, N_("Animated view"), &gd_game_view, true, NULL, N_("Show simplified view of cave in the editor.") },
        { TypeBoolean, N_("Colored objects"), &gd_colored_objects, true, NULL, N_("Cave objects are colored, to make them different from random cave elements.") },
        { TypeBoolean, N_("Object list"), &gd_show_object_list, true, NULL, N_("Show objects list sidebar in the editor.") },
        { TypeBoolean, N_("Fast uncover in test"), &gd_fast_uncover_in_test, false, NULL, N_("Fast uncovering and covering of cave in editor cave test, to reduce waiting time.") },
#endif

#ifdef HAVE_SDL
        { TypePage, N_("Sound") },
        { TypeBoolean, N_("Sound"), &gd_sound_enabled, true, NULL, N_("Play sounds and music in the program.") },
        { TypeBoolean, N_("Classic sounds only"), &gd_classic_sound, true, NULL, N_("Play only classic sounds taken from the original game.") },
        { TypeBoolean, N_("Stereo sounds"), &gd_sound_stereo, true, NULL, N_("If you enable stereo sounds, you will hear the direction of sounds in the caves.") },
        { TypeBoolean, N_("16-bit mixing"), &gd_sound_16bit_mixing, true, NULL, N_("Use 16-bit mixing of sounds. Try changing this setting if sound is clicky.") },
        { TypeBoolean, N_("44kHz mixing"), &gd_sound_44khz_mixing, true, NULL, N_("Use 44kHz mixing of sounds. Try changing this setting if sound is clicky.") },
#endif

        /* end */
        { TypeBoolean, NULL },
    };

    set_page_numbers(settings_static);

    return settings_static;
}



Setting *gtk_get_keyboard_settings_array() {

    // append "GTK" to "Keyboard"
    static std::string keyboard_engine = Printf("%s (%s)", _("Keyboard"), "GTK");

    static Setting settings_static[] = {
        //{ TypePage, N_("Keyboard") }, // without engine
        { TypePage, keyboard_engine.c_str() },
        { TypeKey,     N_("Key left"), &gd_gtk_key_left, false },
        { TypeKey,     N_("Key right"), &gd_gtk_key_right, false },
        { TypeKey,     N_("Key up"), &gd_gtk_key_up, false },
        { TypeKey,     N_("Key down"), &gd_gtk_key_down, false },
        { TypeKey,     N_("Key snap"), &gd_gtk_key_fire_1, false },
        { TypeKey,     N_("Key snap (alt.)"), &gd_gtk_key_fire_2, false },
        { TypeKey,     N_("Key suicide"), &gd_gtk_key_suicide, false },
        { TypeKey,     N_("Key fast forward"), &gd_gtk_key_fast_forward, false },
        { TypeKey,     N_("Key status bar"), &gd_gtk_key_status_bar, false },
        { TypeKey,     N_("Key restart level"), &gd_gtk_key_restart_level, false },
        /* end */
        { TypeBoolean, NULL },
    };

    set_page_numbers(settings_static);

    return settings_static;
}


Setting *sdl_get_keyboard_settings_array() {

    // append "SDL" to "Keyboard"
    static std::string keyboard_engine = Printf("%s (%s)", _("Keyboard"), "SDL");

    static Setting settings_static[] = {
        //{ TypePage, N_("Keyboard") }, // without engine
        { TypePage, keyboard_engine.c_str() },
        { TypeKey,     N_("Key left"), &gd_sdl_key_left, false },
        { TypeKey,     N_("Key right"), &gd_sdl_key_right, false },
        { TypeKey,     N_("Key up"), &gd_sdl_key_up, false },
        { TypeKey,     N_("Key down"), &gd_sdl_key_down, false },
        { TypeKey,     N_("Key snap"), &gd_sdl_key_fire_1, false },
        { TypeKey,     N_("Key snap (alt.)"), &gd_sdl_key_fire_2, false },
        { TypeKey,     N_("Key suicide"), &gd_sdl_key_suicide, false },
        { TypeKey,     N_("Key fast forward"), &gd_sdl_key_fast_forward, false },
        { TypeKey,     N_("Key status bar"), &gd_sdl_key_status_bar, false },
        { TypeKey,     N_("Key restart level"), &gd_sdl_key_restart_level, false },
        /* end */
        { TypeBoolean, NULL },
    };

    set_page_numbers(settings_static);

    return settings_static;
}


Setting *gd_get_keyboard_settings_array(GameInputHandler *gih) {
    // WARNING: gih might contain the wrong handler, therefore we don't use it here
    if (gd_graphics_engine == GRAPHICS_ENGINE_GTK)
        return gtk_get_keyboard_settings_array();
    else
        return sdl_get_keyboard_settings_array();
}


static char *displayname(char const *filename) {
    if (g_str_equal(filename, "")) {
        // TRANSLATORS: Default (builtin) theme, shader, etc.
        return g_strdup(_("[Default]"));
    }
    char *disp = g_filename_display_basename(filename);
    if (strrchr(disp, '.'))    /* remove extension */
        *strrchr(disp, '.') = '\0';
    return disp;
}


void gd_settings_array_prepare(Setting *settings, SettingType which,
                               std::vector<std::string> const & strings, int *var) {
    for (unsigned i = 0; settings[i].name != NULL; ++i) {
        if (settings[i].type == which) {
            settings[i].var = var;
            char **stringv = g_new(char *, strings.size()+1);
            for (unsigned j = 0; j != strings.size(); ++j)
                stringv[j] = displayname(strings[j].c_str());
            stringv[strings.size()] = NULL;
            settings[i].stringv = (char const **) stringv;
        }
    }
}


void gd_settings_array_unprepare(Setting *settings, SettingType which) {
    for (unsigned i = 0; settings[i].name != NULL; ++i) {
        if (settings[i].type == which) {
            settings[i].var = NULL;
            g_strfreev((char **) settings[i].stringv);
            settings[i].stringv = NULL;
        }
    }
}


/* gets boolean value from key file; returns def if not found or unreadable */
static bool keyfile_get_bool_with_default(GKeyFile *keyfile, const char *group, const char *key, bool def) {
    GError *error = NULL;
    gboolean result = g_key_file_get_boolean(keyfile, group, key, &error);
    if (!error)
        return result != FALSE;
    gd_debug(error->message);
    g_error_free(error);
    return def;
}

/* gets integer value from key file; returns def if not found or unreadable */
static int keyfile_get_integer_with_default(GKeyFile *keyfile, const char *group, const char *key, int def) {
    GError *error = NULL;
    int result = g_key_file_get_integer(keyfile, group, key, &error);
    if (!error)
        return result;
    gd_debug(error->message);
    g_error_free(error);
    return def;
}

/* gets double value from key file; returns def if not found or unreadable */
static double keyfile_get_double_with_default(GKeyFile *keyfile, const char *group, const char *key, double def) {
    GError *error = NULL;
    double result = g_key_file_get_double(keyfile, group, key, &error);
    if (!error)
        return result;
    gd_debug(error->message);
    g_error_free(error);
    return def;
}

static std::string keyfile_get_string(GKeyFile *keyfile, const char *group, const char *key) {
    if (!g_key_file_has_key(keyfile, group, key, NULL))
        return "";

    GError *error = NULL;
    AutoGFreePtr<char> result(g_key_file_get_string(keyfile, group, key, &error));
    if (result != NULL)
        return std::string(result);
    gd_debug(error->message);
    g_error_free(error);
    return "";
}


void gd_settings_init() {
    gd_username = g_get_real_name();
    settings_bools["no_invisible_outbox"] = &gd_no_invisible_outbox;
    settings_bools["all_caves_selectable"] = &gd_all_caves_selectable;
    settings_bools["import_as_all_caves_selectable"] = &gd_import_as_all_caves_selectable;
    settings_bools["use_bdcff_highscore"] = &gd_use_bdcff_highscore;
    settings_bools["fine_scroll"] = &gd_fine_scroll;
    settings_bools["particle_effects"] = &gd_particle_effects;
    settings_bools["full_cave_view"] = &gd_full_cave_view;
    settings_doubles["full_cave_scaling_factor"] = &gd_full_cave_scaling_factor;
    settings_bools["open_gl_center"] = &gd_open_gl_center;
    settings_bools["show_story"] = &gd_show_story;
    settings_bools["show_name_of_game"] = &gd_show_name_of_game;
    settings_integers["pal_emu_scanline_shade"] = &gd_pal_emu_scanline_shade;
    settings_integers["status_bar_colors"] = &gd_status_bar_colors;
    settings_integers["c64_palette"] = &gd_c64_palette;
    settings_integers["c64dtv_palette"] = &gd_c64dtv_palette;
    settings_integers["atari_palette"] = &gd_atari_palette;
    settings_integers["preferred_palette"] = &gd_preferred_palette;
    settings_strings["username"] = &gd_username;
    settings_strings["theme"] = &gd_theme;
    settings_bools["alternate_vertical_animation"] = &gd_alternate_vertical_animation;

    settings_bools["game_view"] = &gd_game_view;
    settings_bools["colored_objects"] = &gd_colored_objects;
    settings_bools["show_object_list"] = &gd_show_object_list;
    settings_bools["show_preview"] = &gd_show_preview;
    settings_bools["pal_emulation_game"] = &gd_pal_emulation_game;
    settings_bools["pal_emulation_editor"] = &gd_pal_emulation_editor;
    settings_bools["fast_uncover_in_test"] = &gd_fast_uncover_in_test;
    settings_integers["editor_window_width"] = &gd_editor_window_width;
    settings_integers["editor_window_height"] = &gd_editor_window_height;
    settings_integers["language"] = &gd_language;
    settings_bools["fullscreen"] = &gd_fullscreen;
    settings_integers["graphics_engine"] = &gd_graphics_engine;
    settings_doubles["cell_scale_factor_game"] = &gd_cell_scale_factor_game;
    settings_integers["cell_scale_type_game"] = &gd_cell_scale_type_game;
    settings_doubles["cell_scale_factor_editor"] = &gd_cell_scale_factor_editor;
    settings_integers["cell_scale_type_editor"] = &gd_cell_scale_type_editor;

#ifdef HAVE_GTK
    settings_integers["gtk_key_left"] = &gd_gtk_key_left;
    settings_integers["gtk_key_right"] = &gd_gtk_key_right;
    settings_integers["gtk_key_up"] = &gd_gtk_key_up;
    settings_integers["gtk_key_down"] = &gd_gtk_key_down;
    settings_integers["gtk_key_fire_1"] = &gd_gtk_key_fire_1;
    settings_integers["gtk_key_fire_2"] = &gd_gtk_key_fire_2;
    settings_integers["gtk_key_suicide"] = &gd_gtk_key_suicide;
    settings_integers["gtk_key_fast_forward"] = &gd_gtk_key_fast_forward;
    settings_integers["gtk_key_status_bar"] = &gd_gtk_key_status_bar;
    settings_integers["gtk_key_restart_level"] = &gd_gtk_key_restart_level;
#endif

#ifdef HAVE_SDL    /* only if having sdl */
    settings_integers["sdl2_key_left"] = &gd_sdl_key_left;
    settings_integers["sdl2_key_right"] = &gd_sdl_key_right;
    settings_integers["sdl2_key_up"] = &gd_sdl_key_up;
    settings_integers["sdl2_key_down"] = &gd_sdl_key_down;
    settings_integers["sdl2_key_fire_1"] = &gd_sdl_key_fire_1;
    settings_integers["sdl2_key_fire_2"] = &gd_sdl_key_fire_2;
    settings_integers["sdl2_key_suicide"] = &gd_sdl_key_suicide;
    settings_integers["sdl2_key_fast_forward"] = &gd_sdl_key_fast_forward;
    settings_integers["sdl2_key_status_bar"] = &gd_sdl_key_status_bar;
    settings_integers["sdl2_key_restart_level"] = &gd_sdl_key_restart_level;
    settings_strings["shader"] = &gd_shader;

    settings_integers["shader_pal_radial_distortion"] = &shader_pal_radial_distortion;
    settings_integers["shader_pal_random_scanline_displace"] = &shader_pal_random_scanline_displace;
    settings_integers["shader_pal_luma_x_blur"] = &shader_pal_luma_x_blur;
    settings_integers["shader_pal_chroma_x_blur"] = &shader_pal_chroma_x_blur;
    settings_integers["shader_pal_chroma_y_blur"] = &shader_pal_chroma_y_blur;
    settings_integers["shader_pal_chroma_to_luma_strength"] = &shader_pal_chroma_to_luma_strength;
    settings_integers["shader_pal_luma_to_chroma_strength"] = &shader_pal_luma_to_chroma_strength;
    settings_integers["shader_pal_random_y"] = &shader_pal_random_y;
    settings_integers["shader_pal_random_uv"] = &shader_pal_random_uv;
    settings_integers["shader_pal_scanline_shade_luma"] = &shader_pal_scanline_shade_luma;
    settings_integers["shader_pal_phosphor_shade"] = &shader_pal_phosphor_shade;
#endif    /* use_sdl */

#ifdef HAVE_SDL
    settings_bools["sound"] = &gd_sound_enabled;
    settings_bools["sound_16bit_mixing"] = &gd_sound_16bit_mixing;
    settings_bools["sound_44khz_mixing"] = &gd_sound_44khz_mixing;
    settings_bools["sound_stereo"] = &gd_sound_stereo;
    settings_bools["classic_sound"] = &gd_classic_sound;
    settings_integers["sound_chunks_volume_percent"] = &gd_sound_chunks_volume_percent;
    settings_integers["sound_music_volume_percent"] = &gd_sound_music_volume_percent;
#endif
}


static void add_dirs(std::vector<std::string>& dirs, const char *specific) {
    // user's own config
    dirs.push_back(gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, gd_user_config_dir.c_str(), specific, NULL)));
    dirs.push_back(gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, gd_user_config_dir.c_str(), NULL)));
    // system-wide gdash config
    dirs.push_back(gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, gd_system_data_dir.c_str(), specific, NULL)));
    dirs.push_back(gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, gd_system_data_dir.c_str(), NULL)));
    // for testing: actual directory
    dirs.push_back(gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, ".", specific, NULL)));
    dirs.push_back(gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, ".", NULL)));
}

/* sets up directiories and loads translations */
void gd_settings_init_dirs() {
#ifdef G_OS_WIN32
    /* on win32, use the glib function. */
    gd_system_data_dir = g_win32_get_package_installation_directory(NULL, NULL);
#else
    /* on linux, this is a defined, built-in string, $perfix/share/locale */
    gd_system_data_dir = PKGDATADIR;
#endif
    //gd_system_caves_dir = gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, gd_system_data_dir.c_str(), "caves", NULL));
    gd_system_caves_dir = gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, g_get_current_dir(), "caves", NULL));
    //gd_system_music_dir = gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, gd_system_data_dir.c_str(), "music", NULL));
    gd_system_music_dir = gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, g_get_current_dir(), "music", NULL));
    gd_user_config_dir = gd_tostring_free(g_build_path(G_DIR_SEPARATOR_S, g_get_user_config_dir(), PACKAGE, NULL));

    add_dirs(gd_sound_dirs, "sound");
    add_dirs(gd_themes_dirs, "themes");
    add_dirs(gd_fonts_dirs, "fonts");
    add_dirs(gd_shaders_dirs, "shaders");
}

/* set locale from the gdash setting variable. */
/* only bother setting the locale cleverly when we are in the gtk version. */
/* for sdl version, not really matters. */
/* if no gtk, just set the system default locale. */
void gd_settings_set_locale() {
    if (gd_language < 0 || gd_language >= int(G_N_ELEMENTS(language_locale)))
        gd_language = 0;  /* switch to default, if out of bounds. */

    /* we set the language environment variable, as gtk and other stuff will use it. */
    g_assert(G_N_ELEMENTS(language_locale) == G_N_ELEMENTS(languages_for_env));
    if (languages_for_env[gd_language] != NULL) {
        g_setenv("LANG", languages_for_env[gd_language], TRUE);
        g_setenv("LANGUAGE", languages_for_env[gd_language], TRUE);
    }

    /* try to set the locale. */
    int i = 0;
    char *result = NULL;
    while (result == NULL && language_locale[gd_language][i] != NULL) {
        result = setlocale(LC_ALL, language_locale[gd_language][i]);
        i++;
    }
    if (result == NULL) {
        /* failed to set. switch to system default */
        if (gd_language != 0)
            gd_message("Failed to set language to '%s'. Switching to system default locale.", gd_languages_names[gd_language]);
        setlocale(LC_ALL, "");
    }
}

/* sets up directiories and loads translations */
/* also instructs gettext to give all strings as utf8, as gtk uses that */
void gd_settings_init_translation() {
    /* different directories storing the translation files on unix and win32. */
    /* gdash (and gtk) always uses utf8, so convert translated strings to utf8 if needed. */
#ifdef G_OS_WIN32
    bindtextdomain("gtk20-properties", gd_system_data_dir.c_str());
    bind_textdomain_codeset("gtk20-properties", "UTF-8");
    bindtextdomain("gtk20", gd_system_data_dir.c_str());
    bind_textdomain_codeset("gtk20", "UTF-8");
    bindtextdomain("glib20", gd_system_data_dir.c_str());
    bind_textdomain_codeset("glib20", "UTF-8");
    bindtextdomain(PACKAGE, gd_system_data_dir.c_str());        /* gdash */
    bind_textdomain_codeset(PACKAGE, "UTF-8");
#else
    //bindtextdomain(PACKAGE, LOCALEDIR);                 /* gdash */
    bindtextdomain(PACKAGE, g_get_current_dir());
    bind_textdomain_codeset(PACKAGE, "UTF-8");
#endif
    textdomain(PACKAGE);    /* set default textdomain to gdash */
}


/* load settings from .config/gdash/gdash.ini */
void gd_load_settings() {
    GError *error = NULL;
    gchar *data;
    gsize length;

    AutoGFreePtr<char> filename(g_build_path(G_DIR_SEPARATOR_S, gd_user_config_dir.c_str(), SETTINGS_INI_FILE, NULL));
    if (!g_file_get_contents(filename, &data, &length, &error)) {
        /* no ini file found */
        gd_debug(error->message);
        g_error_free(error);
        return;
    }
    /* if zero length file, also return */
    if (length == 0)
        return;

    GKeyFile *ini = g_key_file_new();
    gboolean success = g_key_file_load_from_data(ini, data, length, G_KEY_FILE_NONE, &error);
    g_free(data);
    if (!success) {
        gd_message("INI file contents error: %s", error->message);
        g_error_free(error);
        return;
    }

    /* load the settings */
    for (std::map<char const *, int *>::const_iterator it = settings_integers.begin(); it != settings_integers.end(); ++it) {
        char const *key = it->first;
        int &var = *it->second;
        var = keyfile_get_integer_with_default(ini, SETTINGS_GDASH_GROUP, key, var);
    }
    for (std::map<char const *, double *>::const_iterator it = settings_doubles.begin(); it != settings_doubles.end(); ++it) {
        char const *key = it->first;
        double &var = *it->second;
        var = keyfile_get_double_with_default(ini, SETTINGS_GDASH_GROUP, key, var);
    }
    for (std::map<char const *, bool *>::const_iterator it = settings_bools.begin(); it != settings_bools.end(); ++it) {
        char const *key = it->first;
        bool &var = *it->second;
        var = keyfile_get_bool_with_default(ini, SETTINGS_GDASH_GROUP, key, var);
    }
    for (std::map<char const *, std::string *>::const_iterator it = settings_strings.begin(); it != settings_strings.end(); ++it) {
        char const *key = it->first;
        std::string &var = *it->second;
        var = keyfile_get_string(ini, SETTINGS_GDASH_GROUP, key);
    }

    g_key_file_free(ini);

    /* check settings */
    gd_cell_scale_factor_game = gd_clamp_double(gd_cell_scale_factor_game, 1.0, 8.0);
    gd_cell_scale_factor_editor = gd_clamp_double(gd_cell_scale_factor_editor, 1.0, 8.0);
    gd_cell_scale_type_game = gd_clamp(gd_cell_scale_type_game, 0, GD_SCALING_MAX-1);
    gd_cell_scale_type_editor = gd_clamp(gd_cell_scale_type_editor, 0, GD_SCALING_MAX-1);
    if (gd_preferred_palette < 0 || gd_preferred_palette >= int(GdColor::TypeInvalid))
        gd_preferred_palette = GdColor::TypeRGB;
    if (gd_status_bar_colors < 0 || gd_status_bar_colors > int(GD_STATUS_BAR_MAX))
        gd_status_bar_colors = GD_STATUS_BAR_ORIGINAL;
}


/* save settings to .config/gdash.ini */
void gd_save_settings() {
    GKeyFile *ini = g_key_file_new();

    /* save them */
    for (std::map<char const *, int *>::const_iterator it = settings_integers.begin(); it != settings_integers.end(); ++it) {
        char const *key = it->first;
        int &var = *it->second;
        g_key_file_set_integer(ini, SETTINGS_GDASH_GROUP, key, var);
    }
    for (std::map<char const *, double *>::const_iterator it = settings_doubles.begin(); it != settings_doubles.end(); ++it) {
        char const *key = it->first;
        double &var = *it->second;
        g_key_file_set_double(ini, SETTINGS_GDASH_GROUP, key, var);
    }
    for (std::map<char const *, bool *>::const_iterator it = settings_bools.begin(); it != settings_bools.end(); ++it) {
        char const *key = it->first;
        bool &var = *it->second;
        g_key_file_set_boolean(ini, SETTINGS_GDASH_GROUP, key, (gboolean) var);
    }
    for (std::map<char const *, std::string *>::const_iterator it = settings_strings.begin(); it != settings_strings.end(); ++it) {
        char const *key = it->first;
        std::string &var = *it->second;
        g_key_file_set_string(ini, SETTINGS_GDASH_GROUP, key, var.c_str());
    }

    GError *error = NULL;
    /* convert to string and free */
    AutoGFreePtr<char> data(g_key_file_to_data(ini, NULL, &error));
    g_key_file_free(ini);
    if (error) {
        /* this is highly unlikely - why would g_key_file_to_data report error? docs do not mention. */
        gd_warning("Unable to save settings: %s", error->message);
        g_error_free(error);
        return;
    }

    AutoGFreePtr<char> filename(g_build_path(G_DIR_SEPARATOR_S, gd_user_config_dir.c_str(), SETTINGS_INI_FILE, NULL));
    g_mkdir_with_parents(gd_user_config_dir.c_str(), 0700);
    g_file_set_contents(filename, data, -1, &error);
    if (error) {
        /* error saving the file */
        gd_warning("Unable to save settings: %s", error->message);
        g_error_free(error);
        return;
    }
}

GOptionContext *gd_option_context_new() {
    GOptionEntry const entries[] = {
        {"license", 'L', 0, G_OPTION_ARG_NONE, &gd_param_license, N_("Show license and quit")},
        {"debug", 'v', 0, G_OPTION_ARG_NONE, &gd_param_debug, N_("Show some debug messages")},
        {"default-settings", 0, 0, G_OPTION_ARG_NONE, &gd_param_load_default_settings, N_("Load default settings")},
        {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &gd_param_cavenames, N_("Cave names")},
        {NULL}
    };
    GOptionContext *context = g_option_context_new(_("[FILE NAME]"));
    g_option_context_set_help_enabled(context, TRUE);
    g_option_context_add_main_entries(context, entries, PACKAGE);    /* gdash parameters */

    return context;
}
