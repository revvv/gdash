/*
 * Copyright (c) 2007, 2008 Czirkos Zoltan <cirix@fw.hu>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include "caveset.h"
#include "c64import.h"
#include "cave.h"
#include "gtk_gfx.h"
#include "gtk_main.h"
#include "config.h"
#include "help.h"
#include "caveobject.h"
#include "settings.h"
#include "gtk_ui.h"
#include "editor-export.h"
#include "editor-widgets.h"
#include "util.h"

#include "editor.h"

static GtkWidget *caveset_popup, *object_list_popup, *drawing_area_popup, *level_scale;
static GtkActionGroup *actions, *actions_edit_tools, *actions_edit_cave, *actions_edit_caveset, *actions_edit_map,
	*actions_edit_random, *actions_edit_object, *actions_edit_one_object, *actions_cave_selector, *actions_toggle,
	*actions_clipboard_paste, *actions_edit_undo, *actions_edit_redo, *actions_clipboard;
GtkWidget *editor_window;

static int action;	/**< activated edit tool, like move, plot, line... can be a gdobject, or there are other indexes which have meanings. */
static int edit_level; /**< level shown in the editor... does not really affect editing */

static int clicked_x, clicked_y, mouse_x, mouse_y;	/**< variables for mouse movement handling */
static gboolean button1_clicked;	/**< true if we got button1 press event, then set to false on release */

static GList *undo_caves=NULL, *redo_caves=NULL;
static gboolean undo_move_flag=FALSE;	/* this is set to false when the mouse is clicked. on any movement, undo is saved and set to true */

static Cave *edited_cave;	/**< cave data actually edited in the editor */
static Cave *rendered_cave;	/**< a cave with all objects rendered, and to be drawn */

static GList *object_clipboard=NULL;	/**< cave object clipboard. */
static GList *cave_clipboard=NULL;	/**< copied caves */


static GtkWidget *scroll_window, *scroll_window_objects;
static GtkWidget *iconview_cavelist, *drawing_area;
static GtkWidget *toolbars;
static GtkWidget *element_button, *fillelement_button;
static GtkWidget *label_coordinate, *label_object, *label_first_element, *label_second_element;
static GHashTable *cave_pixbufs;

static int **gfx_buffer;
static gboolean **object_highlight;

static GtkListStore *object_list;
static GtkWidget *object_tree_view;
static GList *selected_objects=NULL;

/* objects index */
enum {
	INDEX_COLUMN,
	TYPE_PIXBUF_COLUMN,
	ELEMENT_PIXBUF_COLUMN,
	FILL_PIXBUF_COLUMN,
	TEXT_COLUMN,
	POINTER_COLUMN,
	NUM_EDITOR_COLUMNS
};

/* cave index */
enum {
	CAVE_COLUMN,
	NAME_COLUMN,
	PIXBUF_COLUMN,
	NUM_CAVESET_COLUMNS
};


#define FREEHAND -1
#define MOVE -2
#define VISIBLE_SIZE -3

/* edit tools; check variable "action". this is declared here, as it stores the names of cave drawing objects */
static GtkRadioActionEntry action_objects[]={
	{"Move", GD_ICON_EDITOR_MOVE, N_("_Move"), "F1", N_("Move object"), MOVE},
	
	/********** the order of these real cave object should be the same as the GdObject enum ***********/
	{"Plot", GD_ICON_EDITOR_POINT, N_("_Point"), "F2", N_("Draw single element"), POINT},
	{"Line", GD_ICON_EDITOR_LINE, N_("_Line"), "F4", N_("Draw line of elements"), LINE},
	{"Rectangle", GD_ICON_EDITOR_RECTANGLE, N_("_Outline"), "F5", N_("Draw rectangle outline"), RECTANGLE},
	{"FilledRectangle", GD_ICON_EDITOR_FILLRECT, N_("R_ectangle"), "F6", N_("Draw filled rectangle"), FILLED_RECTANGLE},
	{"Raster", GD_ICON_EDITOR_RASTER, N_("Ra_ster"), NULL, N_("Draw raster"), RASTER},
	{"Join", GD_ICON_EDITOR_JOIN, N_("_Join"), NULL, N_("Draw join"), JOIN},
	{"FloodFillReplace", GD_ICON_EDITOR_FILL_REPLACE, N_("_Flood fill"), NULL, N_("Fill by replacing elements"), FLOODFILL_REPLACE},
	{"FloodFillBorder", GD_ICON_EDITOR_FILL_BORDER, N_("_Boundary fill"), NULL, N_("Fill a closed area"), FLOODFILL_BORDER},
	{"Maze", GD_ICON_EDITOR_MAZE, N_("Ma_ze"), NULL, N_("Draw maze"), MAZE},
	{"UnicursalMaze", GD_ICON_EDITOR_MAZE_UNI, N_("U_nicursal maze"), NULL, N_("Draw unicursal maze"), MAZE_UNICURSAL},
	{"BraidMaze", GD_ICON_EDITOR_MAZE_BRAID, N_("Bra_id maze"), NULL, N_("Draw braid maze"), MAZE_BRAID},
	{"RandomFill", GD_ICON_RANDOM_FILL, N_("R_andom fill"), NULL, N_("Draw random elements"), RANDOM_FILL},
	/* end of real cave objects */
	
	{"Freehand", GD_ICON_EDITOR_FREEHAND, N_("F_reehand"), "F3", N_("Draw freely"), FREEHAND},
	{"Visible", GTK_STOCK_ZOOM_FIT, N_("Set _visible size"), NULL, N_("Select visible part of cave during play"), VISIBLE_SIZE},
};


/* forward function declarations */
static void select_cave_for_edit (Cave *);

















/*****************************************
 * OBJECT LIST
 * which shows objects in a cave,
 * and also stores current selection.
 */
/* return a list of selected objects. */

static int
object_list_count_selected()
{
	return g_list_length(selected_objects);
}

static gboolean
object_list_is_any_selected()
{
	return selected_objects!=NULL;
}

static GdObject*
object_list_first_selected()
{
	if (!object_list_is_any_selected())
		return NULL;
	
	return selected_objects->data;
}

/* check if current iter points to the same object as data. if it is, select the iter */
static gboolean
object_list_select_object_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	GdObject *object;

	gtk_tree_model_get (model, iter, POINTER_COLUMN, &object, -1);
	if (object==data) {
		gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (object_tree_view)), iter);
		return TRUE;
	}
	return FALSE;
}

/* check if current iter points to the same object as data. if it is, select the iter */
static gboolean
object_list_unselect_object_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	GdObject *object;

	gtk_tree_model_get (model, iter, POINTER_COLUMN, &object, -1);
	if (object==data) {
		gtk_tree_selection_unselect_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (object_tree_view)), iter);
		return TRUE;
	}
	return FALSE;
}

static void
object_list_clear_selection()
{
	GtkTreeSelection *selection;

	selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(object_tree_view));
	gtk_tree_selection_unselect_all(selection);
}

static void
object_list_add_to_selection(GdObject *object)
{
	gtk_tree_model_foreach (GTK_TREE_MODEL (object_list), (GtkTreeModelForeachFunc) object_list_select_object_func, object);
}

static void
object_list_remove_from_selection(GdObject *object)
{
	gtk_tree_model_foreach (GTK_TREE_MODEL (object_list), (GtkTreeModelForeachFunc) object_list_unselect_object_func, object);
}

static void
object_list_select_one_object(GdObject *object)
{
	object_list_clear_selection();
	object_list_add_to_selection(object);
}


static void
object_list_selection_changed_signal (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeModel *model;
	GList *selected_rows, *siter;
	int x, y;
	int count;
	
	g_list_free(selected_objects);
	selected_objects=NULL;

	/* check all selected objects, and set all selected objects to highlighted */
	selected_rows=gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(object_tree_view)), &model);
	for (siter=selected_rows; siter!=NULL; siter=siter->next) {
		GtkTreePath *path=siter->data;
		GtkTreeIter iter;
		GdObject *object;
		
		gtk_tree_model_get_iter(model, &iter, path);
		gtk_tree_model_get(model, &iter, POINTER_COLUMN, &object, -1);
		selected_objects=g_list_append(selected_objects, object);
	}
	g_list_foreach(selected_rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(selected_rows);
	
	/* object highlight all to false */
	for (y=0; y<rendered_cave->h; y++)
		for (x=0; x<rendered_cave->w; x++)
			object_highlight[y][x]=FALSE;
	
	/* check all selected objects, and set all selected objects to highlighted */
	for (siter=selected_objects; siter!=NULL; siter=siter->next) {
		GdObject *object=siter->data;
		
		for (y=0; y<rendered_cave->h; y++)
			for (x=0; x<rendered_cave->w; x++)
				if (rendered_cave->objects_order[y][x]==object)
					object_highlight[y][x]=TRUE;
		
	}
	
	/* how many selected objects? */
	count=g_list_length(selected_objects);

	/* enable actions */
	gtk_action_group_set_sensitive (actions_edit_one_object, count==1);
	gtk_action_group_set_sensitive (actions_edit_object, count!=0);
	gtk_action_group_set_sensitive (actions_clipboard, count!=0);

	if (count==0) {
		/* NO object selected */
		gtk_label_set_markup (GTK_LABEL (label_object), NULL);
	}
	else if (count==1) {
		/* exactly ONE object is selected */
		char *text;
		GdObject *object=object_list_first_selected();

		text=gd_get_object_description_text (object);	/* to be g_freed */
		gtk_label_set_markup (GTK_LABEL (label_object), text);
		g_free (text);
	}
	else if (count>1)
		/* more than one object is selected */
		gd_label_set_markup_printf (GTK_LABEL (label_object), _("%d objects selected"), count);
}

/* for popup menu, by properties key */
static void
object_tree_view_popup_menu(GtkWidget *widget, gpointer data)
{
	gtk_menu_popup(GTK_MENU(object_list_popup), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

/* for popup menu, by right-click */
static gboolean
object_tree_view_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->type==GDK_BUTTON_PRESS && event->button==3) {
		gtk_menu_popup(GTK_MENU(object_list_popup), NULL, NULL, NULL, NULL, event->button, event->time);
		return TRUE;
	}
	return FALSE;
}













static void
help_cb (GtkWidget *widget, gpointer data)
{
	gd_show_editor_help (editor_window);
}




/****************************************************
 *
 * FUNCTIONS FOR UNDO
 *
 */

static void
redo_free()
{
	g_list_foreach(redo_caves, (GFunc) gd_cave_free, NULL);
	g_list_free(redo_caves);
	redo_caves=NULL;
}

/* delete the cave copies saved for undo. */
static void
undo_free()
{
	redo_free();
	g_list_foreach(undo_caves, (GFunc) gd_cave_free, NULL);
	g_list_free(undo_caves);
	undo_caves=NULL;
}

/* save a copy of the current state of edited cave. this is to be used
   internally; as it does not delete redo. */
#define UNDO_STEPS 10
static void
undo_save_current_state()
{
	/* if more than four, forget some (should only forget one) */
	while (g_list_length(undo_caves) >= UNDO_STEPS) {
		gpointer deleted=g_list_first(undo_caves)->data;
		
		gd_cave_free((Cave *)deleted);
		undo_caves=g_list_remove(undo_caves, deleted);
	}
	
	undo_caves=g_list_append(undo_caves, gd_cave_new_from_cave(edited_cave));
}

/* save a copy of the current state of edited cave, after some operation.
   this destroys the redo list, as from that point that is useless. */
static void
undo_save()
{
	g_return_if_fail(edited_cave!=NULL);

	/* we also use this function to set the edited flag, as it is called for any action */
	gd_caveset_edited=TRUE;
	
	undo_save_current_state();
	redo_free();	
	
	/* now we have a cave to do an undo from, so sensitize the menu item */
	gtk_action_group_set_sensitive (actions_edit_undo, TRUE);
	gtk_action_group_set_sensitive (actions_edit_redo, FALSE);
}

static void
do_undo()
{
	Cave *backup;

	/* revert to last in list */
	backup=(Cave *)g_list_last(undo_caves)->data;
	
	/* push current to redo list. we do not check the redo list size, as the undo size limits it automatically... */
	redo_caves=g_list_prepend(redo_caves, gd_cave_new_from_cave(edited_cave));
	
	/* copy to edited one, and free backup */
	gd_cave_copy(edited_cave, backup);
	gd_cave_free(backup);
	undo_caves=g_list_remove(undo_caves, backup);

	/* call to renew editor window */
	select_cave_for_edit(edited_cave);
}

/* do the undo - callback */
static void
undo_cb(GtkWidget *widget, gpointer data)
{
	g_return_if_fail(undo_caves!=NULL);
	g_return_if_fail(edited_cave!=NULL);
	
	do_undo();
}

/* do the redo - callback */
static void
redo_cb(GtkWidget *widget, gpointer data)
{
	Cave *backup;

	g_return_if_fail(redo_caves!=NULL);
	g_return_if_fail(edited_cave!=NULL);

	/* push back current to undo */
	undo_save_current_state();
	
	/* and select the first from redo */
	backup=g_list_first(redo_caves)->data;
	gd_cave_copy(edited_cave, backup);
	gd_cave_free(backup);
	
	redo_caves=g_list_remove(redo_caves, backup);
	/* call to renew editor window */
	select_cave_for_edit(edited_cave);
}

















/************************************
 * cave_properties
 *
 * edit properties of a cave.
 *
 */
#define GDASH_TYPE "gdash-type"
#define GDASH_VALUE "gdash-value"
#define GDASH_DEFAULT_VALUE "gdash-default-value"

/* every "set to default" button has a list in its user data connected to the clicked signal.
   this list holds a number of widgets, which are to be set to defaults.
   the widgets themselves hold the type and value; and also a pointer to the default value.
 */
static void
set_default(GtkWidget *widget, gpointer data)
{
	GList *rowwidgets=(GList *)data;
	GList *iter;
	
	for (iter=rowwidgets; iter!=NULL; iter=iter->next) {
		gpointer defvalue=g_object_get_data (G_OBJECT (iter->data), GDASH_DEFAULT_VALUE);

		switch (GPOINTER_TO_INT (g_object_get_data (iter->data, GDASH_TYPE))) {
		case GD_TYPE_BOOLEAN:
			gtk_toggle_button_set_active (iter->data, *(gboolean *)defvalue);
			break;
		case GD_TYPE_STRING:
			gtk_entry_set_text (iter->data, (char *)defvalue);
			break;
		case GD_TYPE_INT:
		case GD_TYPE_RATIO:	/* ratio: % in bdcff, integer in editor */
			gtk_spin_button_set_value (iter->data, *(int *)defvalue);
			break;
		case GD_TYPE_PROBABILITY:
			gtk_spin_button_set_value (iter->data, (*(double *) defvalue)*100.0);	/* *100% */
			break;
		case GD_TYPE_ELEMENT:
		case GD_TYPE_EFFECT:	/* effects are also elements; bdcff file is different. */
			gd_element_button_set (iter->data, *(GdElement *) defvalue);
			break;
		case GD_TYPE_COLOR:
			gd_color_combo_set(iter->data, *(GdColor *) defvalue);
			break;
		}
		
	}
}

/* for a destroyed signal; frees a list which is its user data */
static void
free_list(GtkWidget *widget, gpointer data)
{
	g_list_free((GList *) data);
}


