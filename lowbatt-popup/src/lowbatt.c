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
#include "lowbatt.h"
#include <vconf.h>
#include <vconf-keys.h>
#include <Ecore_X.h>
#include <utilX.h>
#include <syspopup.h>
#include <dd-display.h>
#include <aul.h>
#include <sound_manager.h>
#include <wav_player.h>
#include "common.h"

#define DEVICED_PATH_SYSNOTI        "/Org/Tizen/System/DeviceD/SysNoti"
#define DEVICED_INTERFACE_SYSNOTI   "org.tizen.system.deviced.SysNoti"
#define SIGNAL_CHARGEERR_RESPONSE   "ChargeErrResponse"

#define EDJ_PATH "/usr/apps/org.tizen.lowbat-syspopup/res/edje/lowbatt"
#define EDJ_NAME EDJ_PATH"/lowbatt.edj"

#define BUF_MAX 256
#define RETRY_MAX 5

#define LOWBAT_SOUND_PATH          "/usr/share/feedback/sound/operation/system.ogg"

#define LOWBATT_WARNING_TITLE      "IDS_ST_BODY_LEDOT_LOW_BATTERY"
#define LOWBATT_WARNING_CHARGE_NOW_TITLE LOWBATT_WARNING_TITLE
#define LOWBATT_WARNING_CONTENT    "IDS_COM_POP_CONNECT_TO_A_PLUG_SOCKET_TO_CHARGE_DEVICE_AND_EXIT_UNUSED_APPLICATIONS_IN_TASK_MANAGER_MSG"
#define LOWBATT_WARNING_CHARGE_NOW_CONTENT "IDS_COM_POP_BATTERYLOW"
#define LOWBATT_POWEROFF_TITLE     "IDS_ST_BODY_LEDOT_LOW_BATTERY"
#define LOWBATT_POWEROFF_CONTENT   "IDS_COM_POP_LOW_BATTERY_PHONE_WILL_SHUT_DOWN"
#define CHARGE_ERR_TITLE           "IDS_ST_POP_WARNING_MSG"
#define CHARGE_ERR_CONTENT         "IDS_COM_BODY_CHARGING_PAUSED_DUE_TO_EXTREME_TEMPERATURE"
#define CHARGE_ERR_LOW_TITLE       "IDS_ST_POP_WARNING_MSG"
#define CHARGE_ERR_LOW_CONTENT     "IDS_IDLE_POP_UNABLE_CHANGE_BATTERY_TEMA_LOW"
#define CHARGE_ERR_HIGH_TITLE      "IDS_ST_POP_WARNING_MSG"
#define CHARGE_ERR_HIGH_CONTENT    "IDS_IDLE_POP_UNABLE_CHANGE_BATTERY_TEMA_HIGH"
#define CHARGE_ERR_OVP_TITLE       "IDS_ST_POP_WARNING_MSG"
#define CHARGE_ERR_OVP_CONTENT     "IDS_COM_POP_CHARGING_PAUSED_VOLTAGE_TOO_HIGH"
#define BATT_DISCONNECTED_TITLE    "IDS_COM_BODY_NO_BATTERY"
#define BATT_DISCONNECTED_CONTENT  "IDS_COM_POP_BATTERY_DISCONNECTED_ABB"

enum lowbat_options {
	LOWBAT_WARNING,
	LOWBAT_POWEROFF,
	LOWBAT_CRITICAL,
	LOWBAT_EXETREME,
	LOWBAT_NO_TOUCH,
	LOWBAT_CHARGE_ERR,
	LOWBAT_CHARGE_ERR_LOW,
	LOWBAT_CHARGE_ERR_HIGH,
	LOWBAT_CHARGE_ERR_OVP,
	LOWBAT_BATT_DISCONNECT,
};

struct popup_type {
	char *name;
	int type;
};

