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
#include <bundle.h>
#include <E_DBus.h>
#include <aul.h>
#include "common.h"

#define RETRY_MAX 10
#define SLEEP_USEC 200000

/* DBus paths, interfaces */
#define BUS_NAME              "org.tizen.system.popup"
#define POPUP_DBUS_PATH       "/Org/Tizen/System/Popup"
#define POPUP_DBUS_IFACE      BUS_NAME

#define POPUP_PATH_POWEROFF   POPUP_DBUS_PATH"/Poweroff"
#define POPUP_IFACE_POWEROFF  BUS_NAME".Poweroff"

#define POPUP_PATH_LOWBAT     POPUP_DBUS_PATH"/Lowbat"
#define POPUP_IFACE_LOWBAT    BUS_NAME".Lowbat"

#define POPUP_PATH_LOWMEM     POPUP_DBUS_PATH"/Lowmem"
#define POPUP_IFACE_LOWMEM    BUS_NAME".Lowmem"

#define POPUP_PATH_MMC        POPUP_DBUS_PATH"/Mmc"
#define POPUP_IFACE_MMC       BUS_NAME".Mmc"

#define POPUP_PATH_USB        POPUP_DBUS_PATH"/Usb"
#define POPUP_IFACE_USB       BUS_NAME".Usb"

#define POPUP_PATH_USBOTG     POPUP_DBUS_PATH"/Usbotg"
#define POPUP_IFACE_USBOTG    BUS_NAME".Usbotg"

#define POPUP_PATH_DATAUSAGE  POPUP_DBUS_PATH"/DataUsage"
#define POPUP_IFACE_DATAUSAGE BUS_NAME".DataUsage"

#define POPUP_PATH_LED        POPUP_DBUS_PATH"/Led"
#define POPUP_IFACE_LED       BUS_NAME".Led"

#define POPUP_PATH_ODE        POPUP_DBUS_PATH"/Ode"
#define POPUP_IFACE_ODE       BUS_NAME".Ode"

#define POPUP_PATH_SYSTEM     POPUP_DBUS_PATH"/System"
#define POPUP_IFACE_SYSTEM    BUS_NAME".System"

#define POPUP_PATH_TIMA       POPUP_DBUS_PATH"/Tima"
#define POPUP_IFACE_TIMA      BUS_NAME".Tima"

#define POPUP_PATH_CRASH      POPUP_DBUS_PATH"/Crash"
#define POPUP_IFACE_CRASH     BUS_NAME".Crash"

#define POPUP_PATH_TICKER     POPUP_DBUS_PATH"/Ticker"
#define POPUP_IFACE_TICKER    BUS_NAME".Ticker"

#define POPUP_PATH_APP        POPUP_DBUS_PATH"/Apps"
#define POPUP_IFACE_APP       BUS_NAME".Apps"

#define POPUP_PATH_USBHOST    POPUP_DBUS_PATH"/Usbhost"
#define POPUP_IFACE_USBHOST   BUS_NAME".Usbhost"

#define POPUP_PATH_BATTERY    POPUP_DBUS_PATH"/Battery"
#define POPUP_IFACE_BATTERY   BUS_NAME".Battery"

#define POPUP_PATH_SERVANT    POPUP_DBUS_PATH"/Servant"
#define POPUP_IFACE_SERVANT   BUS_NAME".Servant"

/* Popup names */
#define POWEROFF_SYSPOPUP  "poweroff-syspopup"
#define LOWBAT_SYSPOPUP    "lowbat-syspopup"
#define LOWMEM_SYSPOPUP    "lowmem-syspopup"
#define MMC_SYSPOPUP       "mmc-syspopup"
#define USB_SYSPOPUP       "usb-syspopup"
#define USBOTG_SYSPOPUP    "usbotg-syspopup"
#define DATAUSAGE_SYSPOPUP "datausage-syspopup"
#define SYSTEM_SYSPOPUP    "system-syspopup"
#define CRASH_SYSPOPUP     "crash-popup"

/* Setting ugs */
#define SETTING_DATAUSAGE_UG      "setting-datausage-efl"
#define SETTING_ACCESSIBILITY_UG  "setting-accessibility-efl"
#define SETTING_MMC_ENCRYPTION_UG "setting-mmc-encryption-efl"
#define SETTING_ENCRYPTING_APP    "org.tizen.setting.encrypting"
#define SETTING_LIGHTOFF_APP      "org.tizen.setting.turnofflight"

/* ODE */
#define ODE_ENCRYPT "encrypt"
#define ODE_DECRYPT "decrypt"

/* App to launch a popup on a notification */
#define SYSTEM_SIGNAL_SENDER          "org.tizen.system-signal-sender"
#define SIGNAL_SENDER_TYPE            "_SIGNAL_TYPE_"
#define SIGNAL_SENDER_TYPE_RECOVERY   "recovery-popup"
#define SIGNAL_SENDER_TYPE_USBSTORAGE_UNMOUNT "usbstorage-unmount-popup"
#define SIGNAL_SENDER_TYPE_ENCRYPT    ODE_ENCRYPT
#define SIGNAL_SENDER_TYPE_DECRYPT    ODE_DECRYPT
#define SIGNAL_SENDER_TYPE_ODE_UG     "ode-setting-ug"
#define SIGNAL_SENDER_DEVICE_PATH     "_DEVICE_PATH_"
#define SIGNAL_SENDER_ERROR_TYPE      "_ERROR_TYPE_"
#define SIGNAL_SENDER_MEMORY_SPACE    "_MEMORY_SPACE_"

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

/* Notification parameters */
#define VALUE_DATA_WARNING "data_warning"

/* Ticker noti for USB client */
#define USB_CLIENT_DEFAULT "usb-client-default"
#define USB_CLIENT_SSH     "usb-client-ssh"

/* App launching */
#define CRADLE_APP_NAME  "org.tizen.desk-dock"
#define PWLOCK_APP_NAME  "org.tizen.pwlock"
#define SERVANT_APP_NAME "/usr/bin/system-servant"

enum noti_type {
	DATAUSAGE_WARNING = 0,
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

enum ode_error_type {
	NOT_ENOUGH_SPACE,
	OPERATION_FAILED,
	ODE_ERROR_MAX,
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
DBusMessage *launch_popup_no_param(E_DBus_Object *obj,
				DBusMessage *msg, char *popup_name);
DBusMessage *launch_popup_single_param(E_DBus_Object *obj,
				DBusMessage *msg, char *popup_name);
DBusMessage *launch_popup_double_param(E_DBus_Object *obj,
				DBusMessage *msg, char *popup_name);
DBusMessage *launch_popup_triple_param(E_DBus_Object *obj,
				DBusMessage *msg, char *popup_name);

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

