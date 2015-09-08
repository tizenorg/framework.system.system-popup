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

#include "popup-common.h"

#define DEVICED_BUS_NAME    "org.tizen.system.deviced"
#define DEVICED_OBJECT_PATH "/Org/Tizen/System/DeviceD"
#define DEVICED_IFACE_NAME  DEVICED_BUS_NAME

#define DEVICED_PATH_SYSNOTI      DEVICED_OBJECT_PATH"/SysNoti"
#define DEVICED_INTERFACE_SYSNOTI DEVICED_IFACE_NAME".SysNoti"
#define SIGNAL_COOL_DOWN_RESPONSE "CoolDownResponse"
#define SIGNAL_COOL_DOWN_CHANGED  "CoolDownChanged"
#define COOL_DOWN_RELEASE         "Release"

#define DEVICED_PATH_POWER  DEVICED_OBJECT_PATH"/Power"
#define DEVICED_IFACE_POWER DEVICED_IFACE_NAME".power"
#define METHOD_POWEROFF "Poweroff"


#define TIMEOUT_POWEROFF 10 /* seconds */
#define TIMEOUT_BEEP_40  40 /* seconds */
#define TIMEOUT_BEEP_5   5 /* seconds */
#define TIMEOUT_BEEP_2_5 (2.5) /* seconds */
#define TIMEOUT_BEEP_1   1 /* seconds */
#define TIMEOUT_BEEP_0_5 (0.5) /* seconds */

enum beep_type {
	BEEP_SINGLE = FEEDBACK_PATTERN_LOWBATT, /* should be changed to FEEDBACK_PATTERN_TEMPERATURE_WARNING */
	BEEP_DOUBLE = FEEDBACK_PATTERN_LOWBATT, /* should be changed to double beep */
	BEEP_DONE,
};

static const struct beep_style {
	int type;
	double timeout;
} beep[] = {
	{ BEEP_DOUBLE	,	TIMEOUT_BEEP_5		},
	{ BEEP_DOUBLE	,	TIMEOUT_BEEP_2_5	},
	{ BEEP_DOUBLE	,	TIMEOUT_BEEP_2_5	},
	{ BEEP_SINGLE	,	TIMEOUT_BEEP_1		},
	{ BEEP_SINGLE	,	TIMEOUT_BEEP_1		},
	{ BEEP_SINGLE	,	TIMEOUT_BEEP_1		},
	{ BEEP_SINGLE	,	TIMEOUT_BEEP_1		},
	{ BEEP_SINGLE	,	TIMEOUT_BEEP_1		},
	{ BEEP_DOUBLE	,	TIMEOUT_BEEP_1		},
	{ BEEP_DOUBLE	,	TIMEOUT_BEEP_1		},
	{ BEEP_DOUBLE	,	TIMEOUT_BEEP_1		},
	{ BEEP_DOUBLE	,	TIMEOUT_BEEP_0_5	},
	{ BEEP_DOUBLE	,	TIMEOUT_BEEP_0_5	},
	{ BEEP_DOUBLE	,	TIMEOUT_BEEP_0_5	},
	{ BEEP_DOUBLE	,	TIMEOUT_BEEP_0_5	},
	{ BEEP_DONE		,   0					},
};

static const struct popup_ops cooldown_poweroff_ops;
static const struct popup_ops cooldown_warning_ops;
static const struct popup_ops cooldown_poweron_ops;

static Ecore_Timer *timer = NULL;
static E_DBus_Signal_Handler *release_handler = NULL;

static int device_poweroff(void)
{
	return dbus_method_sync(
			DEVICED_BUS_NAME,
			DEVICED_PATH_POWER,
			DEVICED_IFACE_POWER,
			METHOD_POWEROFF,
			NULL, NULL);
}

static void remove_dbus_signal_handler(void)
{
	E_DBus_Connection *conn;

	if (!release_handler)
		return;
	conn = get_dbus_connection();
	if (!conn) {
		release_handler = NULL;
		return;
	}

	e_dbus_signal_handler_del(conn, release_handler);
	release_handler = NULL;
}

static void cooldown_changed(void *data, DBusMessage *msg)
{
	DBusError err;
	char *state;
	const struct popup_ops *ops = data;

	if (dbus_message_is_signal(msg, DEVICED_INTERFACE_SYSNOTI, SIGNAL_COOL_DOWN_CHANGED) == 0)
		return;

	dbus_error_init(&err);
	if (dbus_message_get_args(msg, &err,
				DBUS_TYPE_STRING, &state,
				DBUS_TYPE_INVALID) == 0) {
		dbus_error_free(&err);
		return;
	}
	dbus_error_free(&err);

	if (strncmp(state, COOL_DOWN_RELEASE, strlen(state)))
		return;

	remove_dbus_signal_handler();

	if (timer) {
		ecore_timer_del(timer);
		timer = NULL;
	}

	unload_simple_popup(ops);

	terminate_if_no_popup();
}

static int add_dbus_signal_handler(const struct popup_ops *ops)
{
	E_DBus_Connection *conn;

	conn = get_dbus_connection();
	if (!conn) {
		_E("Failed to get dbus connection");
		return -ENOMEM;
	}

	release_handler = e_dbus_signal_handler_add(
			conn, NULL,
			DEVICED_PATH_SYSNOTI,
			DEVICED_INTERFACE_SYSNOTI,
			SIGNAL_COOL_DOWN_CHANGED,
			cooldown_changed,
			(void *)ops);
	if (!release_handler) {
		_E("Failed to add signal handler");
		return -ENOMEM;
	}

	return 0;
}

