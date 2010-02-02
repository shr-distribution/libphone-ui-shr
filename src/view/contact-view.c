
#include <glib.h>
#include <Evas.h>
#include <Elementary.h>
#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils.h>
#include "views.h"
#include "common-utils.h"
#include "ui-utils.h"
#include "contact-view.h"


static Elm_Genlist_Item_Class itc;
/* keep a list of open contact views - to open only one view per contact */
static GHashTable *contactviews = NULL;

static void _contact_delete_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _contact_add_field_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _contact_photo_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _contact_call_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _contact_sms_clicked(void *_data, Evas_Object * obj, void *event_info);

static void _contact_photo_back_clicked(void *_data, Evas_Object *obj, void *event_info);
static void _contact_photo_remove_clicked(void *_data, Evas_Object *obj, void *event_info);

static void _contact_save_clicked(void *_data, Evas_Object *obj, void *event_info);
static void _contact_cancel_clicked(void *_data, Evas_Object *obj, void *event_info);

static void _field_clicked(void *_data, Evas_Object *obj, void *event_info);
static void _value_changed(void *_data, Evas_Object *obj, void *event_info);
static void _load_name(struct ContactViewData *view);
static void _load_number(struct ContactViewData *view);
static void _load_photo(struct ContactViewData *view);
static void _load_fields(struct ContactViewData *view);
static struct ContactFieldData *_add_field(struct ContactViewData *view, const char *key, const char *value);

static Evas_Object *gl_field_icon_get(const void *_data, Evas_Object * obj, const char *part);
static void gl_field_del(const void *_data, Evas_Object * obj);

static void _delete_cb(struct View *view, Evas_Object * win, void *event_info);
static void _destroy_cb(struct View *_view);

static void _set_modify(struct ContactViewData *view, int dirty);
static void _update_cb(GError *error, gpointer data);
static void _add_cb(GError *error, char *path, gpointer data);
static int _changes_to_properties(struct ContactViewData *view, int dry_run);
static void _update_one_field(struct ContactViewData *view, struct ContactFieldData *fd);
static void _load_cb(GHashTable *content, gpointer data);

