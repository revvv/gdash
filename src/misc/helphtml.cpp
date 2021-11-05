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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <fstream>

#include "misc/helphtml.hpp"
#include "misc/helptext.hpp"
#include "editor/editorcellrenderer.hpp"
#include "misc/printf.hpp"
#include "misc/about.hpp"
#include "gtk/gtkscreen.hpp"
#include "gtk/gtkpixbuffactory.hpp"

#include "for_html.cpp"

static std::string help_to_html_string(helpdata const help_text[], GtkWidget *widget) {
    GTKPixbufFactory pf;
    GTKScreen screen(pf, NULL);
    EditorCellRenderer cr(screen, "");
    std::string result;
    static unsigned image = 0;
    bool first = true;

    for (unsigned int i = 0; g_strcmp0(help_text[i].stock_id, HELP_LAST_LINE) != 0; ++i) {
        // impossible: stock id & element together
        g_assert(!(help_text[i].stock_id != NULL && help_text[i].element != O_NONE));
        // impossible: key without explanation
        g_assert(!(help_text[i].keyname != NULL && help_text[i].description == NULL));
        GdkPixbuf *pixbuf = NULL;
        std::string heading;
        
        /* stock icon? */
        if (help_text[i].stock_id) {
            pixbuf = gtk_widget_render_icon_pixbuf(widget, help_text[i].stock_id, GTK_ICON_SIZE_LARGE_TOOLBAR);
        }

        /* element? */
        GdElementEnum element = help_text[i].element;
        if (element != O_NONE) {
            pixbuf = cr.combo_pixbuf_simple(element);
            // The cellrenderer holds the ref, so we add one ref as we will unref later
            g_object_ref(pixbuf);
            heading = visible_name_no_attribute(element);
        }

        /* heading? (if there is one for an element, overwrites) */
        if (help_text[i].heading) {
            heading = _(help_text[i].heading);
        }

        if (heading != "")
            result += first ? "<h2>" : "<h3>";
        if (pixbuf) {
            std::string filename = Printf("image_%d.png", image++);
            gdk_pixbuf_save(pixbuf, filename.c_str(), "png", NULL, "compression", "9", NULL);
            g_object_unref(pixbuf);
            result += Printf("<img src=\"%s\"> ", filename);
        }
        if (heading != "") {
            result += Printf("%ms", heading);
            result += first ? "</h2>\n" : "</h3>\n";
        }

        /* some words in big letters */
        /* keyboard stuff in bold */
        if (help_text[i].description) {
            std::string kbd;
            if (help_text[i].keyname) {
                // first translate! the translators got the original string.
                char **keys = g_strsplit(_(help_text[i].keyname), ",", -1);
                for (unsigned i = 0; keys[i] != NULL; ++i) {
                    g_strstrip(keys[i]);
                    if (!g_str_equal(keys[i], ""))
                        kbd += Printf("<kbd>%ms</kbd> ", keys[i]);
                }
                g_strfreev(keys);
            }
            /* the long text. may be a part of a list? if so, write a list item, and skip the "- " by +2. */
            if (help_text[i].description[0]=='-' && help_text[i].description[1]==' ')
                result += Printf("<ul><li>%s%ms</li></ul>\n", kbd, _(help_text[i].description) + 2);
            else
                result += Printf("<p>%s%ms</p>\n", kbd, _(help_text[i].description));
        }

        first = false;
    }
    return result;
}


static std::string about_to_html() {
    // TRANSLATORS: about dialog box categories.
    struct String {
        char const *title;
        char const *text;
    } strings[] = {
        { _("About GDash"), About::comments },
        { _("Copyright"), About::copyright},
        { _("License"), About::license },
        { _("Website"), About::website },
        { NULL, NULL }
    };
    // TRANSLATORS: about dialog box categories.
    struct StringArray {
        char const *title;
        char const **texts;
    } stringarrays[] = {
        { _("Authors"), About::authors },
        { _("Artists"), About::artists },
        { _("Documenters"), About::documenters },
        { NULL, NULL }
    };
    std::string text;

    for (String *s = strings; s->title != NULL; ++s) {
        if (s == strings)
            text += Printf("<h2>%ms</h2>\n<p>%ms</p>\n", s->title, _(s->text));
        else
            text += Printf("<h3>%ms</h3>\n<p>%ms</p>\n", s->title, _(s->text));
    }
    for (StringArray *s = stringarrays; s->title != NULL; ++s)  {
        text += Printf("<h3>%ms</h3>\n<ul>\n", s->title);
        for (char const **t = s->texts; *t != NULL; ++t) {
            text += Printf("<li>%ms</li>\n", *t);
        }
        text += "</ul>\n";
    }

    return text;
}

/**
 * A widget is needed to be able to gtk-render icons.
 */
void save_help_to_html(char const *filename, GtkWidget *widget) {
    std::string htmltext;
    helpdata const *helps[] = { titlehelp, gamehelp, replayhelp, editorhelp };

    htmltext += "<div class=\"section\">\n";
    htmltext += Printf("<h2>%ms</h2>", _("Table of contents"));
    htmltext += "<ol>\n";
    htmltext += Printf("<li><a href=\"#%d\">%ms</a></li>\n", 0, _("About GDash"));
    for (unsigned i = 0; i < G_N_ELEMENTS(helps); ++i) {
        /* the first line should be (should have) a heading */
        g_assert(helps[i][0].heading != NULL);
        htmltext += Printf("<li><a href=\"#%d\">%ms</a></li>\n", i + 1, _(helps[i][0].heading));
    }
    htmltext += "</ol>\n";
    htmltext += "</div>\n";

    htmltext += "<div class=\"section\">\n";
    htmltext += Printf("<a name=\"%d\"></a>\n", 0);
    htmltext += about_to_html();
    htmltext += "</div>\n";
    for (unsigned i = 0; i < G_N_ELEMENTS(helps); ++i) {
        htmltext += "<div class=\"section\">\n";
        htmltext += Printf("<a name=\"%d\"></a>\n", i + 1);
        htmltext += help_to_html_string(helps[i], widget);
        htmltext += "</div>\n";
    }

    std::ofstream os(filename);

    os << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n";
    os << "<html>\n";
    os << "<head>\n";
    os << "<link rel=\"stylesheet\" href=\"style.css\" media=\"all\">\n";
    os << "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n";
    os << "</head>\n";
    os << "<body>\n";
    os << "<h1>GDash " PACKAGE_VERSION "</h1>\n";
    os << htmltext;
    os << "</body>\n";
    os << "</html>\n";
    
    g_file_set_contents("style.css", (gchar*) style, sizeof(style), NULL);
    g_file_set_contents("background.png", (gchar*) background, sizeof(background), NULL);
    g_file_set_contents("gdash.png", (gchar*) Screen::gdash_icon_48_png, Screen::gdash_icon_48_size, NULL);
}
