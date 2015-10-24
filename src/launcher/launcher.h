/*
 * popup-launcher
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __LAUNCHER_H__
#define __LAUNCHER_H__

#include <stdio.h>
#include <bundle_internal.h>
#include <E_DBus.h>
#include <Ecore.h>
#include <Elementary.h>
#include <appcore-common.h>
#include <aul.h>
#include "macro.h"

/* Popup names */
#define POWEROFF_SYSPOPUP  "poweroff-syspopup"
#define SYSTEM_SYSPOPUP    "system-syspopup"
#define CRASH_SYSPOPUP     "crash-popup"

/* Setting ugs */
#define SETTING_ENCRYPTING_APP    "org.tizen.setting.encrypting"
#define SETTING_LIGHTOFF_APP      "org.tizen.setting.turnofflight"
#define SETTING_DATAUSAGE_UG      "setting-datausage-efl"


/* App to show Host devices list */
#define HOST_DEVICES              "org.tizen.host-devices"
#define APPOPER_TYPE              "_TYPE_"
#define APPOPER_TYPE_DEVICE_LIST  "DEVICE_LIST"

/* Notification icons */
#define DATAUSAGE_ICON   SYSTEM_ICONDIR"/datausage_warning.png"
#define LED_TORCH_ICON   SYSTEM_ICONDIR"/led_torch.png"
#define ODE_ENCRYPT_ICON SYSTEM_ICONDIR"/sdcard_encryption.png"
#define ODE_DECRYPT_ICON SYSTEM_ICONDIR"/sdcard_decryption.png"
#define ODE_ENCRYPT_ERROR_ICON SYSTEM_ICONDIR"/sdcard_encryption_error.png"
#define ODE_DECRYPT_ERROR_ICON SYSTEM_ICONDIR"/sdcard_decryption_error.png"
#define TIMA_ICON        SYSTEM_ICONDIR"/tima.png"
#define USB_ICON         SYSTEM_ICONDIR"/usb.png"
#define BATT_INDI_ICON   SYSTEM_ICONDIR"/batt_full_indicator.png"
#define BATT_NOTI_ICON   SYSTEM_ICONDIR"/batt_full_icon.png"

/* App launching */
#define CRADLE_APP_NAME  "org.tizen.desk-dock"
#define PWLOCK_APP_NAME  "org.tizen.pwlock"
#define SERVANT_APP_NAME "/usr/bin/system-servant"

enum noti_type {
	NOTI_NONE = 0,
	DATAUSAGE_WARNING,
	DATAUSAGE_DISABLED,
	LED_TORCH,
	ODE_COMPLETE,
	ODE_PROGRESS,
	ODE_ERROR,
	TIMA_LKM_PREVENTION,
	TIMA_PKM_DETECTION,
	TICKER,
	USB_STORAGE,
	USB_STORAGE_RO,
	USB_DEVICE,
	BATTERY_FULL,
	/* Add here additional notificatoins */
	NOTI_TYPE_MAX
};

enum service_type {
	TTS_SCREENOFF,
	TTS_ENABLED,
	TTS_DISABLED,
	CHARGER_CONNECTION,
	SERVICE_MAX,
};

struct edbus_method {
	const char *member;
	const char *signature;
	const char *reply_signature;
	E_DBus_Method_Cb func;
	int type;
};

struct edbus_object {
	const char *path;
	const char *interface;
	E_DBus_Object *obj;
	E_DBus_Interface *iface;
	const struct edbus_method *methods;
	const int methods_len;
};

/* launch popup */
DBusMessage *launch_popup(E_DBus_Object *obj,
				DBusMessage *msg, char *name);
DBusMessage *launch_poweroff_popup(E_DBus_Object *obj,
				DBusMessage *msg, char *name);

/* Activate notification */
DBusMessage *activate_notification_no_param(E_DBus_Object *obj,
				DBusMessage *msg, int type);
DBusMessage *activate_notification_single_param(E_DBus_Object *obj,
				DBusMessage *msg, int type);
DBusMessage *activate_notification_double_param(E_DBus_Object *obj,
				DBusMessage *msg, int type);
DBusMessage *activate_notification_triple_param(E_DBus_Object *obj,
				DBusMessage *msg, int type);

/* Deactivate notification */
DBusMessage *deactivate_notification(E_DBus_Object *obj, DBusMessage *msg);

/* Update progress bar on the notification */
DBusMessage *progress_update_notification(E_DBus_Object *obj, DBusMessage *msg);

/* Update text of notification */
DBusMessage *update_notification_double_param(E_DBus_Object *obj,
				DBusMessage *msg, int type);

/* launch app */
DBusMessage *launch_app_no_param(E_DBus_Object *obj,
		DBusMessage *msg, char *appname);
DBusMessage *launch_app_single_param(E_DBus_Object *obj,
		DBusMessage *msg, char *appname);
DBusMessage *terminate_app_by_pid(E_DBus_Object *obj, DBusMessage *msg);
DBusMessage *launch_system_servant_app(E_DBus_Object *obj,
		DBusMessage *msg, char **argv);

#endif /* __LAUNCHER_H__ */

