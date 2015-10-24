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

#ifndef __MACRO_H__
#define __MACRO_H__

#include <dlog.h>

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

#define RETRY_MAX 10
#define SLEEP_USEC 200000

/* DBus paths, interfaces */
#define BUS_NAME				"org.tizen.system.popup"
#define POPUP_DBUS_PATH			"/Org/Tizen/System/Popup"
#define POPUP_DBUS_IFACE		BUS_NAME

#define POPUP_PATH_NOTI			POPUP_DBUS_PATH"/Noti"
#define POPUP_IFACE_NOTI		BUS_NAME".Noti"

#define POPUP_PATH_SYSTEM		POPUP_DBUS_PATH"/System"
#define POPUP_IFACE_SYSTEM		BUS_NAME".System"

#define POPUP_PATH_POWEROFF		POPUP_DBUS_PATH"/Poweroff"
#define POPUP_IFACE_POWEROFF	BUS_NAME".Poweroff"

#define POPUP_PATH_CRASH		POPUP_DBUS_PATH"/Crash"
#define POPUP_IFACE_CRASH		BUS_NAME".Crash"

#define POPUP_PATH_APP			POPUP_DBUS_PATH"/Apps"
#define POPUP_IFACE_APP			BUS_NAME".Apps"

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

/* ODE */
#define ODE_ENCRYPT "encrypt"
#define ODE_DECRYPT "decrypt"

#endif /* __MACRO_H__ */

