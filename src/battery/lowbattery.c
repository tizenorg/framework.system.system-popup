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
#include <appsvc.h>

#define UPS_APPNAME       "org.tizen.clocksetting.powersaving"
#define UPS_PARAM_KEY     "viewtype"
#define UPS_PARAM_ENABLE  "enable"
#define UPS_PARAM_DISABLE "disable"

static int launch_ups_app(char *enable)
{
	int ret;
	bundle *b;

	if (!enable)
		return -EINVAL;

	b = bundle_create();
	if (!b) {
		_E("Failed to make bundle");
		return -ENOMEM;
	}

	appsvc_set_operation(b, APPSVC_OPERATION_VIEW);
	appsvc_add_data(b, UPS_PARAM_KEY, enable);
	appsvc_set_pkgname(b, UPS_APPNAME);

	ret = appsvc_run_service(b, 0, NULL, NULL);
	if (ret < 0)
		_E("Failed to launch power saving app(%d)", ret);
	bundle_free(b);

	return ret;
}

static void lowbattery_powersaving(const struct popup_ops *ops)
{
	int ret;

	_I("Power saving is selected");

	unload_simple_popup(ops);

	ret = launch_ups_app(UPS_PARAM_ENABLE);
	if (ret < 0)
		_E("Failed to launch power saving app(%d)", ret);

	terminate_if_no_popup();
}

static const struct popup_ops lowbattery_warning_ops = {
	.name		= "lowbattery_warning",
	.show_popup	= load_simple_popup,
	.content	= "IDS_ST_POP_THE_BATTERY_POWER_IS_LOW_RECHARGE_YOUR_GEAR_SOON_TO_KEEP_USING_IT",
	.left		= lowbattery_powersaving,
	.left_text	= "IDS_ST_BUTTON2_POWER_SAVING_ABB",
	.right_text	= "IDS_COM_SK_OK",
	.flags		= SCROLLABLE,
};

static const struct popup_ops lowbattery_warning_ups_ops = {
	.name		= "lowbattery_warning",
	.show_popup	= load_simple_popup,
	.content	= "IDS_ST_POP_THE_BATTERY_POWER_IS_LOW_RECHARGE_YOUR_GEAR_SOON_TO_KEEP_USING_IT",
	.left_text	= "IDS_COM_SK_OK",
	.flags		= SCROLLABLE,
};

static const struct popup_ops lowbattery_critical_ops = {
	.name		= "lowbattery_critical",
	.show_popup	= load_simple_popup,
	.content	= "IDS_ST_POP_THE_BATTERY_POWER_IS_CRITICALLY_LOW_RECHARGE_YOUR_GEAR_NOW_TO_KEEP_USING_IT",
	.left		= lowbattery_powersaving,
	.left_text	= "IDS_ST_BUTTON2_POWER_SAVING_ABB",
	.right_text	= "IDS_COM_SK_OK",
	.flags		= SCROLLABLE,
};

static const struct popup_ops lowbattery_critical_ups_ops = {
	.name		= "lowbattery_critical",
	.show_popup	= load_simple_popup,
	.content	= "IDS_ST_POP_THE_BATTERY_POWER_IS_CRITICALLY_LOW_RECHARGE_YOUR_GEAR_NOW_TO_KEEP_USING_IT",
	.left_text	= "IDS_COM_SK_CANCEL",
	.flags		= SCROLLABLE,
};

/* Constructor to register lowbattery button */
static __attribute__ ((constructor)) void lowbattery_register_popup(void)
{
	int val;

	if (vconf_get_int(VCONFKEY_TESTMODE_LOW_BATT_POPUP, &val) == 0
			&& val == 1)
		return;

	if (power_saving_mode()) {
		register_popup(&lowbattery_warning_ups_ops);
		register_popup(&lowbattery_critical_ups_ops);
		return;
	}

	register_popup(&lowbattery_warning_ops);
	register_popup(&lowbattery_critical_ops);
}
