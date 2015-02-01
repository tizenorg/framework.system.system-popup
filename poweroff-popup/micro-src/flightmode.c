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

#include <stdio.h>
#include <dd-deviced.h>
#include "device-options.h"

#define FLIGHTMODE_ID        300
#define FLIGHTMODE_STYLE     "device_option.2text.1icon"
#define FLIGHTMODE_CONTENT   "IDS_ST_HEADER_FLIGHT_MODE_ABB"

static const char *item_icon[] = {
	"micro-flightmodeoff.png",
	"micro-flightmodeon.png",
};

static const char *item_subtitle[] = {
	"IDS_ST_BODY_OFF_M_STATUS",
	"IDS_ST_BODY_ON_M_STATUS",
};

static const char *subpopup_title[] = {
	"IDS_PN_OPT_ENABLE_FLIGHT_MODE_ABB",
	"IDS_PN_OPT_DISABLE_FLIGHT_MODE_ABB",
};

static const char *subpopup_content[] = {
	"IDS_ST_POP_FLIGHT_MODE_ALLOWS_YOU_TO_DISABLE_THE_CALLING_AND_MESSAGING_FUNCTIONS_IT_ALSO_TURNS_OFF_MOBILE_DATA_AND_BLUETOOTH_MSG",
	"IDS_COM_POP_FLIGHT_MODE_WILL_BE_DISABLED",
};


static Elm_Genlist_Item_Class *flightmode_itc = NULL;

static int get_flightmode(void)
{
	static bool already = false;
	static int flightmode = -1;
	int ret;

	if (!already) {
		ret = vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &flightmode);
		if (ret != 0)
			return ret;
		already = true;
		_I("Flight mode: (%d)", flightmode);
	}

	return flightmode;
}

static void response_flightmode_ok_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	int mode;

	_I("OK is selected");
	release_evas_object(&(ad->popup));

	mode = get_flightmode();

	if (deviced_change_flightmode(mode) < 0)
		_E("Failed to send ready signal to deviced");

	popup_terminate();
}

static void response_flightmode_cancel_clicked(void *data, Evas_Object * obj, void *event_info)
{
	_I("Cancel is selected");
	object_cleanup(data);
	popup_terminate();
}

static int show_flightmode_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	ad->popup = load_scrollable_popup(ad,
			LAYOUT_SCROLLABLE,
			EDJ_NAME,
			_(subpopup_title[get_flightmode()]),
			_(subpopup_content[get_flightmode()]),
			_("IDS_COM_SK_CANCEL"),
			response_flightmode_cancel_clicked,
			_("IDS_COM_SK_OK"),
			response_flightmode_ok_clicked);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static void response_flightmode_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	int ret;

	if (!ad)
		return;

	_I("Flightmode is selected");

	unregister_main_list_handlers(ad);
	release_evas_object(&(ad->popup));

	ret = show_flightmode_popup(ad);
	if (ret < 0) {
		_E("Failed to launch flight mode popup(%d)", ret);
		popup_terminate();
	}
}

static int get_flightmode_id(void)
{
	return FLIGHTMODE_ID;
}

static Evas_Object *get_flightmode_icon(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	if (strcmp(part, "elm.icon"))
		return NULL;

	icon = elm_icon_add(obj);
	if (!icon)
		return NULL;

	elm_image_file_set(icon, EDJ_NAME, item_icon[get_flightmode()]);
	evas_object_size_hint_min_set(icon, IMAGE_SIZE, IMAGE_SIZE);
	evas_object_size_hint_max_set(icon, IMAGE_SIZE, IMAGE_SIZE);
	return icon;
}

static char *get_flightmode_content(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text.2"))
		return strdup(_(item_subtitle[get_flightmode()]));
	else if (!strcmp(part, "elm.text.1"))
		return strdup(_(FLIGHTMODE_CONTENT));
	else
		return NULL;
}

static bool is_flightmode_enabled(void)
{
	return true;
}

static int init_flightmode_itc(void)
{
	flightmode_itc = elm_genlist_item_class_new();
	if (!flightmode_itc)
		return -ENOMEM;
	return 0;
}

static void deinit_flightmode_itc(void)
{
	if (flightmode_itc) {
		elm_genlist_item_class_free(flightmode_itc);
		flightmode_itc = NULL;
	}
}

static int get_flightmode_itc(Elm_Genlist_Item_Class **itc)
{
	flightmode_itc->item_style = FLIGHTMODE_STYLE;
	flightmode_itc->func.text_get = get_flightmode_content;
	flightmode_itc->func.content_get = get_flightmode_icon;
	*itc = flightmode_itc;
	return 0;
}

static const struct device_option flightmode_ops = {
	.name                = "flightmode",
	.get_id              = get_flightmode_id,
	.is_enabled          = is_flightmode_enabled,
	.init_itc            = init_flightmode_itc,
	.deinit_itc          = deinit_flightmode_itc,
	.get_itc             = get_flightmode_itc,
	.store_item          = NULL,
	.response_clicked    = response_flightmode_clicked,
	.register_handlers   = NULL,
	.unregister_handlers = NULL
};

/* Constructor to register flightmode item */
static __attribute__ ((constructor)) void register_device_option_flightmode(void)
{
	if (get_flightmode() < 0)
		return;
	register_option(&flightmode_ops);
}
