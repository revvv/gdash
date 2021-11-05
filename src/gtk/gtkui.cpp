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

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "fileops/loadfile.hpp"
#include "fileops/highscore.hpp"
#include "cave/caveset.hpp"
#include "misc/logger.hpp"
#include "gtk/gtkpixbuf.hpp"
#include "gtk/gtkpixbuffactory.hpp"
#include "gtk/gtkscreen.hpp"
#include "editor/editorcellrenderer.hpp"
#include "settings.hpp"
#include "gtk/gtkui.hpp"
#include "misc/about.hpp"
#include "misc/helptext.hpp"
#include "misc/autogfreeptr.hpp"
#include "misc/util.hpp"

/* pixbufs of icons and the like */
#include "icons.cpp"

static std::string last_folder;

void gd_register_stock_icons() {
    /* a table of icon data (guint8*, static arrays included from icons.h) and stock id. */
    static struct {
        const guint8 *data;
        const char *stock_id;
    } icons[] = {
        { cave_editor, GD_ICON_CAVE_EDITOR },
        { move, GD_ICON_EDITOR_MOVE },
        { add_join, GD_ICON_EDITOR_JOIN },
        { add_freehand, GD_ICON_EDITOR_FREEHAND },
        { add_point, GD_ICON_EDITOR_POINT },
        { add_line, GD_ICON_EDITOR_LINE },
        { add_rectangle, GD_ICON_EDITOR_RECTANGLE },
        { add_filled_rectangle, GD_ICON_EDITOR_FILLRECT },
        { add_raster, GD_ICON_EDITOR_RASTER },
        { add_fill_border, GD_ICON_EDITOR_FILL_BORDER },
        { add_fill_replace, GD_ICON_EDITOR_FILL_REPLACE },
        { add_maze, GD_ICON_EDITOR_MAZE },
        { add_maze_uni, GD_ICON_EDITOR_MAZE_UNI },
        { add_maze_braid, GD_ICON_EDITOR_MAZE_BRAID },
        { snapshot, GD_ICON_SNAPSHOT },
        { restart_level, GD_ICON_RESTART_LEVEL },
        { random_fill, GD_ICON_RANDOM_FILL },
        { award, GD_ICON_AWARD },
        { to_top, GD_ICON_TO_TOP },
        { to_bottom, GD_ICON_TO_BOTTOM },
        { object_on_all, GD_ICON_OBJECT_ON_ALL },
        { object_not_on_all, GD_ICON_OBJECT_NOT_ON_ALL },
        { object_not_on_current, GD_ICON_OBJECT_NOT_ON_CURRENT },
        { replay, GD_ICON_REPLAY },
        { keyboard, GD_ICON_KEYBOARD },
        { image, GD_ICON_IMAGE },
        { statistics, GD_ICON_STATISTICS },
    };

    GtkIconFactory *factory = gtk_icon_factory_new();
    for (unsigned i = 0; i < G_N_ELEMENTS(icons); ++i) {
        /* 3rd param: copy pixels = false */
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_inline(-1, icons[i].data, FALSE, NULL);
        GtkIconSet *iconset = gtk_icon_set_new_from_pixbuf(pixbuf);
        g_object_unref(pixbuf);
        gtk_icon_factory_add(factory, icons[i].stock_id, iconset);
    }
    gtk_icon_factory_add_default(factory);
    g_object_unref(factory);
}


GdkPixbuf *gd_icon() {
    GInputStream *is = g_memory_input_stream_new_from_data(Screen::gdash_icon_48_png, Screen::gdash_icon_48_size, NULL);
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_stream(is, NULL, &error);
    g_object_unref(is);
    if (error != NULL) {
        throw std::runtime_error("cannot open inlined icon");
    }
    return pixbuf;
}


