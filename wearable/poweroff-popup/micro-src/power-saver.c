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
#include <ail.h>
#include "device-options.h"

#define POWER_SAVER_ID                900
#define POWER_SAVER_STYLE             "2text"
#define POWER_SAVER_TEXT              "IDS_ST_MBODY_POWER_SAVER_ABB"
#define POWER_SAVER_TITLE             POWER_SAVER_TEXT
#define POWER_SAVER_CONTENT           "IDS_ST_POP_POWER_SAVER_WILL_BE_ENABLED_THIS_WILL_LIMIT_THE_MAXIMUM_PERFORMANCE_OF_THE_CPU_TURN_OFF_BLUETOOTH_AND_A_LOWER_SCREEN_POWER_LEVEL_WILL_BE_USED_MSG"
#define POWER_SAVER_SUB_TEXT_ENABLED  "IDS_COM_POP_ENABLED"
#define POWER_SAVER_SUB_TEXT_DISABLED "IDS_COM_POP_DISABLED"
#define SYSTEMD_STOP_POWER_RESTART  5
#define TOAST_TIMEOUT               3
#define EXECUTE_TIMEOUT             0.5

enum power_saver_state {
	POWER_SAVER_OFF = SETTING_PSMODE_NORMAL,
	POWER_SAVER_ON  = 3,//SETTING_PSMODE_WEARABLE,
};

static bool power_saver_enabled = false;
static int power_saver_mode = POWER_SAVER_OFF;
static Elm_Object_Item *gl_item = NULL;
static Ecore_Timer *power_saver_toast_timer = NULL;
static Ecore_Timer *power_saver_execute_timer = NULL;
static Evas_Object *ps_popup = NULL;

static int store_item(Elm_Object_Item *item)
{
	if (!item) {
		return -EINVAL;
	}
	gl_item = item;
	return 0;
}

static void update_item(void)
{
	if (!gl_item)
		return;

	elm_genlist_item_update(gl_item);
	elm_genlist_item_selected_set(gl_item, EINA_FALSE);
}

static void change_power_saver_mode(int mode)
{
	if (vconf_set_int(VCONFKEY_SETAPPL_PSMODE, mode) != 0)
		_E("Failed to change power_saver mode");
}

static int kill_each_app(const aul_app_info *info, void *data)
{
	ail_appinfo_h handle;
	ail_error_e ret;
	bool task_managed;

	if (!info || info->pid <= 0 || !(info->pkg_name))
		return -EINVAL;

	if (info->pid == getpid())
		return 0;

	ret = ail_get_appinfo(info->pkg_name, &handle);
	if (ret != AIL_ERROR_OK) {
		_E("Failed to get ail handle (%d)", ret);
		return ret;
	}

	ret = ail_appinfo_get_bool(handle,
			AIL_PROP_X_SLP_TASKMANAGE_BOOL, &task_managed);
	if (ret != AIL_ERROR_OK) {
		_E("Failed to get ail task managed info (%d)", ret);
		goto out;
	}

	if (task_managed)
		aul_terminate_pid(info->pid);

	ret = 0;

out:
	ail_destroy_appinfo(handle);
	return ret;
}

static int terminate_all_apps(void)
{
	int ret;

	ret = aul_app_get_running_app_info(kill_each_app, NULL);
	if (ret != AUL_R_OK) {
		_E("Failed to kill all apps (%d)", ret);
	}

	return ret;
}

static void unregister_power_saver_execute_timer(void)
{
	if (power_saver_execute_timer) {
		ecore_timer_del(power_saver_execute_timer);
		power_saver_execute_timer = NULL;
	}
}

static Eina_Bool power_saver_execute_timeout(void *data)
{
	unregister_power_saver_execute_timer();
	change_power_saver_mode(power_saver_mode);
	if (terminate_all_apps() < 0)
		_E("Failed to terminate all apps");
	return ECORE_CALLBACK_CANCEL;
}

static int register_power_saver_execute_timer(void)
{
	unregister_power_saver_execute_timer();
	power_saver_execute_timer = ecore_timer_add(
			EXECUTE_TIMEOUT, power_saver_execute_timeout, NULL);
	if (!power_saver_execute_timer) {
		_E("Failed to register execute timer");
		return -ENOMEM;
	}

	return 0;
}

static void unregister_power_saver_toast_timer(void)
{
	if (power_saver_toast_timer) {
		ecore_timer_del(power_saver_toast_timer);
		power_saver_toast_timer = NULL;
	}
}

