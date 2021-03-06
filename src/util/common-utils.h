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


#ifndef _COMMON_UTILS_H
#define _COMMON_UTILS_H
#include <glib.h>
#include <glib-object.h>

#define LTR_STRING "\xE2\x80\x8E"

GValue *
common_utils_new_gvalue_string(const char *value);

GValue *
common_utils_new_gvalue_int(int value);

GValue *
common_utils_new_gvalue_boolean(int value);

GValue *
common_utils_new_gvalue_pointer(gpointer value);

GValue *
common_utils_new_gvalue_boxed(GType type, gpointer value);

void
common_utils_gvalue_free(gpointer val);

GHashTable *
common_utils_gvalue_ghashtable_clone(GHashTable *hash_table);

void *
common_utils_object_ref(void *object);

int
common_utils_object_unref(void *object);
void
common_utils_object_unref_free(void *object);
int
common_utils_object_get_ref(void *object);

int
common_utils_is_pin(const char *string);
int
common_utils_is_puk(const char *string);

char *
common_utils_string_strip_newline(char *string);

char *
common_utils_timestamp_to_date(long timestamp);

void
common_utils_debug_dump_hashtable(GHashTable *hasht);

#endif
