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

#define RESTART_ID      500
#define RESTART_STYLE   "device_option.1text.1icon"
#define RESTART_CONTENT "IDS_ST_BUTTON_RESTART"
#define RESTART_ICON    "micro-restart.png"
#define SYSTEMD_STOP_POWER_RESTART		5

#define REMOVE_SUB_POPUP

static Elm_Genlist_Item_Class *restart_itc = NULL;

static void response_restart_yes_clicked(void *data, Evas_Object * obj, void *event_info)
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

	if (vconf_set_int(VCONFKEY_SYSMAN_POWER_OFF_STATUS, SYSTEMD_STOP_POWER_RESTART) != 0)
		_E("Failed to request restart to deviced");
}

#ifdef REMOVE_SUB_POPUP
static void response_restart_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;

	if (!ad)
		return;

	_I("Restart is selected");

	unregister_main_list_handlers(ad);

	response_restart_yes_clicked(ad, NULL, NULL);
}
#else
static void response_restart_no_clicked(void *data, Evas_Object * obj, void *event_info)
{
	_D("Cancel is selected");
	if (data) {
		object_cleanup(data);
		unregister_main_list_handlers(data);
	}

	popup_terminate();
}

static int show_restart_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	ad->popup = load_normal_popup(ad,
			_("IDS_ST_BUTTON_RESTART"),
			_("IDS_ST_POP_DEVICE_WILL_RESTART"),
			_("IDS_COM_SK_CANCEL"),
			response_restart_no_clicked,
			_("IDS_COM_SK_OK"),
			response_restart_yes_clicked);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static void response_restart_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;

	if (!ad)
		return;

	_I("Restart is selected");

	unregister_main_list_handlers(ad);
	release_evas_object(&(ad->popup));

	if (show_restart_popup(ad) < 0) {
		_E("FAIL: show_restart_popup()");
		popup_terminate();
	}
}
#endif

static int get_restart_id(void)
{
	return RESTART_ID;
}

static Evas_Object *get_restart_icon(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	if (strcmp(part, "elm.icon"))
		return NULL;

	icon = elm_icon_add(obj);
	if (!icon)
		return NULL;

	elm_image_file_set(icon, EDJ_NAME, RESTART_ICON);
	evas_object_size_hint_min_set(icon, IMAGE_SIZE, IMAGE_SIZE);
	evas_object_size_hint_max_set(icon, IMAGE_SIZE, IMAGE_SIZE);
	return icon;
}

static char *get_restart_content(void *data, Evas_Object *obj, const char *part)
{
	if (strcmp(part, "elm.text"))
		return NULL;
	return strdup(_(RESTART_CONTENT));
}

static bool is_restart_enabled(void)
{
	return true;
}

static int init_restart_itc(void)
{
	restart_itc = elm_genlist_item_class_new();
	if (!restart_itc)
		return -ENOMEM;
	return 0;
}

static void deinit_restart_itc(void)
{
	if (restart_itc) {
		elm_genlist_item_class_free(restart_itc);
		restart_itc = NULL;
	}
}

static int get_restart_itc(Elm_Genlist_Item_Class **itc)
{
	restart_itc->item_style = RESTART_STYLE;
	restart_itc->func.text_get = get_restart_content;
	restart_itc->func.content_get = get_restart_icon;
	*itc = restart_itc;
	return 0;
}

static const struct device_option restart_ops = {
	.name                = "restart",
	.get_id              = get_restart_id,
	.is_enabled          = is_restart_enabled,
	.init_itc            = init_restart_itc,
	.deinit_itc          = deinit_restart_itc,
	.get_itc             = get_restart_itc,
	.store_item          = NULL,
	.response_clicked    = response_restart_clicked,
	.register_handlers   = NULL,
	.unregister_handlers = NULL
};

/* Constructor to register restart item */
static __attribute__ ((constructor)) void register_device_option_restart(void)
{
	register_option(&restart_ops);
}
