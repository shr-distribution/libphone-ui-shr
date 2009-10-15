#include "views.h"
#include "call-common.h"

#include <frameworkd-phonegui/frameworkd-phonegui-utility.h>

#include <frameworkd-glib/odeviced/frameworkd-glib-odeviced-audio.h>


/* speaker state */
static CallSoundState sound_state = CALL_SOUND_STATE_CLEAR;



/* FIXME: add locks to all the active calls functions!!!
 */
static GQueue *active_calls_list = NULL;

static void
_call_activate_callback(GError * error, struct CallActiveViewData *win)
{
	if (!error) {
		g_debug("%s:%d activated call (id=%d)", __FILE__, __LINE__,
			win->parent.id);
		call_common_window_to_active(win);
	}
	else {
		g_prefix_error(error, " activating call failed (id=%d)",
			       win->parent.id);
		g_error_free(error);
	}
}

void
call_common_activate_call(struct CallActiveViewData *win)
{
	g_debug("%s:%d attempting to set last call as active (id=%d)", __FILE__,
		__LINE__, win->parent.id);
#if 0
	phongeui_call_activate(win->parent.id, _call_activate_callback, win);
#else
	phongeui_call_activate(win->parent.id, NULL, NULL);
	call_common_window_to_active(win);
#endif
}

void
call_common_contact_callback(GError * error, char *name, void *_data)
{
	struct CallIncomingViewData *data =
		(struct CallIncomingViewData *) _data;
	/* data->number is used to make sure this still exists
	 * FIXME: locking needed! I'm not doing it here since
	 * it's needed everywhere, will happen in the rewrite*/
	g_debug("got contact: \"%s\" error? (%d)", name, error);
	if (error == NULL && *name && data->parent.number_state) {
		data->parent.number_state = CALL_NUMBER_CONTACT;
		data->parent.number = strdup(name);
		elm_label_label_set(data->number, data->parent.number);
	}

}



CallSoundState
call_common_get_sound_state()
{
	return sound_state;
}


int
call_common_set_sound_state(CallSoundState state)
{
	/* remove last one from stack 
	 * unless it's an init and then make init.
	 */
	/* init only if sound_state was CLEAR */
	if (state == CALL_SOUND_STATE_INIT) {
		state = CALL_SOUND_STATE_HANDSET;
		if (sound_state != CALL_SOUND_STATE_CLEAR) {
			return 1;
		}
	}

	if (sound_state != CALL_SOUND_STATE_CLEAR) {
		odeviced_audio_pull_scenario(NULL, NULL);
	}

	g_debug("%s:%d setting sound state (%d)", __FILE__, __LINE__, state);
	sound_state = state;
	switch (sound_state) {
	case CALL_SOUND_STATE_SPEAKER:
		odeviced_audio_push_scenario("gsmspeakerout", NULL, NULL);
		break;
	case CALL_SOUND_STATE_HEADSET:
		break;
	case CALL_SOUND_STATE_HANDSET:
		odeviced_audio_push_scenario("gsmhandset", NULL, NULL);
		break;
	case CALL_SOUND_STATE_BT:
		break;
	case CALL_SOUND_STATE_CLEAR:
		break;
	default:
		break;
	}

	if (active_calls_list) {
		g_queue_foreach(active_calls_list,
				call_common_window_update_state,
				(void *) sound_state);
	}
	return 0;

}

void
call_common_window_update_state(struct CallActiveViewData *win,
				CallSoundState state)
{
	const char *state_string = "";

	switch (state) {
	case CALL_SOUND_STATE_SPEAKER:
		state_string = D_("Handset");
		break;
	case CALL_SOUND_STATE_HEADSET:
		break;
		/* default to handset */
	case CALL_SOUND_STATE_CLEAR:
	case CALL_SOUND_STATE_HANDSET:
		state_string = D_("Speaker");
		break;
	case CALL_SOUND_STATE_BT:
		break;
	default:
		break;
	}

	elm_button_label_set(win->bt_sound_state, state_string);
}

static void
_foreach_new_active(struct CallActiveViewData *win, int id)
{
	if (id != win->parent.id) {
		call_common_window_to_pending(win);
	}
	else {
		call_common_window_to_active(win);
		g_queue_remove(active_calls_list, win);
		g_queue_push_head(active_calls_list, win);
	}
}

void
call_common_window_new_active(int id)
{
	g_debug("%s:%d setting new active call (id=%d)", __FILE__, __LINE__,
		id);
	struct CallActiveViewData *win;
	if (active_calls_list) {
		g_queue_foreach(active_calls_list, _foreach_new_active,
				(void *) id);
	}
}

