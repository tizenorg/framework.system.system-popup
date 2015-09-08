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

#define COOLDOWN_POWERON_POPUP       "PowerOn"
#define LONGTEXT_SIZE                1024

static int show_cooldown_poweron_popup(void *data, bundle *b);

struct popup_ops cooldown_poweron_ops = {
	.name = COOLDOWN_POWERON_POPUP,
	.popup = NULL,
	.show_popup = show_cooldown_poweron_popup
};

static void ok_clicked(void *data, Evas_Object *obj, void *event_info)
{
	release_evas_object(&(cooldown_poweron_ops.popup));
	terminate_if_no_popup();
}

static int show_cooldown_poweron_popup(void *data, bundle *b)
{
	struct appdata *ad = data;
	char *text, content[LONGTEXT_SIZE];

	if (!ad || !(ad->win_main))
		return -EINVAL;

	if (cooldown_poweron_ops.popup) {
		_E("Popup already exists");
		return 0;
	}

	text = elm_entry_utf8_to_markup(
			_("IDS_QP_POP_YOUR_DEVICE_OVERHEATED_IT_POWERED_OFF_TO_PREVENT_DAMAGE_MSG"));
	if (text) {
		snprintf(content, sizeof(content), "%s", text);
		free(text);
	} else {
		snprintf(content, sizeof(content), "%s",
				_("IDS_QP_POP_YOUR_DEVICE_OVERHEATED_IT_POWERED_OFF_TO_PREVENT_DAMAGE_MSG"));
	}

	evas_object_show(ad->win_main);

	cooldown_poweron_ops.popup = load_normal_popup(ad,
			_("IDS_QP_HEADER_DEVICE_POWERED_OFF_AUTOMATICALLY"),
			content,
			_("IDS_COM_SK_OK"),
			ok_clicked,
			NULL, NULL);
	if (!(cooldown_poweron_ops.popup)) {
		_E("FAIL: load_normal_popup()");
		terminate_if_no_popup();
		return -ENOMEM;
	}

	if (set_display_feedback(PATTERN_WARNING) < 0)
		_E("Failed to set display and feedback");

	return 0;
}

static __attribute__ ((constructor)) void register_cooldown_poweron_popup(void)
{
	register_popup(&cooldown_poweron_ops);
}
