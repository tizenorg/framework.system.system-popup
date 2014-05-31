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

#include "launcher.h"

#define TERMINATE_TIMEOUT  5

static E_DBus_Connection *edbus_conn;
static DBusPendingCall *edbus_request_name;
static Ecore_Timer *term_timer = NULL;

static Eina_Bool exit_idler_cb(void *data)
{
	e_dbus_connection_close(edbus_conn);
	e_dbus_shutdown();

	ecore_main_loop_quit();
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool terminate_launcher(void *data)
{
	if (term_timer)
		ecore_timer_del(term_timer);

	if (ecore_idler_add(exit_idler_cb, NULL))
		return ECORE_CALLBACK_CANCEL;

	exit_idler_cb(NULL);
	return ECORE_CALLBACK_CANCEL;
}

static void set_timer_to_terminate(void)
{
	if (term_timer)
		ecore_timer_reset(term_timer);
	else {
		term_timer = ecore_timer_add(TERMINATE_TIMEOUT, terminate_launcher, NULL);
		if (!term_timer)
			terminate_launcher(NULL);
	}
}

/* Poweroff popup */
static DBusMessage *edbus_poweroff_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_no_param(obj, msg, POWEROFF_SYSPOPUP);
}

static const struct edbus_method
edbus_poweroff_methods[] = {
	{ "PopupLaunch" ,   NULL,  "i", edbus_poweroff_popup      },
	/* Add methods here */
};

/* Lowbat popup */
static DBusMessage *edbus_lowbat_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, LOWBAT_SYSPOPUP);
}

static const struct edbus_method
edbus_lowbat_methods[] = {
	{ "PopupLaunch" ,   "ss",  "i", edbus_lowbat_popup      },
	/* Add methods here */
};

/* Lowmem popup */
static DBusMessage *edbus_lowmem_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, LOWMEM_SYSPOPUP);
}

static const struct edbus_method
edbus_lowmem_methods[] = {
	{ "PopupLaunch" ,   "ss",  "i", edbus_lowmem_popup      },
	/* Add methods here */
};

/* Crash popup */
static DBusMessage *edbus_crash_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_double_param(obj, msg, CRASH_SYSPOPUP);
}

static const struct edbus_method
edbus_crash_methods[] = {
	{ "PopupLaunch"  ,     "ssss",  "i", edbus_crash_popup    },
	/* Add methods here */
};

/* Battery notifications */
static DBusMessage *play_notification(DBusMessage *msg, int pattern)
{
	DBusMessageIter iter;
	DBusMessage *reply;
	int ret = 0;

	play_feedback(FEEDBACK_TYPE_NONE, pattern);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);
	return reply;
}

static DBusMessage *edbus_battery_full_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return play_notification(msg, FEEDBACK_PATTERN_FULLCHARGED);
}

static DBusMessage *edbus_battery_charge_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return play_notification(msg, FEEDBACK_PATTERN_CHARGERCONN);
}

static const struct edbus_method
edbus_battery_methods[] = {
	{ "BatteryFullNotiOn"   ,   NULL,  "i", edbus_battery_full_noti_on         },
	{ "BatteryChargeNotiOn" ,   NULL,  "i", edbus_battery_charge_noti_on       },
	/* Add methods here */
};

static struct edbus_object
edbus_objects[]= {
	{ POPUP_PATH_POWEROFF    , POPUP_IFACE_POWEROFF    , NULL, NULL,
		edbus_poweroff_methods   , ARRAY_SIZE(edbus_poweroff_methods)  },
	{ POPUP_PATH_LOWBAT      , POPUP_IFACE_LOWBAT      , NULL, NULL,
		edbus_lowbat_methods     , ARRAY_SIZE(edbus_lowbat_methods)    },
	{ POPUP_PATH_LOWMEM      , POPUP_IFACE_LOWMEM      , NULL, NULL,
		edbus_lowmem_methods     , ARRAY_SIZE(edbus_lowmem_methods)    },
	{ POPUP_PATH_CRASH       , POPUP_IFACE_CRASH       , NULL, NULL,
		edbus_crash_methods      , ARRAY_SIZE(edbus_crash_methods)     },
	{ POPUP_PATH_BATTERY     , POPUP_IFACE_BATTERY     , NULL, NULL,
		edbus_battery_methods    , ARRAY_SIZE(edbus_battery_methods)   },
	/* Add new object & interface here*/
};

static int init_methods(void)
{
	int ret;
	int i, j;

	for (i = 0; i < ARRAY_SIZE(edbus_objects); i++) {
		for (j = 0; j < edbus_objects[i].methods_len; j++) {
			ret = e_dbus_interface_method_add(edbus_objects[i].iface,
					edbus_objects[i].methods[j].member,
					edbus_objects[i].methods[j].signature,
					edbus_objects[i].methods[j].reply_signature,
					edbus_objects[i].methods[j].func);
			if (!ret) {
				_E("fail to add method %s!", edbus_objects[i].methods[j].member);
				return -ECONNREFUSED;
			}
		}
	}
	return 0;
}

static int register_dbus(void)
{
	DBusError error;
	int retry, ret, i;

	dbus_error_init(&error);

	retry = 0;
	do {
		if (e_dbus_init())
			break;
		if (++retry == RETRY_MAX) {
			_E("fail to init edbus");
			return -ECONNREFUSED;
		}
	} while (retry <= RETRY_MAX);

	retry = 0;
	do {
		edbus_conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
		if (edbus_conn)
			break;
		if (++retry == RETRY_MAX) {
			_E("fail to get edbus");
			ret = -ECONNREFUSED;
			goto out1;
		}
	} while (retry <= RETRY_MAX);

	retry = 0;
	do {
		edbus_request_name = e_dbus_request_name(edbus_conn, BUS_NAME, 0, NULL, NULL);
		if (edbus_request_name)
			break;
		if (++retry == RETRY_MAX) {
			_E("fail to request edbus name");
			ret = -ECONNREFUSED;
			goto out2;
		}
	} while (retry <= RETRY_MAX);

	for (i = 0; i < ARRAY_SIZE(edbus_objects); i++) {
		edbus_objects[i].obj = e_dbus_object_add(edbus_conn, edbus_objects[i].path, NULL);
		if (!(edbus_objects[i].obj)) {
			_E("fail to add edbus obj");
			ret = -ECONNREFUSED;
			goto out2;
		}

		edbus_objects[i].iface = e_dbus_interface_new(edbus_objects[i].interface);
		if (!(edbus_objects[i].iface)) {
			_E("fail to add edbus interface");
			ret = -ECONNREFUSED;
			goto out2;
		}

		e_dbus_object_interface_attach(edbus_objects[i].obj, edbus_objects[i].iface);
	}

	return 0;

out2:
	e_dbus_connection_close(edbus_conn);
out1:
	e_dbus_shutdown();

	return ret;
}

int main (int argc, char *argv[])
{
	int ret;

	ecore_init();

	ret = register_dbus();
	if (ret < 0)
		return ret;

	ret= init_methods();
	if (ret < 0)
		return ret;

	ecore_main_loop_begin();
	ecore_shutdown();
	return 0;
}