static void
cave_properties (Cave *cave, gboolean caveset_only)
{
	GtkWidget *dialog, *notebook, *table=NULL;
	GList *hashkeys, *iter;
	GList *widgets=NULL;
	int i, j, row=0;
	int result;

	dialog=gtk_dialog_new_with_buttons (caveset_only ? _("Cave Set Properties") : _("Cave Properties"), GTK_WINDOW (editor_window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);

	/* tabbed notebook */
	notebook=gtk_notebook_new();
	gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (dialog)->vbox), notebook);

	/* create the entry widgets */
	for (i=0; gd_cave_properties[i].identifier!=NULL; i++)
		if ((!caveset_only || (caveset_only && (gd_cave_properties[i].flags & GD_FOR_CAVESET))) && !(gd_cave_properties[i].flags&GD_DONT_SHOW_IN_EDITOR)) {
			GtkWidget *widget=NULL, *label;
			GtkWidget *button;
			GList *rowwidgets;
			
			if (gd_cave_properties[i].type==GD_TAB) {
				/* create a notebook tab */
				GtkWidget *align=gtk_alignment_new (0.5, 0, 1, 0);
				/* if needed, create notebook page */
				/* create a table which will be the widget inside */
				table=gtk_table_new (1, 1, FALSE);
				gtk_container_set_border_width (GTK_CONTAINER (table), 6);
				gtk_table_set_row_spacings (GTK_TABLE(table), 4);
				/* put the widget in an alignment */
				gtk_container_add (GTK_CONTAINER (align), table);
				/* and to the notebook */
				gtk_notebook_append_page (GTK_NOTEBOOK (notebook), align, gtk_label_new(gettext(gd_cave_properties[i].name)));
				row=0;
				continue;
			}

			g_assert (table!=NULL);
			
			if (gd_cave_properties[i].type==GD_LABEL) {
				/* create a label. */
				label=gd_label_new_printf(gettext(gd_cave_properties[i].name));
				gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
				gtk_table_attach(GTK_TABLE(table), label, 0, 7, row, row+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
				row++;
				continue;
			}

			if (gd_cave_properties[i].type==GD_LEVEL_LABEL) {
				int i=0;
				for (i=0; i<5; i++)
					gtk_table_attach(GTK_TABLE(table), gd_label_new_printf_centered(_("Level %d"), i+1), i+2, i+3, row, row+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 3, 0);
				row++;
				continue;
			}

			/* name of setting */
			label=gtk_label_new (gettext (gd_cave_properties[i].name));
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
			rowwidgets=NULL;
			for (j=0; j < gd_cave_properties[i].count; j++) {
				gpointer value=G_STRUCT_MEMBER_P (cave, gd_cave_properties[i].offset);
				gpointer defpoint=G_STRUCT_MEMBER_P (gd_default_cave, gd_cave_properties[i].offset);
				char *defval=NULL;
				char *tip;

				switch (gd_cave_properties[i].type) {
				case GD_LEVEL_LABEL:
				case GD_TAB:
				case GD_LABEL:
					/* handled above */
					g_assert_not_reached();
					break;
				case GD_TYPE_BOOLEAN:
					value=((gboolean *) value) + j;
					defpoint=((gboolean *) defpoint) + j;
					widget=gtk_check_button_new ();
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), *(gboolean *) value);
					defval=g_strdup (*(gboolean *)defpoint ? _("Yes") : _("No"));
					break;
				case GD_TYPE_STRING:
					value=((GdString *) value) + j;
					defpoint=((GdString *) defpoint) + j;
					widget=gtk_entry_new ();
					/* little inconsistency below: max length has unicode characters, while gdstring will have utf-8.
					   however this does not make too much difference */
					gtk_entry_set_max_length(GTK_ENTRY(widget), sizeof(GdString));
					gtk_entry_set_text (GTK_ENTRY (widget), (char *) value);
					defval=NULL;
					break;
				case GD_TYPE_RATIO:
				case GD_TYPE_INT:
					value=((int *) value) + j;
					defpoint=((int *) defpoint) + j;
					widget=gtk_spin_button_new_with_range (gd_cave_properties[i].min, gd_cave_properties[i].max, 1);
					gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), *(int *) value);
					defval=g_strdup_printf ("%d", *(int *) defpoint);
					break;
				case GD_TYPE_PROBABILITY:
					value=((double *) value)+j;
					defpoint=((double *) defpoint)+j;
					widget=gtk_spin_button_new_with_range (0.0, 100.0, 0.001);
					gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), *(double *) value*100.0);	/* *100% */
					defval=g_strdup_printf ("%.1f%%", *(double *) defpoint*100.0);
					break;
				case GD_TYPE_EFFECT:	/* effects also specify elements; only difference is bdcff. */
				case GD_TYPE_ELEMENT:
					value=((GdElement *) value) + j;
					defpoint=((GdElement *) defpoint) + j;
					widget=gd_element_button_new (*(GdElement *) value);
					defval=g_strdup_printf ("%s", gettext (gd_elements[*(GdElement *) defpoint].name));
					break;
				case GD_TYPE_COLOR:
					value=((GdColor *) value) + j;
					defpoint=((GdColor *) defpoint) + j;
					widget=gd_color_combo_new (*(GdColor *) value);
					defval=g_strdup_printf ("%s", gd_get_color_name(*(GdColor *) defpoint));
					break;
				case GD_TYPE_DIRECTION:
					value=((GdDirection *) value)+j;
					defpoint=((GdDirection *) defpoint) + j;
					widget=gd_direction_combo_new(*(GdDirection *) value);
					defval=g_strdup_printf("%s", gd_direction_name[*(GdDirection *)defpoint]);
					break;
				}
				/* put widget into list so values can be extracted later */
				widgets=g_list_prepend (widgets, widget);
				rowwidgets=g_list_prepend (rowwidgets, widget);
				g_object_set_data (G_OBJECT (widget), GDASH_TYPE, GINT_TO_POINTER (gd_cave_properties[i].type));
				g_object_set_data (G_OBJECT (widget), GDASH_VALUE, value);
				g_object_set_data (G_OBJECT (widget), GDASH_DEFAULT_VALUE, defpoint);
				/* put widget to table */
				gtk_table_attach(GTK_TABLE(table), widget, j+2, (gd_cave_properties[i].count==1) ? 7 : j + 3, row, row+1, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_SHRINK, 3, 0);
				if (gd_cave_properties[i].tooltip)
					tip=g_strdup_printf (_("%s\nDefault value: %s"), gettext (gd_cave_properties[i].tooltip), defval ? defval : N_("empty"));
				else
					tip=g_strdup_printf ("Default value: %s", defval ? defval : N_("empty"));
				g_free (defval);
				gtk_widget_set_tooltip_text(widget, tip);
				g_free (tip);
			}
			
			if (gd_cave_properties[i].type!=GD_TYPE_STRING) {
				button=gtk_button_new();
				gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
				gtk_widget_set_tooltip_text(button, _("Set to default value"));
				gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
				g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(set_default), rowwidgets);
				/* this will free the list when the button is destroyed */
				g_signal_connect (G_OBJECT (button), "destroy", G_CALLBACK(free_list), rowwidgets);
				gtk_table_attach(GTK_TABLE(table), button, 1, 2, row, row+1, GTK_SHRINK, GTK_SHRINK, 3, 0);
			} else
				g_list_free(rowwidgets);
			 			
			row++;
		}
	
	/* add unknown tags to the last table - XXX */
	hashkeys=g_hash_table_get_keys(cave->tags);
	for (iter=hashkeys; iter!=NULL; iter=iter->next) {
		GtkWidget *label, *entry;
		gchar *key=(gchar *)iter->data;
		/* name of setting */
		label=gtk_label_new (key);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row+1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
		
		entry=gtk_entry_new();
		gtk_entry_set_text(GTK_ENTRY(entry), (char *) g_hash_table_lookup(cave->tags, key));
		widgets=g_list_prepend (widgets, entry);
		g_object_set_data (G_OBJECT (entry), GDASH_TYPE, GINT_TO_POINTER (-1));
		g_object_set_data (G_OBJECT (entry), GDASH_VALUE, key);
		gtk_table_attach(GTK_TABLE(table), entry, 2, 3, row, row+1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_SHRINK, 3, 0);
		row++;
	}
	g_list_free(hashkeys);

	/* running the dialog */
	gtk_widget_show_all (dialog);

	result=gtk_dialog_run (GTK_DIALOG (dialog));

	/* getting data */
	if (result==GTK_RESPONSE_ACCEPT) {
		int old_w, old_h;
		GList *iter;
		
		/* if changing cave things, save original for undo */
		if (!caveset_only)
			undo_save();
		else
			/* otherwise editing caveset properties, and we set the caveset edited flag to on. for caves, it is set by undo_save! */
			gd_caveset_edited=TRUE;

		/* remember old size, as the cave map might have to be resized */
		old_w=cave->w;
		old_h=cave->h;

		/* read values from different spin buttons and ranges etc */
		for (iter=widgets; iter; iter=g_list_next (iter)) {
			gpointer value=g_object_get_data (G_OBJECT (iter->data), GDASH_VALUE);

			switch (GPOINTER_TO_INT (g_object_get_data (iter->data, GDASH_TYPE))) {
			case -1:	/* from hash table */
				g_hash_table_insert(cave->tags, value, g_strdup(gtk_entry_get_text(GTK_ENTRY(iter->data))));
				break;
			case GD_TYPE_BOOLEAN:
				*(gboolean *) value=gtk_toggle_button_get_active (iter->data);
				break;
			case GD_TYPE_STRING:
				g_strlcpy((char *) value, gtk_entry_get_text(iter->data), sizeof(GdString));
				break;
			case GD_TYPE_INT:
				*(int *) value=gtk_spin_button_get_value_as_int(iter->data);
				break;
			case GD_TYPE_RATIO:
				*(int *) value=gtk_spin_button_get_value_as_int(iter->data);
				if (*(int *) value >= cave->w*cave->h)
					*(int *) value=cave->w*cave->h;
				break;
			case GD_TYPE_PROBABILITY:
				*(double *) value=gtk_spin_button_get_value(iter->data)/100.0;	/* /100% */
				break;
			case GD_TYPE_ELEMENT:
			case GD_TYPE_EFFECT:	/* effects are also elements; bdcff file is different. */
				*(GdElement *) value=gd_element_button_get(iter->data);
				break;
			case GD_TYPE_COLOR:
				*(GdColor *) value=gd_color_combo_get_color(iter->data);
				break;
			case GD_TYPE_DIRECTION:
				*(GdDirection *) value=gd_direction_combo_get(iter->data);
			}
		}

		/* if cave has a map, resize it also */
		if (cave->map && (old_w!=cave->w || old_h!=cave->h)) {
			/* create new map, with the new sizes */
			GdElement **new_map=gd_cave_map_new(cave, GdElement);
			int minx=MIN(old_w, cave->w);
			int miny=MIN(old_h, cave->h);
			int x, y;

			/* default value is the same as the border */
			for (y=0; y<cave->h; y++)
				for (x=0; x<cave->w; x++)
					new_map[y][x]=cave->initial_border;
			/* make up the new map - either the old or the new is smaller */
			for (y=0; y<miny; y++)
				for (x=0; x<minx; x++)
					/* copy values from rendered cave. */
					new_map[y][x]=gd_cave_get_rc(rendered_cave, x, y);
			gd_cave_map_free(cave->map);
			cave->map=new_map;
		}

		/* ensure update of drawing window and title. also the list store! */
		/* but only if not editing the default values */
		if (!caveset_only)
			select_cave_for_edit (cave);
	}
	g_list_free (widgets);
	/* this destroys everything inside the notebook also */
	gtk_widget_destroy (dialog);
	
	/* do some validation */
	gd_cave_correct_visible_size(cave);
}
#undef GDASH_TYPE
#undef GDASH_VALUE
#undef GDASH_DEFAULT_VALUE











/*
	render cave - ie. draw it as a map,
	so it can be presented to the user.
*/
static void
render_cave ()
{
	int i;
	int x, y;
	GList *iter;
	GList *selected;
	g_return_if_fail (edited_cave!=NULL);

	gd_cave_free (rendered_cave);
	rendered_cave=gd_cave_new_rendered (edited_cave, edit_level);
	/* create a gfx buffer for displaying */
	gd_cave_map_free(gfx_buffer);
	gd_cave_map_free(object_highlight);
	gfx_buffer=gd_cave_map_new (rendered_cave, int);
	object_highlight=gd_cave_map_new (rendered_cave, gboolean);
	for (y=0; y<rendered_cave->h; y++)
		for (x=0; x<rendered_cave->w; x++) {
			gfx_buffer[y][x]=-1;
			object_highlight[y][x]=FALSE;
		}

	/* fill object list store with the objects. */
	/* save previous selection to a list */
	selected=g_list_copy(selected_objects);
	
	gtk_list_store_clear(object_list);
	for (iter=edited_cave->objects, i=0; iter; iter=g_list_next (iter), i++) {
		GdObject *object=iter->data;
		GtkTreeIter treeiter;
		GdkPixbuf *fillelement;
		gchar *text;

		text=gd_get_object_coordinates_text (object);
		switch(object->type) {
			case FILLED_RECTANGLE:
			case JOIN:
			case FLOODFILL_BORDER:
			case FLOODFILL_REPLACE:
			case MAZE:
			case MAZE_UNICURSAL:
			case MAZE_BRAID:
				fillelement=gd_get_element_pixbuf_with_border (object->fill_element);
				break;
			default:
				fillelement=NULL;
				break;
		}

		/* use atomic insert with values */
		gtk_list_store_insert_with_values (object_list, &treeiter, i, INDEX_COLUMN, i, TYPE_PIXBUF_COLUMN, action_objects[object->type].stock_id, ELEMENT_PIXBUF_COLUMN, gd_get_element_pixbuf_with_border (object->element), FILL_PIXBUF_COLUMN, fillelement, TEXT_COLUMN, text, POINTER_COLUMN, object, -1);

		/* also do selection as now we have the iter in hand */
		if (g_list_index(selected, object)!=-1)
			gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (object_tree_view)), &treeiter);
		g_free (text);
	}
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW (object_tree_view));
	g_list_free(selected);

	gtk_action_group_set_sensitive (actions_edit_map, edited_cave->map!=NULL);
	gtk_action_group_set_sensitive (actions_edit_random, edited_cave->map==NULL);
}


/* this is the same scroll routine as the one used for the game. only the parameters are changed a bit. */
static void
scroll (int player_x, int player_y)
{
	static int scroll_desired_x=0, scroll_desired_y=0;
	static int scroll_speed_x=0, scroll_speed_y=0;
	GtkAdjustment *adjustment;
	int scroll_center_x, scroll_center_y;
	int i;
	/* hystheresis size is this, multiplied by two. */
	int scroll_start_x=scroll_window->allocation.width/2-2*cell_size;
	int scroll_start_y=scroll_window->allocation.height/2-2*cell_size;
	
	/* get the size of the window so we know where to place player.
	 * first guess is the middle of the screen.
	 * drawing_area->parent->parent is the viewport.
	 * +cellsize/2 gets the stomach of player :) so the very center */
	scroll_center_x=player_x * cell_size + cell_size/2 - drawing_area->parent->parent->allocation.width/2;
	scroll_center_y=player_y * cell_size + cell_size/2 - drawing_area->parent->parent->allocation.height/2;

	/* HORIZONTAL */
	/* hystheresis function.
	 * when scrolling left, always go a bit less left than player being at the middle.
	 * when scrolling right, always go a bit less to the right. */
	adjustment=gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (scroll_window));
	if (adjustment->value + scroll_start_x < scroll_center_x)
		scroll_desired_x=scroll_center_x - scroll_start_x;
	if (adjustment->value - scroll_start_x > scroll_center_x)
		scroll_desired_x=scroll_center_x + scroll_start_x;

	scroll_desired_x=CLAMP (scroll_desired_x, 0, adjustment->upper - adjustment->step_increment - adjustment->page_increment);
	/* adaptive scrolling speed.
	 * gets faster with distance.
	 * minimum speed is 1, to allow scrolling precisely to the desired positions (important at borders).
	 */
	if (scroll_speed_x < ABS (scroll_desired_x - adjustment->value)/12 + 1)
		scroll_speed_x++;
	if (scroll_speed_x > ABS (scroll_desired_x - adjustment->value)/12 + 1)
		scroll_speed_x--;
	if (adjustment->value < scroll_desired_x) {
		for (i=0; i < scroll_speed_x; i++)
			if (adjustment->value < scroll_desired_x)
				adjustment->value++;
		gtk_adjustment_value_changed (adjustment);
	}
	if (adjustment->value > scroll_desired_x) {
		for (i=0; i < scroll_speed_x; i++)
			if (adjustment->value > scroll_desired_x)
				adjustment->value--;
		gtk_adjustment_value_changed (adjustment);
	}

	/* VERTICAL */
	adjustment=gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (scroll_window));
	if (adjustment->value + scroll_start_y < scroll_center_y)
		scroll_desired_y=scroll_center_y - scroll_start_y;
	if (adjustment->value - scroll_start_y > scroll_center_y)
		scroll_desired_y=scroll_center_y + scroll_start_y;

	scroll_desired_y=CLAMP (scroll_desired_y, 0, adjustment->upper - adjustment->step_increment - adjustment->page_increment);
	if (scroll_speed_y < ABS (scroll_desired_y - adjustment->value)/12 + 1)
		scroll_speed_y++;
	if (scroll_speed_y > ABS (scroll_desired_y - adjustment->value)/12 + 1)
		scroll_speed_y--;
	if (adjustment->value < scroll_desired_y) {
		for (i=0; i < scroll_speed_y; i++)
			if (adjustment->value < scroll_desired_y)
				adjustment->value++;
		gtk_adjustment_value_changed (adjustment);
	}
	if (adjustment->value > scroll_desired_y) {
		for (i=0; i < scroll_speed_y; i++)
			if (adjustment->value > scroll_desired_y)
				adjustment->value--;
		gtk_adjustment_value_changed (adjustment);
	}
}


/* timeout 'interrupt', drawing cave in cave editor. */
static gboolean
drawcave_int (const gpointer data)
{
	static int animcycle=0;
	static int player_blinking=0;
	static int cursor=0;
	int x, y, draw;

	/* if nothing to draw or nowhere to draw :) exit.
	 * this is necessary as the interrupt is not uninstalled when the selector is running. */
	if (!drawing_area || !rendered_cave)
		return TRUE;

	g_return_val_if_fail (gfx_buffer!=NULL, TRUE);

	/* when mouse over a drawing object, cursor changes */
	if (mouse_x >= 0 && mouse_y >= 0) {
		int new_cursor=rendered_cave->objects_order[mouse_y][mouse_x] ? GDK_HAND1 : -1;
		if (cursor!=new_cursor) {
			cursor=new_cursor;
			gdk_window_set_cursor (drawing_area->window, cursor==-1?NULL:gdk_cursor_new (cursor));
		}
	}

	/* only do cell animations when window is active.
	 * otherwise... user is testing the cave, animation would just waste cpu. */
	if (gtk_window_has_toplevel_focus (GTK_WINDOW (editor_window)))
		animcycle=(animcycle+1) & 7;

	if (animcycle==0)			/* player blinking is started at the beginning of animation sequences. */
		player_blinking=g_random_int_range (0, 4)==0;	/* 1/4 chance of blinking, every sequence. */

	for (y=0; y<rendered_cave->h; y++) {
		for (x=0; x<rendered_cave->w; x++) {
			if (gd_game_view)
				draw=gd_elements[rendered_cave->map[y][x]].image_simple;
			else
				draw=gd_elements[rendered_cave->map[y][x]].image;
			/* special case is player - sometimes blinking :) */
			if (player_blinking && (rendered_cave->map[y][x]==O_INBOX))
				draw=gd_elements[O_PLAYER_BLINK].image_simple;
			/* the biter switch also shows its state */
			if (rendered_cave->map[y][x]==O_BITER_SWITCH)
				draw=gd_elements[O_BITER_SWITCH].image_simple+rendered_cave->biter_delay_frame;

			/* negative value means animation */
			if (draw<0)
				draw=-draw + animcycle;

			/* object coloring */
			if (action==VISIBLE_SIZE) {
				/* if showing visible size, different color applies for: */
				if (x>=rendered_cave->x1 && x<=rendered_cave->x2 && y>=rendered_cave->y1 && y<=rendered_cave->y2)
					draw+=NUM_OF_CELLS;
				if (x==rendered_cave->x1 || x==rendered_cave->x2 || y==rendered_cave->y1 || y==rendered_cave->y2)
					draw+=NUM_OF_CELLS;	/* once again */
			} else {
				if (object_highlight[y][x]) /* if it is a selected object, make it colored */
					draw+=2*NUM_OF_CELLS;
				else if (gd_colored_objects && rendered_cave->objects_order[y][x]!=NULL)
					/* if it belongs to any other element, make it colored a bit */
					draw+=NUM_OF_CELLS;
			}
			
			/* the drawing itself */
			if (gfx_buffer[y][x]!=draw) {
				gdk_draw_drawable(drawing_area->window, drawing_area->style->black_gc, cells[draw], 0, 0, x*cell_size, y*cell_size, cell_size, cell_size);
				gfx_buffer[y][x]=draw;
			}
			
			/* for fill objects, we show their origin */	
			if (object_highlight[y][x]
				&& (((GdObject *)rendered_cave->objects_order[y][x])->type==FLOODFILL_BORDER
					|| ((GdObject *)rendered_cave->objects_order[y][x])->type==FLOODFILL_REPLACE)) {
				GdObject *object=rendered_cave->objects_order[y][x];
				int x=object->x1;
				int y=object->y1;

				/* only draw if inside bounds */
				if (x>=0 && x<rendered_cave->w && y>=0 && y<rendered_cave->h) {
					gdk_draw_rectangle (drawing_area->window, drawing_area->style->white_gc, FALSE, x*cell_size, y*cell_size, cell_size - 1, cell_size - 1);
					gdk_draw_line (drawing_area->window, drawing_area->style->white_gc, x*cell_size, y*cell_size, (x+1)*cell_size-1, (y+1)*cell_size-1);
					gdk_draw_line (drawing_area->window, drawing_area->style->white_gc, x*cell_size, (y+1)*cell_size-1, (x+1)*cell_size-1, y*cell_size);
					gfx_buffer[object->y1][object->x1]=-1;
				}
			}
		}
	}

	if (mouse_x>=0 && mouse_y>=0) {
		/* this is the cell the mouse is over */
		gdk_draw_rectangle (drawing_area->window, drawing_area->style->white_gc, FALSE, mouse_x*cell_size, mouse_y*cell_size, cell_size-1, cell_size-1);
		/* always redraw this cell the next frame - the easiest way to always get rid of the rectangle when the mouse is moved */
		gfx_buffer[mouse_y][mouse_x]=-1;
	}

	if (mouse_x>=0 && mouse_y>=0 && button1_clicked)
		scroll(mouse_x, mouse_y);

	return TRUE;
}

/* cave drawing area expose event.
*/
static gboolean
drawing_area_expose_event (const GtkWidget *widget, const GdkEventExpose *event, const gpointer data)
{
	int x, y, x1, y1, x2, y2;

	if (!widget->window || gfx_buffer==NULL)
		return FALSE;

	x1=event->area.x/cell_size;
	y1=event->area.y/cell_size;
	x2=(event->area.x+event->area.width-1)/cell_size;
	y2=(event->area.y+event->area.height-1)/cell_size;

	for (y=y1; y<=y2; y++)
		for (x=x1; x<=x2; x++)
			if (gfx_buffer[y][x]!=-1)
				gdk_draw_drawable (drawing_area->window, drawing_area->style->black_gc, cells[gfx_buffer[y][x]], 0, 0, x*cell_size, y*cell_size, cell_size, cell_size);
	return TRUE;
}







/*****************************************************
 *
 * spin button with instant update
 *
 */
#define GDASH_DATA_POINTER "gdash-data-pointer"
static void
spinbutton_changed(GtkWidget *widget, gpointer data)
{
	int value=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
	int *pi;
	
	pi=g_object_get_data(G_OBJECT(widget), GDASH_DATA_POINTER);
	if (*pi!=value) {
		*pi=value;
		render_cave();
	}
}