static void remove_other_popups(const struct popup_ops *ops)
{
	if (ops != &cooldown_poweroff_ops)
		unload_simple_popup(&cooldown_poweroff_ops);

	if (ops != &cooldown_warning_ops) {
		remove_dbus_signal_handler();
		unload_simple_popup(&cooldown_warning_ops);
	}

	if (ops != &cooldown_poweron_ops)
		unload_simple_popup(&cooldown_poweron_ops);
}

static void cooldown_send_warning_signal(const struct popup_ops *ops)
{
	int ret;
	ret = broadcast_dbus_signal(
			DEVICED_PATH_SYSNOTI,
			DEVICED_INTERFACE_SYSNOTI,
			SIGNAL_COOL_DOWN_RESPONSE,
			NULL, NULL);
	if (ret < 0)
		_E("Failed to broadcast signal(%d)", ret);
}

static void cooldown_poweroff(const struct popup_ops *ops)
{
	int ret;

	_I("Poweroff is selected");

	unload_simple_popup(ops);

	ret = device_poweroff();
	if (ret < 0)
		_E("Failed to request poweroff to deviced(%d)", ret);

	terminate_if_no_popup();
}

static void cooldown_warning(const struct popup_ops *ops)
{
	_I("Warning is selected");

	remove_dbus_signal_handler();

	unload_simple_popup(ops);
	cooldown_send_warning_signal(ops);
	popup_terminate();
}

static Eina_Bool beep_time_expired(void *data)
{
	const struct popup_ops *ops = data;
	static int index = 0;

	if (timer)
		ecore_timer_del(timer);

	if (beep[index].type == BEEP_DONE) {
		cooldown_warning(ops);
		return ECORE_CALLBACK_CANCEL;
	}

	timer = ecore_timer_add(beep[index].timeout, beep_time_expired, ops);
	if (!timer)
		_E("Failed to add timer");

	notify_feedback(beep[index].type);

	index++;

	return ECORE_CALLBACK_CANCEL;
}

static void cooldown_warning_timer(const struct popup_ops *ops)
{
	int ret;

	remove_other_popups(ops);

	if (timer)
		ecore_timer_del(timer);
	timer = ecore_timer_add(TIMEOUT_BEEP_40, beep_time_expired, ops);
	if (!timer)
		_E("Failed to add timer");

	ret = add_dbus_signal_handler(ops);
	if (ret < 0)
		_E("Failed to add dbus handler(%d)", ret);

}

static Eina_Bool poweroff_time_expired(void *data)
{
	const struct popup_ops *ops = data;

	cooldown_poweroff(ops);

	return ECORE_CALLBACK_CANCEL;
}

static void cooldown_poweroff_timer(const struct popup_ops *ops)
{
	remove_other_popups(ops);

	if (timer)
		ecore_timer_del(timer);
	timer = ecore_timer_add(TIMEOUT_POWEROFF, poweroff_time_expired, ops);
	if (!timer)
		_E("Failed to add timer");
}

static int cooldown_poweroff_content(const struct popup_ops *ops, char *content, unsigned int len)
{
	snprintf(content, len, "The Gear will power off to cool down.");
	return 0;
}

static int cooldown_warning_content(const struct popup_ops *ops, char *content, unsigned int len)
{
	snprintf(content, len, "Gear is overheating. This task will end shortly. You will only be able to make emergency calls until it has cooled down.");
	return 0;
}

static const struct popup_ops cooldown_poweroff_ops = {
	.name		= "cooldown_poweroff",
	.show_popup	= load_simple_popup,
//	.content	= "Your device is overheating. It will now power off to cool down.",
	.get_content = cooldown_poweroff_content,
	.left_text	= "IDS_COM_SK_OK",
	.left_icon	= "circle-ok.png",
	.left		= cooldown_poweroff,
	.launch		= cooldown_poweroff_timer,
	.terminate	= cooldown_poweroff,
	.flags		= SCROLLABLE,
};

static const struct popup_ops cooldown_poweron_ops = {
	.name		= "cooldown_poweron",
	.show_popup	= load_simple_popup,
	.content	= "Your device overheated. It powered off to prevent damage.",
	.left_text	= "IDS_COM_SK_OK",
	.left_icon	= "circle-ok.png",
	.flags		= SCROLLABLE,
};

static const struct popup_ops cooldown_warning_ops = {
	.name		= "cooldown_warning",
	.show_popup	= load_simple_popup,
//	.content	= "Your device is overheating. It will be changed to Clock mode.",
	.get_content = cooldown_warning_content,
	.left_text	= "IDS_COM_SK_OK",
	.left_icon	= "circle-ok.png",
	.left		= cooldown_warning,
	.launch		= cooldown_warning_timer,
	.terminate	= cooldown_warning,
	.flags		= SCROLLABLE,
};

/* Constructor to register cooldown button */
static __attribute__ ((constructor)) void cooldown_register_popup(void)
{
	register_popup(&cooldown_poweron_ops);
	register_popup(&cooldown_poweroff_ops);
	register_popup(&cooldown_warning_ops);
}
