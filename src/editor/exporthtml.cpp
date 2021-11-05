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
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "editor/exporthtml.hpp"
#include "cave/cavetypes.hpp"
#include "cave/caveset.hpp"
#include "gtk/gtkscreen.hpp"
#include "settings.hpp"
#include "misc/logger.hpp"
#include "cave/caverendered.hpp"
#include "cave/titleanimation.hpp"
#include "editor/editorcellrenderer.hpp"
#include "gtk/gtkpixbuffactory.hpp"
#include "gtk/gtkpixbuf.hpp"

/**
 * Save caveset as html gallery.
 * @param htmlname filename
 */
void gd_save_html(char *htmlname, CaveSet &caveset) {
    GError *error = NULL;
    std::string contents;

    char *pngoutbasename;   /* used as a base name for png output files */
    if (g_str_has_suffix(htmlname, ".html")) {
        /* has html extension */
        pngoutbasename = g_strdup(htmlname);
        *g_strrstr(pngoutbasename, ".html") = 0;
    } else {
        /* has no html extension */
        pngoutbasename = g_strdup(htmlname);
        htmlname = g_strdup_printf("%s.html", pngoutbasename);
    }
    /* used as a base for img src= tags */
    char *pngbasename = g_path_get_basename(pngoutbasename);

    contents += "<HTML>\n";
    contents += "<HEAD>\n";
    contents += Printf("<TITLE>%ms</TITLE>\n", caveset.name);
    contents += "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n";
    if (gd_html_stylesheet_filename)
        contents += Printf("<link rel=\"stylesheet\" href=\"%s\">\n", gd_html_stylesheet_filename);
    if (gd_html_favicon_filename)
        contents += Printf("<link rel=\"shortcut icon\" href=\"%s\">\n", gd_html_favicon_filename);
    contents += "</HEAD>\n\n";

    contents += "<BODY>\n";

    // CAVESET DATA
    contents += Printf("<H1>%ms</H1>\n", caveset.name);
    /* if the game has its own title screen */
    if (caveset.title_screen != "") {
        GTKPixbufFactory pf;

        /* create the title image and save it */
        std::vector<std::unique_ptr<Pixbuf>> title_images = get_title_animation_pixbuf(caveset.title_screen, caveset.title_screen_scroll, true, pf);
        if (!title_images.empty()) {
            GdkPixbuf *title_image = static_cast<GTKPixbuf &>(*title_images[0]).get_gdk_pixbuf();

            char *pngname = g_strdup_printf("%s_%03d.png", pngoutbasename, 0); /* it is the "zeroth" image */
            GError *error = NULL;
            gdk_pixbuf_save(title_image, pngname, "png", &error, "compression", "9", NULL);
            if (error) {
                gd_warning(error->message);
                g_error_free(error);
                error = NULL;
            }
            g_free(pngname);

            contents += Printf("<IMAGE SRC=\"%s_%03d.png\" WIDTH=\"%d\" HEIGHT=\"%d\">\n", pngbasename, 0, gdk_pixbuf_get_width(title_image), gdk_pixbuf_get_height(title_image));
            contents += "<BR>\n";
        }
    }
    contents += "<TABLE>\n";
    contents += Printf("<TR><TH>%ms<TD>%d\n", _("Caves"), caveset.caves.size());
    if (caveset.author != "")
        contents += Printf("<TR><TH>%ms<TD>%ms\n", _("Author"), caveset.author);
    if (caveset.description != "")
        contents += Printf("<TR><TH>%ms<TD>%ms\n", _("Description"), caveset.description);
    if (caveset.www != "")
        contents += Printf("<TR><TH>%ms<TD>%ms\n", _("WWW"), caveset.www);
    if (caveset.remark != "")
        contents += Printf("<TR><TH>%ms<TD>%ms\n", _("Remark"), caveset.remark);
    if (caveset.story != "")
        contents += Printf("<TR><TH>%ms<TD>%ms\n", _("Story"), caveset.story);
    contents += "</TABLE>\n";

    /* cave names, descriptions, hrefs */
    contents += "<DL>\n";
    for (unsigned n = 0; n < caveset.caves.size(); n++) {
        CaveStored &cave = caveset.caves[n];

        contents += Printf("<DT><A HREF=\"#cave%03d\">%s</A></DT>\n", n + 1, cave.name);
        if (cave.description != "")
            contents += Printf("    <DD>%s</DD>\n", cave.description);
    }
    contents += "</DL>\n\n";

    GTKPixbufFactory pf;
    GTKScreen screen(pf, NULL);
    EditorCellRenderer cr(screen, gd_theme);
    for (unsigned i = 0; i < caveset.caves.size(); i++) {
        /* rendering cave for png: seed=0 */
        CaveStored &cave = caveset.caves[i];
        CaveRendered rendered(cave, 0, 0);

        /* check cave to see if we have amoeba or magic wall. properties will be shown in html, if so. */
        bool has_amoeba = false, has_magic = false;
        for (int y = 0; y < cave.h; y++)
            for (int x = 0; x < cave.w; x++) {
                if (rendered.map(x, y) == O_AMOEBA)
                    has_amoeba = true;
                if (rendered.map(x, y) == O_MAGIC_WALL)
                    has_magic = true;
                break;
            }

        /* cave header */
        contents += Printf("<A NAME=\"cave%03d\"></A>\n<H2>%ms</H2>\n", i + 1, cave.name);

        /* save image */
        char *pngname = g_strdup_printf("%s_%03d.png", pngoutbasename, i + 1);
        GdkPixbuf *pixbuf = gd_drawcave_to_pixbuf(rendered, cr, 0, 0, true, false);
        gdk_pixbuf_save(pixbuf, pngname, "png", &error, "compression", "9", NULL);
        if (error) {
            gd_warning(error->message);
            g_error_free(error);
            error = NULL;
        }
        g_free(pngname);
        contents += Printf("<IMAGE SRC=\"%s_%03d.png\" WIDTH=\"%d\" HEIGHT=\"%d\">\n", pngbasename, i + 1, gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
        g_object_unref(pixbuf);

        contents += "<BR>\n";
        contents += "<TABLE>\n";
        if (cave.author != "")
            contents += Printf("<TR><TH>%ms<TD>%ms\n", _("Author"), cave.author);
        if (cave.description != "")
            contents += Printf("<TR><TH>%ms<TD>%ms\n", _("Description"), cave.description);
        if (cave.remark != "")
            contents += Printf("<TR><TH>%ms<TD>%ms\n", _("Remark"), cave.remark);
        if (cave.story != "")
            contents += Printf("<TR><TH>%ms<TD>%ms\n", _("Story"), cave.story);
        contents += Printf("<TR><TH>%ms<TD>%ms\n", _("Type"), cave.intermission ? _("Intermission") : _("Normal cave"));
        contents += Printf("<TR><TH>%ms<TD>%ms\n", _("Selectable as start"), cave.selectable ? _("Yes") : _("No"));
        contents += Printf("<TR><TH>%ms<TD>%d\n", _("Diamonds needed"), cave.level_diamonds[0]);
        contents += Printf("<TR><TH>%ms<TD>%d\n", _("Diamond value"), cave.diamond_value);
        contents += Printf("<TR><TH>%ms<TD>%d\n", _("Extra diamond value"), cave.extra_diamond_value);
        contents += Printf("<TR><TH>%ms<TD>%d\n", _("Time (s)"), cave.level_time[0]);
        if (has_amoeba)
            contents += Printf("<TR><TH>%ms<TD>%d, %d\n", _("Amoeba threshold and time (s)"), cave.level_amoeba_threshold[0], cave.level_amoeba_time[0]);
        if (has_magic)
            contents += Printf("<TR><TH>%ms<TD>%d\n", _("Magic wall milling time (s)"), cave.level_magic_wall_time[0]);
        contents += "</TABLE>\n";

        contents += "\n";

    }
    contents += "</BODY>\n";
    contents += "</HTML>\n";
    g_free(pngoutbasename);
    g_free(pngbasename);

    if (!g_file_set_contents(htmlname, contents.c_str(), contents.size(), &error)) {
        /* could not save properly */
        gd_critical(error->message);
        g_error_free(error);
    }
}
