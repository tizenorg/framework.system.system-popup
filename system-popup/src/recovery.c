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

#define RECOVERY_POPUP "recovery"
#define SYSTEMD_STOP_POWER_RESTART_RECOVERY 6

int show_recovery_popup(void *data, bundle *b);

struct popup_ops recovery_ops = {
	.name = RECOVERY_POPUP,
	.popup = NULL,
	.show_popup = show_recovery_popup
};

static void cancel_clicked(void *data, Evas_Object *obj, void *event_info)
{
	_I("Cancel is clicked");
	release_evas_object(&(recovery_ops.popup));
	terminate_if_no_popup();
}

static void restart_clicked(void *data, Evas_Object *obj, void *event_info)
{
	_I("Restart is clicked");
	release_evas_object(&(recovery_ops.popup));

	if (vconf_set_int(VCONFKEY_SYSMAN_POWER_OFF_STATUS,
				SYSTEMD_STOP_POWER_RESTART_RECOVERY) != 0)
		_E("Fail to request restart to deviced");

	terminate_if_no_popup();
}

int show_recovery_popup(void *data, bundle *b)
{
	struct appdata *ad = (struct appdata *)data;

	if (!ad || !(ad->win_main))
		return -EINVAL;

	if (recovery_ops.popup) {
		_E("Popup already exists");
		return 0;
	}

	evas_object_show(ad->win_main);

	recovery_ops.popup = load_normal_popup(ad,
			"Prevention information",		/* Title */
			"To protect your device, "
			"it is recommended to reboot it",		/* Text */
			"Reboot now", restart_clicked,		/* Left button */
			"Reboot later", cancel_clicked);	/* Right button */
	if (!(recovery_ops.popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	if (set_display_feedback(PATTERN_RECOVERY) < 0)
		_E("Failed to set display and feedback");

	return 0;
}

static __attribute__ ((constructor)) void register_recovery_popup(void)
{
	register_popup(&recovery_ops);
}