void
call_common_window_to_pending(struct CallActiveViewData *win)
{
	if (win->state == CALL_STATE_ACTIVE) {
		elm_button_label_set(win->bt_call_state, D_("Pickup"));
	}
	else if (win->state == CALL_STATE_PENDING) {
		/*Do nothing as we want it to be pending */
		g_debug("Found a pending call while expecting none! (%d)",
			win->parent.id);
	}
	else {
		g_debug("Bad state (%d) for an active call!", win->state);
	}
	win->state = CALL_STATE_PENDING;
}

void
call_common_window_to_active(struct CallActiveViewData *win)
{
	if (win->state == CALL_STATE_ACTIVE) {
		/*Do nothing as we want it to be active */
		g_debug("Found an active call while expecting none! (%d)",
			win->parent.id);
	}
	else if (win->state == CALL_STATE_PENDING) {
		elm_button_label_set(win->bt_call_state, D_("Release"));
	}
	else {
		g_debug("Bad state (%d) for an active call!", win->state);
	}
	win->state = CALL_STATE_ACTIVE;
}

int
call_common_active_call_get_last_id()
{
	struct CallActiveViewData *win;
	if (active_calls_list) {
		win = g_queue_peek_head(active_calls_list);
		return win->parent.id;
	}
	else {
		return 0;
	}
}

int
call_common_active_call_add(struct CallActiveViewData *win)
{
	/* if it's not the first call, update all the windows */
	if (active_calls_list) {
		g_queue_foreach(active_calls_list, _foreach_new_active,
				(void *) -1);
	}
	/*init */
	/* if first, init state */
	else {
		call_common_set_sound_state(CALL_SOUND_STATE_INIT);
		g_debug("Initialized active calls list");
		active_calls_list = g_queue_new();
	}

	g_queue_push_head(active_calls_list, win);
	g_debug("%s:%d adding a call to active list (id=%d)", __FILE__,
		__LINE__, win->parent.id);

	return 0;
}

static int
_queue_find_by_id(struct CallActiveViewData *win, int id)
{
	return !(win->parent.id == id);
}

int
call_common_active_call_remove(int id)
{
	struct CallActiveViewData *win = NULL;
	if (active_calls_list) {
		GList *link = g_queue_find_custom(active_calls_list, id,
						  _queue_find_by_id);
		win = g_queue_peek_nth(active_calls_list,
				       g_queue_link_index(active_calls_list,
							  link));

		g_queue_delete_link(active_calls_list, link);
	}

	/* if we haven't found abort */
	if (!win) {
		g_debug("%s:%d no such id! was it active? (id=%d)", __FILE__,
			__LINE__, id);
		return 1;
	}


	g_debug("%s:%d removing a call from active list (id=%d)", __FILE__,
		__LINE__, win->parent.id);

	/* if was active, get a new active */
	if (win->state == CALL_STATE_ACTIVE) {
		win = g_queue_peek_head(active_calls_list);
		if (win) {
			call_common_activate_call(win);
		}
	}
	if (g_queue_get_length(active_calls_list) == 0) {
		g_debug("Freed active calls list");
		g_queue_free(active_calls_list);
		active_calls_list = NULL;
		call_common_set_sound_state(CALL_SOUND_STATE_CLEAR);
	}
	return 0;
}

/*FIXME: why does button_keypad_clicked get the data parametr? */
void
call_button_keypad_clicked(struct CallViewData *data, Evas_Object * obj,
			   void *event_info)
{
	char string[2];
	string[0] = (char) event_info;
	string[1] = '\0';
	g_debug("call_button_keypad_clicked(): %s", string);
	phonegui_call_send_dtmf(string, NULL, NULL);
}

void
call_dtmf_enable(struct CallViewData *data)
{
	g_debug("call_dtmf_enable()");
	data->keypad = elm_keypad_add(window_evas_object_get(data->win));
	evas_object_smart_callback_add(data->keypad, "clicked",
				       call_button_keypad_clicked, data);
	window_swallow(data->win, "keypad", data->keypad);
	evas_object_show(data->keypad);
}

void
call_dtmf_disable(struct CallViewData *data)
{
	g_debug("call_dtmf_disable()");
	evas_object_smart_callback_del(data->keypad, "clicked",
				       call_button_keypad_clicked);
	evas_object_del(data->keypad);
}