/* return a list of image gtk_image_filter's. */
/* they have floating reference. */
/* the list is to be freed by the caller. */
static GList *image_load_filters() {
    GtkFileFilter *all_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(all_filter, _("All image files"));

    /* iterate the list of formats given by gdk. create file filters for each. */
    GSList *formats = gdk_pixbuf_get_formats();
    GList *filters = NULL;  /* new list of filters */
    for (GSList *iter = formats; iter != NULL; iter = iter->next) {
        GdkPixbufFormat *frm = (GdkPixbufFormat *)iter->data;

        if (!gdk_pixbuf_format_is_disabled(frm)) {
            GtkFileFilter *filter = gtk_file_filter_new();
            gtk_file_filter_set_name(filter, gdk_pixbuf_format_get_description(frm));
            char **extensions = gdk_pixbuf_format_get_extensions(frm);
            for (int i = 0; extensions[i] != NULL; i++) {
                std::string pattern = Printf("*.%s", extensions[i]);
                gtk_file_filter_add_pattern(filter, pattern.c_str());
                gtk_file_filter_add_pattern(all_filter, pattern.c_str());
            }
            g_strfreev(extensions);

            filters = g_list_append(filters, filter);
        }
    }
    g_slist_free(formats);

    /* add "all image files" filter */
    filters = g_list_prepend(filters, all_filter);

    return filters;
}


/* file open dialog, with filters for image types gdk-pixbuf recognizes. */
char *gd_select_image_file(const char *title) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new(title, guess_active_toplevel(), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);

    /* obtain list of image filters, and add all to the window */
    GList *filters = image_load_filters();
    for (GList *iter = filters; iter != NULL; iter = iter->next)
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(iter->data));
    g_list_free(filters);

    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    char *filename = NULL;
    if (result == GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    gtk_widget_destroy(dialog);

    return filename;
}


/**
 * Try to guess which window is active.
 */
GtkWindow *guess_active_toplevel() {
    GtkWidget *parent = NULL;

    /* if we find a modal window, it is active. */
    GList *toplevels = gtk_window_list_toplevels();
    for (GList *iter = toplevels; iter != NULL; iter = iter->next)
        if (gtk_window_get_modal(GTK_WINDOW(iter->data)))
            parent = (GtkWidget *)iter->data;

    /* if no modal window found, search for a focused toplevel */
    if (!parent)
        for (GList *iter = toplevels; iter != NULL; iter = iter->next)
            if (gtk_window_has_toplevel_focus(GTK_WINDOW(iter->data)))
                parent = (GtkWidget *)iter->data;

    /* if any of them is focused, just choose the last from the list as a fallback. */
    if (!parent && toplevels)
        parent = (GtkWidget *) g_list_last(toplevels)->data;
    g_list_free(toplevels);

    if (parent)
        return GTK_WINDOW(parent);
    else
        return NULL;
}


/**
 * Show a message dialog, with the specified message type (warning, error, info) and texts.
 *
 * @param type GtkMessageType - sets icon to show.
 * @param primary Primary text.
 * @param secondary Secondary (small) text - may be null.
 */
static void show_message(GtkMessageType type, const char *primary, const char *secondary) {
    GtkWidget *dialog = gtk_message_dialog_new(guess_active_toplevel(),
                        GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                        type, GTK_BUTTONS_OK,
                        "%s", primary);
    gtk_window_set_title(GTK_WINDOW(dialog), "GDash");
    /* secondary message exists an is not empty string: */
    if (secondary && secondary[0] != 0)
        gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog), "%s", secondary);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


void gd_warningmessage(const char *primary, const char *secondary) {
    show_message(GTK_MESSAGE_WARNING, primary, secondary);
}


void gd_errormessage(const char *primary, const char *secondary) {
    show_message(GTK_MESSAGE_ERROR, primary, secondary);
}


void gd_infomessage(const char *primary, const char *secondary) {
    show_message(GTK_MESSAGE_INFO, primary, secondary);
}


/**
 * If necessary, ask the user if he doesn't want to save changes to cave.
 *
 * If the caveset has no modification, this function simply returns true.
 */