static GtkWidget *
spin_button_new_with_update(int min, int max, int *value)
{
	GtkWidget *spin;
	
	/* change range if needed */
	if (*value<min) min=*value;
	if (*value>max) max=*value;
	
	spin=gtk_spin_button_new_with_range (min, max, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), *value);
	g_object_set_data(G_OBJECT(spin), GDASH_DATA_POINTER, value);
	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(spinbutton_changed), NULL);
	
	return spin;
}

/*****************************************************
 *
 * element button with instant update
 *
 */
static void
elementbutton_changed(GtkWidget *widget, gpointer data)
{
	GdElement *elem=g_object_get_data(G_OBJECT(widget), GDASH_DATA_POINTER);
	GdElement new_elem;
	
	new_elem=gd_element_button_get(widget);
	if (*elem!=new_elem) {
		*elem=new_elem;
		render_cave();
	}
}

static GtkWidget *
element_button_new_with_update(GdElement *value)
{
	GtkWidget *button;
	
	button=gd_element_button_new(*value);
	g_object_set_data(G_OBJECT(button), GDASH_DATA_POINTER, value);
	g_signal_connect(button, "clicked", G_CALLBACK(elementbutton_changed), NULL);

	return button;
}
#undef GDASH_DATA_POINTER



/*****************************************************
 *
 * a new hscale which switches level
 *
 */
static void
random_setup_level_scale_changed(GtkWidget *widget, gpointer data)
{
	int value=gtk_range_get_value(GTK_RANGE(widget));
	gtk_range_set_value(GTK_RANGE(level_scale), value);
}

static GtkWidget *
hscale_new_switches_level()
{
	GtkWidget *scale;
	
	scale=gtk_hscale_new_with_range (1.0, 5.0, 1.0);
	gtk_range_set_value(GTK_RANGE(scale), edit_level+1);	/* internally we use level 0..4 */
	gtk_scale_set_digits (GTK_SCALE (scale), 0);
	gtk_scale_set_value_pos (GTK_SCALE (scale), GTK_POS_LEFT);
	g_signal_connect(G_OBJECT(scale), "value-changed", G_CALLBACK(random_setup_level_scale_changed), NULL);
	
	return scale;
}

/*****************************************************
 *
 * add a callback to a spin button's "focus in" so it updates the level hscale
 *
 */
#define GDASH_LEVEL_NUMBER "gdash-level-number"
static void
widget_focus_in_set_level_cb(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	int value=GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), GDASH_LEVEL_NUMBER));
	gtk_range_set_value(GTK_RANGE(data), value);
}

static void
widget_focus_in_set_level(GtkWidget *spin, GtkScale *scale, int level)
{
	g_object_set_data(G_OBJECT(spin), GDASH_LEVEL_NUMBER, GINT_TO_POINTER(level));
	gtk_widget_add_events(spin, GDK_FOCUS_CHANGE_MASK);
	g_signal_connect(spin, "focus-in-event", G_CALLBACK(widget_focus_in_set_level_cb), scale);
}
#undef GDASH_LEVEL_NUMBER








/****************************************************/
#define GDASH_SPIN_PROBABILITIES "gdash-spin-probabilities"

static gboolean
random_setup_da_expose (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	int *pprob=(int *)data;
	int fill[256];
	int i, j;
	
	if (!widget->window)
		return TRUE;

	for (j=0; j<256; j++)
		fill[j]=4;	/* for initial fill */
	for (i=0; i<4; i++)
		for (j=0; j<pprob[i]; j++)
			fill[j]=i;	/* random fill no. i */

	/* draw 256 small vertical lines with the color needed */
	for (j=0; j<256; j++) {
		gdk_draw_line(widget->window, widget->style->fg_gc[fill[j]], j*2, 0, j*2, 15);	
		gdk_draw_line(widget->window, widget->style->fg_gc[fill[j]], j*2+1, 0, j*2+1, 15);	
	}
	return TRUE;
}

static GtkWidget *scales[4];

#define GDASH_DATA_POINTER "gdash-data-pointer"
static void
random_setup_probability_scale_changed(GtkWidget *widget, gpointer data)
{
	int *pprob;	/* all 4 values */
	int i,j;
	gboolean changed;
	
	pprob=g_object_get_data(G_OBJECT(widget), GDASH_SPIN_PROBABILITIES);
	
	/* as probabilites take precedence over each other, set them in a way that the user sees it. */
	/* this way, the scales move on their own sometimes */
	changed=FALSE;
	for(i=0; i<4; i++)
		for (j=i+1; j<4; j++)
			if (pprob[j]>pprob[i]) {
				changed=TRUE;
				pprob[i]=pprob[j];
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(scales[i]), pprob[i]);
			}
	if (changed)
		render_cave();
	
	/* redraw color bars and re-render cave */
	gtk_widget_queue_draw(GTK_WIDGET(data));
}

/* the small color "icons" which tell the user which color corresponds to which element */
static gboolean
random_setup_fill_color_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	if (widget->window) {
		GtkStyle *style;

		style=gtk_widget_get_style (widget);

		gdk_draw_rectangle (widget->window, style->bg_gc[GTK_STATE_NORMAL], TRUE, event->area.x, event->area.y, event->area.width, event->area.height);
	}

	return TRUE;
}


/* parent: parent window
   pborder: pointer to initial border
   pinitial: pointer to initial fill
   pfill: pointer to random_fill[4]
   pprob: pointer to random_fill_probability[4]
   pseed: pointer to random seed[5]
 */
 
static void
random_setup_widgets_to_table(GtkTable *table, int firstrow, GdElement *pborder, GdElement *pinitial, GdElement *pfill, int *pprob, int *pseed)
{
	int i, row;
	GtkWidget *da, *align, *frame, *spin, *label, *wid;
	GtkWidget *scale;
	const unsigned int cols[]={ 0xffff59, 0x59ffac, 0x5959ff, 0xff59ac, 0x000000 };

	row=firstrow;
	/* drawing area, which shows the portions of elements. */
	da=gtk_drawing_area_new();
	gtk_widget_set_size_request(da, 512, 16);
	g_signal_connect(G_OBJECT(da), "expose-event", G_CALLBACK(random_setup_da_expose), pprob);
	frame=gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(frame), da);
	align=gtk_alignment_new(0.5, 0.5, 0, 0);
	gtk_container_add(GTK_CONTAINER(align), frame);

	/* five random seed spin buttons. */
	if (pseed) {
		/* scale which sets the level1..5 shown - only if we also draw the random seed buttons */
		gtk_table_attach(table, gd_label_new_printf(_("Level shown")), 0, 1, row, row+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
		scale=hscale_new_switches_level();
		gtk_table_attach_defaults(table, scale, 1, 3, row, row+1);
		row++;

		for (i=0; i<5; i++) {
			spin=spin_button_new_with_update(-1, 255, &pseed[i]);
			widget_focus_in_set_level(spin, GTK_SCALE(scale), i+1);

			gtk_table_attach(table, gd_label_new_printf(_("Random seed %d"), i+1), 0, 1, row, row+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
			gtk_table_attach_defaults(table, spin, 1, 3, row, row+1);

			row++;
		}

		gtk_table_attach_defaults(table, gtk_hseparator_new(), 0, 3, row, row+1);
		row++;
	}
	
	/* label */
	if (pborder) {
		gtk_table_attach(table, gd_label_new_printf(_("Initial border")), 0, 1, row, row+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
		gtk_table_attach_defaults(table, element_button_new_with_update(pborder), 1, 2, row, row+1);
		row++;
	}
	
	/* five rows: four random elements + one initial fill */
	for (i=4; i>=0; i--) {
		GtkWidget *colorbox, *frame, *hbox;
		GdkColor color;
		GdElement *pelem;

		color.red=(cols[i] >> 16)<<8;
		color.green=(cols[i] >> 8)<<8;
		color.blue=(cols[i] >> 0)<<8;
		
		gtk_widget_modify_fg (da, i, &color);	/* set fg color[i] for big drawing area */

		hbox=gtk_hbox_new(FALSE, 6);

		/* label */
		if (i==4)
			label=gd_label_new_printf(_("Initial fill"));
		else
			label=gd_label_new_printf(_("Random fill %d"), 4-i);
		gtk_box_pack_start_defaults(GTK_BOX(hbox), label);

		/* small drawing area with only one color */
		colorbox=gtk_drawing_area_new();
		gtk_widget_set_size_request(colorbox, 16, 16);
		gtk_widget_modify_bg (colorbox, GTK_STATE_NORMAL, &color);
		g_signal_connect(G_OBJECT(colorbox), "expose-event", G_CALLBACK(random_setup_fill_color_expose), NULL);

		frame=gtk_frame_new (NULL);
		gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
		gtk_container_add(GTK_CONTAINER(frame), colorbox);
		gtk_box_pack_end(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
		gtk_table_attach(table, hbox, 0, 1, row, row+1, GTK_FILL|GTK_SHRINK, GTK_SHRINK, 0, 0);


		/* element button */		
		pelem=(i==4)?pinitial:&(pfill[i]);
		wid=element_button_new_with_update(pelem);
		gtk_table_attach_defaults(table, wid, 1, 2, row, row+1);
		
		/* probability control, if this row is not for the initial fill */
		if (i!=4) {
			int *value=&(pprob[i]);	/* pointer to integer value to modify */
			
			wid=spin_button_new_with_update(0, 255, value);
			scales[i]=wid;
			g_object_set_data(G_OBJECT(wid), GDASH_SPIN_PROBABILITIES, pprob);
			g_signal_connect(wid, "value-changed", G_CALLBACK(random_setup_probability_scale_changed), da);	/* spin button updates color bars */
			gtk_table_attach_defaults(table, wid, 2, 3, row, row+1);
		}
		
		row++;
	}

	/* attach the drawing area for color bars - this gives the user a hint about the ratio of elements */
	row++;
	gtk_table_attach(table, align, 0, 3, row, row+1, GTK_SHRINK, GTK_SHRINK, 0, 0);
}
#undef GDASH_DATA_POINTER








/* select cave edit tool.
   this activates the gtk action in question, so
   toolbar and everything else will be updated */
static void
select_tool(int tool)
{
	int i;
	
	/* find specific object type in action_objects array. */
	/* (the array indexes and tool integers do not correspond) */
	for (i=0; i<G_N_ELEMENTS(action_objects); i++)
		if (tool==action_objects[i].value)
			gtk_action_activate (gtk_action_group_get_action (actions_edit_tools, action_objects[i].name));
}










/***************************************************
 *
 * OBJECT_PROPERTIES
 *
 * edit properties of a cave drawing object.
 *
 */

#define GDASH_LEVEL_NUMBER "gdash-level-number"
#define GDASH_DATA_POINTER "gdash-data-pointer"
static void
check_button_level_toggled(GtkWidget *widget, gpointer data)
{
	gboolean current=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	GdObjectLevels *levels=g_object_get_data(G_OBJECT(widget), GDASH_DATA_POINTER);
	int level=GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), GDASH_LEVEL_NUMBER));
	
	if (current) /* true */
		*levels=*levels | gd_levels_mask[level-1];
	else
		*levels=*levels & ~gd_levels_mask[level-1];
	render_cave();
}

static GtkWidget *
check_button_new_level_enable(GdObjectLevels *levels, int level)
{
	GtkWidget *check;
	char s[20];
	
	g_snprintf(s, sizeof(s), "%d", level);
	check=gtk_check_button_new_with_label(s);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), (*levels & gd_levels_mask[level-1])!=0);
	g_object_set_data(G_OBJECT(check), GDASH_LEVEL_NUMBER, GINT_TO_POINTER(level));
	g_object_set_data(G_OBJECT(check), GDASH_DATA_POINTER, levels);
	g_signal_connect(G_OBJECT(check), "toggled", G_CALLBACK(check_button_level_toggled), NULL);
	
	return check;
}
#undef GDASH_LEVEL_NUMBER
#undef GDASH_DATA_POINTER

