/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 */



#include <ctype.h> /* to upper */
#include <phoneui-utils-contacts.h>

#include "phoneui-shr.h"
#include "views.h"

#include "common-utils.h"

static Elm_Genlist_Item_Class itc;

/* --- genlist callbacks --- */
static char *
gl_label_get(void *data, Evas_Object * obj, const char *part)
{
	(void) obj;
	GHashTable *parameters = (GHashTable *) data;
	char *s = NULL;

	if (!strcmp(part, "elm.text")) {
		s = phoneui_utils_contact_display_name_get(parameters);
		if (s && *s) {
			return s;
		}
		else {
			s = strdup(CONTACT_NAME_UNDEFINED_STRING);
		}
	}
	else if (!strcmp(part, "elm.text.sub")) {
		s = phoneui_utils_contact_display_phone_get(parameters);
		if (s && *s) {
			return s;
		}
		else {
			s = strdup(CONTACT_PHONE_UNDEFINED_STRING);
		}
	}

	return s;
}



static Evas_Object *
gl_icon_get(void *data, Evas_Object * obj, const char *part)
{
	GHashTable *parameters = (GHashTable *) data;
	if (!strcmp(part, "elm.swallow.icon")) {
		const char *photo_file;
		GValue *tmp = g_hash_table_lookup(parameters, "Photo");
		if (tmp)
			photo_file = g_value_get_string(tmp);
		else
			photo_file = CONTACT_DEFAULT_PHOTO;
		Evas_Object *photo = elm_image_add(obj);
		elm_image_file_set(photo, photo_file, NULL);
		evas_object_size_hint_aspect_set(photo,
						 EVAS_ASPECT_CONTROL_VERTICAL,
						 1, 1);
		return (photo);
	}
	return (NULL);
}



static Eina_Bool
gl_state_get(void *data, Evas_Object * obj, const char *part)
{
	(void) obj;
	(void) data;
	(void) part;
	return (0);
}


static void
gl_del(void *data, Evas_Object * obj)
{
	(void) obj;
	(void) data;
}


static void
gl_index_changed(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
}

	static void
gl_index_changed2(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	elm_genlist_item_top_bring_in(event_info);
}


static void
gl_index_selected(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	elm_genlist_item_top_bring_in(event_info);
}



int /* thanks to evas */
utf8_get_next(const unsigned char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iindex
    * to the next code point after this.
    *
    * Returns 0 to indicate there is no next char
    */
	int index = *iindex, len, r;
	unsigned char d, d2, d3, d4;

	/* if this char is the null terminator, exit */
	if (!buf[index])
		return 0;

	d = buf[index++];

	while (buf[index] && ((buf[index] & 0xc0) == 0x80))
		index++;

	len = index - *iindex;

	if (len == 1)
		r = d;
	else if (len == 2)
	{
		/* 2 bytes */
		d2 = buf[*iindex + 1];
		r = d & 0x1f; /* copy lower 5 */
		r <<= 6;
		r |= (d2 & 0x3f); /* copy lower 6 */
	}
	else if (len == 3)
	{
		/* 3 bytes */
		d2 = buf[*iindex + 1];
		d3 = buf[*iindex + 2];
		r = d & 0x0f; /* copy lower 4 */
		r <<= 6;
		r |= (d2 & 0x3f);
		r <<= 6;
		r |= (d3 & 0x3f);
	}
	else
	{
		/* 4 bytes */
		d2 = buf[*iindex + 1];
		d3 = buf[*iindex + 2];
		d4 = buf[*iindex + 3];
		r = d & 0x0f; /* copy lower 4 */
		r <<= 6;
		r |= (d2 & 0x3f);
		r <<= 6;
		r |= (d3 & 0x3f);
		r <<= 6;
		r |= (d4 & 0x3f);
	}

	*iindex = index;
	return r;
}


/* allocates space and returns the index part of a string */
static char *
_new_get_index(const char *_string)
{
	/*FIXME: handle the upper in a more sane manner,
	 * i.e, unicode support */
	char *string;
	int i;

	if (!_string) {
		return NULL;
	}
	i = 0;
	utf8_get_next((const unsigned char *) _string, &i);

	string = malloc(i + 1);
	if (!string) {
		return NULL;
	}
	strncpy(string, _string, i);
	string[i] = '\0';

	if (i == 1) {/* i.e, an ascii char */
		string[0] = toupper(string[0]);
	}

	return string;
}

