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
#include <aul.h>
#include <appsvc.h>
#include "device-options.h"

#define POWER_SAVING_ID                600
#define POWER_SAVING_STYLE             "device_option.2text"
#define POWER_SAVING_TEXT              "IDS_ST_HEADER_ULTRA_POWER_SAVING_MODE"
#define POWER_SAVING_SUB_TEXT_ENABLED  "IDS_COM_POP_ENABLED"
#define POWER_SAVING_SUB_TEXT_DISABLED "IDS_COM_POP_DISABLED"

#define POWER_SAVING_APP               "org.tizen.clocksetting.powersaving"
#define POWER_SAVING_KEY               "viewtype"

enum power_saving_status {
	POWER_SAVING_DISABLED,
	POWER_SAVING_ENABLED,
};

static struct ps_app_status {
	int status;
	char *app_value;
} ps_status [] = {
	{ POWER_SAVING_DISABLED, "enable"  },
	{ POWER_SAVING_ENABLED,  "disable" },
};

static int power_saving_mode = POWER_SAVING_DISABLED;
static Elm_Object_Item *gl_item = NULL;

static int store_item(Elm_Object_Item *item)
{
	if (!item) {
		return -EINVAL;
	}
	gl_item = item;
	return 0;
}

static int get_ps_app_value(char *value, unsigned int len)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ps_status); i++) {
		if (ps_status[i].status != power_saving_mode)
			continue;
		snprintf(value, len, "%s", ps_status[i].app_value);
		return 0;
	}
	return -EINVAL;
}

static void response_power_saving_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	int ret;
	char value[BUF_MAX];
	bundle *b;

	if (!ad)
		goto out;

	_I("Power saving mode is selected");

	ret = get_ps_app_value(value, sizeof(value));
	if (ret < 0) {
		_E("Failed to get app value(%d)", ret);
		goto out;
	}

	b = bundle_create();
	if (!b) {
		_E("Failed to make bundle");
		goto out;
	}
	appsvc_set_operation(b, APPSVC_OPERATION_VIEW);
	appsvc_add_data(b, POWER_SAVING_KEY, value);
	appsvc_set_pkgname(b, POWER_SAVING_APP);
	ret = appsvc_run_service(b, 0, NULL, NULL);
	if (ret < 0)
		_E("Failed to launch power saving app(%d)", ret);
	bundle_free(b);

out:
	popup_terminate();
}

static int get_power_saving_id(void)
{
	return POWER_SAVING_ID;
}

static char *get_power_saving_content(void *data, Evas_Object *obj, const char *part)
{
	_I("text(%s)", part);
	if (!strcmp(part, "elm.text.1"))
		return strdup(_(POWER_SAVING_TEXT));
	if (!strcmp(part, "elm.text.2")) {
		if (power_saving_mode == POWER_SAVING_ENABLED)
			return strdup(_(POWER_SAVING_SUB_TEXT_ENABLED));
		else
			return strdup(_(POWER_SAVING_SUB_TEXT_DISABLED));
	}
	return NULL;
}

static bool is_power_saving_enabled(void)
{
	return true;
}

static Elm_Genlist_Item_Class *power_saving_itc = NULL;

static int init_power_saving_itc(void)
{
	power_saving_itc = elm_genlist_item_class_new();
	if (!power_saving_itc)
		return -ENOMEM;
	return 0;
}

static void deinit_power_saving_itc(void)
{
	if (power_saving_itc) {
		elm_genlist_item_class_free(power_saving_itc);
		power_saving_itc = NULL;
	}
}

static int get_power_saving_itc(Elm_Genlist_Item_Class **itc)
{
	power_saving_itc->item_style = POWER_SAVING_STYLE;
	power_saving_itc->func.text_get = get_power_saving_content;
	power_saving_itc->func.content_get = NULL;
	*itc = power_saving_itc;
	return 0;
}

static const struct device_option power_saving_ops = {
	.name                = "power_saving",
	.get_id              = get_power_saving_id,
	.is_enabled          = is_power_saving_enabled,
	.init_itc            = init_power_saving_itc,
	.deinit_itc          = deinit_power_saving_itc,
	.get_itc             = get_power_saving_itc,
	.store_item          = store_item,
	.response_clicked    = response_power_saving_clicked,
	.register_handlers   = NULL,
	.unregister_handlers = NULL
};

/* Constructor to register power_saving item */
static __attribute__ ((constructor)) void register_device_option_power_saving(void)
{
	int state;

	if (vconf_get_int(VCONFKEY_SETAPPL_PSMODE, &state) == 0
			&& state == SETTING_PSMODE_WEARABLE)
		power_saving_mode = POWER_SAVING_ENABLED;
	else
		power_saving_mode = POWER_SAVING_DISABLED;

	register_option(&power_saving_ops);
}