bool gd_discard_changes(CaveSet const &caveset) {
    /* save highscore on every ocassion when the caveset is to be removed from memory */
    save_highscore(caveset);

    /* caveset is not edited, so pretend user confirmed */
    if (!caveset.edited)
        return TRUE;

    GtkWidget *dialog = gtk_message_dialog_new(guess_active_toplevel(), GtkDialogFlags(0), GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, _("Caveset \"%s\" is edited or new replays are added. Discard changes?"), caveset.name.c_str());
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), _("If you discard the caveset, all changes and new replays will be lost."));
    gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
    /* create a discard button with a trash icon and Discard text */
    GtkWidget *button = gtk_button_new_with_mnemonic(_("_Discard"));
    gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(GTK_STOCK_DELETE, GTK_ICON_SIZE_BUTTON));
    gtk_widget_show(button);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_YES);

    bool discard = gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES;
    gtk_widget_destroy(dialog);

    /* return button pressed */
    return discard;
}


/* file operation was successful, put it into the recent manager */
static void caveset_file_operation_successful(const char *filename) {
    AutoGFreePtr<char> uri(g_filename_to_uri(filename, NULL, NULL));
    gtk_recent_manager_add_item(gtk_recent_manager_get_default(), uri);
}


/**
 * Save caveset to specified directory, and pop up error message if failed.
 */
static void caveset_save(const gchar *filename, CaveSet &caveset) {
    try {
        caveset.save_to_file(filename);
        caveset_file_operation_successful(filename);
    } catch (std::exception &e) {
        gd_errormessage(e.what(), filename);
    }
}


/**
 * Pops up a "save file as" dialog to the user, to select a file to save the caveset to.
 * If selected, saves the file.
 *
 * @param parent Parent window for the dialog.
 * @param caveset The caveset to save.
 */
void gd_save_caveset_as(CaveSet &caveset) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Save File As"), guess_active_toplevel(), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("BDCFF cave sets (*.bd)"));
    gtk_file_filter_add_pattern(filter, "*.bd");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("All files (*)"));
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), Printf("%s.bd", caveset.name).c_str());

    std::string filename;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
        filename = gd_tostring_free(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
    gtk_widget_destroy(dialog);

    /* if we have a filename, do the save */
    if (filename != "") {
        /* if it has no .bd extension, add one */
        if (!g_str_has_suffix(filename.c_str(), ".bd"))
            filename = Printf("%s.bd", filename);
        caveset_save(filename.c_str(), caveset);
    }
}


/**
 * Save the current caveset. If no filename is stored, asks the user for a new filename before proceeding.
 *
 * @param parent Parent window for dialogs.
 * @param caveset Caveset to save.
 */
void gd_save_caveset(CaveSet &caveset) {
    if (caveset.filename == "")
        /* if no filename remembered, rather start the save_as function, which asks for one. */
        gd_save_caveset_as(caveset);
    else
        /* if given, save. */
        caveset_save(caveset.filename.c_str(), caveset);
}


/**
 * Pops up a file selection dialog; and loads the caveset selected.
 *
 * Before doing anything, asks the user if he wants to save the current caveset.
 * If it is edited and not saved, this function will do nothing.
 */
void gd_open_caveset(const char *directory, CaveSet &caveset) {
    /* if caveset is edited, and user does not want to discard changes */
    if (!gd_discard_changes(caveset))
        return;

    GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Open File"), guess_active_toplevel(), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("GDash cave sets"));
    for (int i = 0; gd_caveset_extensions[i] != NULL; i++)
        gtk_file_filter_add_pattern(filter, gd_caveset_extensions[i]);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    /* if callback shipped with a directory name, show that directory by default */
    if (directory)
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), directory);
    else if (last_folder != "")
        /* if we previously had an open command, the directory was remembered */
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), last_folder.c_str());
    else
        /* otherwise user home */
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());

    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    std::string filename;
    if (result == GTK_RESPONSE_ACCEPT) {
        filename = gd_tostring_free(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
        last_folder = gd_tostring_free(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog)));
    }

    /* WINDOWS GTK+ 20080926 HACK */
    /* gtk bug - sometimes the above widget destroy creates an error message. */
    /* so we delete the error flag here. */
    /* MacOS GTK+ hack */
    /* well, in MacOS, the file open dialog does report this error: */
    /* "Unable to find default local directory monitor type" - original text */
    /* "Vorgegebener Überwachungstyp für lokale Ordner konnte nicht gefunden werden" - german text */
    /* so better to always clear the error flag. */
    {
        Logger l;
        gtk_widget_destroy(dialog);
        l.clear();
    }

    /* if got a filename, load the file */
    if (filename != "") {
        try {
            caveset = load_caveset_from_file(filename.c_str());
        } catch (std::exception &e) {
            gd_errormessage(_("Error loading caveset."), e.what());
        }
    }
}


