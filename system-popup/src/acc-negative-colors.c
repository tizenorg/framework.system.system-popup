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
#include "accessibility.h"

#include <Ecore_X.h>
#include <Ecore_Input.h>
#include <utilX.h>

#define NEGATIVE_ID        20
#define NEGATIVE_ICON      "accessibility.png"

#define PROP_HIGH_CONTRAST "_E_ACC_ENABLE_HIGH_CONTRAST_"

static void run_contrast(void)
{
	int ret;
	unsigned int value;
	Ecore_X_Window rootWin;
	Ecore_X_Atom atomHighContrast;

	rootWin = ecore_x_window_root_first_get();
	atomHighContrast = ecore_x_atom_get(PROP_HIGH_CONTRAST);

	ret = ecore_x_window_prop_card32_get(rootWin, atomHighContrast, &value, 1);
	if (ret == 1 && value == 1)
		value = 0;
	else
		value = 1;

	ecore_x_window_prop_card32_set(rootWin, atomHighContrast, &value, 1);
	ecore_x_flush();

	vconf_set_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_HIGH_CONTRAST, value);
}

static void response_negative_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;

	if (!ad)
		return;

	remove_accessibility_popup();
	unregister_acc_option_full();

	run_contrast();

	terminate_if_no_popup();
}

static int get_negative_id(void)
{
	return NEGATIVE_ID;
}

static int get_negative_icon(char *icon, int size)
{
	if (!icon || size <= 0)
		return -EINVAL;
	snprintf(icon, size, "%s", NEGATIVE_ICON);
	return 0;
}

static int get_negative_content(char *content, int size)
{
	Ecore_X_Window rootWin;
	Ecore_X_Atom atomUI;
	unsigned int state;
	int ret;
	char *text;

	if (!content || size <= 0)
		return -EINVAL;

	rootWin = ecore_x_window_root_first_get();
	atomUI = ecore_x_atom_get(PROP_HIGH_CONTRAST);
	ret = ecore_x_window_prop_card32_get(rootWin, atomUI, &state, 1);
	if (ret < 0) {
		_E("ecore_x_window_prop_card32_get() failed");
		return ret;
	}

	if (state > 0)
		text = "IDS_ST_BODY_DISABLE_NEGATIVE_COLOURS";
	else
		text = "IDS_ST_BODY_ENABLE_NEGATIVE_COLOURS";

	snprintf(content, size, "%s", text);
	return 0;
}

static const struct acc_option negative_ops = {
	.name                = "negative",
	.get_id              = get_negative_id,
	.get_icon            = get_negative_icon,
	.get_content         = get_negative_content,
	.response_clicked    = response_negative_clicked,
	.register_handlers   = NULL,
	.unregister_handlers = NULL,
};

/* Constructor to register negative item */
static __attribute__ ((constructor)) void register_acc_option_negative(void)
{
	int value;

	if (vconf_get_int(VCONFKEY_SETAPPL_PSMODE, &value) == 0
			&& value == SETTING_PSMODE_EMERGENCY)
		return;

	register_acc_option(&negative_ops);
}
