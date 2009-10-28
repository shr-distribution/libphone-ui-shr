#include "views.h"
#include "call-common.h"

#include <phoneui/phoneui-utility.h>

static void call_button_sound_state_clicked(struct CallActiveViewData *data,
					    Evas_Object * obj,
					    void *event_info);
static void call_button_dtmf_clicked(struct CallActiveViewData *data,
				     Evas_Object * obj, void *event_info);
static void call_button_state_clicked(struct CallActiveViewData *data,
				      Evas_Object * obj, void *event_info);


struct CallActiveViewData *
call_active_view_show(struct Window *win, GHashTable * options)
{
	g_debug("call_active_show()");

	struct CallActiveViewData *data =
		g_slice_alloc0(sizeof(struct CallActiveViewData));
	data->parent.options = options;
	data->parent.win = win;
	data->parent.id = GPOINTER_TO_INT(g_hash_table_lookup(options, "id"));
	data->parent.number = g_hash_table_lookup(options, "number");
	data->parent.name = g_hash_table_lookup(options, "name");
	data->parent.photo = g_hash_table_lookup(options, "photo");
	data->parent.dtmf_active = FALSE;
	data->state = CALL_STATE_ACTIVE;
	data->parent.number_state =
		GPOINTER_TO_INT(g_hash_table_lookup(options, "number_state"));

	g_debug("active call: id=%d, number_state=%d, number='%s'", data->parent.id,
			data->parent.number_state, data->parent.number);

	window_layout_set(win, DEFAULT_THEME, "phoneui/call_management/active_call");

	data->parent.elmphoto = elm_icon_add(window_evas_object_get(win));
	window_swallow(win, "photo", data->parent.elmphoto);
	evas_object_show(data->parent.elmphoto);

	window_text_set(win, "number", data->parent.number);
	if (data->parent.number_state == CALL_NUMBER_NUMBER) {
		phoneui_contact_lookup(data->parent.number,
					call_common_contact_callback, &data->parent);
	}
	else {
		window_text_set(win, "name", data->parent.name);
		elm_icon_file_set(data->parent.elmphoto, data->parent.photo, NULL);
	}

	g_debug("adding the speaker toggle...");
	data->speaker_toggle = elm_toggle_add(window_evas_object_get(win));
	elm_toggle_label_set(data->speaker_toggle, D_("Speaker"));
	elm_toggle_state_set(data->speaker_toggle, EINA_FALSE);
	window_swallow(win, "speaker_toggle", data->speaker_toggle);
	evas_object_show(data->speaker_toggle);

	g_debug("adding the mute toggle...");
	data->mute_toggle = elm_toggle_add(window_evas_object_get(win));
	elm_toggle_label_set(data->mute_toggle, D_("Silent"));
	elm_toggle_state_set(data->mute_toggle, EINA_FALSE);
	window_swallow(win, "mute_toggle", data->mute_toggle);
	evas_object_show(data->mute_toggle);

	g_debug("adding the volume slider...");
	data->volume_slider = elm_slider_add(window_evas_object_get(win));
	elm_slider_label_set(data->volume_slider, D_("Volume"));
	elm_slider_min_max_set(data->volume_slider, 0.0, 100.0);
	window_swallow(win, "volume_slider", data->volume_slider);
	evas_object_show(data->volume_slider);

	data->bt_call_state = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_call_state, D_("Release"));
	evas_object_smart_callback_add(data->bt_call_state, "clicked",
				       call_button_state_clicked, data);
	window_swallow(win, "button_release", data->bt_call_state);
	evas_object_show(data->bt_call_state);

	data->bt_sound_state = elm_button_add(window_evas_object_get(win));
	evas_object_smart_callback_add(data->bt_sound_state, "clicked",
				       call_button_sound_state_clicked, data);
	window_swallow(win, "button_speaker", data->bt_sound_state);
	call_common_window_update_state(data, call_common_get_sound_state());
	evas_object_show(data->bt_sound_state);

	data->bt_keypad = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_keypad, D_("Keypad"));
	evas_object_smart_callback_add(data->bt_keypad, "clicked",
				       call_button_dtmf_clicked, data);
	window_swallow(win, "button_dtmf", data->bt_keypad);
	evas_object_show(data->bt_keypad);

	window_show(win);

	call_common_active_call_add(data);
	return data;
}

void
call_active_view_hide(struct CallActiveViewData *data)
{
	g_debug("call_active_hide()");

	struct Window *win = data->parent.win;

	if (data->parent.dtmf_active) {
		call_dtmf_disable(&(data->parent));
	}

	data->parent.number_state = CALL_NUMBER_NULL;
	evas_object_del(data->mute_toggle);
	evas_object_del(data->speaker_toggle);
	evas_object_del(data->volume_slider);
	evas_object_del(data->bt_call_state);
	evas_object_del(data->bt_sound_state);
	evas_object_del(data->bt_keypad);
}

/* FIXME: Should fix to handle bt/headset as well */
static void
call_button_sound_state_clicked(struct CallActiveViewData *data,
				Evas_Object * obj, void *event_info)
{
	CallSoundState state = call_common_get_sound_state();
	g_debug("sound_state_clicked(id=%d,state=%d)", data->parent.id, state);

	if (state == CALL_SOUND_STATE_SPEAKER) {
		call_common_set_sound_state(CALL_SOUND_STATE_HANDSET);
	}
	else if (state == CALL_SOUND_STATE_HANDSET) {
		call_common_set_sound_state(CALL_SOUND_STATE_SPEAKER);
	}
	/* else if moree.... */
	else {
		g_debug("trying to set unknown sound state %d failed!", state);
	}
}

static void
call_button_dtmf_clicked(struct CallActiveViewData *data, Evas_Object * obj,
			 void *event_info)
{
	g_debug("dtmf_clicked()");
	if (data->parent.dtmf_active) {
		data->parent.dtmf_active = FALSE;
		call_dtmf_disable(data);
		elm_button_label_set(data->bt_keypad, D_("Keypad"));
	}
	else {
		data->parent.dtmf_active = TRUE;
		call_dtmf_enable(data);
		elm_button_label_set(data->bt_keypad, D_("Hide Keypad"));
	}
}

void
call_button_state_clicked(struct CallActiveViewData *data, Evas_Object * obj,
			  void *event_info)
{
	g_debug("state_clicked(id=%d,state=%d)", data->parent.id, data->state);
	if (data->state == CALL_STATE_ACTIVE) {
		phoneui_call_release(data->parent.id, NULL, NULL);
	}
	else if (data->state == CALL_STATE_PENDING) {
		phoneui_call_activate(data->parent.id, NULL, NULL);
		call_common_window_new_active(data->parent.id);
	}
	else {
		g_debug("bad state, BUG! shouldn't have gotten here");
	}
}
