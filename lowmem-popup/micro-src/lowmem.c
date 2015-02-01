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
#include <dd-deviced.h>
#include "micro-common.h"

#define LOW_STORAGE_TYPE_KEY                "_MEM_NOTI_"

#define LOW_STORAGE_WARNING_TITLE       "IDS_COM_POP_NOT_ENOUGH_MEMORY"
#define LOW_STORAGE_WARNING_CONTENT     "IDS_DAV_BODY_LOW_MEMORY_LEFT_ORANGE"
#define LOW_STORAGE_CRITICAL_TITLE      LOW_STORAGE_WARNING_TITLE
#define LOW_STORAGE_CRITICAL_CONTENT    LOW_STORAGE_WARNING_CONTENT
#define LOW_STORAGE_FULL_TITLE          "IDS_ST_HEADER_STORAGE_FULL_ABB"
#define LOW_STORAGE_FULL_CONTENT        "IDS_ST_POP_UNABLE_TO_RECORD_THERE_IS_NOT_ENOUGH_SPACE_IN_YOUR_GEAR_STORAGE"

#define EDJ_PATH "/usr/apps/org.tizen.lowmem-syspopup/res/edje/lowmem"
#define EDJ_NAME EDJ_PATH"/lowmem.edj"

#ifdef SYSTEM_APPS_MICRO_3
#define LAYOUT_STYLE "micro_3_title_content_button"
#else
#define LAYOUT_STYLE NULL
#endif

enum low_memory_options {
	LOW_STORAGE_WARNING,
	LOW_STORAGE_CRITICAL,
	LOW_STORAGE_FULL,
};

struct popup_type {
	char *name;
	int type;
};

static const struct popup_type low_memory_type[] = {
	{ "warning"   , LOW_STORAGE_WARNING     },
	{ "critical"  , LOW_STORAGE_CRITICAL    },
	{ "full"      , LOW_STORAGE_FULL        },
};

static void low_storage_response(void *data, Evas_Object * obj, void *event_info)
{
	_I("Low storage popup OK button clicked");
	popup_terminate();
}

static int load_low_storage_warning_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	evas_object_show(ad->win_main);

	ad->popup = load_normal_popup(ad,
			_(LOW_STORAGE_WARNING_TITLE),
			_(LOW_STORAGE_WARNING_CONTENT),
			_("IDS_COM_SK_OK"),
			low_storage_response,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int load_low_storage_full_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	evas_object_show(ad->win_main);

	ad->popup = load_scrollable_popup(ad,
			LAYOUT_STYLE,
			EDJ_NAME,
			_(LOW_STORAGE_FULL_TITLE),
			_(LOW_STORAGE_FULL_CONTENT),
			_("IDS_COM_SK_OK"),
			low_storage_response,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int load_low_storage_popup(struct appdata *ad, bundle *b)
{
	int type, i, ret;
	const char *opt;

	if (!ad || !b)
		return -EINVAL;

	opt = bundle_get_val(b, LOW_STORAGE_TYPE_KEY);
	if (!opt) {
		_E("Failed to get storage status");
		return -ENOMEM;
	}

	type = -1;
	for (i = 0 ; i < ARRAY_SIZE(low_memory_type) ; i++) {
		if (!strncmp(opt, low_memory_type[i].name, strlen(opt))) {
			type = low_memory_type[i].type;
			break;
		}
	}
	if (type < 0) {
		_E("Failed to get popup type(%d)", type);
		return -EINVAL;
	}

	switch (type) {
	case LOW_STORAGE_WARNING:
	case LOW_STORAGE_CRITICAL:
		ret = load_low_storage_warning_popup(ad);
		break;
	case LOW_STORAGE_FULL:
		ret = load_low_storage_full_popup(ad);
		break;
	default:
		return -EINVAL;
	}

	play_feedback(FEEDBACK_TYPE_NONE, FEEDBACK_PATTERN_LOWBATT);

	return ret;
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

	elm_theme_overlay_add(NULL,EDJ_NAME);

	ret = appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR);
	if (ret != 0)
		_E("FAIL: appcore_set_i18n()");

	return 0;
}

/* Terminate noti handler */
static int app_terminate(void *data)
{
	struct appdata *ad = data;

	if (ad && ad->win_main)
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
	int ret;

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
		return 0;
	}

	if (syspopup_create(b, &(ad->handler), ad->win_main, ad) < 0) {
		_E("FAIL: syspopup_create()");
		ret = -ENOMEM;
		goto lowmem_reset_out;
	}

	syspopup_reset_timeout(b, -1);

	ret = load_low_storage_popup(ad, b);

lowmem_reset_out:
	if (ret < 0)
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

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