/**
 * Convenience function to create a label with centered text.
 * @param markup The text to show (in pango markup format)
 * @return The new GtkLabel.
 */
GtkWidget *gd_label_new_centered(const char *markup) {
    return gtk_widget_new(GTK_TYPE_LABEL, "label", markup, "use-markup", TRUE, "xalign", (double) 0.5, NULL);
}


/**
 * Convenience function to create a label with left aligned text.
 * @param markup The text to show (in pango markup format)
 * @return The new GtkLabel.
 */
GtkWidget *gd_label_new_leftaligned(const char *markup) {
    return gtk_widget_new(GTK_TYPE_LABEL, "label", markup, "use-markup", TRUE, "xalign", (double) 0.0, NULL);
}


/**
 * Convenience function to create a label with right aligned text.
 * @param markup The text to show (in pango markup format)
 * @return The new GtkLabel.
 */
GtkWidget *gd_label_new_rightaligned(const char *markup) {
    return gtk_widget_new(GTK_TYPE_LABEL, "label", markup, "use-markup", TRUE, "xalign", (double) 1.0, NULL);
}


void gd_show_errors(Logger &l, const char *title, bool always_show) {
    if (l.get_messages().empty() && !always_show)
        return;

    GtkWidget *dialog = gtk_dialog_new_with_buttons(title, guess_active_toplevel(), GTK_DIALOG_MODAL, GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 512, 384);
    GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), sw);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    /* get text and show it */
    GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
    GtkWidget *view = gtk_text_view_new_with_buffer(buffer);
    gtk_container_add(GTK_CONTAINER(sw), view);
    g_object_unref(buffer);

    GdkPixbuf *pixbuf_error = gtk_widget_render_icon_pixbuf(view, GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU);
    GdkPixbuf *pixbuf_warning = gtk_widget_render_icon_pixbuf(view, GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_MENU);
    GdkPixbuf *pixbuf_info = gtk_widget_render_icon_pixbuf(view, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU);
    Logger::Container const &messages = l.get_messages();
    for (Logger::ConstIterator error = messages.begin(); error != messages.end(); ++error) {
        GtkTextIter iter;
        gtk_text_buffer_get_iter_at_offset(buffer, &iter, -1);
        if (error->sev <= ErrorMessage::Message)
            gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf_info);
        else if (error->sev <= ErrorMessage::Warning)
            gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf_warning);
        else
            gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf_error);
        gtk_text_buffer_insert(buffer, &iter, error->message.c_str(), -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    }
    g_object_unref(pixbuf_error);
    g_object_unref(pixbuf_warning);
    g_object_unref(pixbuf_info);

    /* set some tags */
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(view), 3);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 6);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view), 6);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    /* shown to the users - forget them. */
    l.clear();
}


/**
 * Creates a small dialog window with the given text and asks the user a yes/no question
 * @param primary The primary (upper) text to show in the dialog.
 * @param secondary The secondary (lower) text to show in the dialog. May be NULL.
 * @return true, if the user answered yes, no otherwise.
 */
bool gd_question_yesno(const char *primary, const char *secondary) {
    GtkWidget *dialog = gtk_message_dialog_new(guess_active_toplevel(), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s", primary);
    if (secondary && !g_str_equal(secondary, ""))
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", secondary);
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return response == GTK_RESPONSE_YES;
}


/**
 * Adds a hint (text) to the lower part of the dialog.
 * Also adds a little INFO icon.
 * Turns off the separator of the dialog, as it looks nicer without.
 */
void gd_dialog_add_hint(GtkDialog *dialog, const char *hint) {
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);
    gtk_box_pack_end(GTK_BOX(gtk_dialog_get_content_area(dialog)), hbox, FALSE, FALSE, 0);
    GtkWidget *label = gd_label_new_centered(hint);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 55);    // to avoid one label setting the window too wide
    gtk_box_pack_start(GTK_BOX(hbox), gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
}


