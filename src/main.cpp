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

#include <glib.h>
#include <glib/gi18n.h>
#include <fstream>

#ifdef HAVE_GTK
#include <gtk/gtk.h>
#endif

#ifdef HAVE_SDL
/* on windows, sdl does some hack on the main function.
 * therefore we need to include this file,
 * even if it seems to do nothing. */
#include <SDL2/SDL.h>
#endif

#include "cave/caveset.hpp"
#include "sound/sound.hpp"
#include "misc/util.hpp"
#include "misc/logger.hpp"
#include "misc/about.hpp"
#include "settings.hpp"
#include "framework/commands.hpp"
#include "fileops/bdcffload.hpp"
#include "fileops/loadfile.hpp"
#include "fileops/highscore.hpp"
#include "fileops/binaryimport.hpp"
#include "input/joystick.hpp"

#ifdef HAVE_GTK
#include "editor/editor.hpp"
#include "editor/editorcellrenderer.hpp"
#include "editor/exporthtml.hpp"
#include "editor/exporttext.hpp"
#include "editor/exportcrli.hpp"
#include "gtk/gtkpixbuffactory.hpp"
#include "gtk/gtkscreen.hpp"
#include "gtk/gtkui.hpp"
#include "misc/helphtml.hpp"
#endif

#include "mainwindow.hpp"

/* includes cavesets built in to the executable */
#include "levels.cpp"



