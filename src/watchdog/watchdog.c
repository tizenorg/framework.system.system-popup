/*
 *  system-popup
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
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

#include "popup-common.h"

#define DBUS_RESOURCED_WATCHDOG_PATH   "/Org/Tizen/ResourceD/Process"
#define DBUS_RESOURCED_WATCHDOG_IFACE  "org.tizen.resourced.process"
#define DBUS_RESOURCED_WATCHDOG_SIGNAL "WatchdogResult"

#define APP_NAME       "_APP_NAME_"

enum button_selected {
	WATCHDOG_WAIT,
	WATCHDOG_OK,
};

static int watchdog_get_content(const struct popup_ops *ops, char *content, unsigned int len)
{
	char *text, *name;
	struct object_ops *obj;
	int ret;

	if (!ops || !content)
		return -EINVAL;

	ret = get_object_by_ops(ops, &obj);
	if (ret < 0) {
		_E("Failed to get object (%d)", ret);
		return -ENOENT;
	}

	name = (char *)bundle_get_val(obj->b, APP_NAME);
	if (!name) {
		_E("Failed to get app name");
		return -ENOENT;
	}

	text = _("IDS_ST_BODY_PS_IS_NOT_RESPONDING_CLOSE_PS_Q");

	snprintf(content, len, text, name, name);

	return 0;
}

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
		_E("FAIL: broadcast_dbus_signal(%d)", ret);
}

static void watchdog_wait(const struct popup_ops *ops)
{
	_I("Wait is selected");

	unload_simple_popup(ops);

	send_result_dbus_signal(WATCHDOG_WAIT);

	terminate_if_no_popup();
}

static void watchdog_ok(const struct popup_ops *ops)
{
	_I("OK is selected");

	unload_simple_popup(ops);

	send_result_dbus_signal(WATCHDOG_OK);

	terminate_if_no_popup();
}

static const struct popup_ops watchdog_ops = {
	.name			= "watchdog",
	.show_popup		= load_simple_popup,
	.title			= "IDS_CLD_HEADER_NO_RESPONSE",
	.get_content	= watchdog_get_content,
	.left_text		= "IDS_CST_OPT_WAIT",
	.left			= watchdog_wait,
	.right_text		= "IDS_COM_SK_OK",
	.right			= watchdog_ok,
	.flags			= SCROLLABLE,
};

/* Constructor to register watchdog button */
static __attribute__ ((constructor)) void watchdog_register_popup(void)
{
	register_popup(&watchdog_ops);
}