static void
object_properties (GdObject *object)
{
	GtkWidget *dialog, *table, *hbox, *scale;
	int i=0;
	int n;
	char *title;
	int result;

	if (object==NULL) {
		g_return_if_fail(object_list_count_selected()==1);
		object=object_list_first_selected();	/* select first from list... should be only one selected */
	}

	title=g_strdup_printf (_("%s Properties"), _(gd_object_description[object->type].name));
	dialog=gtk_dialog_new_with_buttons (title, GTK_WINDOW (editor_window), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	g_free (title);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	table=gtk_table_new (1, 3, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 6);
	gtk_table_set_row_spacings (GTK_TABLE(table), 6);
	gtk_table_set_col_spacings (GTK_TABLE(table), 6);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table, FALSE, FALSE, 0);

	/* i is the row in gtktable, where the label and value is placed.
	 * then i is incremented. this is to avoid empty lines. the table
	 * seemed nicer than hboxes. and it expands automatically. */

	/* LEVEL INFO ****************/
	/* hscale which selects current level shown */
	gtk_table_attach(GTK_TABLE(table), gd_label_new_printf(_("Level currently shown")), 0, 1, i, i+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
	scale=hscale_new_switches_level();
	gtk_table_attach(GTK_TABLE(table), scale, 1, 3, i, i+1, GTK_FILL, GTK_SHRINK, 0, 0);
	i++;

	hbox=gtk_hbox_new(TRUE, 6);
	gtk_table_attach(GTK_TABLE(table), gd_label_new_printf(_("Enabled on levels")), 0, 1, i, i+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(table), hbox, 1, 3, i, i+1);
	for (n=0; n<5; n++) {
		GtkWidget *wid;
		
		wid=check_button_new_level_enable(&object->levels, n+1);
		widget_focus_in_set_level(wid, GTK_SCALE(scale), n+1);
		gtk_box_pack_start_defaults(GTK_BOX(hbox), wid);
	}
	i++;

	gtk_table_attach(GTK_TABLE(table), gtk_hseparator_new(), 0, 3, i, i+1, GTK_FILL, GTK_SHRINK, 0, 0);
	i++;
	
	/* OBJECT PROPERTIES ************/
	if (gd_object_description[object->type].x1!=NULL) {
		gtk_table_attach(GTK_TABLE(table), gd_label_new_printf(_(gd_object_description[object->type].x1)), 0, 1, i, i+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
		gtk_table_attach_defaults(GTK_TABLE(table), spin_button_new_with_update(0, edited_cave->w-1, &object->x1), 1, 2, i, i+1);
		gtk_table_attach_defaults(GTK_TABLE(table), spin_button_new_with_update(0, edited_cave->h-1, &object->y1), 2, 3, i, i+1);
		i++;
	}

	/* points only have one coordinate. for other elements, */
	if (gd_object_description[object->type].x2!=NULL) {
		gtk_table_attach(GTK_TABLE(table), gd_label_new_printf(_(gd_object_description[object->type].x2)), 0, 1, i, i+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
		gtk_table_attach_defaults(GTK_TABLE(table), spin_button_new_with_update(0, edited_cave->w-1, &object->x2), 1, 2, i, i+1);
		gtk_table_attach_defaults(GTK_TABLE(table), spin_button_new_with_update(0, edited_cave->w-1, &object->y2), 2, 3, i, i+1);
		i++;
	}

	/* mazes have horiz */
	if (gd_object_description[object->type].horiz!=NULL) {
		gtk_table_attach(GTK_TABLE(table), gd_label_new_printf(_(gd_object_description[object->type].horiz)), 0, 1, i, i+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
		gtk_table_attach_defaults(GTK_TABLE(table), spin_button_new_with_update(0, 100, &object->horiz), 1, 3, i, i+1);
		i++;
	}

	/* every element has an object parameter */
	if (gd_object_description[object->type].element!=NULL) {
		gtk_table_attach(GTK_TABLE(table), gd_label_new_printf(_(gd_object_description[object->type].element)), 0, 1, i, i+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
		gtk_table_attach_defaults(GTK_TABLE(table), element_button_new_with_update(&object->element), 1, 3, i, i+1);
		i++;
	}
	
	/* joins and filled rectangles have a second object, but with different meaning */
	if (gd_object_description[object->type].fill_element!=NULL) {
		gtk_table_attach(GTK_TABLE(table), gd_label_new_printf(_(gd_object_description[object->type].fill_element)), 0, 1, i, i+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
		gtk_table_attach_defaults(GTK_TABLE(table), element_button_new_with_update(&object->fill_element), 1, 3, i, i+1);
		i++;
	}

	/* rasters and joins have distance parameters */
	if (gd_object_description[object->type].dx!=NULL) {
		int rangemin=-40;
		
		/* for rasters and mazes, dx>=1 and dy>=1. for joins, these can be zero or even negative */
		if (object->type==RASTER || object->type==MAZE)
			rangemin=1;

		gtk_table_attach(GTK_TABLE(table), gd_label_new_printf(_(gd_object_description[object->type].dx)), 0, 1, i, i+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
		gtk_table_attach_defaults(GTK_TABLE(table), spin_button_new_with_update(rangemin, 40, &object->dx), 1, 2, i, i+1);
		gtk_table_attach_defaults(GTK_TABLE(table), spin_button_new_with_update(rangemin, 40, &object->dy), 2, 3, i, i+1);
		i++;
	}

	/* mazes and random fills have seed params */
	if (gd_object_description[object->type].seed!=NULL) {
		int j;
		
		for (j=0; j<5; j++) {
			GtkWidget *spin;
			int spin_max;
			
			if (object->type==RANDOM_FILL)
				spin_max=255;
			else
				spin_max=1<<24;
			
			gtk_table_attach(GTK_TABLE(table), gd_label_new_printf(_(gd_object_description[object->type].seed), j+1), 0, 1, i, i+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);
			gtk_table_attach_defaults(GTK_TABLE(table), spin=spin_button_new_with_update(-1, spin_max, &object->seed[j]), 1, 3, i, i+1);
			widget_focus_in_set_level(spin, GTK_SCALE(scale), j+1);
			i++;
		}
	}

	if (object->type==RANDOM_FILL)
		/* pborder (first) and pseed (last) parameter are not needed here */
		random_setup_widgets_to_table(GTK_TABLE(table), i, NULL, &object->fill_element, object->random_fill, object->random_fill_probability, NULL);
	
	undo_save();
	gtk_widget_show_all (dialog);
	result=gtk_dialog_run(GTK_DIALOG(dialog));
	/* levels */
	if (object->levels==0) {
		object->levels=GD_OBJECT_LEVEL_ALL;
		
		gd_warningmessage(_("The object should be visible on at least one level."), _("Enabled this object on all levels."));
	}
	gtk_widget_destroy (dialog);

	if (result==GTK_RESPONSE_REJECT)
		do_undo();
	else
		render_cave();
}





/***************************************************
 *
 * mouse events
 *
 *
 */

/* mouse button press event */
static gboolean
button_press_event (GtkWidget *widget, const GdkEventButton * event, const gpointer data)
{
	GdObject *new_object;

	g_return_val_if_fail (edited_cave!=NULL, FALSE);

	/* right click opens popup */
	if (event->button==3) {
		gtk_menu_popup(GTK_MENU(drawing_area_popup), NULL, NULL, NULL, NULL, event->button, event->time);
		return TRUE;
	}

	/* this should be also false for doubleclick! so we do not do if (event->tye....) */	
	button1_clicked=event->type==GDK_BUTTON_PRESS && event->button==1;	
	
	clicked_x=((int) event->x)/cell_size;
	clicked_y=((int) event->y)/cell_size;
	/* middle button picks element from screen */
	if (event->button==2) {
		if (event->state & GDK_CONTROL_MASK)
			gd_element_button_set (fillelement_button, gd_cave_get_rc(rendered_cave, clicked_x, clicked_y));
		else if (event->state & GDK_SHIFT_MASK) {
			GdObjectType type;

			if (rendered_cave->objects_order[clicked_y][clicked_x])
				/* if there is an object, get type. if not, action defaults to move */
				type=((GdObject *) rendered_cave->objects_order[clicked_y][clicked_x])->type;
			else
				type=MOVE;

			select_tool(type);
		}
		else
			gd_element_button_set (element_button, gd_cave_get_rc(rendered_cave, clicked_x, clicked_y));
		return TRUE;
	}
	
	/* we do not handle anything other than buttons3,2 above, and button 1 */
	if (event->button!=1)
		return FALSE;

	/* if double click, open element properties window.
	 * if no element selected, open cave properties window.
	 * (if mouse if over an element, the first click selected it.) */
	/* do not allow this doubleclick for visible size mode as that one does not select objects */
	if (event->type==GDK_2BUTTON_PRESS && action!=VISIBLE_SIZE) {
		if (rendered_cave->objects_order[clicked_y][clicked_x])
			object_properties (rendered_cave->objects_order[clicked_y][clicked_x]);
		else
			cave_properties (edited_cave, FALSE);
		return TRUE;
	}

	switch (action) {
		case MOVE:
			/* action=move: now the user is selecting an object by the mouse click */
			if ((event->state & GDK_CONTROL_MASK) || (event->state & GDK_SHIFT_MASK)) {
				/* CONTROL or SHIFT PRESSED: multiple selection */
				/* if control-click on a non-object, do nothing. */
				if (rendered_cave->objects_order[clicked_y][clicked_x]) {
					if (object_highlight[clicked_y][clicked_x])		/* <- hackish way of checking if the clicked object is currently selected */
						object_list_remove_from_selection(rendered_cave->objects_order[clicked_y][clicked_x]);
					else
						object_list_add_to_selection(rendered_cave->objects_order[clicked_y][clicked_x]);
				}
			} else {
				/* CONTROL NOT PRESSED: single selection */
				/* if the object clicked is not currently selected, we select it. if it is, do nothing, so a multiple selection remains. */
				if (rendered_cave->objects_order[clicked_y][clicked_x]==NULL) {
					/* if clicking on a non-object, deselect all */
					object_list_clear_selection();
				} else
				if (!object_highlight[clicked_y][clicked_x])		/* <- hackish way of checking if the clicked object is currently non-selected */
					object_list_select_one_object(rendered_cave->objects_order[clicked_y][clicked_x]);
			}
			
			/* prepare for undo */
			undo_move_flag=FALSE;
			break;
			
		case FREEHAND:
			/* freehand tool: draw points in each place. */
			/* if already the same element there, which is placed by an object, skip! */
			if (gd_cave_get_rc(rendered_cave, clicked_x, clicked_y)!=gd_element_button_get(element_button) || rendered_cave->objects_order[clicked_y][clicked_x]==NULL) {
				GdObject *new_object;

				/* save undo only on every new click; dragging the mouse will not save it */
				undo_save();
				
				new_object=g_new0 (GdObject, 1);
				new_object->levels=GD_OBJECT_LEVEL_ALL;
				new_object->type=POINT;	/* freehand places points */
				new_object->x1=clicked_x;
				new_object->y1=clicked_y;
				new_object->element=gd_element_button_get(element_button);

				edited_cave->objects=g_list_append(edited_cave->objects, new_object);
				render_cave();		/* new object created, so re-render cave */
				object_list_select_one_object(new_object);	/* also make it selected; so it can be edited further */
			}
			break;
			
		case VISIBLE_SIZE:
			/* new click... prepare for undo! */
			undo_move_flag=FALSE;
			/* do nothing, the motion event will matter */
			break;
			
		case POINT:
		case LINE:
		case RECTANGLE:
		case FILLED_RECTANGLE:
		case RASTER:
		case JOIN:
		case FLOODFILL_REPLACE:
		case FLOODFILL_BORDER: 
		case MAZE:
		case MAZE_UNICURSAL:
		case MAZE_BRAID:
		case RANDOM_FILL:
			/* CREATE A NEW OBJECT */
			
			/* ok not moving so drawing something new. */
			new_object=g_new0 (GdObject, 1);
			new_object->levels=GD_OBJECT_LEVEL_ALL;
			new_object->type=action;	/* set type of object */
			new_object->x1=clicked_x;
			new_object->y1=clicked_y;
			new_object->x2=clicked_x;
			new_object->y2=clicked_y;
			new_object->seed[0]=0;
			new_object->seed[1]=0;
			new_object->seed[2]=0;
			new_object->seed[3]=0;
			new_object->seed[4]=0;
			new_object->horiz=50;

			new_object->element=gd_element_button_get(element_button);
			/* for floodfills, get the first (to-be-found) element from the map */
			if (new_object->type==FLOODFILL_REPLACE)
				new_object->element=rendered_cave->map[clicked_y][clicked_x];
			new_object->fill_element=gd_element_button_get(fillelement_button);

			if (action==RASTER) {		/* for raster */
				new_object->dx=2;
				new_object->dy=2;
			}
			else if (action==JOIN) {	/* for join */
				new_object->dx=0;
				new_object->dy=0;
			}
			else if (action==MAZE || action==MAZE_UNICURSAL || action==MAZE_BRAID) {	/* for maze */
				new_object->dx=1;
				new_object->dy=1;
			}
			else if (action==RANDOM_FILL) {
				int j;

				new_object->element=O_NONE;				/* replace element */
				new_object->random_fill[0]=gd_element_button_get(element_button);	/* first random element */
				new_object->random_fill_probability[0]=32;
				for (j=1; j<4; j++) {
					new_object->random_fill[j]=O_DIRT;
					new_object->random_fill_probability[j]=0;
				}
			}

			undo_save();
			edited_cave->objects=g_list_append(edited_cave->objects, new_object);
			render_cave();		/* new object created, so re-render cave */
			object_list_select_one_object(new_object);	/* also make it selected; so it can be edited further */
			break;
			
		default:
			g_assert_not_reached();
	}
	return TRUE;
}

static gboolean
button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->type==GDK_BUTTON_RELEASE && event->button==1)
		button1_clicked=FALSE;
	return TRUE;
}

/* mouse leaves drawing area event */
static gboolean
leave_event (const GtkWidget *widget, const GdkEventCrossing * event, const gpointer data)
{
	/* do not check if it as enter event, as we did not connect that one. */
	gtk_label_set_text (GTK_LABEL (label_coordinate), "[x:   y:   ]");
	mouse_x=-1;
	mouse_y=-1;
	return FALSE;
}

/* mouse motion event */
static gboolean
motion_event (const GtkWidget *widget, const GdkEventMotion *event, const gpointer data)
{
	int x, y, dx, dy;
	GdkModifierType state;

	/* just to be sure. */
	if (!edited_cave)
		return FALSE;

	if (event->is_hint)
		gdk_window_get_pointer (event->window, &x, &y, &state);
	else {
		x=event->x;
		y=event->y;
		state=event->state;
	}
	
	x/=cell_size;
	y/=cell_size;

	/* if button1 not pressed, remember this. we also use the motion event to see if it. hackish. */
	if (!(state&GDK_BUTTON1_MASK))
		button1_clicked=FALSE;

	/* check if event coordinates inside drawing area. when holding the mouse
	 * button, gdk can send coordinates outside! */
	if (x<0 || y<0 || x>=rendered_cave->w || y>=rendered_cave->h)
		return TRUE;

	/* check if mouse has moved to another cell. also set label showing coordinate. */
	if (mouse_x!=x || mouse_y!=y) {
		mouse_x=x;
		mouse_y=y;
		gd_label_set_markup_printf(GTK_LABEL (label_coordinate), "[x:%d y:%d]", x, y);
	}
	
	/* if we do not remember button 1 press, then don't do anything. */
	/* this solves some misinterpretation of mouse events, when windows appear or mouse pointer exits the drawing area and enters again */
	if (!(state&GDK_BUTTON1_MASK) || !button1_clicked)
		return TRUE;

	dx=x-clicked_x;
	dy=y-clicked_y;
	/* if the mouse pointer did not move at least one cell in x or y direction, return */
	if (dx==0 && dy==0)
		return TRUE;
		
	/* changing visible size is different; independent of cave objects. */	
	if (action==VISIBLE_SIZE) {
		/* save visible size flag only once */
		if (undo_move_flag==FALSE) {
			undo_save();
			undo_move_flag=TRUE;
		}
		
		/* try to drag (x1;y1) corner. */
		if (clicked_x==edited_cave->x1 && clicked_y==edited_cave->y1) {
			edited_cave->x1+=dx;
			edited_cave->y1+=dy;
		}
		else
			/* try to drag (x2;y1) corner. */
		if (clicked_x==edited_cave->x2 && clicked_y==edited_cave->y1) {
			edited_cave->x2+=dx;
			edited_cave->y1+=dy;
		}
		else
			/* try to drag (x1;y2) corner. */
		if (clicked_x==edited_cave->x1 && clicked_y==edited_cave->y2) {
			edited_cave->x1+=dx;
			edited_cave->y2+=dy;
		}
		else
			/* try to drag (x2;y2) corner. */
		if (clicked_x==edited_cave->x2 && clicked_y==edited_cave->y2) {
			edited_cave->x2+=dx;
			edited_cave->y2+=dy;
		}
		else {
			/* drag the whole */
			edited_cave->x1+=dx;
			edited_cave->y1+=dy;
			edited_cave->x2+=dx;
			edited_cave->y2+=dy;
		}
		clicked_x=x;
		clicked_y=y;
		
		/* check and adjust ranges if necessary */
		gd_cave_correct_visible_size(edited_cave);
		
		/* instead of re-rendering the cave, we just copy the changed values. */
		rendered_cave->x1=edited_cave->x1;
		rendered_cave->x2=edited_cave->x2;
		rendered_cave->y1=edited_cave->y1;
		rendered_cave->y2=edited_cave->y2;
		return TRUE;
	}

	if (action==FREEHAND) {
		/* the freehand tool is different a bit. it draws single points automatically */
		/* but only to places where there is no such object already. */
		if (gd_cave_get_rc(rendered_cave, x, y)!=gd_element_button_get (element_button) || rendered_cave->objects_order[y][x]==NULL) {
			GdObject *new_object;
			
			new_object=g_new0 (GdObject, 1);
			new_object->levels=GD_OBJECT_LEVEL_ALL;
			new_object->type=POINT;	/* freehand places points */
			new_object->x1=x;
			new_object->y1=y;
			new_object->element=gd_element_button_get(element_button);
			edited_cave->objects=g_list_append(edited_cave->objects, new_object);
			render_cave();	/* we do this here by hand; do not use changed flag; otherwise object_list_add_to_selection wouldn't work */
			object_list_add_to_selection(new_object);	/* this way all points will be selected together when using freehand */
		}
		return TRUE;
	}
	
	if (!object_list_is_any_selected())
		return TRUE;
	
	if (object_list_count_selected()==1) {
		/* MOVING, DRAGGING A SINGLE OBJECT **************************/
		GdObject *object=object_list_first_selected();
		
		switch (action) {
		case MOVE:
			/* MOVING AN EXISTING OBJECT */
			if (undo_move_flag==FALSE) {
				undo_save();
				undo_move_flag=TRUE;
			}

			switch (object->type) {
				case POINT:
				case FLOODFILL_REPLACE:
				case FLOODFILL_BORDER:
					/* this one will be true for a point */
					/* for other objects, only allow dragging by their origin */
					if (clicked_x==object->x1 && clicked_y==object->y1) {
						object->x1+=dx;
						object->y1+=dy;
					}
					break;
					
				case LINE:
					/* these are the easy ones. for a line, try if endpoints are dragged.
					 * if not, drag the whole line.
					 * for a point, the first if() will always match.
					 */
					if (clicked_x==object->x1 && clicked_y==object->y1) {	/* this one will be true for a point */
						object->x1+=dx;
						object->y1+=dy;
					}
					else if (clicked_x==object->x2 && clicked_y==object->y2) {
						object->x2+=dx;
						object->y2+=dy;
					}
					else {
						object->x1+=dx;
						object->y1+=dy;
						object->x2+=dx;
						object->y2+=dy;
					}
					break;
				case RECTANGLE:
				case FILLED_RECTANGLE:
				case RASTER:
				case MAZE:
				case MAZE_UNICURSAL:
				case MAZE_BRAID:
				case RANDOM_FILL:
					/* dragging objects which are box-shaped */
					/* XXX for raster, this not always works; as raster's x2,y2 may not be visible */
					if (clicked_x==object->x1 && clicked_y==object->y1) {			/* try to drag (x1;y1) corner. */
						object->x1+=dx;
						object->y1+=dy;
					}
					else if (clicked_x==object->x2 && clicked_y==object->y1) {		/* try to drag (x2;y1) corner. */
						object->x2 +=dx;
						object->y1 +=dy;
					}
					else if (clicked_x==object->x1 && clicked_y==object->y2) {		/* try to drag (x1;y2) corner. */
						object->x1+=dx;
						object->y2+=dy;
					}
					else if (clicked_x==object->x2 && clicked_y==object->y2) {		/* try to drag (x2;y2) corner. */
						object->x2+=dx;
						object->y2+=dy;
					}
					else {
						/* drag the whole thing */
						object->x1+=dx;
						object->y1+=dy;
						object->x2+=dx;
						object->y2+=dy;
					}
					break;
				case JOIN:
					object->dx+=dx;
					object->dy+=dy;
					break;
				case NONE:
					g_assert_not_reached();
					break;
			}
			break;

		/* DRAGGING THE MOUSE, WHEN THE OBJECT WAS JUST CREATED */
		case POINT:
		case FLOODFILL_BORDER:
		case FLOODFILL_REPLACE:
			/* only dragging the created point further. wants the user another one, he must press the button again. */
			if (object->x1!=x || object->y1!=y) {
				object->x1=x;
				object->y1=y;
			}
			break;
		case LINE:
		case RECTANGLE:
		case FILLED_RECTANGLE:
		case RASTER:
		case MAZE:
		case MAZE_UNICURSAL:
		case MAZE_BRAID:
		case RANDOM_FILL:
			/* for these, dragging sets the second coordinate. */
			if (object->x2!=x || object->y2!=y) {
				object->x2=x;
				object->y2=y;
			}
			break;
		case JOIN:
			/* dragging sets the distance of the new character placed. */
			object->dx+=dx;
			object->dy+=dy;
			break;
		case NONE:
			g_assert_not_reached();
			break;
		}
	} else
	if (object_list_count_selected()>1 && action==MOVE) {
		/* MOVING MULTIPLE OBJECTS */
		GList *iter;

		if (undo_move_flag==FALSE) {
			undo_save();
			undo_move_flag=TRUE;
		}
		
		for (iter=selected_objects; iter!=NULL; iter=iter->next) {
			GdObject *object=iter->data;

			object->x1+=dx;
			object->y1+=dy;
			object->x2+=dx;
			object->y2+=dy;
			if (object->type==JOIN) {
				object->dx+=dx;
				object->dy+=dy;
			}
		}
	}

	clicked_x=x;
	clicked_y=y;
	render_cave();

	return TRUE;
}

/****************************************************/









/****************************************************
 *
 * CAVE SELECTOR ICON VIEW
 *
 *
 */

/* this is also called as an item activated signal.
	so don't use data! that function has another parameters.
	also do not use widget, as it is once an icon view, once a gtkmenu */
static void
edit_cave_cb ()
{
	GList *list;
	GtkTreeIter iter;
	GtkTreeModel *model;
	Cave *cave;

	list=gtk_icon_view_get_selected_items (GTK_ICON_VIEW(iconview_cavelist));
	g_return_if_fail (list!=NULL);

	model=gtk_icon_view_get_model (GTK_ICON_VIEW (iconview_cavelist));
	gtk_tree_model_get_iter (model, &iter, list->data);
	gtk_tree_model_get (model, &iter, CAVE_COLUMN, &cave, -1);
	g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);	/* free the list of paths */
	g_list_free (list);
	select_cave_for_edit (cave);
}

static void
rename_cave_cb (GtkWidget *widget, gpointer data)
{
	GList *list;
	GtkTreeIter iter;
	GtkTreeModel *model;
	Cave *cave;
	GtkWidget *dialog, *entry;
	int result;

	list=gtk_icon_view_get_selected_items (GTK_ICON_VIEW(iconview_cavelist));
	g_return_if_fail (list!=NULL);

	/* use first element, as icon view is configured to enable only one selection */
	model=gtk_icon_view_get_model (GTK_ICON_VIEW (iconview_cavelist));
	gtk_tree_model_get_iter (model, &iter, list->data);
	gtk_tree_model_get (model, &iter, CAVE_COLUMN, &cave, -1);
	g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);	/* free the list of paths */
	g_list_free (list);

	dialog=gtk_dialog_new_with_buttons(_("Cave Name"), GTK_WINDOW(editor_window), GTK_DIALOG_NO_SEPARATOR|GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
	entry=gtk_entry_new();
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
	gtk_entry_set_text(GTK_ENTRY(entry), cave->name);
	gtk_entry_set_max_length(GTK_ENTRY(entry), sizeof(GdString));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), entry, FALSE, FALSE, 6);

	gtk_widget_show(entry);
	result=gtk_dialog_run(GTK_DIALOG(dialog));
	if (result==GTK_RESPONSE_ACCEPT) {
		g_strlcpy(cave->name, gtk_entry_get_text(GTK_ENTRY(entry)), sizeof(cave->name));
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, NAME_COLUMN, cave->name, -1);
	}
	gtk_widget_destroy(dialog);
}

static void
cave_make_selectable_cb (GtkWidget *widget, gpointer data)
{
	GList *list;
	GtkTreeIter iter;
	GtkTreeModel *model;
	Cave *cave;

	list=gtk_icon_view_get_selected_items (GTK_ICON_VIEW(iconview_cavelist));
	g_return_if_fail (list!=NULL);

	model=gtk_icon_view_get_model (GTK_ICON_VIEW (iconview_cavelist));
	gtk_tree_model_get_iter (model, &iter, list->data);
	gtk_tree_model_get (model, &iter, CAVE_COLUMN, &cave, -1);
	g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);	/* free the list of paths */
	g_list_free (list);
	if (!cave->selectable) {
		cave->selectable=TRUE;
		g_hash_table_remove(cave_pixbufs, cave);
		/* regenerate icon view */
		gtk_widget_destroy (iconview_cavelist);
		select_cave_for_edit(NULL);
	}
}

static void
cave_make_unselectable_cb (GtkWidget *widget, gpointer data)
{
	GList *list;
	GtkTreeIter iter;
	GtkTreeModel *model;
	Cave *cave;

	list=gtk_icon_view_get_selected_items (GTK_ICON_VIEW(iconview_cavelist));
	g_return_if_fail (list!=NULL);

	model=gtk_icon_view_get_model (GTK_ICON_VIEW (iconview_cavelist));
	gtk_tree_model_get_iter (model, &iter, list->data);
	gtk_tree_model_get (model, &iter, CAVE_COLUMN, &cave, -1);
	g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);	/* free the list of paths */
	g_list_free (list);
	if (cave->selectable) {
		cave->selectable=FALSE;
		g_hash_table_remove(cave_pixbufs, cave);
		/* regenerate icon view */
		gtk_widget_destroy (iconview_cavelist);
		select_cave_for_edit(NULL);
	}
}

static void
icon_view_selection_changed_cb (GtkWidget *widget, gpointer data)
{
	GList *list;
	GtkTreeModel *model;

	list=gtk_icon_view_get_selected_items (GTK_ICON_VIEW (widget));
	gtk_action_group_set_sensitive (actions_cave_selector, g_list_length(list)==1);
	gtk_action_group_set_sensitive (actions_clipboard, list!=NULL);
	if (list!=NULL) {
		GtkTreeIter iter;
		Cave *cave;

		model=gtk_icon_view_get_model (GTK_ICON_VIEW (widget));

		gtk_tree_model_get_iter (model, &iter, list->data);
		gtk_tree_model_get (model, &iter, CAVE_COLUMN, &cave, -1);
		gd_label_set_markup_printf (GTK_LABEL (label_object), _("<b>%s</b>, %s, %dx%d, time %d:%02d, diamonds %d"), cave->name, cave->selectable?_("selectable"):_("not selectable"), cave->w, cave->h, cave->level_time[0]/60, cave->level_time[0] % 60, cave->level_diamonds[0]);
	}
	g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (list);
}


