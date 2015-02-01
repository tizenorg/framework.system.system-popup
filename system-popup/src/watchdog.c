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

#define WATCHDOG_POPUP "watchdog" /* Application Not Responding */
#define APP_NAME       "_APP_NAME_"
#define BUF_MAX 256

#define DBUS_RESOURCED_WATCHDOG_PATH   "/Org/Tizen/ResourceD/Process"
#define DBUS_RESOURCED_WATCHDOG_IFACE  "org.tizen.resourced.process"
#define DBUS_RESOURCED_WATCHDOG_SIGNAL "WatchdogResult"

int show_watchdog_popup(void *data, bundle *b);

struct popup_ops watchdog_ops = {
	.name = WATCHDOG_POPUP,
	.popup = NULL,
	.show_popup = show_watchdog_popup
};

static void send_result_dbus_signal(int result)
{
	int ret;
	char buf[8];
	char *param[1];

	snprintf(buf, sizeof(buf), "%d", result);
	param[0] = buf;
	ret = broadcast_dbus_signal(DBUS_RESOURCED_WATCHDOG_PATH,
			DBUS_RESOURCED_WATCHDOG_IFACE,
			DBUS_RESOURCED_WATCHDOG_SIGNAL,
			"i", param);
	if (ret < 0)
		_E("FAIL: broadcast_dbus_signal()");
}

static void wait_clicked(void *data, Evas_Object *obj, void *event_info)
{
	_I("Wait is clicked");
	release_evas_object(&(watchdog_ops.popup));

	/* Send dbus siganl with value 0(wait) */
	send_result_dbus_signal(0);

	terminate_if_no_popup();
}

static void ok_clicked(void *data, Evas_Object *obj, void *event_info)
{
	_I("OK is clicked");
	release_evas_object(&(watchdog_ops.popup));

	/* Send dbus siganl with value 1(OK) */
	send_result_dbus_signal(1);

	terminate_if_no_popup();
}

static int get_app_name(bundle *b, char buf[], int size)
{
	char *name;
	if (!b || !buf || size <= 0)
		return -EINVAL;

	name = (char *)bundle_get_val(b, APP_NAME);
	if (!name) {
		_E("FAIL: bundle_get_val()");
		return -ENOMEM;
	}

	snprintf(buf, size, "%s", name);
	return 0;
}

int show_watchdog_popup(void *data, bundle *b)
{
	struct appdata *ad = (struct appdata *)data;
	char name[BUF_MAX];
	char text[BUF_MAX];
	char *translated;
	int ret;

	if (!ad || !(ad->win_main) || !b)
		return -EINVAL;

	if (watchdog_ops.popup) {
		_E("Popup already exists");
		return 0;
	}

	ret = get_app_name(b, name, sizeof(name));
	if (ret < 0) {
		_E("FAIL: get_app_name()");
		return ret;
	}

	translated = _("IDS_ST_BODY_PS_IS_NOT_RESPONDING_CLOSE_PS_Q");

	snprintf(text, sizeof(text),
			translated, name, name);

	evas_object_show(ad->win_main);

	watchdog_ops.popup = load_normal_popup(ad,
			_("IDS_CLD_HEADER_NO_RESPONSE"),		/* Title */
			text,									/* Text */
			_("IDS_CST_OPT_WAIT"), wait_clicked,	/* Left button */
			_("IDS_ST_BUTTON_OK"), ok_clicked);		/* Right button */
	if (!(watchdog_ops.popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	if (set_display_feedback(PATTERN_WATCHDOG) < 0)
		_E("Failed to set display and feedback");

	return 0;
}


static __attribute__ ((constructor)) void register_watchdog_popup(void)
{
	register_popup(&watchdog_ops);
}
