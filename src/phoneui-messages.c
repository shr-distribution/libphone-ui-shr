
#include <glib.h>
#include <phoneui/phoneui-utils.h>
#include "phoneui-messages.h"
#include "view/message-list-view.h"
#include "view/message-show-view.h"
#include "view/message-new-view.h"
#include "view/views.h"


void
phoneui_backend_messages_show()
{
 	if (!message_list_view_is_init()) {
		if (message_list_view_init()) {
			return;
		}
	}
	message_list_view_show();
}

static void
_message_get_cb(GHashTable *content, gpointer data)
{
	char *path = (char *)data;
	if (!content) {
		g_warning("Failed aquiring data for message %s", path);
		free (path);
		return;
	}
	g_debug("Got data for message %s", path);
	if (message_show_view_init(path, content)) {
		g_warning("Init of view for message %s failed", path);
		return;
	}

	g_debug("Showing message %s", path);
	message_show_view_show(path);
}

void
phoneui_backend_messages_message_show(const char *path)
{
	if (!message_show_view_is_init(path)) {
		g_debug("View for message %s is not yet inited...", path);
		phoneui_utils_message_get(path, _message_get_cb, strdup(path));
		return;
	}
	g_debug("View for message %s is inited - showing", path);
	message_show_view_show(path);
}

void
phoneui_backend_messages_message_new(GHashTable *options)
{
	struct MessageNewViewData *view;
	g_debug("Initing new message view");
	view = message_new_view_init(options);
	if (!view) {
		return;
	}
	g_debug("Showing new message view");
	message_new_view_show(view);
}
