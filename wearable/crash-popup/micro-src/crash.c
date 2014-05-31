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
#include <ail.h>
#include <dd-display.h>
#include <dd-deviced.h>
#include "micro-common.h"
#include "crash.h"

#define DIRECT_LAUNCHED  3
#define MAX_PROCESS_NAME NAME_MAX

static int is_syspoup;
static char process_name[NAME_MAX];
static char exepath[PATH_MAX];

static ail_cb_ret_e appinfo_get_name_func(const ail_appinfo_h appinfo,
		void *user_data)
{
	char *str;
	ail_cb_ret_e ret = AIL_CB_RET_CONTINUE;

	ret = ail_appinfo_get_str(appinfo, AIL_PROP_NAME_STR, &str);
	if (str) {
		(* (char **) user_data) = strdup(str);
		ret = AIL_CB_RET_CANCEL;
	}

	return ret;
}

static char *get_app_name(const char *exepath)
{
	ail_filter_h f;
	ail_error_e ret;
	int count;
	char *name = NULL;

	if (exepath == NULL)
		goto out;

	ret = ail_filter_new(&f);
	if (ret != AIL_ERROR_OK)
		goto out;

	ret = ail_filter_add_str(f, AIL_PROP_EXEC_STR, exepath);
	if (ret != AIL_ERROR_OK)
		goto out_free;

	ret = ail_filter_count_appinfo(f, &count);
	if (ret != AIL_ERROR_OK) {
		_E("Error: failed to count appinfo\n");
		goto out_free;
	}

	if (count < 1)
		goto out_free;

	ret = ail_filter_list_appinfo_foreach(f, appinfo_get_name_func, &name);

out_free:
	ail_filter_destroy(f);
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
	char *text, *subtext;
	int len;

	if (!ad)
		return -EINVAL;

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
			NULL,
			text,
			_("IDS_COM_SK_OK"),
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
		free(str);
	} else {
		snprintf(tname, sizeof(tname), "%s", name);
	}

	ret = load_crash_process_popup(ad, tname);
	if (ret < 0)
		goto crash_reset_out;

	if (display_change_state(LCD_NORMAL) < 0)
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
