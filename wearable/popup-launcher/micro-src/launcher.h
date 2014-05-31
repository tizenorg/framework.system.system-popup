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
#include "micro-common.h"

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

#define POPUP_PATH_CRASH      POPUP_DBUS_PATH"/Crash"
#define POPUP_IFACE_CRASH     BUS_NAME".Crash"

#define POPUP_PATH_BATTERY    POPUP_DBUS_PATH"/Battery"
#define POPUP_IFACE_BATTERY   BUS_NAME".Battery"

/* Popup names */
#define POWEROFF_SYSPOPUP  "poweroff-syspopup"
#define LOWBAT_SYSPOPUP    "lowbat-syspopup"
#define LOWMEM_SYSPOPUP    "lowmem-syspopup"
#define CRASH_SYSPOPUP     "crash-popup"

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

#endif /* __LAUNCHER_H__ */

