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
#include "device-options.h"

#define POWEROFF_ID      200
#define POWEROFF_STYLE   "device_option.1text.1icon"
#define POWEROFF_CONTENT "IDS_ST_BODY_POWER_OFF"
#define POWEROFF_ICON    "micro-poweroff.png"
#define SYSTEMD_STOP_POWER_OFF		4

#define REMOVE_SUB_POPUP

static Elm_Genlist_Item_Class *poweroff_itc = NULL;

static void response_poweroff_yes_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *rect;
	Evas_Coord w, h, size;
	static int bPowerOff = 0;

	if (bPowerOff == 1)
		return;
	bPowerOff = 1;

	if (ad && ad->popup && ad->win_main) {
		unregister_main_list_handlers(ad);
		release_evas_object(&(ad->popup));

		rect = evas_object_rectangle_add(evas_object_evas_get(ad->win_main));
		evas_object_geometry_get(ad->win_main, NULL, NULL, &w, &h);
		size = max(w, h);
		evas_object_resize(rect, size, size);
		evas_object_color_set(rect, 0, 0, 0, 255);
		evas_object_show(rect);
	}

	_I("Turning off the phone !! Bye Bye ");

	if (vconf_set_int(VCONFKEY_SYSMAN_POWER_OFF_STATUS, SYSTEMD_STOP_POWER_OFF) != 0)
		_E("Failed to request poweroff to deviced");
}

#ifdef REMOVE_SUB_POPUP
static void response_poweroff_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;

	if (!ad)
		return;

	_I("Poweroff is selected");

	unregister_main_list_handlers(ad);

	response_poweroff_yes_clicked(ad,NULL,NULL);
}
#else
static void response_poweroff_no_clicked(void *data, Evas_Object * obj, void *event_info)
{
	_D("Cancel is selected");
	if (data) {
		object_cleanup(data);
		unregister_main_list_handlers(data);
	}

	popup_terminate();
}

static int show_poweroff_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	ad->popup = load_normal_popup(ad,
			_("IDS_ST_BODY_POWER_OFF"),
			_("IDS_COM_POP_DEVICE_WILL_TURN_OFF"),
			_("IDS_COM_SK_CANCEL"),
			response_poweroff_no_clicked,
			_("IDS_COM_SK_OK"),
			response_poweroff_yes_clicked);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static void response_poweroff_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;

	if (!ad)
		return;

	_I("Poweroff is selected");

	unregister_main_list_handlers(ad);
	release_evas_object(&(ad->popup));

	if (show_poweroff_popup(ad) < 0) {
		_E("FAIL: show_poweroff_popup()");
		popup_terminate();
	}
}
#endif

static int get_poweroff_id(void)
{
	return POWEROFF_ID;
}

static Evas_Object *get_poweroff_icon(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	if (strcmp(part, "elm.icon"))
		return NULL;

	icon = elm_icon_add(obj);
	if (!icon)
		return NULL;

	elm_image_file_set(icon, EDJ_NAME, POWEROFF_ICON);
	evas_object_size_hint_min_set(icon, IMAGE_SIZE, IMAGE_SIZE);
	evas_object_size_hint_max_set(icon, IMAGE_SIZE, IMAGE_SIZE);
	return icon;
}

static char *get_poweroff_content(void *data, Evas_Object *obj, const char *part)
{
	if (strcmp(part, "elm.text"))
		return NULL;
	return strdup(_(POWEROFF_CONTENT));
}

static bool is_poweroff_enabled(void)
{
	return true;
}

static int init_poweroff_itc(void)
{
	poweroff_itc = elm_genlist_item_class_new();
	if (!poweroff_itc)
		return -ENOMEM;
	return 0;
}

static void deinit_poweroff_itc(void)
{
	if (poweroff_itc) {
		elm_genlist_item_class_free(poweroff_itc);
		poweroff_itc = NULL;
	}
}

static int get_poweroff_itc(Elm_Genlist_Item_Class **itc)
{
	poweroff_itc->item_style = POWEROFF_STYLE;
	poweroff_itc->func.text_get = get_poweroff_content;
	poweroff_itc->func.content_get = get_poweroff_icon;
	*itc = poweroff_itc;
	return 0;
}

static const struct device_option poweroff_ops = {
	.name                = "poweroff",
	.get_id              = get_poweroff_id,
	.is_enabled          = is_poweroff_enabled,
	.init_itc            = init_poweroff_itc,
	.deinit_itc          = deinit_poweroff_itc,
	.get_itc             = get_poweroff_itc,
	.store_item          = NULL,
	.response_clicked    = response_poweroff_clicked,
	.register_handlers   = NULL,
	.unregister_handlers = NULL
};

/* Constructor to register poweroff item */
static __attribute__ ((constructor)) void register_device_option_poweroff(void)
{
	register_option(&poweroff_ops);
}