static const struct popup_type lowbat_type[] = {
	{ "warning"           , LOWBAT_WARNING            },
	{ "poweroff"          , LOWBAT_POWEROFF           },
	{ "critical"          , LOWBAT_CRITICAL           },
	{ "exetreme"          , LOWBAT_EXETREME           },
	{ "notouch"           , LOWBAT_NO_TOUCH           },
	{ "chargeerr"         , LOWBAT_CHARGE_ERR         },
	{ "chargeerrlow"      , LOWBAT_CHARGE_ERR_LOW     },
	{ "chargeerrhigh"     , LOWBAT_CHARGE_ERR_HIGH    },
	{ "chargeerrovp"      , LOWBAT_CHARGE_ERR_OVP     },
	{ "battdisconnect"    , LOWBAT_BATT_DISCONNECT    },
};

static int type;
static int hall_ic = 1;
static E_DBus_Connection *edbus_conn = NULL;
static E_DBus_Signal_Handler *hallic_handler = NULL;

static void hall_state_changed(void *data, DBusMessage *msg);

static bool get_call_state(void)
{
	int state;
	if (vconf_get_int(VCONFKEY_CALL_STATE, &state) == 0
			&& state != VCONFKEY_CALL_OFF)
		return true;
	return false;
}

static bool get_voice_recording_state(void)
{
	int state;
	if (vconf_get_int(VCONFKEY_SOUND_STATUS, &state) == 0
			&& (state & VCONFKEY_SOUND_STATUS_AVRECORDING))
		return true;
	return false;
}

static void play_notification_sound(void)
{
	int ret, sound;
	static bool session = false;

	if (vconf_get_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &sound) != 0) {
		_E("Failed to get sound status");
		return;
	}

	if (sound == 0)
		return;

	if (get_call_state())
		return;
	if (get_voice_recording_state())
		return;

	if (!session) {
		ret = sound_manager_set_session_type(SOUND_SESSION_TYPE_NOTIFICATION);
		if (ret != SOUND_MANAGER_ERROR_NONE) {
			_E("Failed to set session to play sound(%d)", ret);
			return;
		}
	}

	if (access(LOWBAT_SOUND_PATH, F_OK) != 0) {
		_E("The sound file does not exist (%d)", LOWBAT_SOUND_PATH);
		return;
	}

	ret = wav_player_start(LOWBAT_SOUND_PATH, SOUND_TYPE_NOTIFICATION, NULL, NULL, NULL);
	if (ret != WAV_PLAYER_ERROR_NONE)
		_E("Failed to play sound file (%d, %s)", ret, LOWBAT_SOUND_PATH);
}

static bool get_charge_state(void)
{
	int val;
	if (vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, &val) == 0
			&& val == 1)
		return true;
	return false;
}


static void unregister_edbus_signal_handler(void)
{
	if (edbus_conn) {
		if (hallic_handler)
			e_dbus_signal_handler_del(edbus_conn, hallic_handler);
		e_dbus_connection_close(edbus_conn);
	}
	e_dbus_shutdown();
}

static int register_edbus_signal_handler(struct appdata *ad)
{
	int retry, ret;

	retry = 0;
	while (e_dbus_init() == 0) {
		if (retry++ >= RETRY_MAX)
			return -ENOMEM;
	}

	edbus_conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
	if (!edbus_conn) {
		_E("Failed to get dbus bus");
		ret = -ENOMEM;
		goto out;
	}

	hallic_handler = e_dbus_signal_handler_add(edbus_conn, NULL, DEVICED_PATH_HALL,
			DEVICED_IFACE_HALL, HALL_STATE_SIGNAL, hall_state_changed, ad);
	if (!hallic_handler) {
		_E("Failed to register handler");
		ret = -ENOMEM;
		goto out;
	}

	ret = 0;
out:
	if (ret < 0)
		unregister_edbus_signal_handler();
	return ret;
}

static void charger_status_changed(keynode_t *key, void *data)
{
	int status;

	status = vconf_keynode_get_int(key);
	if (status != VCONFKEY_SYSMAN_CHARGER_CONNECTED)
		return;

	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
				charger_status_changed) < 0)
		_E("Failed to release vconf key handler");
	popup_terminate();
}

static void unregister_charger_status_handler(void)
{
	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
			charger_status_changed) < 0)
		_E("Falied to release vconf key handler");
}

