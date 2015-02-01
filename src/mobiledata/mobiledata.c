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

#define VCONFKEY_MOBILEDATA_ON_CHECK  "db/private/mobiledata/on_popup/check"
#define VCONFKEY_MOBILEDATA_OFF_CHECK "db/private/mobiledata/off_popup/check"

enum mobiledata_state {
	MOBILEDATA_DISABLE = 0,
	MOBILEDATA_ENABLE  = 1,
};

static const struct popup_ops mobiledata_enable_ops;
static const struct popup_ops mobiledata_disable_ops;

static void update_check_vconf_state(int mode)
{
	char *name;

	switch (mode) {
	case MOBILEDATA_ENABLE:
		name = VCONFKEY_MOBILEDATA_ON_CHECK;
		break;
	case MOBILEDATA_DISABLE:
		name = VCONFKEY_MOBILEDATA_OFF_CHECK;
		break;
	default:
		return;
	}

	if (vconf_set_bool(name, 1) != 0)
		_E("Failed to set vconf value (%s)", name);
}

static int mobiledata_update(int state)
{
	return vconf_set_bool(VCONFKEY_3G_ENABLE, state);
}

static void mobiledata_ok(const struct popup_ops *ops)
{
	int mode = -1;

	_I("OK is selected");

	if (ops == &mobiledata_enable_ops)
		mode = MOBILEDATA_ENABLE;
	else if (ops == &mobiledata_disable_ops)
		mode = MOBILEDATA_DISABLE;

	if (get_check_state(ops))
		update_check_vconf_state(mode);

	unload_simple_popup(ops);

	if (mobiledata_update(mode) != 0)
		_E("Failed to update mobile data state");

	terminate_if_no_popup();
}

static const struct popup_ops mobiledata_enable_ops = {
	.name		= "mobiledata_enable",
	.show_popup	= load_simple_popup,
	.title		= "IDS_QP_HEADER_TURN_ON_MOBILE_DATA_ABB",
	.content	= "IDS_PN_POP_YOU_WILL_BE_CONNECTED_TO_YOUR_MOBILE_NETWORK_THIS_MAY_RESULT_IN_ADDITIONAL_CHARGES",
	.left_text	= "IDS_COM_SK_CANCEL",
	.right		= mobiledata_ok,
	.right_text	= "IDS_COM_SK_OK",
	.check_text	= "IDS_CAM_OPT_DONT_REPEAT_ABB",
	.flags		= SCROLLABLE | CHECK_BOX,
};

static const struct popup_ops mobiledata_disable_ops = {
	.name		= "mobiledata_disable",
	.show_popup	= load_simple_popup,
	.title		= "IDS_QP_HEADER_TURN_OFF_MOBILE_DATA_ABB",
	.content	= "IDS_PN_POP_IF_YOU_TURN_OFF_MOBILE_DATA_YOU_WILL_NOT_BE_ABLE_TO_RECEIVE_NOTIFICATIONS_OR_USE_APPLICATIONS_THAT_REQUIRE_A_MOBILE_NETWORK_CONNECTION_MSG",
	.left_text	= "IDS_COM_SK_CANCEL",
	.right		= mobiledata_ok,
	.right_text	= "IDS_COM_SK_OK",
	.check_text	= "IDS_CAM_OPT_DONT_REPEAT_ABB",
	.flags		= SCROLLABLE | CHECK_BOX,
};

/* Constructor to register mobiledata button */
static __attribute__ ((constructor)) void mobiledata_register_popup(void)
{
	register_popup(&mobiledata_enable_ops);
	register_popup(&mobiledata_disable_ops);
}
