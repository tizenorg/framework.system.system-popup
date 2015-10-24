/*
 *  system-popup
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef __COMMON_INTERNAL_H__
#define __COMMON_INTERNAL_H__

#include <stdio.h>
#include <Ecore_X.h>
#include <appcore-efl.h>
#include <Elementary.h>
#include <utilX.h>
#include <dlog.h>
#include <glib.h>
#include <vconf.h>
#include <string.h>
#include <syspopup.h>
#include <syspopup_caller.h>
#include <feedback.h>
#include <E_DBus.h>
#include <efl_extension.h>
#include <bundle.h>
#include <bundle_internal.h>

enum win_priority {
	WIN_PRIORITY_LOW    = UTILX_NOTIFICATION_LEVEL_LOW,
	WIN_PRIORITY_NORMAL = UTILX_NOTIFICATION_LEVEL_NORMAL,
	WIN_PRIORITY_HIGH   = UTILX_NOTIFICATION_LEVEL_HIGH,
};

struct popup_ops;

struct object_ops {
	const struct popup_ops	*ops;
	Evas_Object				*popup;
	Evas_Object				*check;
	bundle					*b;
};

/* Common */
GList *get_popup_list(void);
void popup_terminate(void);
void release_evas_object(Evas_Object **obj);
int get_object_by_ops(const struct popup_ops *ops, struct object_ops **obj);

/* Window */
int  create_window(const char *name);
void remove_window(void);
Evas_Object *get_window(void);
int reset_window_priority(int priority);

#endif /* __COMMON_INTERNAL_H__ */
