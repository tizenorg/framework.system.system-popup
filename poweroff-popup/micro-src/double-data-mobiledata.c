/*
 *  system-popup
 *
 * Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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

#include "double-data.h"

#define MOBILEDATA_CONTENT   "IDS_ST_MBODY_MOBILE_DATA_ABB"

#define TOAST_TIMEOUT 3

#define VCONFKEY_SAP_TYPE    "memory/private/sap/conn_type"
#define SAP_MOBILE           0x10
#define SAP_BT               0x01

#define VCONFKEY_MOBILEDATA_ON_CHECK  "db/private/mobiledata/on_popup/check"
#define VCONFKEY_MOBILEDATA_OFF_CHECK "db/private/mobiledata/off_popup/check"

enum current_state {
	MOBILEDATA_OFF,
	MOBILEDATA_ON,
	MOBILEDATA_BT,
	MOBILEDATA_NOSIM,
	MOBILEDATA_NOTSUP,
};

static const char *item_icon[] = {
	"micro-mobiledata-off.png",
	"micro-mobiledata-on.png",
	"micro-mobiledata-disabled.png",
};

static const char *subpopup_title[] = {
	"IDS_QP_HEADER_TURN_ON_MOBILE_DATA_ABB",
	"IDS_QP_HEADER_TURN_OFF_MOBILE_DATA_ABB",
};

static const char *subpopup_text[] = {
	"IDS_PN_POP_YOU_WILL_BE_CONNECTED_TO_YOUR_MOBILE_NETWORK_THIS_MAY_RESULT_IN_ADDITIONAL_CHARGES",
	"IDS_PN_POP_IF_YOU_TURN_OFF_MOBILE_DATA_YOU_WILL_NOT_BE_ABLE_TO_RECEIVE_NOTIFICATIONS_OR_USE_APPLICATIONS_THAT_REQUIRE_A_MOBILE_NETWORK_CONNECTION_MSG",
};

static Evas_Object *mobiledata_popup;
static Evas_Object *check;
static Ecore_Timer *toast_timer;
static Ecore_Event_Handler *mouse_up_handler;

static bool is_bt_connected(void)
{
	int type;

	if (vconf_get_int(VCONFKEY_SAP_TYPE, &type) == 0
			&& type & SAP_BT)
		return true;

	return false;
}

static int get_current_state(void)
{
	int state, sim;

	if (vconf_get_bool(VCONFKEY_3G_ENABLE, &state) != 0)
		return MOBILEDATA_NOTSUP;

	if (vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &sim) != 0)
		return MOBILEDATA_NOTSUP;

	if (sim != VCONFKEY_TELEPHONY_SIM_INSERTED)
		return MOBILEDATA_NOSIM;

	if (is_bt_connected())
		return MOBILEDATA_BT;

	if (state == 1)
		return MOBILEDATA_ON;

	return MOBILEDATA_OFF;
}

static int update_mobiledata(int state)
{
	return vconf_set_bool(VCONFKEY_3G_ENABLE, state);
}

static void unregister_toast_handlers(struct appdata *ad)
{
	if (toast_timer) {
		ecore_timer_del(toast_timer);
		toast_timer = NULL;
	}

	if (mouse_up_handler) {
		ecore_event_handler_del(mouse_up_handler);
		mouse_up_handler = NULL;
	}
}

static Eina_Bool mouse_up_response(void *data, int type, void *event)
{
	unregister_toast_handlers(data);
	release_evas_object(&mobiledata_popup);
	return ECORE_CALLBACK_DONE;
}

static Eina_Bool mobiledata_toast_timeout(void *data)
{
	unregister_toast_handlers(data);
	release_evas_object(&mobiledata_popup);
	return ECORE_CALLBACK_CANCEL;
}

static void register_toast_handlers(struct appdata *ad)
{
	unregister_toast_handlers(ad);

	toast_timer = ecore_timer_add(TOAST_TIMEOUT, mobiledata_toast_timeout, NULL);
	if (!toast_timer)
		_E("Failed to add timer");

	mouse_up_handler = ecore_event_handler_add(
			ECORE_EVENT_MOUSE_BUTTON_UP, mouse_up_response, ad);
	if (!mouse_up_handler)
		_E("Failed to register mouse up handler");
}

static int launch_mobiledata_error(struct appdata *ad, int state)
{
	char *text;
	if (!ad)
		return -EINVAL;

	switch (state) {
	case MOBILEDATA_NOTSUP:
		text = "IDS_ST_POP_NOT_SUPPORTED";
		break;
	case MOBILEDATA_NOSIM:
		text = "IDS_ST_POP_INSERT_SIM_CARD_TO_ACCESS_NETWORK_SERVICES";
		break;
	case MOBILEDATA_BT:
		text = "IDS_ST_TPOP_UNABLE_TO_TURN_ON_MOBILE_DATA_WHILE_CONNECTED_VIA_BLUETOOTH";
		break;
	default:
		return -EINVAL;
	}

	mobiledata_popup = load_popup_toast(ad, _(text));
	if (!mobiledata_popup) {
		_E("Failed to load toast popup(%s)", text);
		return -ENOMEM;
	}

	register_toast_handlers(ad);

	return 0;
}

static void update_check_vconf_state(int state)
{
	char *vconf_name;

	if (!get_check_state(check))
		return;

	switch (state) {
	case MOBILEDATA_OFF:
		vconf_name = VCONFKEY_MOBILEDATA_ON_CHECK;
		break;
	case MOBILEDATA_ON:
		vconf_name = VCONFKEY_MOBILEDATA_OFF_CHECK;
		break;
	default:
		return;
	}

	if (vconf_set_bool(vconf_name, 1) != 0)
		_E("Failed to set vconf value (%s)", vconf_name);
}

static void response_mobiledata_ok_clicked(void *data, Evas_Object * obj, void *event_info)
{
	int state, ret;
	struct appdata *ad = data;

	_I("OK is selected");

	state = get_current_state();
	update_check_vconf_state(state);

	release_evas_object(&mobiledata_popup);

	switch (state) {
	case MOBILEDATA_OFF:
		state= MOBILEDATA_ON;
		break;
	case MOBILEDATA_ON:
		state= MOBILEDATA_OFF;
		break;
	default:
		_E("Current mobile state is (%d)", state);
		ret = launch_mobiledata_error(ad, state);
		if (ret < 0)
			_E("Failed to launch error popup (%d)", ret);
		return ;
	}

	ret = update_mobiledata(state);
	if (ret != 0)
		_E("Failed to update mobile data (%d)", ret);

	update_item();
}

static void response_mobiledata_cancel_clicked(void *data, Evas_Object * obj, void *event_info)
{
	_I("Cancel is selected");
	update_check_vconf_state(get_current_state());
	release_evas_object(&mobiledata_popup);
}

static bool get_popup_check_state(int state)
{
	char *vconf_name;

	switch (state) {
	case MOBILEDATA_OFF:
		vconf_name = VCONFKEY_MOBILEDATA_ON_CHECK;
		break;
	case MOBILEDATA_ON:
		vconf_name = VCONFKEY_MOBILEDATA_OFF_CHECK;
		break;
	default:
		return false;
	}

	if (vconf_get_bool(vconf_name, &state) == 0
			&& state == 1)
		return true;

	return false;
}

static int launch_mobiledata_popup(struct appdata *ad, int state)
{
	if (!ad)
		return -EINVAL;

	if (state != MOBILEDATA_ON && state != MOBILEDATA_OFF)
		return -EINVAL;

	mobiledata_popup = load_scrollable_check_popup(ad,
			EDJ_NAME,
			_(subpopup_title[state]),
			_(subpopup_text[state]),
			_("IDS_CAM_OPT_DONT_REPEAT_ABB"),
			&check,
			_("IDS_COM_SK_CANCEL"),
			response_mobiledata_cancel_clicked,
			_("IDS_COM_SK_OK"),
			response_mobiledata_ok_clicked);
	if (!mobiledata_popup) {
		_E("Failed to launch mobiledata popup");
		return -ENOMEM;
	}

	return 0;
}

static int mobiledata_get_icon(char *icon, int len)
{
	int state = get_current_state();

	if (state != MOBILEDATA_ON && state != MOBILEDATA_OFF)
		state = MOBILEDATA_BT;

	snprintf (icon, len, "%s", item_icon[state]);

	return 0;
}

static int mobiledata_get_text(char *text, int len)
{
	snprintf (text, len, "%s", MOBILEDATA_CONTENT);
	return 0;
}

static void mobiledata_clicked(void *data, Evas_Object *obj, void *event_info)
{
	int state, ret;
	struct appdata *ad = data;

	state = get_current_state();
	_I("Mobile data clicked. Current mobiledata state (%d)", state);

	switch (state) {
	case MOBILEDATA_NOTSUP:
	case MOBILEDATA_NOSIM:
	case MOBILEDATA_BT:
		ret = launch_mobiledata_error(ad, state);
		break;
	case MOBILEDATA_ON:
	case MOBILEDATA_OFF:
		if (get_popup_check_state(state)) {
			response_mobiledata_ok_clicked(ad, NULL, NULL);
			ret = 0;
		} else
			ret = launch_mobiledata_popup(ad, state);
		break;
	default:
		_E("Unknown mobiledata state (%d)", state);
		ret = -EINVAL;
		break;
	}

	if (ret < 0) {
		_E("Failed to launch mobiledata popup (%d)", ret);
		popup_terminate();
	}
}

static void mobiledata_unregister_handlers(void *data)
{
	struct appdata *ad = data;
	unregister_toast_handlers(ad);
}

static struct double_item_option mobiledata_ops = {
	.name = "mobiledata",
	.btn = NULL,
	.get_icon = mobiledata_get_icon,
	.get_text = mobiledata_get_text,
	.response_clicked = mobiledata_clicked,
	.register_handler = NULL,
	.unregister_handler = mobiledata_unregister_handlers
};

/* Constructor to register mobiledata button */
static __attribute__ ((constructor)) void register_double_data_option_mobiledata(void)
{
	register_second_option(&mobiledata_ops);
}
