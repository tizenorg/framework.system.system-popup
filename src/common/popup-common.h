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


#ifndef __POPUP_COMMON_H__
#define __POPUP_COMMON_H__

#include "popup-common-internal.h"

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

enum popup_flags {
	SCROLLABLE        = 0x0001,
	CHECK_BOX         = 0x0002,
	FLAG_MAX          = 0xffff,
};

struct popup_ops {
	char *name;
	int  (*show_popup) (bundle *b, const struct popup_ops *ops);
	char *title;
	char *content;
	int  (*get_content)(const struct popup_ops *ops, char *content, unsigned int len);
	char *left_text;
	char *left_icon;
	void (*left)(const struct popup_ops *ops);
	char *right_text;
	char *right_icon;
	void (*right)(const struct popup_ops *ops);
	char *check_text;
	bool (*skip)(const struct popup_ops *ops);
	void (*launch)(const struct popup_ops *ops);
	void (*terminate)(const struct popup_ops *ops);
	unsigned int flags;
};

/* Common */
void terminate_if_no_popup(void);

/* Popup */
void register_popup(const struct popup_ops *ops);
void unload_simple_popup(const struct popup_ops *ops);
int load_simple_popup(bundle *b, const struct popup_ops *ops);
bool get_check_state(const struct popup_ops *ops);

/* dbus */
int set_dbus_connection(void);
void unset_dbus_connection(void);
E_DBus_Connection *get_dbus_connection(void);
int broadcast_dbus_signal(
		const char *path,
		const char *interface,
		const char *name,
		const char *sig,
		char *param[]);
int dbus_method_sync(
		const char *dest,
		const char *path,
		const char *interface,
		const char *method,
		const char *sig,
		char *param[]);

/* feedback */
void play_feedback(int type, int pattern);
void notify_feedback(int pattern); /* using thread */

void change_display_state(void); /* using thread */

#endif /* __POPUP_COMMON_H__ */