/* for caveset icon view */
static void
caveset_icon_view_destroyed (GtkIconView * icon_view, gpointer data)
{
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeIter iter;

	/* caveset should be an empty list, as the icon view stores the current caves and order */
	g_assert(gd_caveset==NULL);

	model=gtk_icon_view_get_model(icon_view);
	path=gtk_tree_path_new_first();
	while (gtk_tree_model_get_iter (model, &iter, path)) {
		Cave *cave;

		gtk_tree_model_get (model, &iter, CAVE_COLUMN, &cave, -1);
		/* make a new list from the new order obtained from the icon view */
		gd_caveset=g_list_append(gd_caveset, cave);

		gtk_tree_path_next (path);
	}
	gtk_tree_path_free (path);
}


static void
add_cave_to_icon_view(GtkListStore *store, Cave *cave)
{
	GtkTreeIter treeiter;
	GdkPixbuf *pixbuf;
	
	pixbuf=g_hash_table_lookup(cave_pixbufs, cave);
	if (!pixbuf) {
		Cave *rendered;
		
		rendered=gd_cave_new_rendered (cave, 0);	/* render at level 1 */
		pixbuf=gd_drawcave_to_pixbuf (rendered, 128, 128, TRUE);					/* draw 128x128 icons at max */
		if (!cave->selectable) {
			GdkPixbuf *colored;
			
			colored=gdk_pixbuf_composite_color_simple(pixbuf, gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf), GDK_INTERP_NEAREST, 160, 1, gd_flash_color, gd_flash_color);
			g_object_unref(pixbuf);	/* forget original */
			pixbuf=colored;
		}
		gd_cave_free (rendered);
		g_hash_table_insert(cave_pixbufs, cave, pixbuf);
	}

	gtk_list_store_insert_with_values (store, &treeiter, -1, CAVE_COLUMN, cave, NAME_COLUMN, cave->name, PIXBUF_COLUMN, pixbuf, -1);
}

/* does nothing else but sets caveset_edited to true. called by "reordering" (drag&drop), which is implemented by gtk+ by inserting and deleting */
/* we only connect this signal after adding all caves to the icon view, so it is only activated by the user! */
static void
cave_list_row_inserted(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	gd_caveset_edited=TRUE;
}

/* for popup menu, by properties key */
static void
iconview_popup_menu(GtkWidget *widget, gpointer data)
{
	gtk_menu_popup(GTK_MENU(caveset_popup), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

/* for popup menu, by right-click */
static gboolean
iconview_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->type==GDK_BUTTON_PRESS && event->button==3) {
		gtk_menu_popup(GTK_MENU(caveset_popup), NULL, NULL, NULL, NULL, event->button, event->time);
		return TRUE;
	}
	return FALSE;
}

/*
 * selects a cave for edit.
 * if given a cave, creates a drawing area, shows toolbars...
 * if given no cave, creates a gtk icon view for a game overview.
 */

static void
select_cave_for_edit (Cave * cave)
{
	char *title;
	
	object_list_clear_selection();

	gtk_action_group_set_sensitive (actions_edit_object, FALSE);	/* will be enabled later if needed */
	gtk_action_group_set_sensitive (actions_edit_one_object, FALSE);	/* will be enabled later if needed */
	gtk_action_group_set_sensitive (actions_edit_cave, cave!=NULL);
	gtk_action_group_set_sensitive (actions_edit_caveset, cave==NULL);
	gtk_action_group_set_sensitive (actions_edit_tools, cave!=NULL);
	gtk_action_group_set_sensitive (actions_edit_map, FALSE);	/* will be enabled later if needed */
	gtk_action_group_set_sensitive (actions_edit_random, FALSE);	/* will be enabled later if needed */
	gtk_action_group_set_sensitive (actions_toggle, cave!=NULL);
	/* this is sensitized by an icon selector callback. */
	gtk_action_group_set_sensitive (actions_cave_selector, FALSE);	/* will be enabled later if needed */
	gtk_action_group_set_sensitive (actions_clipboard, FALSE);	/* will be enabled later if needed */
	gtk_action_group_set_sensitive (actions_clipboard_paste,
		(cave!=NULL && object_clipboard!=NULL)
		|| (cave==NULL && cave_clipboard!=NULL));
	gtk_action_group_set_sensitive (actions_edit_undo, cave!=NULL && undo_caves!=NULL);
	gtk_action_group_set_sensitive (actions_edit_redo, cave!=NULL && redo_caves!=NULL);

	/* select cave */
	edited_cave=cave;

	/* if cave data given, show it. */
	if (cave) {
		title=g_strdup_printf (_("GDash Cave Editor - %s"), edited_cave->name);
		gtk_window_set_title (GTK_WINDOW (editor_window), title);
		g_free (title);
		if (iconview_cavelist)
			gtk_widget_destroy (iconview_cavelist);

		if (gd_show_object_list)
			gtk_widget_show (scroll_window_objects);

		/* create pixbufs for these colors */
		gd_select_pixbuf_colors(edited_cave->color0, edited_cave->color1, edited_cave->color2, edited_cave->color3, edited_cave->color4, edited_cave->color5);
		gd_create_pixmaps();
		gd_element_button_update_pixbuf(element_button);
		gd_element_button_update_pixbuf(fillelement_button);

		/* put drawing area in an alignment, so window can be any large w/o problems */
		if (!drawing_area) {
			GtkWidget *align;

			align=gtk_alignment_new (0.5, 0.5, 0, 0);
			gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll_window), align);
			gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll_window), GTK_SHADOW_NONE);

			drawing_area=gtk_drawing_area_new ();
			mouse_x=mouse_y=-1;
			/* enable some events */
			gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_LEAVE_NOTIFY_MASK);
			g_signal_connect(G_OBJECT (drawing_area), "destroy", G_CALLBACK(gtk_widget_destroyed), &drawing_area);
			g_signal_connect(G_OBJECT (drawing_area), "button_press_event", G_CALLBACK(button_press_event), NULL);
			g_signal_connect(G_OBJECT (drawing_area), "button_release_event", G_CALLBACK(button_release_event), NULL);
			g_signal_connect(G_OBJECT (drawing_area), "motion_notify_event", G_CALLBACK(motion_event), NULL);
			g_signal_connect(G_OBJECT (drawing_area), "leave_notify_event", G_CALLBACK(leave_event), NULL);
			g_signal_connect(G_OBJECT (drawing_area), "expose_event", G_CALLBACK(drawing_area_expose_event), NULL);
			gtk_container_add (GTK_CONTAINER (align), drawing_area);
		}
		render_cave();
		gtk_widget_set_size_request(drawing_area, edited_cave->w * cell_size, edited_cave->h * cell_size);

		/* remove from pixbuf hash: delete its pixbuf */
		g_hash_table_remove(cave_pixbufs, cave);
	}
	else {
		/* if no cave given, show selector. */
		/* forget undo caves */
		undo_free();
		
		gd_cave_map_free(gfx_buffer);
		gfx_buffer=NULL;
		gd_cave_map_free(object_highlight);
		object_highlight=NULL;

		title=g_strdup_printf(_("GDash Cave Editor - %s"), gd_default_cave->name);
		gtk_window_set_title (GTK_WINDOW (editor_window), title);
		g_free(title);

		gtk_list_store_clear (object_list);
		gtk_widget_hide (scroll_window_objects);

		if (drawing_area)
			gtk_widget_destroy (drawing_area->parent->parent);
		/* parent is the align, parent of align is the viewport automatically added. */

		if (!iconview_cavelist) {
			GtkListStore *cave_list;
			GList *iter;

			gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll_window), GTK_SHADOW_IN);

			/* create list store for caveset */
			cave_list=gtk_list_store_new (NUM_CAVESET_COLUMNS, G_TYPE_POINTER, G_TYPE_STRING, GDK_TYPE_PIXBUF);
			for (iter=gd_caveset; iter; iter=g_list_next (iter))
				add_cave_to_icon_view(cave_list, iter->data);
			/* we only connect this signal after adding all caves to the icon view, so it is only activated by the user! */
			g_signal_connect(G_OBJECT(cave_list), "row-inserted", G_CALLBACK(cave_list_row_inserted), NULL);
			/* forget caveset; now we store caves in the GtkListStore */
			g_list_free(gd_caveset);
			gd_caveset=NULL;

			iconview_cavelist=gtk_icon_view_new_with_model (GTK_TREE_MODEL (cave_list));
			g_object_unref(cave_list);	/* now the icon view holds the reference */
			g_signal_connect(G_OBJECT(iconview_cavelist), "destroy", G_CALLBACK(caveset_icon_view_destroyed), &iconview_cavelist);
			g_signal_connect(G_OBJECT(iconview_cavelist), "destroy", G_CALLBACK(gtk_widget_destroyed), &iconview_cavelist);
			g_signal_connect(G_OBJECT(iconview_cavelist), "popup-menu", G_CALLBACK(iconview_popup_menu), NULL);
			g_signal_connect(G_OBJECT(iconview_cavelist), "button-press-event", G_CALLBACK(iconview_button_press_event), NULL);

			gtk_icon_view_set_text_column(GTK_ICON_VIEW (iconview_cavelist), NAME_COLUMN);
			gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW (iconview_cavelist), PIXBUF_COLUMN);
			gtk_icon_view_set_reorderable(GTK_ICON_VIEW (iconview_cavelist), TRUE);
			gtk_icon_view_set_selection_mode(GTK_ICON_VIEW (iconview_cavelist), GTK_SELECTION_MULTIPLE);
			/* item (cave) activated. the enter button activates the menu item; this one is used for doubleclick */
			g_signal_connect(iconview_cavelist, "item-activated", G_CALLBACK(edit_cave_cb), NULL);
			g_signal_connect(iconview_cavelist, "selection-changed", G_CALLBACK(icon_view_selection_changed_cb), NULL);
			gtk_container_add(GTK_CONTAINER (scroll_window), iconview_cavelist);
		}
	}
	/* show all items inside scrolled window. some may have been newly created. */
	gtk_widget_show_all (scroll_window);

	/* hide toolbars if not editing a cave */
	if (edited_cave)
		gtk_widget_show (toolbars);
	else
		gtk_widget_hide (toolbars);
}

/****************************************************/
static void
cave_random_setup_cb (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog, *table;
	int result;

	g_return_if_fail(edited_cave->map==NULL);

	/* save for undo here, as we do not have cancel button :P */
	undo_save();

	dialog=gtk_dialog_new_with_buttons (_("Cave Initial Random Fill"), GTK_WINDOW (editor_window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
	
	table=gtk_table_new(0, 0, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 6);
	gtk_table_set_row_spacings (GTK_TABLE(table), 6);
	gtk_table_set_col_spacings (GTK_TABLE(table), 6);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox), table);
	
	random_setup_widgets_to_table(GTK_TABLE(table), 0, &edited_cave->initial_border, &edited_cave->initial_fill, edited_cave->random_fill, edited_cave->random_fill_probability, edited_cave->level_rand);

	gtk_widget_show_all(dialog);
	result=gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	/* if cancel clicked, revert to original */
	if (result!=GTK_RESPONSE_ACCEPT)
		do_undo();
}


/********
 * CAVE 
 */
static void
save_cave_png (GdkPixbuf *pixbuf)
{
	/* if no filename given, */
	GtkWidget *dialog;
	GtkFileFilter *filter;
	GError *error=NULL;
	char *suggested_name, *filename=NULL;

	/* check if in cave editor */
	g_return_if_fail (edited_cave!=NULL);

	dialog=gtk_file_chooser_dialog_new (_("Save Cave as PNG Image"), GTK_WINDOW (editor_window), GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	filter=gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("PNG files"));
	gtk_file_filter_add_pattern (filter, "*.png");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	suggested_name=g_strdup_printf("%s.png", edited_cave->name);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), suggested_name);
	g_free(suggested_name);

	if (gtk_dialog_run (GTK_DIALOG (dialog))==GTK_RESPONSE_ACCEPT) {
		gboolean save=FALSE;

		filename=gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		if (!g_str_has_suffix(filename, ".png")) {
			char *suffixed=g_strdup_printf("%s.png", filename);
			
			g_free(filename);
			filename=suffixed;
		}

		/* check if overwrite */
		if (g_file_test(filename, G_FILE_TEST_EXISTS))
			save=gd_ask_overwrite(dialog, filename);
		else
			save=TRUE;

		/* save it */
		if (save)
			gdk_pixbuf_save (pixbuf, filename , "png", &error, "compression", "9", NULL);
	}
	gtk_widget_destroy (dialog);

	if (error) {
		g_warning("%s: %s", filename, error->message);
		gd_show_last_error();
		g_error_free (error);
	}
	g_free(filename);
}

/* CAVE OVERVIEW
   this creates a pixbuf of the cave, and scales it down to fit the screen if needed.
   it is then presented to the user, with the option to save it in png */
static void
cave_overview (gboolean game_view)
{
	/* view the RENDERED one, and the entire cave */
	GdkPixbuf *pixbuf=gd_drawcave_to_pixbuf (rendered_cave, 0, 0, game_view), *scaled;
	GtkWidget *dialog, *button;
	int sx, sy;
	double fx, fy;
	int response;
	
	/* be careful not to use entire screen, there are window title and close button */
	sx=gdk_screen_get_width(gdk_screen_get_default())-64;
	sy=gdk_screen_get_height(gdk_screen_get_default())-128;
	if (gdk_pixbuf_get_width(pixbuf)>sx)
		fx=(double) sx/gdk_pixbuf_get_width(pixbuf);
	else
		fx=1.0;
	if (gdk_pixbuf_get_height(pixbuf)>sy)
		fy=(double) sy/gdk_pixbuf_get_height(pixbuf);
	else
		fy=1.0;
	/* whichever is smaller */
	if (fx<fy)
		fy=fx;
	if (fy<fx)
		fx=fy;
	/* if we have to make it smaller */
	if (fx!=1.0 || fy!=1.0)
		scaled=gdk_pixbuf_scale_simple(pixbuf, gdk_pixbuf_get_width(pixbuf)*fx, gdk_pixbuf_get_height(pixbuf)*fy, GDK_INTERP_BILINEAR);
	else {
		scaled=pixbuf;
		g_object_ref(scaled);
	}
	
	/* simple dialog with this image only */
	dialog=gtk_dialog_new_with_buttons(_("Cave Overview"), GTK_WINDOW(editor_window), GTK_DIALOG_NO_SEPARATOR | GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
	button=gtk_button_new_with_mnemonic(_("Save as _PNG"));
	gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(GTK_STOCK_CONVERT, GTK_ICON_SIZE_BUTTON));
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, 1);
	gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CLOSE);
	
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox), gtk_image_new_from_pixbuf(scaled));

	gtk_widget_show_all(dialog);
	response=gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if (response==1)
		save_cave_png(pixbuf);
	g_object_unref(pixbuf);
	g_object_unref(scaled);
}


static void
cave_overview_cb(GtkWidget *widget, gpointer data)
{
	cave_overview(FALSE);
}

static void
cave_overview_game_cb(GtkWidget *widget, gpointer data)
{
	cave_overview(TRUE);
}





/* automatically shrink cave
 */
static void
auto_shrink_cave_cb (GtkWidget *widget, gpointer data)
{
	undo_save();
	/* shrink the rendered cave, as it has all object and the like converted to a map. */
	gd_cave_shrink(rendered_cave);
	/* then copy the results to the original */
	edited_cave->x1=rendered_cave->x1;
	edited_cave->y1=rendered_cave->y1;
	edited_cave->x2=rendered_cave->x2;
	edited_cave->y2=rendered_cave->y2;
	render_cave();
	/* selecting visible size tool allows the user to see the result, maybe modify */
	select_tool(VISIBLE_SIZE);
}



/* SET CAVE COLORS WITH INSTANT UPDATE TOOL.
 */
/* helper: update pixmaps and the like */
static void
color_changed ()
{
	int x,y;
	/* select new colors */
	gd_select_pixbuf_colors(edited_cave->color0, edited_cave->color1, edited_cave->color2, edited_cave->color3, edited_cave->color4, edited_cave->color5);
	/* recreate cave pixmaps with new colors */
	gd_create_pixmaps();
	/* update element buttons in editor (under toolbar) */
	gd_element_button_update_pixbuf(element_button);
	gd_element_button_update_pixbuf(fillelement_button);
	/* clear gfx buffer, so every element gets redrawn */
	for (y=0; y<edited_cave->h; y++)
		for (x=0; x<edited_cave->w; x++)
			gfx_buffer[y][x]=-1;
	/* object list update with new pixbufs */
	render_cave();
}

static gboolean colorchange_update_disabled;

#define GDASH_PCOLOR "gdash-pcolor"
static void
colorbutton_changed (GtkWidget *widget, gpointer data)
{
	GdColor* value=(GdColor *)g_object_get_data(G_OBJECT(widget), GDASH_PCOLOR);

	g_assert(value!=NULL);
		
	*value=gd_color_combo_get_color(widget);	/* update cave data */
	if (!colorchange_update_disabled)
		color_changed();
}

static void
colorbutton_random_changed(GtkWidget *widget, gpointer data)
{
	GList *combos=(GList *)data;
	GList *iter;

	gd_cave_set_random_colors(edited_cave);	
	color_changed();
	colorchange_update_disabled=TRUE;	/* this is needed, otherwise all combos would want to update the pixbufs, one by one */
	for (iter=combos; iter!=NULL; iter=iter->next) {
		GtkWidget *combo=(GtkWidget *)iter->data;
		GdColor* value=(GdColor *)g_object_get_data(G_OBJECT(combo), GDASH_PCOLOR);
		
		g_assert(value!=NULL);
		
		gd_color_combo_set(GTK_COMBO_BOX(combo), *value);
	}
	colorchange_update_disabled=FALSE;
}

/* set cave colors with instant update */
static void
cave_colors_cb (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog, *table=NULL, *button;
	GList *combos=NULL;
	int i, row=0;
	
	undo_save();

	dialog=gtk_dialog_new_with_buttons (_("Cave Colors"), GTK_WINDOW (editor_window), GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	/* random button */
	button=gtk_button_new_with_mnemonic(_("_Random"));
	gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(GD_ICON_RANDOM_FILL, GTK_ICON_SIZE_BUTTON));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), button, FALSE, FALSE, 0);
	/* close button */
	gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
	
	table=gtk_table_new (1, 1, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 6);
	gtk_table_set_row_spacings (GTK_TABLE(table), 6);
	gtk_table_set_col_spacings (GTK_TABLE(table), 6);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox), table);

	/* search for color properties, and create label&combo for each */
	for (i=0; gd_cave_properties[i].identifier!=NULL; i++)
		if (gd_cave_properties[i].type==GD_TYPE_COLOR) {
			GdColor *value=(GdColor *)G_STRUCT_MEMBER_P (edited_cave, gd_cave_properties[i].offset);
			GtkWidget *combo;

			gtk_table_attach(GTK_TABLE(table), gd_label_new_printf(_(gd_cave_properties[i].name)), 0, 1, row, row+1, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 0, 0);

			combo=gd_color_combo_new (*value);
			combos=g_list_append(combos, combo);
			g_object_set_data(G_OBJECT(combo), GDASH_PCOLOR, value);
			gtk_widget_set_tooltip_text(combo, _(gd_cave_properties[i].tooltip));
			g_signal_connect(combo, "changed", G_CALLBACK(colorbutton_changed), value);
			gtk_table_attach(GTK_TABLE(table), combo, 1, 2, row, row+1, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_SHRINK, 0, 0);
			row++;
		}
	g_signal_connect(button, "clicked", G_CALLBACK(colorbutton_random_changed), combos);
	
	gtk_widget_show_all(dialog);
	colorchange_update_disabled=FALSE;
	gtk_dialog_run(GTK_DIALOG(dialog));
	g_list_free(combos);
	gtk_widget_destroy(dialog);
}
#undef GDASH_PCOLOR