static Eina_Bool power_saver_disabled_toast_timeout(void *data)
{
	unregister_power_saver_toast_timer();
	release_evas_object(&ps_popup);
	popup_terminate();
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool power_saver_enabled_toast_timeout(void *data)
{
	unregister_power_saver_toast_timer();
	release_evas_object(&ps_popup);
	return ECORE_CALLBACK_CANCEL;
}

static int register_power_saver_toast_timer(Eina_Bool (*timeout_func)(void *data))
{
	if (!timeout_func)
		return -EINVAL;

	unregister_power_saver_toast_timer();

	power_saver_toast_timer = ecore_timer_add(TOAST_TIMEOUT, timeout_func, NULL);
	if (!power_saver_toast_timer) {
		_E("Failed to register timer");
		return -ENOMEM;
	}

	return 0;
}

static int show_power_saver_toast_popup(struct appdata *ad,
		char *text, Eina_Bool (*timeout_func)(void *data))
{
	if (!ad || !text || !timeout_func)
		return -EINVAL;

	ps_popup = load_popup_toast(ad, _(text));
	if (ps_popup == NULL) {
		_E("Failed to launch toast popup");
		popup_terminate();
		return -ENOMEM;
	}

	if (register_power_saver_toast_timer(timeout_func) < 0)
		_E("Failed to register timer for power_saver toast popup");

	return 0;
}

static void power_saver_cancel_clicked(void *data, Evas_Object *obj, void *event_info)
{
	release_evas_object(&ps_popup);
	update_item();
}

static void power_saver_ok_clicked(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	int ret;

	release_evas_object(&ps_popup);

	power_saver_mode = POWER_SAVER_ON;
	update_item();

	ret = show_power_saver_toast_popup(ad,
			_("IDS_ST_TPOP_POWER_SAVER_ENABLED"),
			power_saver_enabled_toast_timeout);
	if (ret < 0) {
		_E("Failed to launch toast popup(%d)", ret);
		popup_terminate();
	}

	ret = register_power_saver_execute_timer();
	if (ret < 0)
		_E("Failed to turn on/off power saver");
}

static int show_power_saver_popup(struct appdata *ad)
{
	if (!ad || !(ad->win_main))
		return -EINVAL;

	release_evas_object(&ps_popup);

	ps_popup = load_scrollable_popup(ad,
			_(POWER_SAVER_TITLE),
			_(POWER_SAVER_CONTENT),
			_("IDS_COM_SK_CANCEL"),
			power_saver_cancel_clicked,
			_("IDS_COM_SK_OK"),
			power_saver_ok_clicked);
	if (!ps_popup) {
		_E("Failed to load power saver popup");
		return -ENOMEM;
	}

	return 0;
}

static int turn_on_power_saver(struct appdata *ad)
{
	int ret;

	if (!ad)
		return -EINVAL;

	update_item();

	ret = show_power_saver_popup(ad);
	if (ret < 0)
		_E("Failed to launch power saver popup(%d)", ret);

	return ret;
}

static int turn_off_power_saver(struct appdata *ad)
{
	int ret;

	if (!ad)
		return -EINVAL;

	unregister_main_list_handlers(ad);
	release_evas_object(&(ad->popup));

	power_saver_mode = POWER_SAVER_OFF;
	change_power_saver_mode(power_saver_mode);
	update_item();

	ret = show_power_saver_toast_popup(ad,
			_("IDS_ST_TPOP_POWER_SAVER_DISABLED"),
			power_saver_disabled_toast_timeout);
	if (ret < 0)
		_E("Failed to launch toast popup(%d)", ret);

	return ret;
}

static void response_power_saver_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	int ret;

	if (!ad)
		return;

	_I("Emergency mode is selected");

	switch (power_saver_mode) {
	case POWER_SAVER_ON:
		ret = turn_off_power_saver(ad);
		break;

	case POWER_SAVER_OFF:
		ret = turn_on_power_saver(ad);
		break;

	default:
		_E("Unknown mode (%d)", power_saver_mode);
		popup_terminate();
		return;
	}
	if (ret < 0) {
		_E("Failed to turn on/off power saver");
		popup_terminate();
	}
}

static int get_power_saver_id(void)
{
	return POWER_SAVER_ID;
}

static char *get_power_saver_content(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text.1"))
		return strdup(_(POWER_SAVER_TEXT));
	if (!strcmp(part, "elm.text.2")) {
		if (power_saver_mode == POWER_SAVER_ON)
			return strdup(_(POWER_SAVER_SUB_TEXT_ENABLED));
		else
			return strdup(_(POWER_SAVER_SUB_TEXT_DISABLED));
	}
	return NULL;
}

static bool is_power_saver_enabled(void)
{
	return power_saver_enabled;
}

static Elm_Genlist_Item_Class *power_saver_itc = NULL;

static int init_power_saver_itc(void)
{
	power_saver_itc = elm_genlist_item_class_new();
	if (!power_saver_itc)
		return -ENOMEM;
	return 0;
}

static void deinit_power_saver_itc(void)
{
	if (power_saver_itc) {
		elm_genlist_item_class_free(power_saver_itc);
		power_saver_itc = NULL;
	}
}

static int get_power_saver_itc(Elm_Genlist_Item_Class **itc)
{
	power_saver_itc->item_style = POWER_SAVER_STYLE;
	power_saver_itc->func.text_get = get_power_saver_content;
	power_saver_itc->func.content_get = NULL;
	*itc = power_saver_itc;
	return 0;
}

static const struct device_option power_saver_ops = {
	.name                = "power_saver",
	.get_id              = get_power_saver_id,
	.is_enabled          = is_power_saver_enabled,
	.init_itc            = init_power_saver_itc,
	.deinit_itc          = deinit_power_saver_itc,
	.get_itc             = get_power_saver_itc,
	.store_item          = store_item,
	.response_clicked    = response_power_saver_clicked,
	.register_handlers   = NULL,
	.unregister_handlers = NULL
};

/* Constructor to register power_saver item */
static __attribute__ ((constructor)) void register_device_option_power_saver(void)
{
	int state;

	if (vconf_get_int(VCONFKEY_SETAPPL_PSMODE, &state) != 0) {
		power_saver_enabled = false; /* Dim state */
		power_saver_mode = POWER_SAVER_OFF;
	} else {
		power_saver_enabled = true;
		if (state == 0)
			power_saver_mode = POWER_SAVER_OFF;
		else
			power_saver_mode = POWER_SAVER_ON;
	}

	register_option(&power_saver_ops);
}
