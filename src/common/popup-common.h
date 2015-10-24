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
#include "macro.h"

enum popup_flags {
	SCROLLABLE        = 0x0001,
	CHECK_BOX         = 0x0002,
	FLAG_MAX          = 0xffff,
};

struct popup_ops {
	char *name;
	void (*change_popup)(const struct popup_ops *ops);
	int  (*show_popup) (bundle *b, const struct popup_ops *ops);
	char *title;
	char *content;
	int  (*get_content)(const struct popup_ops *ops, char *content, unsigned int len);
	char *left_text;
	void (*left)(const struct popup_ops *ops);
	char *right_text;
	void (*right)(const struct popup_ops *ops);
	char *check_text;
	bool (*skip)(const struct popup_ops *ops);
	void (*launch)(const struct popup_ops *ops);
	void (*terminate)(const struct popup_ops *ops);
	bool (*term_pause)(const struct popup_ops *ops);
	bool (*term_home)(const struct popup_ops *ops);
	unsigned int flags;
};

/* Common */
void terminate_if_no_popup(void);

/* Popup */
void register_popup(const struct popup_ops *ops);
void update_popup(const struct popup_ops *old_ops, const struct popup_ops *new_ops);
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
int popup_dbus_method_sync(
		const char *dest,
		const char *path,
		const char *interface,
		const char *method,
		const char *sig,
		char *param[]);
int register_dbus_signal_handler(
		E_DBus_Signal_Handler **handler,
		const char *path,
		const char *iface,
		const char *name,
		void (*signal_cb)(void *data, DBusMessage *msg),
		void *data);
void unregister_dbus_signal_handler(E_DBus_Signal_Handler *handler);

/* feedback */
void play_feedback(int type, int pattern);
void notify_feedback(int pattern); /* using thread */

void change_display_state(void); /* using thread */

#endif /* __POPUP_COMMON_H__ */