/***************************************************
 *
 * CAVE EDITING CALLBACKS
 *
 */

/* delete selected cave drawing element or cave.
*/
static void
delete_selected_cb (GtkWidget *widget, gpointer data)
{
	/* deleting caves or cave object. */
	if (edited_cave==NULL) {
		/* WE ARE DELETING ONE OR MORE CAVES HERE */
		GList *list, *listiter;
		GtkTreeModel *model;
		GList *references=NULL;
		GtkWidget *dialog;
		int response;
		
		/* first we ask the user if he is sure, as no undo is implemented yet */
		dialog=gtk_message_dialog_new (GTK_WINDOW (editor_window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _("Do you really want to delete cave(s)?"));
		response=gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		
		if (response!=GTK_RESPONSE_YES)
			return;
		list=gtk_icon_view_get_selected_items (GTK_ICON_VIEW (iconview_cavelist));
		g_return_if_fail (list!=NULL);	/* list should be not empty. otherwise why was the button not insensitized? */
		
		/* if anything was selected */
		model=gtk_icon_view_get_model(GTK_ICON_VIEW(iconview_cavelist));
		/* for all caves selected, convert to tree row references - we must delete them for the icon view, so this is necessary */
		for (listiter=list; listiter!=NULL; listiter=listiter->next)
			references=g_list_append(references, gtk_tree_row_reference_new(model, listiter->data));
		g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
		g_list_free (list);
		
		/* now check the list of references and delete each cave */
		for (listiter=references; listiter!=NULL; listiter=listiter->next) {
			GtkTreeRowReference *reference=listiter->data;
			GtkTreePath *path;
			GtkTreeIter iter;
			Cave *cave;
			
			path=gtk_tree_row_reference_get_path(reference);
			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_model_get (model, &iter, CAVE_COLUMN, &cave, -1);
			gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
			gd_cave_free (cave);	/* and also free memory associated. */
			g_hash_table_remove(cave_pixbufs, cave);
		}
		g_list_foreach (references, (GFunc) gtk_tree_row_reference_free, NULL);
		g_list_free (references);
		
		/* this modified the caveset */
		gd_caveset_edited=TRUE;
	} else {
		/* WE ARE DELETING A CAVE OBJECT HERE */
		GList *iter;

		g_return_if_fail(object_list_is_any_selected());
		
		undo_save();

		/* delete all objects */
		for (iter=selected_objects; iter!=NULL; iter=iter->next) {
			edited_cave->objects=g_list_remove(edited_cave->objects, iter->data);
			g_free(iter->data);
		}
		object_list_clear_selection();
		render_cave ();
	}
}

/* put selected drawing elements to bottom.
*/
static void
bottom_selected_cb (GtkWidget *widget, gpointer data)
{
	GList *iter;
	
	g_return_if_fail (object_list_is_any_selected());

	undo_save();
	
	/* we reverse the list, as prepending changes the order */
	selected_objects=g_list_reverse(selected_objects);
	for (iter=selected_objects; iter!=NULL; iter=iter->next) {
		/* remove from original place */
		edited_cave->objects=g_list_remove(edited_cave->objects, iter->data);
		/* put to beginning */
		edited_cave->objects=g_list_prepend(edited_cave->objects, iter->data);
	}
	render_cave ();
}

/* bring selected drawing element to top.
*/
static void
top_selected_cb (GtkWidget *widget, gpointer data)
{
	GList *iter;
	
	g_return_if_fail (object_list_is_any_selected());

	undo_save();

	for (iter=selected_objects; iter!=NULL; iter=iter->next) {
		/* remove from original place */
		edited_cave->objects=g_list_remove(edited_cave->objects, iter->data);
		/* put to beginning */
		edited_cave->objects=g_list_append(edited_cave->objects, iter->data);
	}
	render_cave ();
}

/* copy selected object or caves to clipboard.
*/
static void
copy_selected_cb (GtkWidget *widget, gpointer data)
{
	if (edited_cave==NULL) {
		/* WE ARE NOW COPYING CAVES FROM A CAVESET */
		GList *list, *listiter;
		GtkTreeModel *model;

		list=gtk_icon_view_get_selected_items (GTK_ICON_VIEW (iconview_cavelist));
		g_return_if_fail (list!=NULL);	/* list should be not empty. otherwise why was the button not insensitized? */
		
		/* forget old clipboard */
		g_list_foreach(cave_clipboard, (GFunc) gd_cave_free, NULL);
		g_list_free(cave_clipboard);
		cave_clipboard=NULL;

		/* now for all caves selected */
		/* we do not need references here (as in cut), as we do not modify the treemodel */
		model=gtk_icon_view_get_model (GTK_ICON_VIEW (iconview_cavelist));
		for (listiter=list; listiter!=NULL; listiter=listiter->next) {
			Cave *cave=NULL;
			GtkTreeIter iter;
			
			gtk_tree_model_get_iter (model, &iter, listiter->data);
			gtk_tree_model_get (model, &iter, CAVE_COLUMN, &cave, -1);
			/* add to clipboard: prepend must be used for correct order */
			/* here, a COPY is added to the clipboard */
			cave_clipboard=g_list_prepend(cave_clipboard, gd_cave_new_from_cave(cave));
		}
		g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
		g_list_free (list);

		/* enable pasting */
		gtk_action_group_set_sensitive (actions_clipboard_paste, TRUE);
	} else {
		GList *iter;
		
		/* delete contents of clipboard */
		g_list_foreach(object_clipboard, (GFunc) g_free, NULL);
		object_clipboard=NULL;
		
		for(iter=selected_objects; iter!=NULL; iter=iter->next)
			object_clipboard=g_list_append(object_clipboard, g_memdup(iter->data, sizeof(GdObject)));
		/* enable pasting */
		gtk_action_group_set_sensitive (actions_clipboard_paste, TRUE);
	}
}

/* paste object or cave from clipboard
*/
static void
paste_clipboard_cb (GtkWidget *widget, gpointer data)
{
	if (edited_cave==NULL) {
		/* WE ARE IN THE CAVESET ICON VIEW */
		GList *iter;
		GtkListStore *store=GTK_LIST_STORE(gtk_icon_view_get_model(GTK_ICON_VIEW(iconview_cavelist)));
		
		for(iter=cave_clipboard; iter!=NULL; iter=iter->next) {
			Cave *cave=iter->data;
			
			add_cave_to_icon_view(store, gd_cave_new_from_cave(cave));
		}
		
		/* this modified the caveset */
		gd_caveset_edited=TRUE;
	} else {
		/* WE ARE IN THE CAVE EDITOR */
		GList *iter;
		GList *new_objects=NULL;
		
		g_return_if_fail (object_clipboard!=NULL);

		/* we have a list of newly added (pasted) objects, so after pasting we can
		   select them. this is necessary, as only after pasting is render_cave() called,
		   which adds the new objects to the gtkliststore. otherwise that one would not
		   contain cave objects. the clipboard also cannot be used, as pointers are different */
		for (iter=object_clipboard; iter!=NULL; iter=iter->next) {
			GdObject *new_object=g_memdup(iter->data, sizeof(GdObject));

			edited_cave->objects=g_list_append(edited_cave->objects, new_object);
			new_objects=g_list_prepend(new_objects, new_object);	/* prepend is ok, as order does not matter */
		}

		render_cave();
		object_list_clear_selection();
		for (iter=new_objects; iter!=NULL; iter=iter->next)
			object_list_add_to_selection(iter->data);
		g_list_free(new_objects);
	}
}


/* cut an object, or cave(s) from the caveset. */
static void
cut_selected_cb (GtkWidget *widget, gpointer data)
{
	if (edited_cave==NULL) {
		/* WE ARE NOW CUTTING CAVES FROM A CAVESET */
		GList *list, *listiter;
		GtkTreeModel *model;
		GList *references=NULL;

		list=gtk_icon_view_get_selected_items (GTK_ICON_VIEW (iconview_cavelist));
		g_return_if_fail (list!=NULL);	/* list should be not empty. otherwise why was the button not insensitized? */
		
		/* forget old clipboard */
		g_list_foreach(cave_clipboard, (GFunc) gd_cave_free, NULL);
		g_list_free(cave_clipboard);
		cave_clipboard=NULL;

		/* if anything was selected */
		model=gtk_icon_view_get_model (GTK_ICON_VIEW (iconview_cavelist));
		/* for all caves selected, convert to tree row references - we must delete them for the icon view, so this is necessary */
		for (listiter=list; listiter!=NULL; listiter=listiter->next)
			references=g_list_append(references, gtk_tree_row_reference_new(model, listiter->data));
		g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
		g_list_free (list);
		
		for (listiter=references; listiter!=NULL; listiter=listiter->next) {
			GtkTreeRowReference *reference=listiter->data;
			GtkTreePath *path;
			Cave *cave=NULL;
			GtkTreeIter iter;
			
			path=gtk_tree_row_reference_get_path(reference);
			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_model_get (model, &iter, CAVE_COLUMN, &cave, -1);
			/* prepend must be used for correct order */
			/* here, the cave is not copied, but the pointer is moved to the clipboard */
			cave_clipboard=g_list_prepend(cave_clipboard, cave);
			gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
			/* remove its pixbuf */
			g_hash_table_remove(cave_pixbufs, cave);
		}
		g_list_foreach (references, (GFunc) gtk_tree_row_reference_free, NULL);
		g_list_free (references);

		/* enable pasting */
		gtk_action_group_set_sensitive (actions_clipboard_paste, TRUE);
		
		/* this modified the caveset */
		gd_caveset_edited=TRUE;
	} else {
		GList *iter;
		/* EDITED OBJECT IS NOT NULL, SO WE ARE CUTTING OBJECTS */
		undo_save();
	
		/* delete contents of clipboard */
		g_list_foreach(object_clipboard, (GFunc) g_free, NULL);
		object_clipboard=NULL;
		
		/* do not make a copy; rather the clipboard holds the reference from now on. */
		for(iter=selected_objects; iter!=NULL; iter=iter->next) {
			object_clipboard=g_list_append(object_clipboard, iter->data);
			edited_cave->objects=g_list_remove(edited_cave->objects, iter->data);
		}

		/* enable pasting */
		gtk_action_group_set_sensitive (actions_clipboard_paste, TRUE);

		object_list_clear_selection();
		render_cave ();
	}
}

static void
select_all_cb (GtkWidget *widget, gpointer data)
{
	if (edited_cave==NULL)	/* in game editor */
		gtk_icon_view_select_all(GTK_ICON_VIEW(iconview_cavelist));		/* SELECT ALL CAVES */
	else					/* in cave editor */
		gtk_tree_selection_select_all(gtk_tree_view_get_selection(GTK_TREE_VIEW(object_tree_view)));		/* SELECT ALL OBJECTS */
}

/* delete all cave elements */
static void
clear_cave_elements_cb (GtkWidget *widget, gpointer data)
{

	g_return_if_fail (edited_cave!=NULL);

	if (edited_cave->objects) {
		GtkWidget *dialog=gtk_message_dialog_new (GTK_WINDOW (editor_window), 0,
													GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
													_("Do you really want to clear cave objects?"));

		if (gtk_dialog_run(GTK_DIALOG (dialog))==GTK_RESPONSE_YES) {
			undo_save();	/* changing cave; save for undo */

			g_list_foreach(edited_cave->objects, (GFunc) g_free, NULL);
			edited_cave->objects=NULL;
			/* element deleted; redraw cave */
			render_cave();
		}
		gtk_widget_destroy (dialog);
	}
}

/* delete map from cave */
static void
remove_map_cb (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	g_return_if_fail (edited_cave!=NULL);
	g_return_if_fail (edited_cave->map!=NULL);

	dialog=gtk_message_dialog_new (GTK_WINDOW (editor_window), 0, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _("Do you really want to remove cave map?"));
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), _("This operation destroys all cave objects."));

	if (gtk_dialog_run (GTK_DIALOG (dialog))==GTK_RESPONSE_YES) {
		undo_save(); /* changing cave; save for undo */

		gd_cave_map_free(edited_cave->map);
		edited_cave->map=NULL;
		/* map deleted; redraw cave */
		render_cave();
	}
	gtk_widget_destroy (dialog);
}

/* flatten cave -> pack everything in a map */
static void
flatten_cave_cb (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;

	g_return_if_fail (edited_cave!=NULL);

	if (!edited_cave->objects) {
		gd_infomessage(_("This cave has no objects."), NULL);
		return;
	}

	dialog=gtk_message_dialog_new (GTK_WINDOW (editor_window), 0, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _("Do you really want to flatten cave?"));
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), _("This operation merges all cave objects currently seen in a single map. Further objects may later be added, but the ones already seen will behave like the random fill elements; they will not be editable."));

	if (gtk_dialog_run (GTK_DIALOG (dialog))==GTK_RESPONSE_YES) {
		undo_save();	/* changing; save for undo */

		gd_flatten_cave (edited_cave, edit_level);
		render_cave ();
	}
	gtk_widget_destroy (dialog);
}


static void
save_html_cb (GtkWidget *widget, gpointer data)
{
	char *htmlname=NULL, *suggested_name;
	/* if no filename given, */
	GtkWidget *dialog;
	GtkFileFilter *filter;
	Cave *edited;
	
	edited=edited_cave;
	/* destroy icon view so it does not interfere with saving (when icon view is active, caveset=NULL) */
	if (iconview_cavelist)
		gtk_widget_destroy(iconview_cavelist);

	dialog=gtk_file_chooser_dialog_new (_("Save Cave Set in HTML"), GTK_WINDOW (editor_window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	filter=gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("HTML files"));
	gtk_file_filter_add_pattern (filter, "*.html");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	suggested_name=g_strdup_printf("%s.html", gd_default_cave->name);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), suggested_name);
	g_free(suggested_name);

	if (gtk_dialog_run (GTK_DIALOG (dialog))==GTK_RESPONSE_ACCEPT)
		htmlname=gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);

	/* saving if got filename */
	if (htmlname) {
		gd_clear_error_flag();
		gd_save_html(htmlname, editor_window);
		if (gd_has_new_error())
			gd_show_last_error();
	}

	g_free(htmlname);
	select_cave_for_edit(edited);	/* go back to edited cave or recreate icon view */
}



/* export cave to a crli editor format */
static void
export_cavefile_cb (GtkWidget *widget, gpointer data)
{
	char *outname=NULL;
	/* if no filename given, */
	GtkWidget *dialog;
	
	g_return_if_fail(edited_cave!=NULL);

	dialog=gtk_file_chooser_dialog_new (_("Export Cave as CrLi Cave File"), GTK_WINDOW (editor_window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), edited_cave->name);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
	
	if (gtk_dialog_run (GTK_DIALOG (dialog))==GTK_RESPONSE_ACCEPT)
		outname=gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);

	/* if accepted, save. */
	if (outname) {
		gd_clear_error_flag();
		gd_export_cave_to_crli_cavefile(edited_cave, edit_level, outname);
		if(gd_has_new_error())
			gd_show_errors();
		g_free(outname);
	}
}


/* export complete caveset to a crli cave pack */
static void
export_cavepack_cb (GtkWidget *widget, gpointer data)
{
	char *outname=NULL;
	/* if no filename given, */
	GtkWidget *dialog;
	Cave *edited;
	
	edited=edited_cave;
	/* destroy icon view so it does not interfere with saving (when icon view is active, caveset=NULL) */
	if (iconview_cavelist)
		gtk_widget_destroy(iconview_cavelist);

	dialog=gtk_file_chooser_dialog_new (_("Export Cave as CrLi Cave Pack"), GTK_WINDOW (editor_window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), gd_default_cave->name);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (gtk_dialog_run (GTK_DIALOG (dialog))==GTK_RESPONSE_ACCEPT)
		outname=gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);

	/* saving if got filename */
	if (outname) {
		gd_clear_error_flag();
		gd_export_cave_list_to_crli_cavepack(gd_caveset, edit_level, outname);
		if(gd_has_new_error())
			gd_show_errors();
		g_free(outname);
	}

	select_cave_for_edit(edited);	/* go back to edited cave or recreate icon view */
}



/* test selected level. main window is brought to focus.
*/
static void
play_level_cb (GtkWidget *widget, gpointer data)
{
	Cave *playcave;

	g_return_if_fail (rendered_cave!=NULL);
	
	/* make copy */
	playcave=gd_cave_new_from_cave(rendered_cave);
	g_snprintf(playcave->name, sizeof(playcave->name), _("Testing %s"), edited_cave->name);
	gd_cave_setup_for_game(playcave);
	gd_main_start_level(playcave);	/* start the level as a snapshot */
	gd_cave_free(playcave);	/* now the game module stores its own copy */
}



static void
object_properties_cb (GtkWidget *widget, gpointer data)
{
	object_properties(NULL);
}



/* edit caveset properties */
/* ... edit gd_default_cave storing default values. */
static void
set_caveset_properties_cb (const GtkWidget *widget, const gpointer data)
{
	cave_properties (gd_default_cave, TRUE);
}


static void
cave_properties_cb (const GtkWidget *widget, const gpointer data)
{
	cave_properties (edited_cave, FALSE);
}



/************************************************
 *
 * MANAGING CAVES
 *
 */

/* go to cave selector */
static void
cave_selector_cb (GtkWidget *widget, gpointer data)
{
	select_cave_for_edit(NULL);
}

/* create new cave */
static void
new_cave_cb (GtkWidget *widget, gpointer data)
{
	Cave *newcave;

	/* create new cave data structure */
	newcave=gd_cave_new();
	g_strlcpy(newcave->name, _("New cave"), sizeof(newcave->name));
	gd_cave_set_random_colors(newcave);

	/* start editing immediately; also this destroys the icon view and recreates GList *caveset */
	select_cave_for_edit (newcave);

	/* append it to caveset (WHICH WAS RECREATED BY DESTROYING THE ICON VIEW, which was triggered by select_cave_for_edit) */
	gd_caveset=g_list_append (gd_caveset, newcave);
	gd_caveset_edited=TRUE;
}


/**
 * caveset file operations.
 * in each, we destroy the iconview, as it might store the modified order of caves!
 * then it is possible to load, save, and the like.
 * after any operation, activate caveset editor again
 */
static void
open_caveset_cb (GtkWidget *widget, gpointer data)
{
	/* destroy icon view so it does not interfere */
	if (iconview_cavelist)
		gtk_widget_destroy(iconview_cavelist);
	g_hash_table_remove_all(cave_pixbufs);
	gd_open_caveset (editor_window, NULL);
	select_cave_for_edit(NULL);
}

static void
save_caveset_as_cb (GtkWidget *widget, gpointer data)
{
	Cave *edited;
	
	edited=edited_cave;
	/* destroy icon view so it does not interfere with the cave order, and the order of caves is saved */
	if (iconview_cavelist)
		gtk_widget_destroy(iconview_cavelist);
	gd_save_caveset_as_cb (widget, data);
	select_cave_for_edit(edited);	/* go back to edited cave or recreate icon view */
}

static void
save_caveset_cb (GtkWidget *widget, gpointer data)
{
	Cave *edited;
	
	edited=edited_cave;
	/* destroy icon view so it does not interfere with the cave order, and the order of caves is saved */
	if (iconview_cavelist)
		gtk_widget_destroy(iconview_cavelist);
	gd_save_caveset_cb (widget, data);
	select_cave_for_edit(edited);	/* go back to edited cave or recreate icon view */
}

static void
new_caveset_cb (GtkWidget *widget, gpointer data)
{
	/* destroy icon view so it does not interfere */
	if (iconview_cavelist)
		gtk_widget_destroy(iconview_cavelist);
	if (gd_discard_changes(editor_window)) {
		gd_caveset_clear();
		g_hash_table_remove_all(cave_pixbufs);
	}
	select_cave_for_edit(NULL);
}