int
contact_view_init(char *path, GHashTable *properties)
{
	struct ContactViewData *view;
	Evas_Object *win, *box1, *box2, *btn;
	int ret;
	char *s;

	/* path MUST always be set! For new contacts to ""
	and it will be freed by destroying the contactviews
	hashtable in here, thus must be a copy */
	if (!path) {
		g_warning("Trying to initialize a contact view without path!");
		return 1;
	}

	g_debug("Initializing the contact view for '%s'", path);

	view = malloc(sizeof(struct ContactViewData));
	if (!view) {
		g_critical("Failed to allocate contact view for '%s'", path);
		free(path);
		return 1;
	}

	ret = ui_utils_view_init(VIEW_PTR(*view), ELM_WIN_BASIC, D_("Contact"),
				 NULL, NULL, _destroy_cb);
	if (ret) {
		g_critical("Failed to init contact view for '%s'", path);
		if (properties) {
			g_hash_table_destroy(properties);
		}
		free(view);
		free(path);
		return ret;
	}

	/* cache the views to open only one view per contact */
	if (contactviews == NULL) {
		contactviews = g_hash_table_new_full(g_str_hash, g_str_equal,
						     free, free);
	}
	g_hash_table_insert(contactviews, path, view);

	view->path = path;
	view->properties = properties;
	view->have_unsaved_changes = 0;

	elm_theme_extension_add(DEFAULT_THEME);

	win = ui_utils_view_window_get(VIEW_PTR(*view));
	evas_object_smart_callback_add(win, "delete,request",
				      (Evas_Smart_Cb) _delete_cb, view);
	ui_utils_view_layout_set(VIEW_PTR(*view), DEFAULT_THEME,
				 "phoneui/contacts/view");

	view->photo = elm_icon_add(win);
	evas_object_smart_callback_add(view->photo, "clicked",
				_contact_photo_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "photo", view->photo);
	evas_object_show(view->photo);

	view->fields = elm_genlist_add(win);
	elm_scroller_policy_set(view->fields, ELM_SCROLLER_POLICY_OFF,
				ELM_SCROLLER_POLICY_AUTO);
	elm_genlist_horizontal_mode_set(view->fields, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(view->fields, 0.0, 0.0);
	elm_object_scale_set(view->fields, 1.0);
        ui_utils_view_swallow(VIEW_PTR(*view), "fields", view->fields);
	itc.item_style = "contactfield";
	itc.func.label_get = NULL;
	itc.func.icon_get = gl_field_icon_get;
	itc.func.state_get = NULL;
	itc.func.del = gl_field_del;

	evas_object_show(view->fields);

	view->btn_save = elm_button_add(win);
	elm_button_label_set(view->btn_save, D_("Save"));
	evas_object_smart_callback_add(view->btn_save, "clicked",
				       _contact_save_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_save", view->btn_save);
	evas_object_show(view->btn_save);

	view->btn_cancel = elm_button_add(win);
	elm_button_label_set(view->btn_cancel, D_("Cancel"));
	evas_object_smart_callback_add(view->btn_cancel, "clicked",
				       _contact_cancel_clicked, view);
        ui_utils_view_swallow(VIEW_PTR(*view), "button_cancel", view->btn_cancel);
	evas_object_show(view->btn_cancel);

	view->btn_photo_remove = elm_button_add(win);
	elm_button_label_set(view->btn_photo_remove, D_("Remove"));
	evas_object_smart_callback_add(view->btn_photo_remove, "clicked",
				       _contact_photo_remove_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_photo_remove",
			      view->btn_photo_remove);
	evas_object_show(view->btn_photo_remove);

	view->btn_photo_back = elm_button_add(win);
	elm_button_label_set(view->btn_photo_back, D_("Back"));
	evas_object_smart_callback_add(view->btn_photo_back, "clicked",
				       _contact_photo_back_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_photo_back",
			      view->btn_photo_back);
	evas_object_show(view->btn_photo_back);

	view->btn_call = elm_button_add(win);
	elm_button_label_set(view->btn_call, D_("Call"));
	evas_object_smart_callback_add(view->btn_call, "clicked",
				       _contact_call_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_call", view->btn_call);
	evas_object_show(view->btn_call);

	view->btn_sms = elm_button_add(win);
	elm_button_label_set(view->btn_sms, D_("SMS"));
	evas_object_smart_callback_add(view->btn_sms, "clicked",
				       _contact_sms_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_sms", view->btn_sms);
	evas_object_show(view->btn_sms);

	view->btn_delete = elm_button_add(win);
	elm_button_label_set(view->btn_delete, D_("Delete"));
	evas_object_smart_callback_add(view->btn_delete, "clicked",
				       _contact_delete_clicked, view);
	evas_object_show(view->btn_delete);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_delete",
			      view->btn_delete);

	view->btn_addfield = elm_button_add(win);
	elm_button_label_set(view->btn_addfield, D_("Add Field"));
	evas_object_smart_callback_add(view->btn_addfield, "clicked",
				       _contact_add_field_clicked, view);
	evas_object_show(view->btn_addfield);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_addfield",
			      view->btn_addfield);

	_load_name(view);
	_load_number(view);
	_load_photo(view);
	_load_fields(view);

	return 0;
}

int
contact_view_is_init(const char *path)
{
	struct ContactViewData *view;

	if (contactviews == NULL) {
		return 0;
	}
	view = (struct ContactViewData *)
			g_hash_table_lookup(contactviews, path);
	if (view && ui_utils_view_is_init(VIEW_PTR(*view))) {
		return 1;
	}
	return 0;
}

void
contact_view_deinit(struct ContactViewData *view)
{
	if (view) {
		g_debug("Deiniting view for contact");
		ui_utils_view_deinit(VIEW_PTR(*view));
	}
	else {
		g_warning("contact_view_deinit without a view!");
	}
}

void
contact_view_show(const char *path)
{
	struct ContactViewData *view;

	g_debug("looking up contact view for '%s'", path);

	if (contactviews == NULL) {
		g_debug("No contact views loaded yet");
		return;
	}
	view = (struct ContactViewData *)
			g_hash_table_lookup(contactviews, path);
	if (view) {
		g_debug("found view [%X] for contact %s", view, path);
		ui_utils_view_show(VIEW_PTR(*view));
	}
	else {
		g_warning("Could not find view for contact '%s'", path);
	}
}

static void
_contact_delete_cb(GError *error, gpointer data)
{
	struct ContactViewData *view = (struct ContactViewData *)data;
	if (error) {
		ui_utils_dialog(VIEW_PTR(*view),
				D_("Deleting the contact failed!"), DIALOG_OK,
				NULL, NULL);
		g_warning("Deleting the contact failed: %s", error->message);
		return;
	}
	contact_view_deinit(view);
}

static void
_contact_delete_confirm_cb(int result, void *data)
{
	struct ContactViewData *view = (struct ContactViewData *)data;
	if (result == DIALOG_YES) {
		if (*view->path) {
			phoneui_utils_contact_delete(view->path,
					_contact_delete_cb, view);
			return;
		}
		contact_view_deinit(view);
	}
}

static void
_contact_delete_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *view = (struct ContactViewData *)_data;

	ui_utils_dialog(VIEW_PTR(*view), D_("Really delete this contact?"),
			DIALOG_YES|DIALOG_NO, _contact_delete_confirm_cb, view);
}

static void
_add_field_cb(const char *field, void *data)
{
	struct ContactViewData *view = (struct ContactViewData *)data;
	if (field) {
		struct ContactFieldData *fd = _add_field(view, field, "");
		fd->isnew = 1;
// 		_set_modify(view, 1);
	}
}

static void
_contact_add_field_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *view = (struct ContactViewData *)_data;
	ui_utils_contacts_field_select(VIEW_PTR(*view), _add_field_cb, view);
}