static void register_charger_status_handler(void)
{
	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
			charger_status_changed, NULL) < 0)
		_E("Failed to register vconf key handler");
}

static void battery_status_changed(keynode_t *key, void *data)
{
	int status;

	status = vconf_keynode_get_int(key);
	_I("BATTERY status: %d", status);
	if (status != VCONFKEY_SYSMAN_BAT_NORMAL &&
		status != VCONFKEY_SYSMAN_BAT_FULL)
		return;

	vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW,
			battery_status_changed);
	popup_terminate();
}

static void register_battery_status_handler(void)
{
	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW,
				battery_status_changed, NULL) < 0)
		_E("Falied to add vconf key handler");
}

static void unregister_battery_status_handler(void)
{
	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW,
				battery_status_changed) < 0)
		_E("Falied to release vconf key handler");
}

static void lowbatt_timeout_func(void *data, Evas_Object *obj, void *event_info)
{
	_D("System-popup : In Lowbatt timeout");
	object_cleanup(data);
	popup_terminate();
}

static void lowbatt_error_response(void *data, Evas_Object *obj, void *event_info)
{
	if (broadcast_dbus_signal(DEVICED_PATH_SYSNOTI,
				DEVICED_INTERFACE_SYSNOTI,
				SIGNAL_CHARGEERR_RESPONSE,
				NULL, NULL) < 0)
		_E("Failed to send signal for error popup button");

	object_cleanup(data);
	popup_terminate();
}

static void lowbatt_shutdown_cb(void *data, Evas_Object *obj, void *event_info)
{
	int ret;

	_D("Device shutdown");
	object_cleanup(data);

	ret = device_poweroff();
	if (ret < 0)
		_E("Failed to power off (%d)", ret);

	popup_terminate();
}