void gd_show_about_info() {
    gtk_show_about_dialog(guess_active_toplevel(), "program-name", "GDash", "license", About::license, "wrap-license", TRUE, "copyright", About::copyright, "authors", About::authors, "version", PACKAGE_VERSION, "comments", _(About::comments), "translator-credits", _(About::translator_credits), "website", About::website, "artists", About::artists, "documenters", About::documenters, NULL);
}






/**
 * Opens a dialog, containing help text.
 * Waits for the user to close dialog.
 *
 * @param help_text The paragraphs of the help text.
 * @param parent Parent widget of the dialog box.
 */
void show_help_window(const helpdata help_text[], GtkWidget *parent) {
    GTKPixbufFactory pf;
    GTKScreen screen(pf, NULL);
    EditorCellRenderer cr(screen, gd_theme);

    /* create text buffer */
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
    gtk_text_buffer_create_tag(buffer, "name", "weight", PANGO_WEIGHT_BOLD, "scale", PANGO_SCALE_LARGE, NULL);
    gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, "scale", PANGO_SCALE_MEDIUM, NULL);
    for (unsigned int i = 0; g_strcmp0(help_text[i].stock_id, HELP_LAST_LINE) != 0; ++i) {
        if (help_text[i].stock_id) {
            GdkPixbuf *pixbuf = gtk_widget_render_icon_pixbuf(parent, help_text[i].stock_id, GTK_ICON_SIZE_LARGE_TOOLBAR);
            gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf);
            gtk_text_buffer_insert(buffer, &iter, " ", -1);
            g_object_unref(pixbuf);
        }

        GdElementEnum element = help_text[i].element;
        if (element != O_NONE) {
            gtk_text_buffer_insert_pixbuf(buffer, &iter, cr.combo_pixbuf_simple(element));
            gtk_text_buffer_insert(buffer, &iter, " ", -1);
            if (help_text[i].heading == NULL) {
                /* add element name only if no other text given */
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, visible_name_no_attribute(element).c_str(), -1, "name", NULL);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            }
        }
        /* some words in big letters */
        if (help_text[i].heading) {
            if (element == O_NONE && i != 0 && help_text[i].stock_id == NULL)
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, _(help_text[i].heading), -1, "name", NULL);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        }
        /* keyboard stuff in bold */
        if (help_text[i].keyname) {
            gtk_text_buffer_insert(buffer, &iter, "  ", -1);
            gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, _(help_text[i].keyname), -1, "bold", NULL);
            gtk_text_buffer_insert(buffer, &iter, "\t", -1);
        }
        if (help_text[i].description) {
            /* the long text */
            gtk_text_buffer_insert(buffer, &iter, gettext(help_text[i].description), -1);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        }
    }

    // TRANSLATORS: Title text capitalization in English
    GtkWidget *dialog = gtk_dialog_new_with_buttons(_("GDash Help"), GTK_WINDOW(parent), GTK_DIALOG_MODAL, GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 512, 384);
    GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content_area), sw, TRUE, TRUE, 0);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    /* get text and show it */
    GtkWidget *view = gtk_text_view_new_with_buffer(buffer);
    g_object_unref(buffer);
    gtk_container_add(GTK_CONTAINER(sw), view);

    /* set some tags */
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(view), 3);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 6);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view), 6);
    PangoTabArray *tabarray = pango_tab_array_new_with_positions(5, FALSE,
        PANGO_TAB_LEFT, 10 * 7 * PANGO_SCALE,
        PANGO_TAB_LEFT, 20 * 7 * PANGO_SCALE,
        PANGO_TAB_LEFT, 30 * 7 * PANGO_SCALE,
        PANGO_TAB_LEFT, 40 * 7 * PANGO_SCALE,
        PANGO_TAB_LEFT, 50 * 7 * PANGO_SCALE);
    gtk_text_view_set_tabs(GTK_TEXT_VIEW(view), tabarray);
    pango_tab_array_free(tabarray);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