static void
_contact_photo_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactViewData *view;
	g_debug("you clicked on the photo :-)");
	view = _data;
}

static void
_contact_photo_back_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactViewData *view;
	view = _data;
	// TODO
}

static void
_contact_photo_remove_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactViewData *view;
	view = _data;
	// TODO: ask for confirmation
	// TODO: actually save it
	if (view->properties) {
		// TODO: mark that somehow as dirty
		g_hash_table_remove(view->properties, "Photo");
	}
	_load_photo(view);
}

static void
_contact_call_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *view = (struct ContactViewData *)_data;
        if (view->properties == NULL)
		return;

	// TODO: show an inwin to select the number if there is more than one
	const char *number =
		phoneui_utils_contact_display_phone_get(view->properties);
	phoneui_utils_dial(number, NULL, NULL);
}

static void
_contact_sms_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	const char *photo;
	char *str;
	GValue *tmp;
	struct ContactViewData *view = (struct ContactViewData *)_data;

	if (view->properties == NULL)
		return;

	// TODO: show an inwin to select the number
	str = phoneui_utils_contact_display_phone_get(view->properties);
	if (!str) {
		g_debug("contact needs a number to send a message ;)");
		return;
	}
	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "Phone",
				common_utils_new_gvalue_string(str));
	free(str);

	str = phoneui_utils_contact_display_name_get(view->properties);
	if (str) {
		g_hash_table_insert(options, "Name",
				common_utils_new_gvalue_string(str));
		free(str);
	}
	tmp = g_hash_table_lookup(view->properties, "Photo");
	if (tmp) {
		photo = g_value_get_string(tmp);
		g_hash_table_insert(options, "Photo",
				common_utils_new_gvalue_string(photo));
	}

	phoneui_messages_message_new(options);
	// TODO: free it !!!
	//g_hash_table_destroy(options);
}

static void
_change_field_cb(char *field, void *data)
{
	struct ContactFieldData *fd = (struct ContactFieldData *)data;
	if (field) {
		g_debug("Changing field: before=%s, new=%s, old=%s",
			fd->name, field, fd->oldname);
		if (!fd->oldname && !fd->isnew) {
			fd->oldname = fd->name;
		}
		/* we MUST NOT free name when it is needed as oldname */
		else if (fd->name) {
			free(fd->name);
		}
		fd->name = field;
		elm_button_label_set(fd->field_button, field);
		elm_label_label_set(fd->field_label, field);
		fd->dirty = 1;
		_set_modify(fd->view, 1);
	}
}

static void
_field_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactFieldData *fd = (struct ContactFieldData *)_data;
	ui_utils_contacts_field_select(VIEW_PTR(*fd->view),
				     _change_field_cb, fd);
}

static void
_value_changed(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactFieldData *fd = (struct ContactFieldData *)_data;
	char *s = ui_utils_entry_utf8_get(obj);
	g_debug("Changed the value of field %s to '%s'", fd->name, s);
	if (s) {
		/* we have to check if it actually really changed, because elm_button_add
		sends changed signals for non-user changes too */
		if (!fd->value || strcmp(fd->value, s)) {
			elm_label_label_set(fd->value_label, s);
			if (fd->value) {
				free(fd->value);
			}
			fd->value = s; // s is a freshly allocated string - no strdup needed
			fd->dirty = 1;
			_set_modify(fd->view, 1);
		}
		// TODO: correctly check if it got changed back to its original
	}
}

static void
_contact_save_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactViewData *view = (struct ContactViewData *)_data;

	if (!_changes_to_properties(view, 0)) {
		/* should not happen ? */
		g_message("No changes found ?");
		_set_modify(view, 0);
		return;
	}

	if (*view->path) {
		g_debug("Updating contact '%s'", view->path);
		phoneui_utils_contact_update(view->path, view->properties,
					     _update_cb, view);
	}
	else {
		g_debug("Saving a new contact");
		phoneui_utils_contact_add(view->properties, _add_cb, view);
	}
}

