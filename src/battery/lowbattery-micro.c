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

static const struct popup_ops lowbattery_warning_ops;
static const struct popup_ops lowbattery_critical_ops;

static void remove_other_lowbattery_popups(const struct popup_ops *ops)
{
	if (ops != &lowbattery_warning_ops)
		unload_simple_popup(&lowbattery_warning_ops);

	if (ops != &lowbattery_critical_ops)
		unload_simple_popup(&lowbattery_critical_ops);
}

static const struct popup_ops lowbattery_warning_ops = {
	.name		= "lowbattery_warning",
	.show_popup	= load_simple_popup,
	.content	= "IDS_ST_POP_THE_BATTERY_POWER_IS_LOW_RECHARGE_YOUR_GEAR_SOON_TO_KEEP_USING_IT",
	.left_text	= "IDS_COM_SK_OK",
	.launch		= remove_other_lowbattery_popups,
	.flags		= SCROLLABLE,
};

static const struct popup_ops lowbattery_critical_ops = {
	.name		= "lowbattery_critical",
	.show_popup	= load_simple_popup,
	.content	= "IDS_ST_POP_THE_BATTERY_POWER_IS_CRITICALLY_LOW_RECHARGE_YOUR_GEAR_NOW_TO_KEEP_USING_IT",
	.left_text	= "IDS_COM_SK_OK",
	.launch		= remove_other_lowbattery_popups,
	.flags		= SCROLLABLE,
};

/* Constructor to register lowbattery button */
static __attribute__ ((constructor)) void lowbattery_register_popup(void)
{
	register_popup(&lowbattery_warning_ops);
	register_popup(&lowbattery_critical_ops);
}
