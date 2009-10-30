#include "views.h"
#include "call-common.h"

#include <phoneui/phoneui-utils.h>


static void call_button_accept_clicked(void *_data, Evas_Object * obj,
					void *event_info);
static void call_button_release_clicked(struct CallIncomingViewData *data,
					Evas_Object * obj, void *event_info);


struct CallIncomingViewData *
call_incoming_view_show(struct Window *win, GHashTable * options)
{
	g_debug("call_incoming_view_show()");

	struct CallIncomingViewData *data =
		g_slice_alloc0(sizeof(struct CallIncomingViewData));
	data->parent.options = options;
	data->parent.win = win;
	data->parent.id = GPOINTER_TO_INT(g_hash_table_lookup(options, "id"));
	data->parent.number = g_hash_table_lookup(options, "number");
	data->parent.name = NULL;
	data->parent.photo = NULL;
	data->parent.dtmf_active = FALSE;
	data->parent.number_state =
		GPOINTER_TO_INT(g_hash_table_lookup(options, "number_state"));

	window_layout_set(win, DEFAULT_THEME, "phoneui/call_management/incoming_call");

	data->parent.elmphoto = elm_icon_add(window_evas_object_get(win));
	window_swallow(win, "photo", data->parent.elmphoto);
	evas_object_show(data->parent.elmphoto);

	window_text_set(win, "number", data->parent.number);

	phoneui_utils_contact_lookup(data->parent.number,
				call_common_contact_callback, &data->parent);

	Evas_Object *ic = elm_icon_add(win);
	elm_icon_file_set(ic, ICON_CALL_ACCEPT, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

	data->bt_accept = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_accept, D_("Accept"));
	elm_button_icon_set(data->bt_accept, ic);
	evas_object_smart_callback_add(data->bt_accept, "clicked",
				       call_button_accept_clicked, data);
	window_swallow(win, "button_accept", data->bt_accept);
	evas_object_show(data->bt_accept);
	evas_object_show(ic);

	ic = elm_icon_add(win);
	elm_icon_file_set(ic, ICON_CALL_REJECT, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	data->bt_reject = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_reject, D_("Reject"));
	elm_button_icon_set(data->bt_reject, ic);
	evas_object_smart_callback_add(data->bt_reject, "clicked",
				       call_button_release_clicked, data);
	window_swallow(win, "button_release", data->bt_reject);
	evas_object_show(data->bt_reject);
	evas_object_show(ic);

	window_show(win);

	return data;
}

void
call_incoming_view_hide(struct CallIncomingViewData *data)
{
	g_debug("call_incoming_view_hide()");

	struct Window *win = data->parent.win;

	evas_object_del(data->number);
	evas_object_del(data->name);
	evas_object_del(data->photo);
	evas_object_del(data->bt_accept);
	evas_object_del(data->bt_reject);
	data->parent.number_state = CALL_NUMBER_NULL;

	if (data->parent.dtmf_active) {
		call_dtmf_disable(&data->parent);
	}
}


static void
call_button_accept_clicked(void *_data, Evas_Object * obj,
			   void *event_info)
{
	struct CallIncomingViewData *data =
		(struct CallIncomingViewData *)_data;
	g_debug("accept_clicked(call_id=%d)", data->parent.id);
	phoneui_utils_call_activate(data->parent.id, NULL, NULL);

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "id", GINT_TO_POINTER(data->parent.id));
	g_hash_table_insert(options, "number", strdup(data->parent.number));
	g_hash_table_insert(options, "number_state", GINT_TO_POINTER(data->parent.number_state));
	if (data->parent.name)
		g_hash_table_insert(options, "name", strdup(data->parent.name));
	if (data->parent.photo)
		g_hash_table_insert(options, "photo", strdup(data->parent.photo));
	window_view_show(data->parent.win, options, call_active_view_show,
			 call_active_view_hide, NULL);
}

static void
call_button_release_clicked(struct CallIncomingViewData *data, Evas_Object * obj,
			    void *event_info)
{
	g_debug("release_clicked(call_id=%d)", data->parent.id);
	phoneui_utils_call_release(data->parent.id, NULL, NULL);
}