static void
_contact_cancel_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactViewData *view = (struct ContactViewData *)_data;
	_set_modify(view, 0);
	_load_fields(view);
}

static void
_set_modify(struct ContactViewData *view, int dirty)
{
	if (view->have_unsaved_changes == dirty)
		return;
	g_debug("modify is %s now", dirty ? "ON" : "OFF");
	view->have_unsaved_changes = dirty;
	if (dirty) {
		edje_object_signal_emit(
			ui_utils_view_layout_get(VIEW_PTR(*view)),
					"elm,state,dirty", "");
	}
	else {
		edje_object_signal_emit(
			ui_utils_view_layout_get(VIEW_PTR(*view)),
					"elm,state,default", "");
	}
}

static void
_update_cb(GError *error, gpointer data)
{
	struct ContactViewData *view = (struct ContactViewData *)data;

	if (error) {
		g_warning("Updating contact %s failed", view->path);
	}
	else {
		phoneui_utils_contact_get(view->path, _load_cb, view);
		_set_modify(view, 0);
	}
}

static void
_add_cb(GError *error, char *path, gpointer data)
{
	struct ContactViewData *view = (struct ContactViewData *)data;
	if (error) {
		g_warning("Adding the contact failed");
	}
	else {
		view->path = path;
		g_hash_table_destroy(view->properties);
		phoneui_utils_contact_get(view->path, _load_cb, view);
		_set_modify(view, 0);
	}
}

static void
_load_cb(GHashTable *content, gpointer data)
{
	struct ContactViewData *view = (struct ContactViewData *)data;
	if (!content) {
		g_critical("Failed loading data of saved contact");
		return;
	}
	/* cleanup up the old data of the contact */
	if (view->properties) {
		g_hash_table_destroy(view->properties);
	}
	view->properties = content;
	_load_name(view);
	_load_number(view);
	_load_fields(view);
}