Elm_Genlist_Item *
contact_list_item_add(struct ContactListData *list_data,
		      GHashTable *entry, int sortin)
{
	GHashTable *other;
	Elm_Genlist_Item *it;

	if (sortin) {
		/* find the correct position to insert the new one */
		it = elm_genlist_first_item_get(list_data->list);
		while (it) {
			other = (GHashTable *)elm_genlist_item_data_get(it);
			if (phoneui_utils_contact_compare(entry, other) < 0)
				break;
			it = elm_genlist_item_next_get(it);
		}
		if (it) {
			return elm_genlist_item_insert_before(list_data->list,
					&itc, g_hash_table_ref(entry), it,
					ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	}
	return elm_genlist_item_append(list_data->list, &itc,
				     g_hash_table_ref(entry) /*item data */ ,
				     NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}

void
contact_list_fill_index(struct ContactListData *list_data)
{
	static const int index_button_height = 25; /*FIXME: get actual size*/
	int limit = 1;
	Evas_Object *win;
	Elm_Genlist_Item *it, *current_index_item = NULL;
	GHashTable *entry;
	char *idx, *current_index = NULL;
	char *name;
	int init_index_count, index_count;
	int new_index;
	int height;

	win = ui_utils_view_window_get(list_data->view);
	if (list_data->index) {
		evas_object_del(list_data->index);
	}
	list_data->index = elm_index_add(win);
	elm_win_resize_object_add(win, list_data->index);
	evas_object_size_hint_weight_set(list_data->index, 1.0, 0.0);
	evas_object_show(list_data->index);
	evas_object_smart_callback_add(list_data->index, "delay,changed",
				       gl_index_changed2, NULL);
	evas_object_smart_callback_add(list_data->index, "changed",
				       gl_index_changed, NULL);
	evas_object_smart_callback_add(list_data->index, "selected",
				       gl_index_selected, NULL);

	evas_object_geometry_get(list_data->index, NULL, NULL, NULL, &height);
	limit = height / index_button_height;
	init_index_count = list_data->count / (limit - 1); /* The number of indexes excluding the first */
	index_count = 0; /* Add the first as well */
	it = elm_genlist_first_item_get(list_data->list);
	while (it) {
		entry = (GHashTable *)elm_genlist_item_data_get(it);
		name = phoneui_utils_contact_display_name_get(entry);
		idx = _new_get_index(name);
		if (idx) {
			if (!current_index || strcmp(idx, current_index)) {
				if (current_index) {
					free(current_index);
				}
				current_index = idx;
				current_index_item = it;
				new_index = 1;
			}
			else {
				new_index = 0;
				free(idx);
			}
			if (index_count < 1 && new_index) {
				g_debug("Adding index %s", current_index);
				elm_index_item_append(list_data->index,
						      current_index,
						      current_index_item);
				index_count = init_index_count;
			}
			index_count--;
		}
		it = elm_genlist_item_next_get(it);
	}
	if (list_data->layout) {
		elm_layout_content_set(list_data->layout, "contacts_index",
				list_data->index);
	}
}

static void
_process_entry(void *_entry, void *_data)
{
	Elm_Genlist_Item *it;
	GHashTable *entry = (GHashTable *)_entry;
	struct ContactListData *list_data = (struct ContactListData *) _data;
	it = contact_list_item_add(list_data, entry, 0);
	if (!it) {
		g_warning("Failed adding a contact to the list");
		return;
	}
	list_data->current++;
	if (list_data->count == list_data->current) {
		contact_list_fill_index(list_data);
	}
}

void
contact_list_fill(struct ContactListData *list_data)
{
	g_debug("contact_list_fill()");
	list_data->current = 0;
	phoneui_utils_contacts_get(&list_data->count, _process_entry, list_data);
}

void
contact_list_add(struct ContactListData *list_data)
{
	Evas_Object *win;
	win = ui_utils_view_window_get(list_data->view);
	list_data->index = NULL;
	list_data->list = elm_genlist_add(win);
	elm_genlist_horizontal_mode_set(list_data->list, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(list_data->list, 0.0, 0.0);
	elm_object_scale_set(list_data->list, 1.0);
	itc.item_style = "contact";
	itc.func.label_get = gl_label_get;
	itc.func.icon_get = gl_icon_get;
	itc.func.state_get = gl_state_get;
	itc.func.del = gl_del;
	evas_object_show(list_data->list);
	if (list_data->layout) {
		elm_layout_content_set(list_data->layout, "contacts_list",
				list_data->list);
	}
}

