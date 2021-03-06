/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
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



#ifndef _CONTACTLIST_COMMON_H
#define _CONTACTLIST_COMMON_H

#include "ui-utils.h"

struct ContactListData {
	struct View *view;
	Evas_Object *layout;
	Evas_Object *list;
	Evas_Object *index;
	Evas_Object *progress;
	int count;
	int current;
};

void contact_list_fill_index(struct ContactListData *list_data);
void contact_list_fill(struct ContactListData *list_data);
void contact_list_add(struct ContactListData *list_data);
Elm_Genlist_Item *contact_list_item_add(struct ContactListData *list_data, GHashTable *entry, int sortin);


#endif
