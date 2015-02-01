/*
 *  system-popup
 *
 * Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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


#ifndef __COMMON_H_
#define __COMMON_H_

#include <Ecore_X.h>
#include <appcore-efl.h>
#include <Elementary.h>
#include <dlog.h>
#include <glib.h>
#include <utilX.h>
#include <syspopup.h>
#include <syspopup_caller.h>
#include <feedback.h>
#include <E_DBus.h>

#undef LOG_TAG
#define LOG_TAG "SYSTEM_APPS"
#define _D(fmt, args...)   SLOGD(fmt, ##args)
#define _E(fmt, args...)   SLOGE(fmt, ##args)
#define _I(fmt, args...)   SLOGI(fmt, ##args)

#define FREE(arg) \
	do { \
		if(arg) { \
			free((void *)arg); \
			arg = NULL; \
		} \
	} while (0);

#define ARRAY_SIZE(name) (sizeof(name)/sizeof(name[0]))

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	   __typeof__ (b) _b = (b);  \
	   _a > _b ? _a : _b; })

enum win_priority {
	WIN_PRIORITY_LOW    = UTILX_NOTIFICATION_LEVEL_LOW,
	WIN_PRIORITY_NORMAL = UTILX_NOTIFICATION_LEVEL_NORMAL,
	WIN_PRIORITY_HIGH   = UTILX_NOTIFICATION_LEVEL_HIGH,
};

struct appdata {
	/* Common */
	Evas_Object *win_main;
	Evas_Object *popup;
	Evas_Object *list;
	bundle      *b;
	GList       *options;

	syspopup_handler handler;
};

void popup_terminate(void);
void release_evas_object(Evas_Object **obj);
void object_cleanup(struct appdata *ad);
Evas_Object *create_win(const char *name);
Evas_Object *load_normal_popup(struct appdata *ad,
		char *title,
		char *content,
		char *lbtnText,
		Evas_Smart_Cb lbtn_cb,
		char *rbtnText,
		Evas_Smart_Cb rbtn_cb);

Evas_Object *load_scrollable_popup(struct appdata *ad,
			char *ly_style,
			char *edj_name,
			char *title,
			char *content,
			char *lbtnText,
			Evas_Smart_Cb lbtn_cb,
			char *rbtnText,
			Evas_Smart_Cb rbtn_cb);
Evas_Object *load_popup_toast(struct appdata *ad,
			char *content);
bool get_check_state(Evas_Object *check);
Evas_Object *load_scrollable_check_popup(struct appdata *ad,
			char *edj_name,
			char *title,
			char *content,
			char *text_check,
			Evas_Object **check,
			char *lbtnText,
			Evas_Smart_Cb lbtn_cb,
			char *rbtnText,
			Evas_Smart_Cb rbtn_cb);


/* dbus */
int set_dbus_connection(void);
void unset_dbus_connection(void);
E_DBus_Connection *get_dbus_connection(void);

int broadcast_dbus_signal(const char *path, const char *interface,
		const char *name, const char *sig, char *param[]);

int reset_window_priority(Evas_Object *win, int priority);

/* feedback */
void play_feedback(int type, int pattern);

/* change integer to string */
int change_pd_to_ps(char *text);
int get_str_from_icu(int number, char *buf, int buf_len);

#endif				/* __COMMON_H__ */
