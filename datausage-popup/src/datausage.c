/*
 *  system-popup
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
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
#include "common.h"
#include <aul.h>
#include <dd-display.h>

#define PACKAGE "datausage-popup"
#define SETTING_DATA_USAGE_UG  "setting-datausage-efl"

#define SYSPOPUP_CONTENT       "_SYSPOPUP_CONTENT_"
#define DATAUSAGE_DATA_BLOCKED "data_blocked"
#define DATAUSAGE_LIMIT        "_DATAUSAGE_LIMIT_"

#define DATA_BLOCKED_POPUP_CONTENT \
	"IDS_QP_BODY_MAXIMUM_DATA_USAGE_LIMIT_HPS_EXCEEDED_MOBILE_DATA_DISABLED_ENABLING_MOBILE_DATA_MAY_INCUR_ADDITIONAL_CHARGES"

#define BUF_MAX 256

static void datausage_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	_I("Button 'Cancel' selected");
	object_cleanup(data);
	popup_terminate();
}

static void launch_settings_data_usage_cb(void *data, Evas_Object *obj, void *event_info)
{
	int ret;
	bundle *b;

	_I("Button 'Settings' selected");

	object_cleanup(data);

	b = bundle_create();
	if (!b) {
		_E("FAIL: bundle_create()");
		return;
	}

	ret = aul_launch_app(SETTING_DATA_USAGE_UG, b);
	if (ret != AUL_R_OK) {
		_E("FAIL: aul_launch_app()");
	}

	if (bundle_free(b) != 0)
		_E("FAIL: bundle_free(b);");

	popup_terminate();
}

static int load_data_blocked_popup(struct appdata *ad, bundle *b)
{
	const char *limit;
	char *transtext;
	char content[BUF_MAX];
	char buf[BUF_MAX];

	if (!ad || !b)
		return -EINVAL;

	limit = bundle_get_val(b, DATAUSAGE_LIMIT);
	if (!limit) {
		_E("FAIL: bundle_get_val()");
		return -ENOMEM;
	}

	snprintf(buf, sizeof(buf), "%sMB", limit);
	transtext = _(DATA_BLOCKED_POPUP_CONTENT);
	snprintf(content, sizeof(content), transtext, buf);

	evas_object_show(ad->win_main);

	ad->popup = load_normal_popup(ad,
			_("IDS_COM_HEADER_MOBILE_DATA_DISABLED"),
			content,
			_("IDS_COM_SK_CANCEL"),
			datausage_cancel_cb,
			_("IDS_COM_BODY_SETTINGS"),
			launch_settings_data_usage_cb);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

/* App init */
static int app_create(void *data)
{
	Evas_Object *win;
	struct appdata *ad = data;

	ad->handler.def_term_fn = NULL;
	ad->handler.def_timeout_fn = NULL;

	/* create window */
	win = create_win(PACKAGE);
	if (win == NULL)
		return -ENOMEM;

	ad->win_main = win;

	if (appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR) != 0)
		_E("FAIL: appcore_set_i18n()");

	return 0;
}

/* Terminate noti handler */
static int app_terminate(void *data)
{
	object_cleanup(data);
	return 0;
}

/* Pause/background */
static int app_pause(void *data)
{
	return 0;
}

/* Resume */
static int app_resume(void *data)
{
	return 0;
}

/* Reset */
static int app_reset(bundle *b, void *data)
{
	struct appdata *ad = data;
	const char *opt;
	int ret;

	if (!ad || !b) {
		ret = -EINVAL;
		goto datausage_reset_out;
	}

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
		return 0;
	}

	opt = bundle_get_val(b, "_SYSPOPUP_CONTENT_");
	if (!opt) {
		_E("FAIL: bundle_get_val()");
		ret = -EINVAL;
		goto datausage_reset_out;
	}

	ret = syspopup_create(b, &(ad->handler), ad->win_main, ad);
	if (ret < 0) {
		_E("FAIL: syspopup_create()");
		goto datausage_reset_out;
	}

	if (!strcmp(opt, DATAUSAGE_DATA_BLOCKED)) {
		_I("Launching datausage-syspopup (data blocked)");
		ret = load_data_blocked_popup(ad, b);
		if (ret < 0) {
			_E("FAIL: load_data_blocked_popup()");
			goto datausage_reset_out;
		}

	} else {
		_E("Option is unknown");
		ret = -EINVAL;
		goto datausage_reset_out;
	}

	if (set_display_feedback(-1) < 0)
		_E("Failed to set display");

	return 0;

datausage_reset_out:
	popup_terminate();
	return ret;
}

int main(int argc, char *argv[])
{
	struct appdata ad;

	/* App life cycle management */
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
