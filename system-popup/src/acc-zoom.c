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
#include <appcore-efl.h>
#include <dd-deviced.h>
#include <dd-display.h>
#include <dd-led.h>
#include <aul.h>
#include "accessibility.h"

#include <Ecore_X.h>
#include <Ecore_Input.h>
#include <utilX.h>

#define ZOOM_ID        40
#define ZOOM_ICON      "accessibility.png"

#define PROP_ZOOM      "_E_ACC_ENABLE_ZOOM_UI_"

#define LIVE_SETTING_APP "org.tizen.live-setting-app"

static void run_zoom(void)
{
	int ret;
	unsigned int value;
	Ecore_X_Window rootWin;
	Ecore_X_Atom atomZoomUI;

	rootWin = ecore_x_window_root_first_get();
	atomZoomUI = ecore_x_atom_get(PROP_ZOOM);

	ret = ecore_x_window_prop_card32_get(rootWin, atomZoomUI, &value, 1);
	if (ret == 1 && value == 1)
		value = 0;
	else
		value = 1;

	ecore_x_window_prop_card32_set(rootWin, atomZoomUI, &value, 1);
	ecore_x_flush();

	vconf_set_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_SCREEN_ZOOM, value);
}

static bool is_aircommand_on(void)
{
	int state;
	if (vconf_get_bool(VCONFKEY_AIRCOMMAND_ENABLED, &state) == 0
			&& state == 1)
		return true;
	return false;
}

static int launch_live_setting_app(void)
{
	bundle *b;
	int ret;

	b = bundle_create();
	if (!b) {
		_E("Failed to create bundle");
		return -ENOMEM;
	}

	ret = bundle_add(b, "popup", "zoom");
	if (ret < 0) {
		_E("Failed to add parameters to bundle");
		goto out;
	}

	ret = aul_launch_app(LIVE_SETTING_APP, b);
	if (ret < 0)
		_E("Failed to launch app(%s)", LIVE_SETTING_APP);

out:
	bundle_free(b);
	return ret;
}

static void response_zoom_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;

	if (!ad)
		return;

	remove_accessibility_popup();
	unregister_acc_option_full();

	if (is_aircommand_on()) {
		if (launch_live_setting_app() < 0)
			_E("Failed to launch (%s)", LIVE_SETTING_APP);
		terminate_if_no_popup();
		return;
	}

	run_zoom();

	terminate_if_no_popup();
}

static int get_zoom_id(void)
{
	return ZOOM_ID;
}

static int get_zoom_icon(char *icon, int size)
{
	if (!icon || size <= 0)
		return -EINVAL;
	snprintf(icon, size, "%s", ZOOM_ICON);
	return 0;
}

static int get_zoom_content(char *content, int size)
{
	Ecore_X_Window rootWin;
	Ecore_X_Atom atomUI;
	unsigned int state;
	int ret;
	char *text;

	if (!content || size <= 0)
		return -EINVAL;

	rootWin = ecore_x_window_root_first_get();
	atomUI = ecore_x_atom_get(PROP_ZOOM);
	ret = ecore_x_window_prop_card32_get(rootWin, atomUI, &state, 1);
	if (ret < 0) {
		_E("ecore_x_window_prop_card32_get() failed");
		return ret;
	}

	if (state > 0)
		text = "IDS_ST_BODY_DISABLE_ZOOM";
	else
		text = "IDS_ST_BODY_ENABLE_ZOOM";

	snprintf(content, size, "%s", text);
	return 0;
}

static const struct acc_option zoom_ops = {
	.name                = "zoom",
	.get_id              = get_zoom_id,
	.get_icon            = get_zoom_icon,
	.get_content         = get_zoom_content,
	.response_clicked    = response_zoom_clicked,
	.register_handlers   = NULL,
	.unregister_handlers = NULL
};

/* Constructor to register zoom item */
static __attribute__ ((constructor)) void register_acc_option_zoom(void)
{
	register_acc_option(&zoom_ops);
}
