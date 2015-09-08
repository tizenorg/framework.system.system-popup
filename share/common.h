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
#include <utilX.h>
#include <dlog.h>
#include <bundle.h>
#include <glib.h>
#include <syspopup.h>
#include <syspopup_caller.h>
#include <feedback.h>

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

#define DEVICED_BUS_NAME    "org.tizen.system.deviced"
#define DEVICED_PATH_HALL   "/Org/Tizen/System/DeviceD/Hall"
#define DEVICED_IFACE_HALL  DEVICED_BUS_NAME".hall"
#define HALL_STATE_SIGNAL   "ChangeState"

enum win_priority {
	WIN_PRIORITY_LOW    = UTILX_NOTIFICATION_LEVEL_LOW,
	WIN_PRIORITY_NORMAL = UTILX_NOTIFICATION_LEVEL_NORMAL,
	WIN_PRIORITY_HIGH   = UTILX_NOTIFICATION_LEVEL_HIGH,
};

enum play_feedback_type {
	PLAY_ALL            = 0,
	PLAY_LED_ONLY       = FEEDBACK_TYPE_LED,
	PLAY_SOUND_ONLY     = FEEDBACK_TYPE_SOUND,
	PLAY_VIBRATION_ONLY = FEEDBACK_TYPE_VIBRATION,
	PLAY_END            = FEEDBACK_TYPE_END
};

enum play_feedback_pattern {
	PATTERN_POWEROFF    = FEEDBACK_PATTERN_POWEROFF,
	PATTERN_HW_TAP      = FEEDBACK_PATTERN_HW_TAP,
	PATTERN_LOWBAT      = FEEDBACK_PATTERN_LOWBATT,
	PATTERN_LOWMEM      = FEEDBACK_PATTERN_LOWBATT,
	PATTERN_MMC         = FEEDBACK_PATTERN_LOWBATT,
	PATTERN_USB         = FEEDBACK_PATTERN_LOWBATT,
	PATTERN_USBOTG      = FEEDBACK_PATTERN_LOWBATT,
	PATTERN_RECOVERY    = FEEDBACK_PATTERN_LOWBATT,
	PATTERN_WATCHDOG    = FEEDBACK_PATTERN_LOWBATT,
	PATTERN_DATAUSAGE   = FEEDBACK_PATTERN_LOWBATT,
	PATTERN_CRASH       = FEEDBACK_PATTERN_LOWBATT,
	PATTERN_WARNING     = FEEDBACK_PATTERN_LOWBATT,
	PATTERN_CHARGERCONN = FEEDBACK_PATTERN_CHARGERCONN,
	PATTERN_FULLCHARGED = FEEDBACK_PATTERN_FULLCHARGED,
	PATTERN_VIBRATION   = FEEDBACK_PATTERN_VIBRATION_ON,
	PATTERN_SOUND       = FEEDBACK_PATTERN_SILENT_OFF,
	PATTERN_END         = FEEDBACK_PATTERN_END
} ;

struct appdata {
	/* Common */
	Evas_Object *win_main;
	Evas_Object *layout_main;
	Evas_Object *popup;
	Evas_Object *list;
	Evas_Object *popup_chk;
	bundle      *b;

	syspopup_handler handler;

	GList       *options;

	/* For usbotg popup */
	Evas_Object *storage_added_popup;
	Evas_Object *storage_unmount_popup;
	Evas_Object *camera_added_popup;
	char *storage_added_path;
	char *storage_unmount_path;

	/* IPC by dbus */
	E_DBus_Signal_Handler *edbus_handler;
	E_DBus_Connection     *edbus_conn;

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
Evas_Object *load_popup_with_vertical_buttons(struct appdata *ad,
		char *title,
		char *content,
		char *ubtnText,
		Evas_Smart_Cb ubtn_cb,
		char *dbtnText,
		Evas_Smart_Cb dbtn_cb);
Evas_Object *load_scrollable_popup(struct appdata *ad,
		char *title,
		char *content,
		char *lbtnText,
		Evas_Smart_Cb lbtn_cb,
		char *rbtnText,
		Evas_Smart_Cb rbtn_cb);

void play_feedback(int type, int pattern);
int set_display_feedback(int type);
int device_poweroff(void);

/* Send dbus signal */
int broadcast_dbus_signal(const char *path,
		const char *interface,
		const char *name,
		const char *sig,
		char *param[]);

int get_hallic_status(void);
int reset_window_priority(Evas_Object *win, int priority);
int set_popup_focus(Evas_Object *win, bool focus);

int dbus_method_sync(const char *dest, const char *path,
		const char *interface, const char *method,
		const char *sig, char *param[]);


#endif				/* __COMMON_H__ */
