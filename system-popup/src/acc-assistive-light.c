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

#define LIGHT_ID        60
#define LIGHT_ICON      "accessibility.png"

static void run_light(void)
{
	int max, state, ret;

	max = led_get_max_brightness();
	if (max < 0)
		max = 1;

	state = led_get_brightness();
	if (state > 0) {
		ret = led_set_brightness_with_noti(0, true);
		if (ret == 0)
			vconf_set_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TORCH_LIGHT, 0);
	} else {
		ret = led_set_brightness_with_noti(max, true);
		if (ret == 0)
			vconf_set_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TORCH_LIGHT, 1);
	}

	play_feedback(PLAY_ALL, PATTERN_HW_TAP);
}

static void response_light_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;

	if (!ad)
		return;

	remove_accessibility_popup();
	unregister_acc_option_full();

	run_light();

	terminate_if_no_popup();
}

static int get_light_id(void)
{
	return LIGHT_ID;
}

static int get_light_icon(char *icon, int size)
{
	if (!icon || size <= 0)
		return -EINVAL;
	snprintf(icon, size, "%s", LIGHT_ICON);
	return 0;
}

static int get_light_content(char *content, int size)
{
	int state;
	char *text;

	if (!content || size <= 0)
		return -EINVAL;

	state = led_get_brightness();

	if (state > 0)
		text = "IDS_ST_BODY_TURN_OFF_ASSISTIVE_LIGHT";
	else
		text = "IDS_ST_BODY_TURN_ON_ASSISTIVE_LIGHT";

	snprintf(content, size, "%s", text);
	return 0;
}

static const struct acc_option light_ops = {
	.name                = "light",
	.get_id              = get_light_id,
	.get_icon            = get_light_icon,
	.get_content         = get_light_content,
	.response_clicked    = response_light_clicked,
	.register_handlers   = NULL,
	.unregister_handlers = NULL
};

/* Constructor to register light item */
static __attribute__ ((constructor)) void register_acc_option_light(void)
{
	register_acc_option(&light_ops);
}
