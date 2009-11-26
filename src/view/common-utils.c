#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <string.h>

#include "common-utils.h"

static GHashTable *ref_counter = NULL;

char *
common_utils_skip_prefix(char *string, const char *prefix)
{
	int prefix_len = strlen(prefix);
	if (!strncmp(string, prefix, prefix_len)) {
		string += prefix_len;
	}
	return string;
}

GValue *
common_utils_new_gvalue_string(const char *value)
{
	GValue *val = calloc(1, sizeof(GValue));
	if (!val) {
		return NULL;
	}
	g_value_init(val, G_TYPE_STRING);
	g_value_set_string(val, value);

	return val;
}

GValue *
common_utils_new_gvalue_int(int value)
{
	GValue *val = calloc(1, sizeof(GValue));
	if (!val) {
		return NULL;
	}
	g_value_init(val, G_TYPE_INT);
	g_value_set_int(val, value);

	return val;
}

GValue *
common_utils_new_gvalue_boolean(int value)
{
	GValue *val = calloc(1, sizeof(GValue));
	if (!val) {
		return NULL;
	}
	g_value_init(val, G_TYPE_BOOLEAN);
	g_value_set_boolean(val, value);

	return val;
}

char *
common_utils_new_with_prefix(const char *_number, const char *prefix)
{
	char *number = NULL;
	int prefix_len = strlen(prefix);
	if (strncmp(_number, prefix, prefix_len)) {
		number = malloc(strlen(_number) + prefix_len + 1); /* 5 is for "tel:" and the null */
		if (!number) {
			return NULL;
		}
		strcpy(number, prefix);
		strcat(number, _number);
	}


	return number;
}
void *
common_utils_object_ref(void *object)
{
	void *ret;
	int count;
	if (!ref_counter) {
		ref_counter = g_hash_table_new_full(g_direct_hash, g_direct_equal,
					NULL, NULL); 
	}
	ret = g_hash_table_lookup(ref_counter, object);
	if (ret) {
		count = GPOINTER_TO_INT(ret);
		count++;
	}
	else {
		count = 1;
	}
	g_hash_table_replace(ref_counter, object, count);
	return object;
}

int
common_utils_object_unref(void *object)
{
	void *ret;
	int count;
	
	ret = g_hash_table_lookup(ref_counter, object);
	if (!ret) {
		return -1;
	}

	count = GPOINTER_TO_INT(ret);
	count--;
	if (count <= 0) {
		return 0;
	}
	else {
		g_hash_table_replace(ref_counter, object, count);
	}
	return count;
}

void
common_utils_object_unref_free(void *object)
{
	if (!common_utils_object_unref(object)) {
		free(object);
	}
}

int
common_utils_object_get_ref(void *object)
{
	void *ret;
	int count;
	ret = g_hash_table_lookup(ref_counter, object);
	if (!ret) {
		return 0;
	}

	count = GPOINTER_TO_INT(ret);
	return count;
}
