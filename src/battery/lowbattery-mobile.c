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

#define DEVICED_PATH_SYSNOTI        "/Org/Tizen/System/DeviceD/SysNoti"
#define DEVICED_INTERFACE_SYSNOTI   "org.tizen.system.deviced.SysNoti"
#define SIGNAL_CHARGEERR_RESPONSE   "ChargeErrResponse"

static const struct popup_ops lowbattery_warning_ops;
static const struct popup_ops lowbattery_critical_ops;
static const struct popup_ops lowbattery_poweroff_ops;
static const struct popup_ops charge_error_low_ops;
static const struct popup_ops charge_error_high_ops;
static const struct popup_ops battery_disconnected_ops;

static void remove_other_lowbattery_popups(const struct popup_ops *ops)
{
	if (ops != &lowbattery_warning_ops)
		unload_simple_popup(&lowbattery_warning_ops);

	if (ops != &lowbattery_critical_ops)
		unload_simple_popup(&lowbattery_critical_ops);

	if (ops != &lowbattery_poweroff_ops)
		unload_simple_popup(&lowbattery_poweroff_ops);
}

static void remove_other_charge_popups(const struct popup_ops *ops)
{
	if (ops != &charge_error_low_ops)
		unload_simple_popup(&charge_error_low_ops);

	if (ops != &charge_error_high_ops)
		unload_simple_popup(&charge_error_high_ops);

	if (ops != &battery_disconnected_ops)
		unload_simple_popup(&battery_disconnected_ops);
}

static void charger_status_changed(keynode_t *key, void *data)
{
	int status;
	const struct popup_ops *ops = data;

	status = vconf_keynode_get_int(key);
	if (status != VCONFKEY_SYSMAN_CHARGER_CONNECTED)
		return;

	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
				charger_status_changed) < 0)
		_E("Failed to release vconf key handler");

	unload_simple_popup(ops);

	terminate_if_no_popup();
}

static void unregister_charger_status_handler(void)
{
	vconf_ignore_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
				charger_status_changed);
}

static void register_charger_status_handler(const struct popup_ops *ops)
{
	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
				charger_status_changed, (void *)ops) < 0)
		_E("Failed to register vconf key handler");
}

static void lowbattery_launch(const struct popup_ops *ops)
{
	unregister_charger_status_handler();
	remove_other_lowbattery_popups(ops);
	register_charger_status_handler(ops);
}

static void lowbattery_terminate(const struct popup_ops *ops)
{
	unregister_charger_status_handler();
}

static void poweroff_clicked(const struct popup_ops *ops)
{
	_I("Power off the phone");
	unload_simple_popup(ops);

	if (vconf_set_int(VCONFKEY_SYSMAN_POWER_OFF_STATUS,
				VCONFKEY_SYSMAN_POWER_OFF_DIRECT) != 0)
		_E("Failed to request poweroff to deviced");

	terminate_if_no_popup();
}

static void charge_error_ok_clicked(const struct popup_ops *ops)
{
	_I("OK is selected");
	unload_simple_popup(ops);

	if (broadcast_dbus_signal(DEVICED_PATH_SYSNOTI,
				DEVICED_INTERFACE_SYSNOTI,
				SIGNAL_CHARGEERR_RESPONSE,
				NULL, NULL) < 0)
		_E("Failed to send signal");

	terminate_if_no_popup();
}

static int remove_battery_popups(bundle *b, const struct popup_ops *ops)
{
	_I("Remove battery related popups");
	unload_simple_popup(&lowbattery_critical_ops);
	unload_simple_popup(&lowbattery_warning_ops);
	unload_simple_popup(&lowbattery_poweroff_ops);
	unload_simple_popup(&charge_error_low_ops);
	unload_simple_popup(&charge_error_high_ops);
	unload_simple_popup(&battery_disconnected_ops);
	terminate_if_no_popup();
	return 0;
}

static const struct popup_ops lowbattery_warning_ops = {
	.name		= "lowbattery_warning",
	.show_popup	= load_simple_popup,
	.content	= "IDS_COM_POP_BATTERYLOW",
	.left_text	= "IDS_COM_SK_OK",
	.launch		= lowbattery_launch,
	.terminate	= lowbattery_terminate,
	.flags		= SCROLLABLE,
};

static const struct popup_ops lowbattery_critical_ops = {
	.name		= "lowbattery_critical",
	.show_popup	= load_simple_popup,
	.content	= "IDS_COM_POP_BATTERYLOW",
	.left_text	= "IDS_COM_SK_OK",
	.launch		= lowbattery_launch,
	.terminate	= lowbattery_terminate,
	.flags		= SCROLLABLE,
};

static const struct popup_ops lowbattery_poweroff_ops = {
	.name		= "lowbattery_poweroff",
	.show_popup	= load_simple_popup,
	.content	= "IDS_COM_POP_LOW_BATTERY_PHONE_WILL_SHUT_DOWN",
	.left_text	= "IDS_COM_SK_OK",
	.left		= poweroff_clicked,
	.launch		= lowbattery_launch,
	.terminate  = lowbattery_terminate,
	.flags		= SCROLLABLE,
};

static const struct popup_ops battery_remove_ops = {
	.name		= "remove_battery_popups",
	.show_popup	= remove_battery_popups,
};

static const struct popup_ops charge_error_low_ops = {
	.name		= "chargeerrlow",//"charge_error_low",
	.show_popup	= load_simple_popup,
	.content	= "IDS_QP_BODY_CHARGING_PAUSED_BATTERY_TEMPERATURE_TOO_LOW",
	.left_text	= "IDS_COM_SK_OK",
	.left		= charge_error_ok_clicked,
	.launch		= remove_other_charge_popups,
	.flags		= SCROLLABLE,
};

static const struct popup_ops charge_error_high_ops = {
	.name		= "chargeerrhigh",//"charge_error_high",
	.show_popup	= load_simple_popup,
	.content	= "IDS_QP_BODY_CHARGING_PAUSED_BATTERY_TEMPERATURE_TOO_HIGH",
	.left_text	= "IDS_COM_SK_OK",
	.left		= charge_error_ok_clicked,
	.launch		= remove_other_charge_popups,
	.flags		= SCROLLABLE,
};

static const struct popup_ops battery_disconnected_ops = {
	.name		= "battdisconnect",//"battery_disconnected",
	.show_popup	= load_simple_popup,
	.content	= "IDS_COM_POP_BATTERY_DISCONNECTED_ABB",
	.left_text	= "IDS_COM_SK_OK",
	.flags		= SCROLLABLE,
};

/* Constructor to register lowbattery button */
static __attribute__ ((constructor)) void lowbattery_register_popup(void)
{
	register_popup(&lowbattery_warning_ops);
	register_popup(&lowbattery_critical_ops);
	register_popup(&lowbattery_poweroff_ops);
	register_popup(&charge_error_low_ops);
	register_popup(&charge_error_high_ops);
	register_popup(&battery_disconnected_ops);
}
