#include "views.h"

#include <phoneui/phoneui-utility.h>

struct ContactDeleteViewData {
	struct Window *win;
	char *path;
	void (*callback) ();
	void *callback_data;
	Evas_Object *bt_yes, *bt_no, *info_label;
};


static void
  frame_delete_show(void *_data);
static void
  frame_delete_hide(void *_data);
static void
  frame_delete_no_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
  frame_delete_yes_clicked(void *_data, Evas_Object * obj, void *event_info);

static void
  frame_deleting_show(void *_data);

static void
  delete_callback(GError * error, gpointer _data);
static void
  delete_callback2(struct ContactDeleteViewData *data);



/* --- main contact delete view --------------------------------------------- */

void *
contact_delete_view_show(struct Window *win, void *_options)
{
	GHashTable *options = (GHashTable *) _options;

	g_debug("contact_delete_view_show()");

	struct ContactDeleteViewData *data =
		g_slice_alloc0(sizeof(struct ContactDeleteViewData));
	data->win = win;

	if (options == NULL) {
		g_error("At least option[path] must be set.");
	}
	else {
		data->path = g_hash_table_lookup(options, "path");
		data->callback =
			g_hash_table_lookup(options, "delete_callback");
		data->callback_data =
			g_hash_table_lookup(options, "delete_callback_data");
	}

	window_frame_show(win, data, frame_delete_show, frame_delete_hide);
	window_show(win);

	return data;
}

void
contact_delete_view_hide(void *_data)
{
	g_slice_free(struct ContactDeleteViewData, _data);
}



/* --- frame "delete" ------------------------------------------------------- */

static void
frame_delete_show(void *userdata)
{
	struct ContactDeleteViewData *data =
		(struct ContactDeleteViewData *) userdata;
	struct Window *win = data->win;

	g_debug("frame_delete_show()");

	window_layout_set(win, DEFAULT_THEME, "phoneui/contacts/delete");

	data->info_label = elm_label_add(window_evas_object_get(win));
	elm_label_label_set(data->info_label,
			    D_("Do you really want to delete it?"));
	window_swallow(win, "info", data->info_label);
	evas_object_show(data->info_label);

	data->bt_yes = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_yes, D_("Yes"));
	evas_object_smart_callback_add(data->bt_yes, "clicked",
				       frame_delete_yes_clicked, data);
	window_swallow(win, "button_yes", data->bt_yes);
	evas_object_show(data->bt_yes);

	data->bt_no = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_no, D_("No"));
	evas_object_smart_callback_add(data->bt_no, "clicked",
				       frame_delete_no_clicked, data);
	window_swallow(win, "button_no", data->bt_no);
	evas_object_show(data->bt_no);
}

static void
frame_delete_hide(void *_data)
{
	struct ContactDeleteViewData *data =
		(struct ContactDeleteViewData *) _data;

	g_debug("frame_delete_hide()");

	evas_object_del(data->bt_yes);
	evas_object_del(data->bt_no);
	evas_object_del(data->info_label);
}

static void
frame_delete_no_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactDeleteViewData *data =
		(struct ContactDeleteViewData *) _data;

	g_debug("frame_delete_no_clicked()");

	window_destroy(data->win, NULL);
}

static void
frame_delete_yes_clicked(void *userdata, Evas_Object * obj, void *event_info)
{
	struct ContactDeleteViewData *data =
		(struct ContactDeleteViewData *) userdata;

	g_debug("frame_delete_yes_clicked() --> deleting contact '%s'",
		data->path);

	window_frame_show(data->win, data, frame_deleting_show, NULL);

	phoneui_contact_delete(data->path, delete_callback, data);
}



/* --- frame "deleting" ----------------------------------------------------- */

static void
frame_deleting_show(void *_data)
{
	struct ContactDeleteViewData *data =
		(struct ContactDeleteViewData *) _data;

	g_debug("frame_deleting_show()");

	window_layout_set(data->win, DEFAULT_THEME, "phoneui/contacts/deleting");
}



/* --- dbus/libframeworkd callbacks ----------------------------------------- */

static void
delete_callback(GError * error, gpointer _data)
{
	struct ContactDeleteViewData *data =
		(struct ContactDeleteViewData *) _data;
	if (data->callback != NULL)
		data->callback(data->callback_data);

	window_destroy(data->win, NULL);
}