static int load_charge_error_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	if (hall_ic == 0)
		return -ECANCELED;

	evas_object_show(ad->win_main);

	ad->popup = load_normal_popup(ad,
			_(CHARGE_ERR_TITLE),
			_(CHARGE_ERR_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbatt_error_response,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int load_charge_error_low_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	if (hall_ic == 0)
		return -ECANCELED;

	reset_window_priority(ad->win_main, 2);
	evas_object_show(ad->win_main);

	ad->popup = load_normal_popup(ad,
			_(CHARGE_ERR_LOW_TITLE),
			_(CHARGE_ERR_LOW_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbatt_error_response,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int load_charge_error_high_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	if (hall_ic == 0)
		return -ECANCELED;

	reset_window_priority(ad->win_main, 2);
	evas_object_show(ad->win_main);

	ad->popup = load_normal_popup(ad,
			_(CHARGE_ERR_HIGH_TITLE),
			_(CHARGE_ERR_HIGH_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbatt_error_response,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int load_charge_error_ovp_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	if (hall_ic == 0)
		return -ECANCELED;

	evas_object_show(ad->win_main);

	ad->popup = load_normal_popup(ad,
			_(CHARGE_ERR_OVP_TITLE),
			_(CHARGE_ERR_OVP_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbatt_timeout_func,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int load_battery_disconnected_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	if (hall_ic == 0)
		return -ECANCELED;

	reset_window_priority(ad->win_main, 2);
	evas_object_show(ad->win_main);

	ad->popup = load_normal_popup(ad,
			_(BATT_DISCONNECTED_TITLE),
			_(BATT_DISCONNECTED_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbatt_timeout_func,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int scover_show_warning_popup(struct appdata *ad)
{
	Evas_Object *conform = NULL;
	Evas_Object *ly = NULL;
	Evas_Object *ly_content = NULL;

	if (!ad)
		return -EINVAL;

	conform = elm_conformant_add(ad->win_main);
	if (!conform) {
		_E("Failed to add conformant");
		return -ENOMEM;
	}

	/* TODO Show indicator */
	/*elm_win_indicator_mode_set(ad->win_main, ELM_WIN_INDICATOR_SHOW);*/
	elm_win_resize_object_add(ad->win_main, conform);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	ly = elm_layout_add(conform);
	if (!ly) {
		_E("Failed to add layout");
		evas_object_del(conform);
		return -ENOMEM;
	}

	elm_layout_file_set(ly, EDJ_NAME, "scover-main");
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ly);

	elm_object_content_set(conform, ly);

	ly_content = elm_layout_add(conform);
	if (!ly_content) {
		_E("Failed to add layout");
		evas_object_del(conform);
		return -ENOMEM;
	}

	elm_layout_file_set(ly_content, EDJ_NAME, "scover-home");
	evas_object_size_hint_weight_set(ly_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ly_content);

	elm_object_part_content_set(ly, "scover.sw.content", ly_content);
	evas_object_show(ly_content);

	ad->popup = conform;
	evas_object_show(ad->popup);

	return 0;
}

static int load_battery_warning_popup(struct appdata *ad)
{
	char *content, *title;

	if (!ad)
		return -EINVAL;

	if (get_charge_state()) {
		title = NULL;
		content = _(LOWBATT_WARNING_CHARGE_NOW_CONTENT);
	} else {
		title = _(LOWBATT_WARNING_TITLE);
		content = _(LOWBATT_WARNING_CONTENT);
	}

	ad->popup = load_normal_popup(ad,
			title,
			content,
			_("IDS_COM_SK_OK"),
			lowbatt_timeout_func,
			NULL,
			NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int load_battery_shutdown_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	ad->popup = load_normal_popup(ad,
			_(LOWBATT_POWEROFF_TITLE),
			_(LOWBATT_POWEROFF_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbatt_shutdown_cb,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int show_low_battery_popup(struct appdata *ad)
{
	int ret ;

	if (!ad)
		return -EINVAL;

	if (hall_ic == 0) { /* S-cover is closed */
		reset_window_priority(ad->win_main, 2);
		ret = scover_show_warning_popup(ad);

	} else { /* S-cover is opened */
		reset_window_priority(ad->win_main, 1);
		switch (type) {
		case LOWBAT_WARNING:
			ret = load_battery_warning_popup(ad);
			break;
		case LOWBAT_POWEROFF:
			ret = load_battery_shutdown_popup(ad);
			break;
		default:
			return -EINVAL;
		}
	}

	if (ret < 0)
		_E("Failed to show los battery popup (%d)", ret);
	else {
		/* If there is no sleep, Volume app will be on the popup */
		usleep(500000);
		evas_object_show(ad->win_main);
	}
	return ret;
}

static int load_low_battery_popup(struct appdata *ad)
{
	int ret, status;

	if (!ad)
		return -EINVAL;

	register_charger_status_handler();
	register_battery_status_handler();

	if (vconf_get_int(VCONFKEY_SYSMAN_CHARGER_STATUS, &status) == 0
			&& status == VCONFKEY_SYSMAN_CHARGER_CONNECTED) {
		_I("Charger is connected so that lowbat popup is terminated");
		return -ECANCELED;
	}

	ret = register_edbus_signal_handler(ad);
	if (ret < 0) {
		_E("Failed to register edbus signal handler(%d)", ret);
		return ret;
	}

	ret = show_low_battery_popup(ad);
	if (ret < 0)
		_E("Failed to show low battery popup(%d)", ret);

	return ret;
}

static void hall_state_changed(void *data, DBusMessage *msg)
{
	DBusError err;
	int state, ret;
	struct appdata *ad = data;

	if (!ad)
		return;

	if (dbus_message_is_signal(msg, DEVICED_IFACE_HALL, HALL_STATE_SIGNAL) == 0)
		return;

	dbus_error_init(&err);
	if (dbus_message_get_args(msg, &err,
				DBUS_TYPE_INT32, &state,
				DBUS_TYPE_INVALID) == 0) {
		dbus_error_free(&err);
		return;
	}
	dbus_error_free(&err);

	_I("Hall state(%d)", state);

	if (hall_ic == state)
		return;

	hall_ic = state;
	release_evas_object(&(ad->popup));
	evas_object_hide(ad->win_main);

	ret = show_low_battery_popup(ad);
	if (ret < 0)
		_E("Failed to show low battery popup(%d)", ret);
}

static int load_low_battery_critical_popup(struct appdata *ad)
{
	type = LOWBAT_WARNING;
	return load_low_battery_popup(ad);
}

static int load_low_battery_exetreme_popup(struct appdata *ad)
{
	type = LOWBAT_WARNING;
	return load_low_battery_popup(ad);
}

static int load_low_battery_no_touch_popup(struct appdata *ad)
{
	return -ECANCELED;
}

static int handle_err_term_event(bundle *b, void *data)
{
	switch (type) {
	case LOWBAT_CHARGE_ERR:
	case LOWBAT_CHARGE_ERR_LOW:
	case LOWBAT_CHARGE_ERR_HIGH:
		lowbatt_error_response(data, NULL, NULL);
		break;
	default:
		break;
	}

	return 0;
}

/* App init */
int app_create(void *data)
{
	Evas_Object *win;
	struct appdata *ad = data;
	int ret;

	ad->handler.def_term_fn = handle_err_term_event;
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

	if (type == LOWBAT_WARNING
			|| type == LOWBAT_POWEROFF) {
		unregister_edbus_signal_handler();
		unregister_charger_status_handler();
		unregister_battery_status_handler();
			_E("Failed to release vconf key handler");
	}

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
	const char *opt;
	int ret, i;

	if (!ad || !b) {
		ret = -EINVAL;
		goto lowbatt_reset_out;
	}

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
		return 0;
	}

	opt = bundle_get_val(b, "_SYSPOPUP_CONTENT_");
	if (!opt) {
		_E("FAIL: bundle_get_val()");
		ret = -EINVAL;
		goto lowbatt_reset_out;
	}

	hall_ic = get_hallic_status();
	if (hall_ic < 0) {
		_E("Getting hall_ic status (%d)", hall_ic);
		hall_ic = 1; /* Cover is opened */
	}
	_I("Hall ic status(%d)", hall_ic);

	if (syspopup_create(b, &(ad->handler), ad->win_main, ad) < 0) {
		_E("FAIL: syspopup_create()");
		ret = -ENOMEM;
		goto lowbatt_reset_out;
	}

	syspopup_reset_timeout(b, -1);

	type = -1;
	for (i = 0 ; i < ARRAY_SIZE(lowbat_type) ; i++) {
		if (!strncmp(opt, lowbat_type[i].name, strlen(opt))) {
			type = lowbat_type[i].type;
			break;
		}
	}
	if (type < 0) {
		_E("Failed to get popup type(%d)", type);
		ret = -EINVAL;
		goto lowbatt_reset_out;
	}

	switch (type) {
	case LOWBAT_WARNING:
	case LOWBAT_POWEROFF:
		ret = load_low_battery_popup(ad);
		break;
	case LOWBAT_CRITICAL:
		ret = load_low_battery_critical_popup(ad);
		break;
	case LOWBAT_EXETREME:
		ret = load_low_battery_exetreme_popup(ad);
		break;
	case LOWBAT_NO_TOUCH:
		ret = load_low_battery_no_touch_popup(ad);
		break;
	case LOWBAT_CHARGE_ERR:
		ret = load_charge_error_popup(ad);
		break;
	case LOWBAT_CHARGE_ERR_LOW:
		ret = load_charge_error_low_popup(ad);
		break;
	case LOWBAT_CHARGE_ERR_HIGH:
		ret = load_charge_error_high_popup(ad);
		break;
	case LOWBAT_CHARGE_ERR_OVP:
		ret = load_charge_error_ovp_popup(ad);
		break;
	case LOWBAT_BATT_DISCONNECT:
		ret = load_battery_disconnected_popup(ad);
		break;
	default:
		_E("Unknown popup type (%d)", type);
		ret = -EINVAL;
		break;
	}
	if (ret < 0)
		goto lowbatt_reset_out;

	if (set_display_feedback(-1) < 0)
		_E("Failed to set display");

	play_notification_sound();

	return 0;

lowbatt_reset_out:
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

	int val = -1, ret = -1;

	ret = vconf_get_int(VCONFKEY_TESTMODE_LOW_BATT_POPUP, &val);
	if(ret == 0 && val == 1) {
		_D("Testmode without launching popup");
		return 0;
	}

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
