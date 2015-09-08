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
#include "dd-deviced.h"

enum flightmode_state {
	FLIGHTMODE_ENABLE  = 0,
	FLIGHTMODE_DISABLE = 1,
};

static const struct popup_ops flightmode_enable_ops;
static const struct popup_ops flightmode_disable_ops;

static void flightmode_ok(const struct popup_ops *ops)
{
	int mode = -1;

	_I("OK is selected");

	unload_simple_popup(ops);

	if (ops == &flightmode_enable_ops)
		mode = FLIGHTMODE_ENABLE;
	else if (ops == &flightmode_disable_ops)
		mode = FLIGHTMODE_DISABLE;

	if (deviced_change_flightmode(mode) < 0)
		_E("Failed to send flightmode signal to deviced");

	terminate_if_no_popup();
}

static const struct popup_ops flightmode_enable_ops = {
	.name		= "flightmode_enable",
	.show_popup	= load_simple_popup,
	.title		= "IDS_PN_OPT_ENABLE_FLIGHT_MODE_ABB",
	.content	= "IDS_ST_BODY_FLIGHT_MODE_DISABLES_CALLING_AND_MESSAGING_FUNCTIONS_MSG",
	.left_text	= "IDS_COM_SK_CANCEL",
	.left_icon	= "circle-cancel.png",
	.right		= flightmode_ok,
	.right_text	= "IDS_COM_SK_OK",
	.right_icon	= "circle-ok.png",
	.flags		= SCROLLABLE,
};

static const struct popup_ops flightmode_disable_ops = {
	.name		= "flightmode_disable",
	.show_popup	= load_simple_popup,
	.title		= "IDS_PN_OPT_DISABLE_FLIGHT_MODE_ABB",
	.content	= "IDS_COM_POP_FLIGHT_MODE_WILL_BE_DISABLED",
	.left_text	= "IDS_COM_SK_CANCEL",
	.left_icon	= "circle-cancel.png",
	.right		= flightmode_ok,
	.right_text	= "IDS_COM_SK_OK",
	.right_icon	= "circle-ok.png",
	.flags		= SCROLLABLE,
};

/* Constructor to register flightmode button */
static __attribute__ ((constructor)) void flightmode_register_popup(void)
{
	register_popup(&flightmode_enable_ops);
	register_popup(&flightmode_disable_ops);
}
