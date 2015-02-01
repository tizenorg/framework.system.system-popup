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
#include <vconf.h>
#include <appcore-efl.h>
#include "common.h"

#define SYSTEMD_STOP_POWER_OFF 4

static void pm_state_changed(keynode_t *key, void *data)
{
	int state;
	struct appdata *ad = data;

	if (!key || !ad)
		return;

	state = vconf_keynode_get_int(key);
	if (state != VCONFKEY_PM_STATE_LCDOFF)
		return;

	if (vconf_ignore_key_changed(VCONFKEY_PM_STATE, pm_state_changed) != 0)
		_E("vconf key ignore failed");

	popup_terminate();
}

static void register_handlers(struct appdata *ad)
{
	if (!ad)
		return;

	if (vconf_notify_key_changed(VCONFKEY_PM_STATE, pm_state_changed, ad) != 0)
		_E("vconf key notify failed");
}

static void unregister_handlers(struct appdata *ad)
{
	if (!ad)
		return;

	if (vconf_ignore_key_changed(VCONFKEY_PM_STATE, pm_state_changed) != 0)
		_E("vconf key ignore failed");
}

static void yes_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *rect;
	Evas_Coord w, h, size;
	static int bPowerOff = 0;

	if (bPowerOff == 1)
		return;
	bPowerOff = 1;

	if (ad && ad->popup && ad->win_main) {
		unregister_handlers(ad);
		release_evas_object(&(ad->popup));

		rect = evas_object_rectangle_add(evas_object_evas_get(ad->win_main));
		evas_object_geometry_get(ad->win_main, NULL, NULL, &w, &h);
		size = max(w, h);
		evas_object_resize(rect, size, size);
		evas_object_color_set(rect, 0, 0, 0, 255);
		evas_object_show(rect);
	}

	_I("Turning off the device !! Bye Bye ");

	if (vconf_set_int(VCONFKEY_SYSMAN_POWER_OFF_STATUS, SYSTEMD_STOP_POWER_OFF) != 0)
		_E("Failed to request poweroff to deviced");
}

static void no_clicked(void *data, Evas_Object * obj, void *event_info)
{
	_D("Cancel is selected");
	unregister_handlers(data);
	popup_terminate();
}

static int show_device_options(void *data)
{
	struct appdata *ad = data;

	/* Create and show popup */
	ad->popup = load_normal_popup(ad,
			_("IDS_ST_BODY_POWER_OFF"),
			_("IDS_TPLATFORM_BODY_POWER_OFF_THE_DEVICE_Q"),
			_("IDS_COM_SK_CANCEL"),
			no_clicked,
			_("IDS_HS_BUTTON_POWER_OFF_ABB2"),
			yes_clicked);
	if (ad->popup == NULL) {
		_E("FAIL: create_and_show_basic_popup()");
		return -ENOMEM;
	}

	/* Change LCD brightness */
	if (set_display_feedback(-1) < 0)
		_E("Failed to set display");

	return 0;
}

static int app_create(void *data)
{
	Evas_Object *win;
	struct appdata *ad = data;
	int ret;

	ad->handler.def_term_fn = NULL;
	ad->handler.def_timeout_fn = NULL;

	/* Create window (Reqd for sys-popup) */
	win = create_win(PACKAGE);
	if (win == NULL)
		return -1;

	ad->win_main = win;

	ret = appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR);
	if (ret != 0)
		_E("FAIL: appcore_set_i18n()");

	register_handlers(ad);

	return 0;
}

static int app_terminate(void *data)
{
	struct appdata *ad = data;

	unregister_handlers(ad);

	if (ad->win_main)
		evas_object_del(ad->win_main);

	return 0;
}

static int app_pause(void *data)
{
	return 0;
}

static int app_resume(void *data)
{
	return 0;
}

static int app_reset(bundle *b, void *data)
{
	struct appdata *ad = data;
	int ret;

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
		return 0;
	}

	ret = syspopup_create(b, &(ad->handler), ad->win_main, ad);
	if (ret < 0) {
		_E("Failed to create syspopup");
		popup_terminate();
		return ret;
	}

	evas_object_show(ad->win_main);

	show_device_options((void *)ad);

	return 0;
}

int main(int argc, char *argv[])
{
	struct appdata ad;

	struct appcore_ops ops = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.reset = app_reset,
	};

	memset(&ad, 0x0, sizeof(struct appdata));
	ops.data = &ad;

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
