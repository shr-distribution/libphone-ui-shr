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


#include "views.h"
#include "phoneui-shr.h"


struct UssdViewData {
	struct Window *win;
	int mode;
	char *message;
	Evas_Object *bt_close;
};


static void
  frame_ussd_show(void *data);
static void
  frame_ussd_hide(void *data);
static void
  frame_ussd_close_clicked(void *data, Evas_Object * obj, void *event_info);
//static void frame_ussd_keypad_clicked(struct UssdViewData *data, Evas_Object *obj, void *event_info);


/* --- ussd view ------------------------------------------------------------ */

void *
ussd_view_show(struct Window *win, void *_options)
{
	GHashTable *options = (GHashTable *) _options;

	g_debug("ussd_view_show()");

	if (!options) {
		g_critical("'options' is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return NULL;
	}

	struct UssdViewData *data = calloc(1, sizeof(struct UssdViewData));
	data->win = win;
	data->mode = GPOINTER_TO_INT(g_hash_table_lookup(options, "mode"));
	data->message =
		elm_entry_utf8_to_markup(g_hash_table_lookup
					 (options, "message"));
	window_frame_show(win, data, frame_ussd_show, frame_ussd_hide);
	window_show(win);

	return data;
}

void
ussd_view_hide(void *_data)
{
	struct UssdViewData *data = (struct UssdViewData *) _data;
	free(data->message);
	free(data);
}


/* --- frame "ussd" --------------------------------------------------------- */

static void
frame_ussd_show(void *_data)
{
	struct UssdViewData *data = (struct UssdViewData *) _data;
	struct Window *win = data->win;

	window_layout_set(win, phoneui_theme, "phoneui/notification/ussd");
	window_text_set(win, "mode", D_("Data:"));
	window_text_set(win, "message", data->message);

	data->bt_close = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_close, D_("Close"));
	evas_object_smart_callback_add(data->bt_close, "clicked",
				       frame_ussd_close_clicked, data);
	window_swallow(win, "button_close", data->bt_close);
	evas_object_show(data->bt_close);

	/*
	 * data->keypad = elm_keypad_add(window_evas_object_get(win));
	 * evas_object_smart_callback_add(data->keypad, "clicked", frame_ussd_keypad_clicked, data);
	 * window_swallow(win, "keypad", data->keypad);
	 * evas_object_show(data->keypad);
	 */
}

static void
frame_ussd_hide(void *_data)
{
	struct UssdViewData *data = (struct UssdViewData *) _data;

	evas_object_del(data->bt_close);

	/*
	 * window_unswallow(win, data->keypad);
	 * evas_object_smart_callback_del(data->keypad, "clicked", frame_ussd_keypad_clicked);
	 * evas_object_del(data->keypad);
	 */
}

static void
frame_ussd_close_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct UssdViewData *data = (struct UssdViewData *) _data;
	window_destroy(data->win, NULL);
}

/*
   static void frame_ussd_keypad_clicked(struct UssdViewData *data, Evas_Object *obj, void *event_info) {
   char input = (char) event_info;

   if(data->number_length < 64) {
   strncat(data->number, &input, 1);
   data->number_length++;
   frame_ussd_number_update(data);
   }

   if(data->number_length == 1) {
   edje_object_signal_emit(window_layout_get(data->win), "number_available", "elm");
   }
   }
   */
