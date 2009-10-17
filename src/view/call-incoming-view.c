#include "views.h"
#include "call-common.h"

#include <phoneui/phoneui-utility.h>


static void call_button_accept_clicked(struct CallIncomingViewData *data,
				       Evas_Object * obj, void *event_info);
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
	data->parent.dtmf_active = FALSE;
	data->parent.number_state = CALL_NUMBER_NUMBER;

	window_layout_set(win, CALL_FILE, "incoming_call");

	data->number = elm_label_add(window_evas_object_get(win));
	elm_label_label_set(data->number, data->parent.number);
	window_swallow(win, "number", data->number);
	evas_object_show(data->number);

	phoneui_contact_lookup(data->parent.number,
				call_common_contact_callback, data);

	data->information = elm_label_add(window_evas_object_get(win));
	elm_label_label_set(data->information, D_("Incoming call"));
	window_swallow(win, "information", data->information);
	evas_object_show(data->information);

	data->bt_accept = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_accept, D_("Accept"));
	evas_object_smart_callback_add(data->bt_accept, "clicked",
				       call_button_accept_clicked, data);
	window_swallow(win, "button_accept", data->bt_accept);
	evas_object_show(data->bt_accept);

	data->bt_reject = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_reject, D_("Reject"));
	evas_object_smart_callback_add(data->bt_reject, "clicked",
				       call_button_release_clicked, data);
	window_swallow(win, "button_release", data->bt_reject);
	evas_object_show(data->bt_reject);

	window_show(win);

	return data;
}

void
call_incoming_view_hide(struct CallIncomingViewData *data)
{
	g_debug("call_incoming_view_hide()");

	struct Window *win = data->parent.win;

	evas_object_del(data->information);
	evas_object_del(data->number);
	data->parent.number_state = CALL_NUMBER_NULL;
	evas_object_del(data->bt_accept);
	evas_object_del(data->bt_reject);

	if (data->parent.dtmf_active) {
		call_dtmf_disable(&data->parent);
	}
}


static void
call_button_accept_clicked(struct CallIncomingViewData *data, Evas_Object * obj,
			   void *event_info)
{
	g_debug("accept_clicked(call_id=%d)", data->parent.id);
	phoneui_call_activate(data->parent.id, NULL, NULL);

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "id", GINT_TO_POINTER(data->parent.id));
	g_hash_table_insert(options, "number", strdup(data->parent.number));
	window_view_show(data->parent.win, options, call_active_view_show,
			 call_active_view_hide, NULL);
}

static void
call_button_release_clicked(struct CallIncomingViewData *data, Evas_Object * obj,
			    void *event_info)
{
	g_debug("release_clicked(call_id=%d)", data->parent.id);
	phoneui_call_release(data->parent.id, NULL, NULL);
}
