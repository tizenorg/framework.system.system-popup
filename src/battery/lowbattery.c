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

static bool lowbattery_check_skip_status(const struct popup_ops *ops)
{
	int ret, val;

	ret = vconf_get_int(VCONFKEY_SYSMAN_CHARGER_STATUS, &val);
	if (ret == 0 && val == VCONFKEY_SYSMAN_CHARGER_CONNECTED)
		return true;

	return false;
}

static void charger_status_changed(keynode_t *key, void *data)
{
	int status;
	const struct popup_ops *ops = data;

	status = vconf_keynode_get_int(key);
	if (status == VCONFKEY_SYSMAN_CHARGER_CONNECTED) {
		unload_simple_popup(ops);
		terminate_if_no_popup();
	}
}

static void battery_status_changed(keynode_t *key, void *data)
{
	int status;
	const struct popup_ops *ops = data;

	status = vconf_keynode_get_int(key);
	_I("BATTERY status: %d", status);
	if (status == VCONFKEY_SYSMAN_BAT_NORMAL ||
		status == VCONFKEY_SYSMAN_BAT_FULL) {
		unload_simple_popup(ops);
		terminate_if_no_popup();
	}
}

static void lowbattery_add_handler(const struct popup_ops *ops)
{
	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
				charger_status_changed, (void *)ops) < 0)
		_E("Falied to add vconf key handler");

	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW,
				battery_status_changed, (void *)ops) < 0)
		_E("Falied to add vconf key handler");
}

static void lowbattery_remove_handler(const struct popup_ops *ops)
{
	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
				charger_status_changed) < 0)
		_E("Falied to release vconf key handler");

	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW,
				battery_status_changed) < 0)
		_E("Falied to release vconf key handler");
}

static const struct popup_ops lowbattery_warning_ops = {
	.name		= "lowbattery_warning",
	.show_popup	= load_simple_popup,
	.skip		= lowbattery_check_skip_status,
	.launch		= lowbattery_add_handler,
	.terminate	= lowbattery_remove_handler,
	.content	= "IDS_ST_POP_THE_BATTERY_POWER_IS_LOW_RECHARGE_YOUR_GEAR_SOON_TO_KEEP_USING_IT",
	.left_text	= "IDS_COM_SK_OK",
	.left_icon	= "circle-ok.png",
	.flags		= SCROLLABLE,
};

static const struct popup_ops lowbattery_critical_ops = {
	.name		= "lowbattery_critical",
	.show_popup	= load_simple_popup,
	.skip		= lowbattery_check_skip_status,
	.launch		= lowbattery_add_handler,
	.terminate	= lowbattery_remove_handler,
	.content	= "IDS_ST_POP_THE_BATTERY_POWER_IS_CRITICALLY_LOW_RECHARGE_YOUR_GEAR_NOW_TO_KEEP_USING_IT",
	.left_text	= "IDS_COM_SK_OK",
	.left_icon	= "circle-ok.png",
	.flags		= SCROLLABLE,
};

/* Constructor to register lowbattery button */
static __attribute__ ((constructor)) void lowbattery_register_popup(void)
{
	int val;

	if (vconf_get_int(VCONFKEY_TESTMODE_LOW_BATT_POPUP, &val) == 0
			&& val == 1)
		return;

	register_popup(&lowbattery_warning_ops);
	register_popup(&lowbattery_critical_ops);
}
