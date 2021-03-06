/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Thomas Zimmermann <bugs@vdm-design.de>
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


#ifndef _PHONEUI_CALENDAR_H
#define _PHONEUI_CALENDAR_H
#include <glib.h>

void phoneui_backend_calendar_month_show();
void phoneui_backend_calendar_deinit();
void phoneui_backend_calendar_day_show(const char *day);
void phoneui_backend_calendar_date_show(const char *path);
void phoneui_backend_calendar_date_new(GHashTable *options);

#endif
