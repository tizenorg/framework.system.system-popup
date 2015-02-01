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

#include <dd-deviced.h>
#include "double-data.h"

#define FLIGHTMODE_CONTENT   "IDS_ST_HEADER_FLIGHT_MODE_ABB"

static const char *item_icon[] = {
	"micro-flightmodeoff.png",
	"micro-flightmodeon.png",
};

static const char *subpopup_title[] = {
	"IDS_PN_OPT_ENABLE_FLIGHT_MODE_ABB",
	"IDS_PN_OPT_DISABLE_FLIGHT_MODE_ABB",
};

static const char *subpopup_content[] = {
	"IDS_ST_POP_FLIGHT_MODE_ALLOWS_YOU_TO_DISABLE_THE_CALLING_AND_MESSAGING_FUNCTIONS_IT_ALSO_TURNS_OFF_MOBILE_DATA_AND_BLUETOOTH_MSG",
	"IDS_COM_POP_FLIGHT_MODE_WILL_BE_DISABLED",
};

static Evas_Object *flightmode_popup;

static int get_flightmode(void)
{
	int flightmode;
	int ret;

	ret = vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &flightmode);
	if (ret != 0)
		return ret;
	_I("Flight mode: (%d)", flightmode);

	return flightmode;
}

static void response_flightmode_ok_clicked(void *data, Evas_Object * obj, void *event_info)
{
	int mode;

	_I("OK is selected");
	release_evas_object(&flightmode_popup);

	mode = get_flightmode();

	if (deviced_change_flightmode(mode) < 0)
		_E("Failed to send ready signal to deviced");
}

static void response_flightmode_cancel_clicked(void *data, Evas_Object * obj, void *event_info)
{
	_I("Cancel is selected");
	release_evas_object(&flightmode_popup);
}

static int show_flightmode_popup(struct appdata *ad)
{
	int mode;

	if (!ad)
		return -EINVAL;

	mode = get_flightmode();
	if (mode < 0)
		return mode;

	flightmode_popup = load_scrollable_popup(ad,
			LAYOUT_SCROLLABLE,
			EDJ_NAME,
			_(subpopup_title[mode]),
			_(subpopup_content[mode]),
			_("IDS_COM_SK_CANCEL"),
			response_flightmode_cancel_clicked,
			_("IDS_COM_SK_OK"),
			response_flightmode_ok_clicked);
	if (!(flightmode_popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}


static void flightmode_clicked(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	int ret;

	if (!ad)
		return;

	_I("Flightmode is selected");

	ret = show_flightmode_popup(ad);
	if (ret < 0) {
		_E("Failed to launch flight mode popup(%d)", ret);
		popup_terminate();
	}
}

static void flightmode_changed(keynode_t *in_key, void *data)
{
	update_item();
}

static int flightmode_get_icon(char *icon, int len)
{
	int mode;

	mode = get_flightmode();
	if (mode < 0)
		return mode;

	snprintf (icon, len, "%s", item_icon[mode]);
	return 0;
}

static int flightmode_get_text(char *text, int len)
{
	snprintf (text, len, "%s", FLIGHTMODE_CONTENT);
	return 0;
}

static int flightmode_register_handlers(void *data)
{
	return vconf_notify_key_changed(VCONFKEY_TELEPHONY_FLIGHT_MODE, flightmode_changed, data);
}

static void flightmode_unregister_handlers(void *data)
{
	vconf_ignore_key_changed(VCONFKEY_TELEPHONY_FLIGHT_MODE, flightmode_changed);
}

static struct double_item_option flightmode_ops = {
	.name = "flightmode",
	.btn = NULL,
	.get_icon = flightmode_get_icon,
	.get_text = flightmode_get_text,
	.response_clicked = flightmode_clicked,
	.register_handler = flightmode_register_handlers,
	.unregister_handler = flightmode_unregister_handlers
};

/* Constructor to register flightmode button */
static __attribute__ ((constructor)) void register_double_data_option_flightmode(void)
{
	if (get_flightmode() < 0)
		return;
	register_first_option(&flightmode_ops);
}