static int
_changes_to_properties(struct ContactViewData *view, int dry_run)
{
	Elm_Genlist_Item *it;
	struct ContactFieldData *fd;
	int have_changes = 0;

	g_debug("Checking dirtyness of fields");
	it = elm_genlist_first_item_get(view->fields);
	while (it) {
		fd = (struct ContactFieldData *)elm_genlist_item_data_get(it);
		if (fd->dirty) {
			g_debug("found dirty field %s", fd->name);
			have_changes = 1;
			if (!dry_run) {
				_update_one_field(view, fd);
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	g_debug("Done - there were %s changes", have_changes ? "some" : "NO");

	return have_changes;
}

static void
_update_one_field(struct ContactViewData *view, struct ContactFieldData *fd)
{
	/* for new contacts we might have to create the properties hashtable */
	if (view->properties == NULL) {
		g_debug("no properties hashtable yet... creating");
		view->properties = g_hash_table_new_full(g_str_hash,
						g_str_equal, free, free);
	}
	/* if oldname is set the field got either removed or renamed */
	if (fd->oldname) {
		g_debug("Delete/rename field %s", fd->oldname);
		g_hash_table_insert(view->properties, strdup(fd->oldname),
			common_utils_new_gvalue_string(""));
	}
	g_debug("Set field %s to '%s'", fd->name, fd->value);
	g_hash_table_insert(view->properties, strdup(fd->name),
			common_utils_new_gvalue_string(fd->value));
}

/* genlist callbacks */

static Evas_Object *
gl_field_icon_get(const void *_data, Evas_Object * obj, const char *part)
{
	g_debug("gl_field_icon_get (part=%s)", part);
	struct ContactFieldData *fd = (struct ContactFieldData *) _data;
	if (strcmp(part, "elm.swallow.field_label") == 0) {
		Evas_Object *lbl = elm_label_add(obj);
// 		evas_object_size_hint_align_set(obj, 1.0, 0.0);
		elm_label_label_set(lbl, fd->name);
		fd->field_label = lbl;
		return lbl;
	}
	else if (strcmp(part, "elm.swallow.field_button") == 0) {
		Evas_Object *btn = elm_button_add(obj);
		elm_button_label_set(btn, fd->name);
		evas_object_smart_callback_add(btn, "clicked",
					       _field_clicked, fd);
		fd->field_button = btn;
		return btn;
	}
	else if (strcmp(part, "elm.swallow.value_label") == 0) {
		Evas_Object *lbl = elm_label_add(obj);
		elm_label_label_set(lbl, fd->value);
		fd->value_label = lbl;
		return lbl;
	}
	else if (strcmp(part, "elm.swallow.value_entry") == 0) {
		Evas_Object *entry = elm_entry_add(obj);
		elm_entry_entry_set(entry, fd->value);
		evas_object_size_hint_align_set(entry, EVAS_HINT_FILL,
						EVAS_HINT_FILL);
		evas_object_smart_callback_add(entry, "changed", _value_changed,
					       fd);
		fd->value_entry = entry;
		return entry;
	}
	return NULL;
}

static void
gl_field_del(const void *_data, Evas_Object * obj)
{
	g_debug("gl_field_del");
	struct ContactFieldData *fd = (struct ContactFieldData *) _data;
	if (fd) {
		if (fd->name)
			free(fd->name);
		if (fd->value)
			free(fd->value);
		if (fd->oldname)
			free(fd->oldname);
		free(fd);
	}
	g_debug("gl_field_del DONE");
}

static void
_load_name(struct ContactViewData *view)
{
	char *s = NULL;
	g_debug("Loading name");
	if (view->properties) {
		s = phoneui_utils_contact_display_name_get
							(view->properties);
	}
	if (s) {
		g_debug("Found name '%s'", s);
		ui_utils_view_text_set(&view->parent, "name", s);
		elm_win_title_set(ui_utils_view_window_get(VIEW_PTR(*view)), s);
		free(s);
	}
	else {
		g_debug("No name found");
		ui_utils_view_text_set(&view->parent, "name", "");
	}
}

static void
_load_number(struct ContactViewData *view)
{
	char *s = NULL;

	g_debug("Loading number");

	if (view->properties) {
		s = phoneui_utils_contact_display_phone_get(view->properties);
	}
	if (s) {
		g_debug("Found number '%s'", s);
		elm_object_disabled_set(view->btn_call, EINA_FALSE);
		elm_object_disabled_set(view->btn_sms, EINA_FALSE);
		ui_utils_view_text_set(VIEW_PTR(*view), "number", s);
		free(s);
	}
	else {
		g_debug("No number found");
		elm_object_disabled_set(view->btn_call, EINA_TRUE);
		elm_object_disabled_set(view->btn_sms, EINA_TRUE);
		ui_utils_view_text_set(VIEW_PTR(*view), "number", "");
	}
}

static void
_load_photo(struct ContactViewData *view)
{
	const char *s;
	GValue *tmp = NULL;

	g_debug("Loading photo");
	if (view->properties) {
		tmp = g_hash_table_lookup(view->properties, "Photo");
	}
	if (tmp) {
		s = g_value_get_string(tmp);
		if (!strncmp(s, "file://", 7))
		s += 7;
		g_debug("Found photo %s", s);
	}
	else {
		g_debug("No photo found");
		s = CONTACT_DEFAULT_PHOTO;
	}
	elm_icon_file_set(view->photo, s, NULL);
}

static void
_load_fields(struct ContactViewData *view)
{
	GHashTableIter iter;
	gpointer key, value;

	g_debug("Loading field list");

	elm_genlist_clear(view->fields);
	if (view->properties) {
		g_hash_table_iter_init(&iter, view->properties);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			if (!strcmp(key, "Path"))
				continue;
			_add_field(view, key, g_value_get_string(value));
		}
	}
}

static struct ContactFieldData *
_add_field(struct ContactViewData *view,
	   const char *key, const char *value)
{
	g_debug("Adding field <%s> with value '%s' to list", key, value);
	struct ContactFieldData *fd =
		malloc(sizeof(struct ContactFieldData));
	if (fd == NULL) {
		g_critical("Failed allocating field data!");
		return NULL;
	}
	fd->name = strdup(key);
	fd->value = strdup(value);
	fd->oldname = NULL;
	fd->field_label = NULL;
	fd->field_button = NULL;
	fd->value_label = NULL;
	fd->value_entry = NULL;
	fd->view = view;
	fd->dirty = 0;
	fd->isnew = 0;

	elm_genlist_item_append(view->fields, &itc, fd, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	return fd;
}


static void
_delete_cb(struct View *view, Evas_Object * win, void *event_info)
{
	(void)win;
	(void)event_info;
	g_debug("_delete_cb");
	contact_view_deinit((struct ContactViewData *)view);
	g_debug("_delete_cb DONE");
}

static void
_destroy_cb(struct View *_view)
{
	struct ContactViewData *view = (struct ContactViewData *)_view;
	g_debug("_destroy_cb");
	if (view->properties)
		g_hash_table_destroy(view->properties);
	g_hash_table_remove(contactviews, view->path);
	g_debug("_destroy_cb DONE");
}