/* make all caves selectable */
static void
selectable_all_cb(GtkWidget *widget, gpointer data)
{
	Cave *edited;
	GList *iter;
	
	edited=edited_cave;
	/* destroy icon view so it does not interfere with the cave order, and the order of caves is saved */
	if (iconview_cavelist)
		gtk_widget_destroy(iconview_cavelist);
	for (iter=gd_caveset; iter!=NULL; iter=iter->next) {
		Cave *cave=(Cave *)iter->data;
		
		if (!cave->selectable) {
			cave->selectable=TRUE;
			g_hash_table_remove(cave_pixbufs, cave);
		}
	}
	select_cave_for_edit(edited);	/* go back to edited cave or recreate icon view */
}

/* make all but intermissions selectable */
static void
selectable_all_but_intermissions_cb(GtkWidget *widget, gpointer data)
{
	Cave *edited;
	GList *iter;
	
	edited=edited_cave;
	/* destroy icon view so it does not interfere with the cave order, and the order of caves is saved */
	if (iconview_cavelist)
		gtk_widget_destroy(iconview_cavelist);
	for (iter=gd_caveset; iter!=NULL; iter=iter->next) {
		Cave *cave=(Cave *)iter->data;
		gboolean desired=!cave->intermission;
		
		if (cave->selectable!=desired) {
			cave->selectable=desired;
			g_hash_table_remove(cave_pixbufs, cave);
		}
	}
	select_cave_for_edit(edited);	/* go back to edited cave or recreate icon view */
}


/* make all after intermissions selectable */
static void
selectable_all_after_intermissions_cb(GtkWidget *widget, gpointer data)
{
	gboolean was_intermission=TRUE;	/* treat the 'zeroth' cave as intermission, so the very first cave will be selectable */
	Cave *edited;
	GList *iter;
	
	edited=edited_cave;
	/* destroy icon view so it does not interfere with the cave order, and the order of caves is saved */
	if (iconview_cavelist)
		gtk_widget_destroy(iconview_cavelist);
	for (iter=gd_caveset; iter!=NULL; iter=iter->next) {
		Cave *cave=(Cave *)iter->data;
		gboolean desired;
		
		desired=!cave->intermission && was_intermission;	/* selectable if this is a normal cave, and the previous one was an interm. */
		if (cave->selectable!=desired) {
			cave->selectable=desired;
			g_hash_table_remove(cave_pixbufs, cave);
		}

		was_intermission=cave->intermission;	/* remember for next iteration */
	}
	select_cave_for_edit(edited);	/* go back to edited cave or recreate icon view */
}





/******************************************************
 *
 * some necessary callbacks for the editor
 *
 *
 */

/* level shown in editor */
static void
level_scale_changed_cb (GtkWidget *widget, gpointer data)
{
	edit_level=gtk_range_get_value (GTK_RANGE (widget))-1;
	render_cave();				/* re-render cave */
}

/* edit tool selected */
static void
action_objects_cb (GtkWidget *widget, gpointer data)
{
	action=gtk_radio_action_get_current_value (GTK_RADIO_ACTION (widget));
	
	/* actions below zero are not real objects */
	if (action>0 || action==FREEHAND) {
		int number=action;	/* index in array */
		
		if (action==FREEHAND)	/* freehand tool places points, so we use the labels from the point */
			number=POINT;
		
		gtk_label_set_text(GTK_LABEL(label_first_element), _(gd_object_description[number].first_button));
		gtk_widget_set_sensitive(element_button, gd_object_description[number].first_button!=NULL);
		gtk_label_set_text(GTK_LABEL(label_second_element), _(gd_object_description[number].second_button));
		gtk_widget_set_sensitive(fillelement_button, gd_object_description[number].second_button!=NULL);
	} else {
		gtk_label_set_text(GTK_LABEL(label_first_element), NULL);
		gtk_widget_set_sensitive(element_button, FALSE);
		gtk_label_set_text(GTK_LABEL(label_second_element), NULL);
		gtk_widget_set_sensitive(fillelement_button, FALSE);
	}
}

static void
toggle_game_view_cb (GtkWidget *widget, gpointer data)
{
	gd_game_view=gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (widget));
}

static void
toggle_colored_objects_cb (GtkWidget *widget, gpointer data)
{
	gd_colored_objects=gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (widget));
}

static void
toggle_object_list_cb (GtkWidget *widget, gpointer data)
{
	gd_show_object_list=gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (widget));
	if (gd_show_object_list && edited_cave)
		gtk_widget_show (scroll_window_objects);	/* show the scroll window containing the view */
	else
		gtk_widget_hide (scroll_window_objects);
}

static void
toggle_test_label_cb (GtkWidget *widget, gpointer data)
{
	gd_show_test_label=gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(widget));
}


/* destroy editor window - do some cleanups */
static gboolean
destroy_event (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	/* remove drawing interrupt. */
	g_source_remove_by_user_data (drawcave_int);
	/* if cave is drawn, free. */
	gd_cave_free (rendered_cave);
	rendered_cave=NULL;
	
	g_hash_table_destroy(cave_pixbufs);
	/* stop test is running. this also restores main window action sensitized states */
	gd_main_stop_game();

	return FALSE;
}

static void
close_editor_cb (GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(editor_window);
}




static void
highscore_cb(GtkWidget *widget, gpointer data)
{
	/* if the iconview is active, we destroy it, as it interferes with GList *caveset.
	   after highscore editing, we call editcave(null) to recreate */
	gboolean has_iconview=iconview_cavelist!=NULL;
	
	if (has_iconview)
		gtk_widget_destroy(iconview_cavelist);
	gd_show_highscore(editor_window, edited_cave?edited_cave:gd_default_cave, TRUE, NULL);
	if (has_iconview)
		select_cave_for_edit(NULL);
}


/* shift cave map left, one step. */
static void
shift_left_cb(GtkWidget *widget, gpointer data)
{
	int y;
	
	g_return_if_fail(edited_cave!=NULL);
	g_return_if_fail(edited_cave->map!=NULL);
	
	for (y=0; y<edited_cave->h; y++) {
		GdElement temp;
		int x;
		
		temp=edited_cave->map[y][0];
		for (x=0; x<edited_cave->w-1; x++)
			edited_cave->map[y][x]=edited_cave->map[y][x+1];
		edited_cave->map[y][edited_cave->w-1]=temp; 
	}
	render_cave();
}

/* shift cave map right, one step. */
static void
shift_right_cb(GtkWidget *widget, gpointer data)
{
	int y;
	
	g_return_if_fail(edited_cave!=NULL);
	g_return_if_fail(edited_cave->map!=NULL);
	
	for (y=0; y<edited_cave->h; y++) {
		GdElement temp;
		int x;
		
		temp=edited_cave->map[y][edited_cave->w-1];
		for (x=edited_cave->w-2; x>=0; x--)
			edited_cave->map[y][x+1]=edited_cave->map[y][x];
		edited_cave->map[y][0]=temp; 
	}
	render_cave();
}

/* shift cave map up, one step. */
static void
shift_up_cb(GtkWidget *widget, gpointer data)
{
	int x, y;
	GdElement *temp;
	g_return_if_fail(edited_cave!=NULL);
	g_return_if_fail(edited_cave->map!=NULL);
	
	/* remember first line */
	temp=g_new(GdElement, edited_cave->w);
	for (x=0; x<edited_cave->w; x++)
		temp[x]=edited_cave->map[0][x];

	/* copy everything up */	
	for (y=0; y<edited_cave->h-1; y++)
		for (x=0; x<edited_cave->w; x++)
			edited_cave->map[y][x]=edited_cave->map[y+1][x];

	for (x=0; x<edited_cave->w; x++)
		edited_cave->map[edited_cave->h-1][x]=temp[x];
	g_free(temp);
	render_cave();
}

/* shift cave map down, one step. */
static void
shift_down_cb(GtkWidget *widget, gpointer data)
{
	int x, y;
	GdElement *temp;
	g_return_if_fail(edited_cave!=NULL);
	g_return_if_fail(edited_cave->map!=NULL);
	
	/* remember last line */
	temp=g_new(GdElement, edited_cave->w);
	for (x=0; x<edited_cave->w; x++)
		temp[x]=edited_cave->map[edited_cave->h-1][x];

	/* copy everything up */	
	for (y=edited_cave->h-2; y>=0; y--)
		for (x=0; x<edited_cave->w; x++)
			edited_cave->map[y+1][x]=edited_cave->map[y][x];

	for (x=0; x<edited_cave->w; x++)
		edited_cave->map[0][x]=temp[x];
	g_free(temp);

	render_cave();
}






/*******
 * SIGNALS which are used when the object list is reordered by drag and drop.
 *
 * non-trivial.
 * drag and drop emits the following signals: insert, changed, and delete.
 * so there is a moment when the reordered row has TWO copies in the model.
 * therefore we cannot use the changed signal to see the new order.
 *
 * we also cannot use the delete signal on its own, as it is also emitted when
 * rendering the cave.
 *
 * instead, we connect to the changed and the delete signal. we have a flag,
 * named "rowchangedbool", which is set to true by the changed signal, and
 * therefore we know that the next delete signal is emitted by a reordering.
 * at that point, we have the new order, and _AFTER_ resetting the flag,
 * we rerender the cave. */
static gboolean rowchangedbool=FALSE;

static void
row_changed(GtkTreeModel *model, GtkTreePath *changed_path, GtkTreeIter *changed_iter, gpointer user_data)
{
	/* set the flag so we know that the next delete signal is emitted by a drag-and-drop */
	rowchangedbool=TRUE;
}

static gboolean
object_list_new_order_func(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	GList **list=(GList **)data;
	GdObject *object;
	
	gtk_tree_model_get(model, iter, POINTER_COLUMN, &object, -1);
	*list=g_list_append(*list, object);
	
	return FALSE;	/* continue traversing */
}

static void
row_delete(GtkTreeModel *model, GtkTreePath *changed_path, gpointer user_data)
{
	if (rowchangedbool) {
		GList *new_order=NULL;

		/* reset the flag, as we handle the reordering here. */
		rowchangedbool=FALSE;
		
		/* this will build the new object order to the list */
		gtk_tree_model_foreach(model, object_list_new_order_func, &new_order);
		
		g_list_free(edited_cave->objects);
		edited_cave->objects=new_order;

		render_cave();
	}
}

/* row activated - set properties of object */
static void
object_tree_view_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
	GtkTreeModel *model=gtk_tree_view_get_model(tree_view);
	GtkTreeIter iter;
	GdObject *object;
	
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, POINTER_COLUMN, &object, -1);
	object_properties(object);
	/* render cave after changing object properties */
	render_cave();
}







/*
 *
 * start cave editor.
 *
 */

