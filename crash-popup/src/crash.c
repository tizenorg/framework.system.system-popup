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
#include <Ecore_X.h>
#include <utilX.h>
#include <pkgmgr-info.h>
#include <syspopup.h>
#include <dd-display.h>
#include <dd-deviced.h>
#include "common.h"
#include "crash.h"
#define DIRECT_LAUNCHED 3

static int is_syspoup;
static char process_name[NAME_MAX];
static char exepath[PATH_MAX];

/* pkgmgrinfo filter list function for getting application ID */
static int appinfo_get_appname_func(pkgmgrinfo_appinfo_h handle,
		void *user_data)
{
	char *str = NULL;
	char *label;
	int ret = PMINFO_R_ERROR;

	if (!user_data)
		return ret;

	ret = pkgmgrinfo_appinfo_get_label(handle, &str);
	if (ret != PMINFO_R_OK)
		return ret;

	if (!str)
		return PMINFO_R_ERROR;

	label = strdup(str);
	if (!label)
		return PMINFO_R_ERROR;

	(*(char**)user_data) = label;

	return PMINFO_R_OK;
}

/* get application ID by ail filter */
static char *get_app_name(char *exepath)
{
	pkgmgrinfo_appinfo_filter_h handle = NULL;
	int count, ret;
	char *name = NULL;

	ret = pkgmgrinfo_appinfo_filter_create(&handle);
	if (ret != PMINFO_R_OK) {
		goto out;
	}

	ret = pkgmgrinfo_appinfo_filter_add_string(handle, PMINFO_APPINFO_PROP_APP_EXEC, exepath);
	if (ret != PMINFO_R_OK) {
		goto out_free;
	}

	ret = pkgmgrinfo_appinfo_filter_count(handle, &count);
	if (ret != PMINFO_R_OK) {
		goto out_free;
	}

	if (count < 1) {
		goto out_free;
	} else {
		ret = pkgmgrinfo_appinfo_filter_foreach_appinfo(handle, appinfo_get_appname_func, &name);
		if (ret != PMINFO_R_OK) {
			name = NULL;
			goto out_free;
		}
	}

out_free:
	pkgmgrinfo_appinfo_filter_destroy(handle);
out:
	return name;
}

void popup_ok_cb(void *data, Evas_Object * obj, void *event_info)
{
	fflush(stdout);
	popup_terminate();
}

static int load_crash_process_popup(struct appdata *ad, char *name)
{
	char buf[MAX_PROCESS_NAME] = {0, };
	char *title, *text, *subtext;
	int len;

	if (!ad)
		return -EINVAL;

	title = _("IDS_COM_HEADER_ATTENTION");
	subtext = _("IDS_ST_BODY_PS_HAS_CLOSED_UNEXPECTEDLY");
	snprintf(buf, sizeof(buf), subtext, name);

	len = strlen(buf) + 3; /* 3: one period, one space, and one '\0' */
	text = (char *)malloc(len);
	if (!text) {
		_E("FAIL: malloc()");
		return -ENOMEM;
	}

	snprintf(text, len, "%s", buf);
	_I("Popup content: %s", text);

	evas_object_show(ad->win_main);

	ad->popup = load_normal_popup(ad,
			title,
			text,
			_( "IDS_COM_SK_OK"),
			popup_ok_cb,
			NULL, NULL);

	FREE(text);

	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

/* App init */
int app_create(void *data)
{
	Evas_Object *win;
	struct appdata *ad = data;
	int ret;

	ad->handler.def_term_fn = NULL;
	ad->handler.def_timeout_fn = NULL;

	/* create window */
	win = create_win(PACKAGE);
	if (win == NULL)
		return -1;

	ad->win_main = win;
	reset_window_priority(win, UTILX_NOTIFICATION_LEVEL_HIGH);

	ret = appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR);
	if (ret != 0)
		_E("FAIL: appcore_set_i18n()");

	return 0;

}

/* Terminate noti handler */
static int app_terminate(void *data)
{
	struct appdata *ad = data;

	if (ad->layout_main)
		evas_object_del(ad->layout_main);

	if (ad->win_main)
		evas_object_del(ad->win_main);

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
	const char *name, *path;
	char *str;
	char tname[NAME_MAX + 1] = {0,};
	int ret;

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
		return 0;
	}
	if (is_syspoup) {
		if (syspopup_create(b, &(ad->handler), ad->win_main, ad) < 0) {
			_E("FAIL: syspopup_create()");
			ret = -ENOMEM;
			goto crash_reset_out;
		}
		name = bundle_get_val(b, "_PROCESS_NAME_");
		if (!name) {
			_E("FAIL: bundle_get_val()");
			ret = -ENOMEM;
			goto crash_reset_out;
		}
		_D("bundle_get_val - process_name:%s", name);
		path = bundle_get_val(b, "_EXEPATH_");
		if (!path) {
			_E("FAIL: bundle_get_val()");
			ret = -ENOMEM;
			goto crash_reset_out;
		}
		_D("bundle_get_val - exepath:%s", path);
	} else {
		name = process_name;
		path = exepath;
	}

	str = get_app_name(path);
	if (str) {
		snprintf(tname, sizeof(tname), "%s", str);
	} else {
		snprintf(tname, sizeof(tname), "%s", name);
	}

	ret = load_crash_process_popup(ad, tname);
	if (ret < 0)
		goto crash_reset_out;

	if (set_display_feedback(-1) < 0)
		_E("Failed to set display");

	return 0;

crash_reset_out:
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

	deviced_conf_set_mempolicy(OOM_IGNORE);

	if (!ecore_x_init(NULL)) {
		_E("Cannot connect to X11 display\n");
		return -1;
	}

	if (argc == DIRECT_LAUNCHED) {
		_I("direct launched - process_name:%s exepath:%s", argv[1], argv[2]);
		snprintf(process_name, sizeof(process_name), "%s", argv[1]);
		snprintf(exepath, sizeof(exepath), "%s", argv[2]);
		is_syspoup = false;
	} else {
		is_syspoup = true;
	}

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