int main(int argc, char *argv[]) {
    CaveSet caveset;
    int quit = 0;
#ifdef HAVE_GTK
    gboolean editor = FALSE;
#endif
    char *gallery_filename = NULL;
    char *text_dump_filename = NULL;
    char *png_filename = NULL, *png_size = NULL;
    char *save_cave_name = NULL, *save_gds_name = NULL;
    int exportcrli = 0;
    char *save_cave_name_flat = NULL;
#ifdef HAVE_GTK
    int save_doc_lang = -1;
#endif

    GError *error = NULL;
    GOptionEntry entries[] = {
#ifdef HAVE_GTK
        {"editor", 'e', 0, G_OPTION_ARG_NONE, &editor, N_("Start editor")},
        {"save-gallery", 'g', 0, G_OPTION_ARG_FILENAME, &gallery_filename, N_("Save caveset in a HTML gallery")},
        {"save-text", 't', 0, G_OPTION_ARG_FILENAME, &text_dump_filename, N_("Save caveset as BDCFF plain text")},
        {"stylesheet", 0, 0, G_OPTION_ARG_STRING  /* not filename! */, &gd_html_stylesheet_filename, N_("Link stylesheet from file to a HTML gallery, eg. \"../style.css\"")},
        {"favicon", 0, 0, G_OPTION_ARG_STRING /* not filename! */, &gd_html_favicon_filename, N_("Link shortcut icon to a HTML gallery, eg. \"../favicon.ico\"")},
        {"save-png", 'p', 0, G_OPTION_ARG_FILENAME, &png_filename, N_("Save image of first cave to PNG")},
        {"png-size", 0, 0, G_OPTION_ARG_STRING, &png_size, N_("Set PNG image size. Default is 128x96, set to 0x0 for unscaled")},
#endif
        {"save-bdcff", 's', 0, G_OPTION_ARG_FILENAME, &save_cave_name, N_("Save caveset in a BDCFF file")},
        {"save-gds", 'd', 0, G_OPTION_ARG_FILENAME, &save_gds_name, N_("Save imported binary data to a GDS file. An input file name is required.")},
        {"save-crli", 'x', 0, G_OPTION_ARG_NONE, &exportcrli, N_("Save caveset in CrLi files")},
        {"save-flat", 'f', 0, G_OPTION_ARG_FILENAME, &save_cave_name_flat, N_("Save caveset in flattened format")},
#ifdef HAVE_GTK
        {"save-docs", 0, 0, G_OPTION_ARG_INT, &save_doc_lang, N_("Save documentation in HTML, in the given language identified by an integer.")},
#endif
        {"quit", 'q', 0, G_OPTION_ARG_NONE, &quit, N_("Batch mode: quit after specified tasks")},
        {NULL}
    };

    GOptionContext *context = gd_option_context_new();
    g_option_context_add_main_entries(context, entries, PACKAGE);   /* gdash (gtk version) parameters */
#ifdef HAVE_GTK
    g_option_context_add_group(context, gtk_get_option_group(FALSE));   /* add gtk parameters */
#endif
    g_option_context_parse(context, &argc, &argv, &error);
    if (error) {
        gd_warning(error->message);
        g_error_free(error);
    }

    /* show license? */
    if (gd_param_license) {
        /* print license and quit. */
        g_print("%s %s\n\n%s\n\n", PACKAGE_NAME, PACKAGE_VERSION, About::copyright);
        std::vector<std::string> wrapped_license = gd_wrap_text(About::license, 72);
        for (unsigned i = 0; i < wrapped_license.size(); ++i)
            g_print("%s\n", wrapped_license[i].c_str());
        return 0;
    }

    Logger global_logger;

    gd_settings_init();
    gd_settings_init_dirs();
    if (!gd_param_load_default_settings)
        gd_load_settings();
    gd_settings_set_locale();
    gd_settings_init_translation();

    /* translated --help */
    if (gd_param_help_localized) {
        // needs to be after gd_settings_set_locale()
        g_print(g_option_context_get_help(context, TRUE, NULL), NULL);
        return 0;
    }
    g_option_context_free(context);

#ifdef HAVE_GTK
    /* init gtk and set gtk default icon */
    gboolean force_quit_no_gtk = FALSE;
    if (!gtk_init_check(&argc, &argv))
        force_quit_no_gtk = TRUE;
    GdkPixbuf *logo = gd_icon();
    gtk_window_set_default_icon(logo);
    g_object_unref(logo);
#endif

    gd_cave_types_init();
    Joystick::init();

    /* if memory snapshot -> gds file conversion requested */
    if (save_gds_name != NULL) {
        Logger thislogger;
        if (gd_param_cavenames == NULL || g_str_equal(gd_param_cavenames, "")) {
            g_print("An input filename must be given for GDS conversion.\n");
            return 1;
        }
        std::vector<unsigned char> file = load_file_to_vector(gd_param_cavenames[0]);
        /* -1 because the file loader adds a terminating zero */
        std::vector<unsigned char> memory = load_memory_dump(&file[0], file.size() - 1);
        std::vector<unsigned char> gds = gdash_binary_import(memory);
        std::fstream os(save_gds_name, std::ios::out | std::ios::binary);
        os.write((char *) &gds[0], gds.size());

        thislogger.clear();
    }

    /* LOAD A CAVESET FROM A FILE, OR AN INTERNAL ONE */
    /* if remaining arguments, they are filenames */
    try {
        if (gd_param_cavenames && gd_param_cavenames[0]) {
            caveset = load_caveset_from_file(gd_param_cavenames[0]);
            load_highscore(caveset);
        } else {
            /* export-fork: load BD1 by default */
            try {
                caveset = load_caveset_from_file("caves/First_Star_Software/Boulder_Dash_1.bd");
                load_highscore(caveset);
            } catch (std::exception &e) {
                gd_warning("BD1 not found, loading 'Afl Posocopi' instead");
                /* if nothing requested, load default */
                caveset = create_from_buffer(level_pointers[0], -1);
                caveset.name = level_names[0];
                load_highscore(caveset);
            }
        }
    } catch (std::exception &e) {
        /// @todo show error to the screen
        gd_critical(e.what());
    }

#ifdef HAVE_GTK
    /* see if generating a gallery. */
    /* but only if there are any caves at all. */
    if (caveset.has_caves() && gallery_filename)
        gd_save_html(gallery_filename, caveset);

    /* see if generating a BDCFF plain text cave dump. */
    /* but only if there are any caves at all. */
    if (caveset.has_caves() && text_dump_filename)
        gd_save_text(text_dump_filename, caveset);

    /* save cave png */
    if (png_filename) {
        unsigned int size_x = 128, size_y = 96; /* default size */

        if (png_size && (sscanf(png_size, "%ux%u", &size_x, &size_y) != 2))
            gd_warning(_("Invalid image size: %s"), png_size);
        if (size_x < 1 || size_y < 1) {
            size_x = 0;
            size_y = 0;
        }

        /* rendering cave for png: seed=0 */
        CaveRendered renderedcave(caveset.caves[0], 0, 0);
        GTKPixbufFactory pf;
        GTKScreen scr(pf, NULL);
        EditorCellRenderer cr(scr, gd_theme);

        GdkPixbuf *pixbuf = gd_drawcave_to_pixbuf(renderedcave, cr, size_x, size_y, true, false);
        GError *error = NULL;
        if (!gdk_pixbuf_save(pixbuf, png_filename, "png", &error, "compression", "9", NULL)) {
            gd_critical("Error saving PNG image %s: %s", png_filename, error->message);
            g_error_free(error);
        }
        g_object_unref(pixbuf);
    }
#endif

    if (save_cave_name)
        caveset.save_to_file(save_cave_name);

    /* export all caves into Crazy Light engine file format */
    if (exportcrli) {
        gd_message("Exporting caves into CrLi format...");
        gd_message("cave set name: %s", caveset.name);
        gd_message("number of caves: %d", caveset.caves.size());
        for (unsigned n = 0; n < caveset.caves.size(); n++) {
                CaveStored cave = caveset.caves.at(n);
                char filename[50];
                snprintf(filename, sizeof(filename), "%02d-%s.CrLi", n + 1, cave.name.c_str());
                gd_message("Writing cave: %s", filename);
                gd_export_cave_to_crli_cavefile(cave, 0, filename);
        }
    }

    /* flatten all caves */
    if (save_cave_name_flat) {
       gd_message("Flatten caves...");
       gd_message("cave set name: %s", caveset.name);
       gd_message("number of caves: %d", caveset.caves.size());
       for (unsigned n = 0; n < caveset.caves.size(); n++) {
               CaveStored cave = caveset.caves.at(n);
               int edit_level = 0;
               CaveRendered rendered(cave, edit_level, 0);   /* render cave at specified level to obtain map. seed=0 */
               cave.map = rendered.map;
               cave.objects.clear();
               caveset.caves.at(n) = cave;
       }
       caveset.save_to_file(save_cave_name_flat);
   }

#ifdef HAVE_GTK
    gd_register_stock_icons();

    if (save_doc_lang != -1) {
        if (save_doc_lang < 1 || save_doc_lang >= (int)g_strv_length((gchar **) gd_languages_names)) {
            gd_critical("No such language");
            return 1;
        }
        /* switch to the doc language requested */
        int language_previous = gd_language;
        gd_language = save_doc_lang;
        gd_settings_set_locale();
        gd_settings_init_translation();

        /* the html saver needs a realized widget to render gtk icons, so create a toplevel window,
         * realize it, but do not show it */
        GtkWidget *widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_widget_realize(widget);
        save_help_to_html(Printf("Doc-%s.html", gd_languages_names[save_doc_lang]).c_str(), widget);
        gtk_widget_destroy(widget);

        /* switch back to original language */
        gd_language = language_previous;
        gd_settings_set_locale();
        gd_settings_init_translation();
    }
#endif

    /* if batch mode, quit now */
    if (quit) {
        global_logger.clear();
        return 0;
    }
#ifdef HAVE_GTK
    if (force_quit_no_gtk) {
        gd_critical("Cannot initialize GTK+");
        return 1;
    }
#endif

#ifdef HAVE_SDL
    if (SDL_Init(0) == -1) {
        gd_critical("Cannot initialize SDL");
        return 1;
    }
#endif

    /* select first action */
    NextAction na = StartTitle;
#ifdef HAVE_GTK
    if (editor)
        na = StartEditor;
#endif

restart_from_here:

    gd_sound_init();
    gd_sound_set_music_volume();
    gd_sound_set_chunk_volumes();

    while (na != Quit && na != Restart) {
        switch (na) {
            case StartTitle:
                main_window_run_title_screen(&caveset, na);
                break;
#ifdef HAVE_GTK
            case StartEditor:
                na = StartTitle;
                gd_cave_editor_run(&caveset);
                break;
#endif
            case Restart:
            case Quit:
                break;
        }
    }

    gd_sound_close();

    if (na == Restart) {
        na = StartTitle;
        goto restart_from_here;
    }

    save_highscore(caveset);

    gd_save_settings();

    global_logger.clear();

#ifdef HAVE_SDL
    SDL_Quit();
#endif

    /* free the stuff created by the option context */
    g_free(gallery_filename);
    g_free(gd_html_stylesheet_filename);
    g_free(gd_html_favicon_filename);
    g_free(text_dump_filename);
    g_free(png_filename);
    g_free(png_size);
    g_free(save_gds_name);

    return 0;
}

