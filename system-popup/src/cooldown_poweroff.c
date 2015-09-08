/*
 * system-popup
 * Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
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
 */

#include "core.h"
#include "common.h"

#define COOLDOWN_POWEROFF_POPUP       "PowerOff"

static int show_cooldown_poweroff_popup(void *data, bundle *b);

struct popup_ops cooldown_poweroff_ops = {
	.name = COOLDOWN_POWEROFF_POPUP,
	.popup = NULL,
	.show_popup = show_cooldown_poweroff_popup
};

static void not_poweroff_clicked(void *data, Evas_Object *obj, void *event_info)
{
	release_evas_object(&(cooldown_poweroff_ops.popup));
	terminate_if_no_popup();
}

static int show_cooldown_poweroff_popup(void *data, bundle *b)
{
	struct appdata *ad = data;

	if (!ad || !(ad->win_main))
		return -EINVAL;

	if (cooldown_poweroff_ops.popup) {
		_E("Popup already exists");
		return 0;
	}

	evas_object_show(ad->win_main);

	cooldown_poweroff_ops.popup = load_normal_popup(ad,
			_("IDS_ST_HEADER_POWER_OFF_ABB"),
			_("IDS_QP_POP_YOUR_DEVICE_IS_OVERHEATING_IT_WILL_NOW_POWER_OFF_TO_COOL_DOWN"),
			_("IDS_QP_BUTTON_DO_NOT_POWER_OFF_ABB"),
			not_poweroff_clicked,
			NULL, NULL);
	if (!(cooldown_poweroff_ops.popup)) {
		_E("FAIL: load_normal_popup()");
		terminate_if_no_popup();
		return -ENOMEM;
	}

	if (set_display_feedback(PATTERN_WARNING) < 0)
		_E("Failed to set display and feedback");

	return 0;
}

static __attribute__ ((constructor)) void register_cooldown_poweroff_popup(void)
{
	register_popup(&cooldown_poweroff_ops);
}
