/*
 * system-popup
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#define SAFEMODE_POPUP "safemode"

#define DEVICED_OBJECT_PATH     "/Org/Tizen/System/DeviceD"
#define DEVICED_INTERFACE_NAME  "org.tizen.system.deviced"
#define DEVICED_POWEROFF_SIGNAL "poweroffpopup"
#define REQUEST_REBOOT          "reboot"

static struct popup_ops safemode_ops;

static void cancel_clicked(void *data, Evas_Object *obj, void *event_info)
{
	_I("Cancel is clicked");
	release_evas_object(&(safemode_ops.popup));
	terminate_if_no_popup();
}

static int request_to_reboot(void)
{
	char *param[2];
	char reboot[32];

	snprintf(reboot, sizeof(reboot), "%s", REQUEST_REBOOT);
	param[0] = reboot;
	param[1] = "0";

	return broadcast_dbus_signal(
			DEVICED_OBJECT_PATH,
			DEVICED_INTERFACE_NAME,
			DEVICED_POWEROFF_SIGNAL,
			"si", param);
}

static void disable_clicked(void *data, Evas_Object *obj, void *event_info)
{
	static bool already = false;

	if (already)
		return;
	already = true;

	_I("Disable is clicked");
	release_evas_object(&(safemode_ops.popup));

	if(request_to_reboot() < 0)
		_E("Fail to request restart to deviced");
}

static int show_safemode_popup(void *data, bundle *b)
{
	struct appdata *ad = (struct appdata *)data;

	if (!ad || !(ad->win_main))
		return -EINVAL;

	if (safemode_ops.popup) {
		_E("Popup already exists");
		return 0;
	}

	evas_object_show(ad->win_main);

	safemode_ops.popup = load_normal_popup(ad,
			_("IDS_IDLE_HEADER_DISABLE_SAFE_MODE_ABB"),
			_("IDS_TPLATFORM_BODY_DISABLING_SAFE_MODE_WILL_RESTART_YOUR_DEVICE_DISABLE_Q"),
			_("IDS_COM_SK_CANCEL"),
			cancel_clicked,
			_("IDS_ST_SK_DISABLE_ABB"),
			disable_clicked);
	if (!(safemode_ops.popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	if (set_display_feedback(PATTERN_WARNING) < 0)
		_E("Failed to set display and feedback");

	return 0;
}

static struct popup_ops safemode_ops = {
	.name = SAFEMODE_POPUP,
	.popup = NULL,
	.show_popup = show_safemode_popup
};

static __attribute__ ((constructor)) void register_safemode_popup(void)
{
	register_popup(&safemode_ops);
}