void
cave_editor_cb (GtkWidget *widget, gpointer data)
{
	/* normal menu items */
	static GtkActionEntry action_entries_normal[]={
		{"FileMenu", NULL, N_("_File")},
		{"CaveMenu", NULL, N_("_Cave")},
		{"EditMenu", NULL, N_("_Edit")},
		{"ViewMenu", NULL, N_("_View")},
		{"ToolsMenu", NULL, N_("_Tools")},
		{"HelpMenu", NULL, N_("_Help")},
		{"Close", GTK_STOCK_CLOSE, NULL, NULL, N_("Close cave editor"), G_CALLBACK(close_editor_cb)},
		{"NewCave", GTK_STOCK_NEW, N_("New _cave"), NULL, N_("Create new cave"), G_CALLBACK(new_cave_cb)},
		{"Help", GTK_STOCK_HELP, NULL, NULL, NULL, G_CALLBACK(help_cb)},
		{"SaveFile", GTK_STOCK_SAVE, NULL, NULL, N_("Save cave set to file"), G_CALLBACK(save_caveset_cb)},
		{"SaveAsFile", GTK_STOCK_SAVE_AS, NULL, NULL, N_("Save cave set as new file"), G_CALLBACK(save_caveset_as_cb)},
		{"OpenFile", GTK_STOCK_OPEN, NULL, NULL, N_("Load cave set from file"), G_CALLBACK(open_caveset_cb)},
		{"HighScore", GD_ICON_AWARD, N_("Hi_ghscores"), NULL, NULL, G_CALLBACK(highscore_cb)},
		{"SelectAll", GTK_STOCK_SELECT_ALL, NULL, "<control>A", N_("Select all items"), G_CALLBACK(select_all_cb)},
		{"CaveSetProps", GTK_STOCK_PROPERTIES, N_("Cave set _properties"), NULL, N_("Set properties of cave set"), G_CALLBACK(set_caveset_properties_cb)},
	};

	/* cave_selector menu items */
	static const GtkActionEntry action_entries_cave_selector[]={
		{"EditCave", GD_ICON_CAVE_EDITOR, N_("_Edit cave"), NULL, N_("Edit selected cave"), G_CALLBACK(edit_cave_cb)},
		{"RenameCave", NULL, N_("_Rename cave"), NULL, N_("Rename selected cave"), G_CALLBACK(rename_cave_cb)},
		{"MakeSelectable", NULL, N_("Make cave _selectable"), NULL, N_("Make the cave selectable as game start"), G_CALLBACK(cave_make_selectable_cb)},
		{"MakeUnselectable", NULL, N_("Make cave _unselectable"), NULL, N_("Make the cave unselectable as game start"), G_CALLBACK(cave_make_unselectable_cb)},
	};

	/* caveset editing */
	static const GtkActionEntry action_entries_edit_caveset[]={
		{"NewCaveset", GTK_STOCK_NEW, N_("_New cave set"), "", N_("Create new cave set with no caves"), G_CALLBACK(new_caveset_cb)},
		{"SaveHTML", GTK_STOCK_FILE, N_("Save _HTML gallery"), NULL, N_("Save game in a HTML gallery"), G_CALLBACK(save_html_cb)},
		{"ExportCavePack", GTK_STOCK_CONVERT, N_("Export _CrLi cave pack"), NULL, NULL, G_CALLBACK(export_cavepack_cb)},
		{"SelectMenu", NULL, N_("_Make caves selectable")},
		{"AllCavesSelectable", NULL, N_("All _caves"), NULL, N_("Make all caves selectable as game start"), G_CALLBACK(selectable_all_cb)},
		{"AllButIntermissionsSelectable", NULL, N_("All _but intermissions"), NULL, N_("Make all caves but intermissions selectable as game start"), G_CALLBACK(selectable_all_but_intermissions_cb)},
		{"AllAfterIntermissionsSelectable", NULL, N_("All _after intermissions"), NULL, N_("Make all caves after intermissions selectable as game start"), G_CALLBACK(selectable_all_after_intermissions_cb)},
	};

	/* normal menu items */
	static const GtkActionEntry action_entries_edit_cave[]={
		{"ExportAsCrLiCave", GTK_STOCK_CONVERT, N_("_Export as CrLi cave file"), NULL, NULL, G_CALLBACK(export_cavefile_cb)},
		{"CaveSelector", GTK_STOCK_INDEX, NULL, "Escape", N_("Open cave selector"), G_CALLBACK(cave_selector_cb)},
		{"Test", GTK_STOCK_MEDIA_PLAY, N_("_Test"), "<control>T", N_("Test cave"), G_CALLBACK(play_level_cb)},
		{"CaveProperties", GTK_STOCK_PROPERTIES, N_("Ca_ve properties"), NULL, N_("Cave settings"), G_CALLBACK(cave_properties_cb)},
		{"CaveColors", GTK_STOCK_SELECT_COLOR, NULL, NULL, N_("Select cave colors"), G_CALLBACK(cave_colors_cb)},
		{"RemoveObjects", GTK_STOCK_CLEAR, N_("Remove objects"), NULL, N_("Clear cave objects"), G_CALLBACK(clear_cave_elements_cb)},
		{"FlattenCave", NULL, N_("Convert to map"), NULL, N_("Flatten cave to a single cave map without objects"), G_CALLBACK(flatten_cave_cb)},
		{"Overview", GTK_STOCK_ZOOM_FIT, N_("O_verview"), NULL, N_("Full screen overview of cave"), G_CALLBACK(cave_overview_cb)},
		{"OverviewGame", GTK_STOCK_ZOOM_FIT, N_("O_verview (game)"), NULL, N_("Full screen overview of cave as in game"), G_CALLBACK(cave_overview_game_cb)},
		{"AutoShrink", NULL, N_("_Auto shrink"), NULL, N_("Automatically set the visible part of the cave"), G_CALLBACK(auto_shrink_cave_cb)},
	};

	/* action entries which relate to a selected cave element (line, rectangle...) */
	static const GtkActionEntry action_entries_edit_object[]={
		{"Bottom", GD_ICON_TO_BOTTOM, N_("To _bottom"), "End", N_("Push object to bottom"), G_CALLBACK(bottom_selected_cb)},
		{"Top", GD_ICON_TO_TOP, N_("To t_op"), "Home", N_("Bring object to top"), G_CALLBACK(top_selected_cb)},
	};

	static const GtkActionEntry action_entries_edit_one_object[]={
		{"ObjectProperties", GTK_STOCK_PREFERENCES, N_("Ob_ject properties"), NULL, N_("Set object properties"), G_CALLBACK(object_properties_cb)},
	};

	/* map actions */
	static const GtkActionEntry action_entries_edit_map[]={
		{"MapMenu", NULL, N_("Map")},
		{"ShiftLeft", GTK_STOCK_GO_BACK, N_("Shift _left"), NULL, NULL, G_CALLBACK(shift_left_cb)},
		{"ShiftRight", GTK_STOCK_GO_FORWARD, N_("Shift _right"), NULL, NULL, G_CALLBACK(shift_right_cb)},
		{"ShiftUp", GTK_STOCK_GO_UP, N_("Shift _up"), NULL, NULL, G_CALLBACK(shift_up_cb)},
		{"ShiftDown", GTK_STOCK_GO_DOWN, N_("Shift _down"), NULL, NULL, G_CALLBACK(shift_down_cb)},
		{"RemoveMap", NULL, N_("Remove m_ap"), NULL, N_("Remove cave map, if it has one"), G_CALLBACK(remove_map_cb)},
	};

	/* random element actions */
	static const GtkActionEntry action_entries_edit_random[]={
		{"SetupRandom", GD_ICON_RANDOM_FILL, N_("Setup cave _random fill"), NULL, N_("Setup initial fill random elements for the cave"), G_CALLBACK(cave_random_setup_cb)},
	};
	
	/* clipboard actions */
	static const GtkActionEntry action_entries_clipboard[]={
		{"Cut", GTK_STOCK_CUT, NULL, NULL, N_("Cut to clipboard"), G_CALLBACK(cut_selected_cb)},
		{"Copy", GTK_STOCK_COPY, NULL, NULL, N_("Copy to clipboard"), G_CALLBACK(copy_selected_cb)},
		{"Delete", GTK_STOCK_DELETE, NULL, "Delete", N_("Delete"), G_CALLBACK(delete_selected_cb)},
	};

	/* clipboard paste */
	static const GtkActionEntry action_entries_clipboard_paste[]={
		{"Paste", GTK_STOCK_PASTE, NULL, NULL, N_("Paste object from clipboard"), G_CALLBACK(paste_clipboard_cb)},
	};

	/* action entries for undo */
	static const GtkActionEntry action_entries_edit_undo[]={
		{"Undo", GTK_STOCK_UNDO, NULL, "<control>Z", N_("Undo last action"), G_CALLBACK(undo_cb)},
	};

	/* action entries for redo */
	static const GtkActionEntry action_entries_edit_redo[]={
		{"Redo", GTK_STOCK_REDO, NULL, "<control><shift>Z", N_("Redo last action"), G_CALLBACK(redo_cb)},
	};

	/* edit commands */
	/* defined at the top of the file! */

	/* toggle buttons: nonstatic as they use values from settings */
	const GtkToggleActionEntry action_entries_toggle[]={
		{"SimpleView", NULL, N_("_Animated view"), NULL, N_("Animated view"), G_CALLBACK(toggle_game_view_cb), gd_game_view},
		{"ColoredObjects", NULL, N_("_Colored objects"), NULL, N_("Cave objects are coloured"), G_CALLBACK(toggle_colored_objects_cb), gd_colored_objects},
		{"ShowObjectList", GTK_STOCK_INDEX, N_("_Object list"), "F9", N_("Object list sidebar"), G_CALLBACK(toggle_object_list_cb), gd_show_object_list},
		{"ShowTestLabel", GTK_STOCK_INDEX, N_("_Show variables in test"), NULL, N_("Show a label during tests with some cave parameters"), G_CALLBACK(toggle_test_label_cb), gd_show_test_label}
	};

	static const char *ui_info =
		"<ui>"

		"<popup name='DrawingAreaPopup'>"
			"<menuitem action='Undo'/>"
			"<menuitem action='Redo'/>"
			"<separator/>"
			"<menuitem action='Cut'/>"
			"<menuitem action='Copy'/>"
			"<menuitem action='Paste'/>"
			"<menuitem action='Delete'/>"
			"<separator/>"
			"<menuitem action='Top'/>"
			"<menuitem action='Bottom'/>"
			"<menuitem action='ObjectProperties'/>"
			"<separator/>"
			"<menuitem action='CaveProperties'/>"
		"</popup>"

		"<popup name='CavesetPopup'>"
			"<menuitem action='Cut'/>"
			"<menuitem action='Copy'/>"
			"<menuitem action='Paste'/>"
			"<menuitem action='Delete'/>"
			"<separator/>"
			"<menuitem action='NewCave'/>"
			"<menuitem action='EditCave'/>"
			"<menuitem action='RenameCave'/>"
			"<menuitem action='MakeSelectable'/>"
			"<menuitem action='MakeUnselectable'/>"
		"</popup>"

		"<popup name='ObjectListPopup'>"
			"<menuitem action='Cut'/>"
			"<menuitem action='Copy'/>"
			"<menuitem action='Paste'/>"
			"<menuitem action='Delete'/>"
			"<separator/>"
			"<menuitem action='Top'/>"
			"<menuitem action='Bottom'/>"
			"<menuitem action='ObjectProperties'/>"
		"</popup>"

		"<menubar name='MenuBar'>"
			"<menu action='FileMenu'>"
				"<menuitem action='NewCave'/>"
				"<menuitem action='NewCaveset'/>"
				"<menuitem action='EditCave'/>"
				"<menuitem action='RenameCave'/>"
				"<menuitem action='MakeSelectable'/>"
				"<menuitem action='MakeUnselectable'/>"
				"<separator/>"
				"<menuitem action='OpenFile'/>"
				"<separator/>"
				"<menuitem action='SaveFile'/>"
				"<menuitem action='SaveAsFile'/>"
				"<separator/>"
				"<menuitem action='CaveSelector'/>"
				"<menuitem action='CaveSetProps'/>"
				"<menu action='SelectMenu'>"
					"<menuitem action='AllCavesSelectable'/>"
					"<menuitem action='AllButIntermissionsSelectable'/>"
					"<menuitem action='AllAfterIntermissionsSelectable'/>"
				"</menu>"
				"<separator/>"
				"<menuitem action='ExportCavePack'/>"
				"<menuitem action='ExportAsCrLiCave'/>"
				"<menuitem action='SaveHTML'/>"
				"<separator/>"
				"<menuitem action='Close'/>"
			"</menu>"
			"<menu action='EditMenu'>"
				"<menuitem action='Undo'/>"
				"<menuitem action='Redo'/>"
				"<separator/>"
				"<menuitem action='Cut'/>"
				"<menuitem action='Copy'/>"
				"<menuitem action='Paste'/>"
				"<menuitem action='Delete'/>"
				"<separator/>"
				"<menuitem action='SelectAll'/>"
				"<separator/>"
				"<menuitem action='Top'/>"
				"<menuitem action='Bottom'/>"
				"<menuitem action='ObjectProperties'/>"
				"<separator/>"
				"<menuitem action='SetupRandom'/>"
				"<menuitem action='CaveColors'/>"
				"<separator/>"
				"<menuitem action='HighScore'/>"
				"<menuitem action='CaveProperties'/>"
			"</menu>"
			"<menu action='ViewMenu'>"
				"<menuitem action='Overview'/>"
				"<menuitem action='OverviewGame'/>"
				"<separator/>"
				"<menuitem action='SimpleView'/>"
				"<menuitem action='ColoredObjects'/>"
				"<menuitem action='ShowObjectList'/>"
				"<menuitem action='ShowTestLabel'/>"
			"</menu>"
			"<menu action='ToolsMenu'>"
				"<menuitem action='Test'/>"
				"<separator/>"
				"<menuitem action='Move'/>"
				"<menuitem action='Plot'/>"
				"<menuitem action='Freehand'/>"
				"<menuitem action='Line'/>"
				"<menuitem action='Rectangle'/>"
				"<menuitem action='FilledRectangle'/>"
				"<menuitem action='Raster'/>"
				"<menuitem action='Join'/>"
				"<menuitem action='FloodFillBorder'/>"
				"<menuitem action='FloodFillReplace'/>"
				"<menuitem action='RandomFill'/>"
				"<menuitem action='Maze'/>"
				"<menuitem action='UnicursalMaze'/>"
				"<menuitem action='BraidMaze'/>"
				"<separator/>"
				"<menuitem action='Visible'/>"
				"<menuitem action='AutoShrink'/>"
				"<separator/>"
				"<menuitem action='FlattenCave'/>"
				"<menu action='MapMenu'>"
					"<menuitem action='ShiftLeft'/>"
					"<menuitem action='ShiftRight'/>"
					"<menuitem action='ShiftUp'/>"
					"<menuitem action='ShiftDown'/>"
					"<separator/>"
					"<menuitem action='RemoveMap'/>"
				"</menu>"
				"<menuitem action='RemoveObjects'/>"
			"</menu>"
			"<menu action='HelpMenu'>"
				"<menuitem action='Help'/>"
			"</menu>"
		"</menubar>"

		"<toolbar name='ToolBar'>"
			"<toolitem action='CaveSelector'/>"
			"<separator/>"
			"<toolitem action='ObjectProperties'/>"
			"<toolitem action='CaveProperties'/>"
			"<separator/>"
			"<toolitem action='Move'/>"
			"<toolitem action='Plot'/>"
			"<toolitem action='Freehand'/>"
			"<toolitem action='Line'/>"
			"<toolitem action='Rectangle'/>"
			"<toolitem action='FilledRectangle'/>"
			"<toolitem action='Raster'/>"
			"<toolitem action='Join'/>"
			"<toolitem action='FloodFillBorder'/>"
			"<toolitem action='FloodFillReplace'/>"
			"<toolitem action='RandomFill'/>"
			"<toolitem action='Maze'/>"
			"<toolitem action='UnicursalMaze'/>"
			"<toolitem action='BraidMaze'/>"
			"<separator/>"
			"<toolitem action='Test'/>"
		"</toolbar>"
		"</ui>";
	GtkWidget *vbox, *hbox;
	GtkUIManager *ui;
	GtkWidget *hbox_combo;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	if (editor_window) {
		/* if exists, only show it to the user. */
		gtk_window_present (GTK_WINDOW (editor_window));
		return;
	}
	
	/* hash table which stores cave pointer -> pixbufs. deleting a pixbuf calls g_object_unref. */
	cave_pixbufs=g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_object_unref);

	editor_window=gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (editor_window), 800, 520);
	g_signal_connect (G_OBJECT (editor_window), "destroy", G_CALLBACK(gtk_widget_destroyed), &editor_window);
	g_signal_connect (G_OBJECT (editor_window), "destroy", G_CALLBACK(destroy_event), NULL);

	vbox=gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (editor_window), vbox);

	/* menu and toolbar */
	actions_edit_tools=gtk_action_group_new ("edit_tools");
	gtk_action_group_set_translation_domain (actions_edit_tools, PACKAGE);
	gtk_action_group_add_radio_actions (actions_edit_tools, action_objects, G_N_ELEMENTS(action_objects), MOVE, G_CALLBACK(action_objects_cb), NULL);

	actions=gtk_action_group_new ("edit_normal");
	gtk_action_group_set_translation_domain (actions, PACKAGE);
	gtk_action_group_add_actions (actions, action_entries_normal, G_N_ELEMENTS(action_entries_normal), NULL);

	actions_edit_object=gtk_action_group_new ("edit_object");
	gtk_action_group_set_translation_domain (actions_edit_object, PACKAGE);
	gtk_action_group_add_actions (actions_edit_object, action_entries_edit_object, G_N_ELEMENTS(action_entries_edit_object), NULL);

	actions_edit_one_object=gtk_action_group_new ("edit_one_object");
	gtk_action_group_set_translation_domain (actions_edit_one_object, PACKAGE);
	gtk_action_group_add_actions (actions_edit_one_object, action_entries_edit_one_object, G_N_ELEMENTS(action_entries_edit_one_object), NULL);

	actions_edit_map=gtk_action_group_new ("edit_map");
	gtk_action_group_set_translation_domain (actions_edit_map, PACKAGE);
	gtk_action_group_add_actions (actions_edit_map, action_entries_edit_map, G_N_ELEMENTS(action_entries_edit_map), NULL);

	actions_edit_random=gtk_action_group_new ("edit_random");
	gtk_action_group_set_translation_domain (actions_edit_random, PACKAGE);
	gtk_action_group_add_actions (actions_edit_random, action_entries_edit_random, G_N_ELEMENTS(action_entries_edit_random), NULL);

	actions_clipboard=gtk_action_group_new ("clipboard");
	gtk_action_group_set_translation_domain (actions_clipboard, PACKAGE);
	gtk_action_group_add_actions (actions_clipboard, action_entries_clipboard, G_N_ELEMENTS(action_entries_clipboard), NULL);

	actions_clipboard_paste=gtk_action_group_new ("clipboard_paste");
	gtk_action_group_set_translation_domain (actions_clipboard_paste, PACKAGE);
	gtk_action_group_add_actions (actions_clipboard_paste, action_entries_clipboard_paste, G_N_ELEMENTS(action_entries_clipboard_paste), NULL);

	actions_edit_undo=gtk_action_group_new ("edit_undo");
	gtk_action_group_set_translation_domain (actions_edit_undo, PACKAGE);
	gtk_action_group_add_actions (actions_edit_undo, action_entries_edit_undo, G_N_ELEMENTS(action_entries_edit_undo), NULL);

	actions_edit_redo=gtk_action_group_new ("edit_redo");
	gtk_action_group_set_translation_domain (actions_edit_redo, PACKAGE);
	gtk_action_group_add_actions (actions_edit_redo, action_entries_edit_redo, G_N_ELEMENTS(action_entries_edit_redo), NULL);

	actions_edit_cave=gtk_action_group_new ("edit_cave");
	gtk_action_group_set_translation_domain (actions_edit_cave, PACKAGE);
	gtk_action_group_add_actions (actions_edit_cave, action_entries_edit_cave, G_N_ELEMENTS(action_entries_edit_cave), NULL);
	g_object_set (gtk_action_group_get_action (actions_edit_cave, "Test"), "is_important", TRUE, NULL);
	g_object_set (gtk_action_group_get_action (actions_edit_cave, "CaveSelector"), "is_important", TRUE, NULL);

	actions_edit_caveset=gtk_action_group_new ("edit_caveset");
	gtk_action_group_set_translation_domain (actions_edit_caveset, PACKAGE);
	gtk_action_group_add_actions (actions_edit_caveset, action_entries_edit_caveset, G_N_ELEMENTS(action_entries_edit_caveset), NULL);

	actions_cave_selector=gtk_action_group_new ("cave_selector");
	gtk_action_group_set_translation_domain (actions_cave_selector, PACKAGE);
	gtk_action_group_add_actions (actions_cave_selector, action_entries_cave_selector, G_N_ELEMENTS(action_entries_cave_selector), NULL);

	actions_toggle=gtk_action_group_new ("toggles");
	gtk_action_group_set_translation_domain (actions_toggle, PACKAGE);
	gtk_action_group_add_toggle_actions (actions_toggle, action_entries_toggle, G_N_ELEMENTS(action_entries_toggle), NULL);

	ui=gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui, actions, 0);
	gtk_ui_manager_insert_action_group (ui, actions_edit_tools, 0);
	gtk_ui_manager_insert_action_group (ui, actions_edit_map, 0);
	gtk_ui_manager_insert_action_group (ui, actions_edit_random, 0);
	gtk_ui_manager_insert_action_group (ui, actions_edit_object, 0);
	gtk_ui_manager_insert_action_group (ui, actions_edit_one_object, 0);
	gtk_ui_manager_insert_action_group (ui, actions_clipboard, 0);
	gtk_ui_manager_insert_action_group (ui, actions_clipboard_paste, 0);
	gtk_ui_manager_insert_action_group (ui, actions_edit_cave, 0);
	gtk_ui_manager_insert_action_group (ui, actions_edit_caveset, 0);
	gtk_ui_manager_insert_action_group (ui, actions_edit_undo, 0);
	gtk_ui_manager_insert_action_group (ui, actions_edit_redo, 0);
	gtk_ui_manager_insert_action_group (ui, actions_cave_selector, 0);
	gtk_ui_manager_insert_action_group (ui, actions_toggle, 0);
	gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, NULL);
	gtk_box_pack_start (GTK_BOX (vbox), gtk_ui_manager_get_widget (ui, "/MenuBar"), FALSE, FALSE, 0);

	toolbars=gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), toolbars, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (toolbars), gtk_ui_manager_get_widget (ui, "/ToolBar"), FALSE, FALSE, 0);
	gtk_toolbar_set_tooltips (GTK_TOOLBAR (gtk_ui_manager_get_widget (ui, "/ToolBar")), TRUE);
	gtk_toolbar_set_style(GTK_TOOLBAR(gtk_ui_manager_get_widget (ui, "/ToolBar")), GTK_TOOLBAR_BOTH_HORIZ);
	gtk_window_add_accel_group (GTK_WINDOW (editor_window), gtk_ui_manager_get_accel_group (ui));

	/* get popups and attach them to the window, so they are not destroyed (the window holds the ref) */	
	drawing_area_popup=gtk_ui_manager_get_widget (ui, "/DrawingAreaPopup");
	gtk_menu_attach_to_widget(GTK_MENU(drawing_area_popup), editor_window, NULL);
	object_list_popup=gtk_ui_manager_get_widget (ui, "/ObjectListPopup");
	gtk_menu_attach_to_widget(GTK_MENU(object_list_popup), editor_window, NULL);
	caveset_popup=gtk_ui_manager_get_widget (ui, "/CavesetPopup");
	gtk_menu_attach_to_widget(GTK_MENU(caveset_popup), editor_window, NULL);
	
	g_object_unref (actions);
	g_object_unref (ui);

	/* combo boxes under toolbar */
	hbox_combo=gtk_hbox_new(FALSE, 6);
	gtk_box_pack_start (GTK_BOX (toolbars), hbox_combo, FALSE, FALSE, 0);

	level_scale=gtk_hscale_new_with_range (1.0, 5.0, 1.0);
	gtk_scale_set_digits (GTK_SCALE (level_scale), 0);
	gtk_scale_set_value_pos (GTK_SCALE (level_scale), GTK_POS_LEFT);
	g_signal_connect (G_OBJECT (level_scale), "value-changed", G_CALLBACK(level_scale_changed_cb), NULL);
	gtk_box_pack_end_defaults (GTK_BOX (hbox_combo), level_scale);
	gtk_box_pack_end (GTK_BOX (hbox_combo), gtk_label_new (_("Level shown:")), FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (hbox_combo), label_first_element=gtk_label_new (NULL), FALSE, FALSE, 0);
	element_button=gd_element_button_new (O_DIRT);	/* combo box of object, default element dirt (not really important what it is) */
	gtk_widget_set_tooltip_text(element_button, _("Element used to draw points, lines, and rectangles. You can use middle-click to pick one from the cave."));
	gtk_box_pack_start_defaults (GTK_BOX (hbox_combo), element_button);

	gtk_box_pack_start (GTK_BOX (hbox_combo), label_second_element=gtk_label_new (NULL), FALSE, FALSE, 0);
	fillelement_button=gd_element_button_new (O_SPACE);	/* combo box, default element space (not really important what it is) */
	gtk_widget_set_tooltip_text (fillelement_button, _("Element used to fill rectangles, and second element of joins. You can use Ctrl + middle-click to pick one from the cave."));
	gtk_box_pack_start_defaults (GTK_BOX (hbox_combo), fillelement_button);
	
	/* hbox for drawing area and object list */
	hbox=gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), hbox);

	/* scroll window for drawing area and icon view ****************************************/
	scroll_window=gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), scroll_window);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	/* object list ***************************************/
	scroll_window_objects=gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scroll_window_objects, FALSE, FALSE, 0);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll_window_objects), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window_objects), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	object_list=gtk_list_store_new (NUM_EDITOR_COLUMNS, G_TYPE_INT, G_TYPE_STRING, GDK_TYPE_PIXBUF, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_POINTER);
	object_tree_view=gtk_tree_view_new_with_model (GTK_TREE_MODEL (object_list));
	g_object_unref (object_list);
	g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (object_tree_view))), "changed", G_CALLBACK(object_list_selection_changed_signal), NULL);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(object_tree_view)), GTK_SELECTION_MULTIPLE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(object_tree_view), TRUE);
	gtk_container_add (GTK_CONTAINER (scroll_window_objects), object_tree_view);
	/* two signals which are required to handle cave object drag-and-drop reordering */
	g_signal_connect(G_OBJECT(object_list), "row-changed", G_CALLBACK(row_changed), NULL);
	g_signal_connect(G_OBJECT(object_list), "row-deleted", G_CALLBACK(row_delete), NULL);
	/* object double-click: */
	g_signal_connect(G_OBJECT(object_tree_view), "row-activated", G_CALLBACK(object_tree_view_row_activated), NULL);
	g_signal_connect(G_OBJECT(object_tree_view), "popup-menu", G_CALLBACK(object_tree_view_popup_menu), NULL);
	g_signal_connect(G_OBJECT(object_tree_view), "button-press-event", G_CALLBACK(object_tree_view_button_press_event), NULL);

	/* tree view column which holds all data */
	/* we do not allow sorting, as it disables drag and drop */
	column=gtk_tree_view_column_new ();
	gtk_tree_view_column_set_spacing (column, 1);
	gtk_tree_view_column_set_title (column, _("_Objects"));
	renderer=gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "stock-id", TYPE_PIXBUF_COLUMN, NULL);
	renderer=gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "pixbuf", ELEMENT_PIXBUF_COLUMN, NULL);
	renderer=gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_attributes (column, renderer, "text", TEXT_COLUMN, NULL);
	renderer=gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_end (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "pixbuf", FILL_PIXBUF_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (object_tree_view), column);
	
	/* something like a statusbar, maybe that would be nicer */
	hbox=gtk_hbox_new (FALSE, 6);
	gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	label_coordinate=gtk_label_new ("[x:   y:   ]");
	gtk_box_pack_start (GTK_BOX (hbox), label_coordinate, FALSE, FALSE, 0);
	label_object=gtk_label_new (NULL);
	gtk_box_pack_start (GTK_BOX (hbox), label_object, FALSE, FALSE, 0);

	edit_level=0;		/* view: level 1 */
	select_tool(LINE);	/* here we force selection and update */
	select_tool(MOVE);	/* here we force selection and update */
	g_timeout_add(40, drawcave_int, drawcave_int);

	gtk_widget_show_all(editor_window);

	select_cave_for_edit(NULL);
}

