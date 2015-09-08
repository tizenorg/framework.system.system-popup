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
#include <vconf.h>
#include <efl_assist.h>
#include <sound_manager.h>
#include <wav_player.h>
#include "micro-common.h"
#include "launcher.h"

#define DEVICED_PATH_SYSNOTI        "/Org/Tizen/System/DeviceD/SysNoti"
#define DEVICED_INTERFACE_SYSNOTI   "org.tizen.system.deviced.SysNoti"
#define SIGNAL_CHARGEERR_RESPONSE   "ChargeErrResponse"

#define DEVICED_PATH_BATTERY   "/Org/Tizen/System/DeviceD/Battery"
#define DEVICED_INTERFACE_BATTERY  "org.tizen.system.deviced.Battery"
#define SIGNAL_TEMP_GOOD   "TempGood"

#define SIGNAL_LOWBAT_EXTREME       "Extreme"
#define SIGNAL_LOWBAT_NOT_EXTREME   "NotExtreme"

#define BUF_MAX 256
#define LOWBAT_SOUND_PATH          "/usr/share/feedback/sound/operation/low_battery.ogg"

#define LOWBATT_WARNING_TITLE      "IDS_COM_HEADER_BATTERY_LOW_ABB"
#define LOWBATT_WARNING_CONTENT    "IDS_ST_POP_THE_BATTERY_POWER_IS_LOW_RECHARGE_YOUR_GEAR_SOON_TO_KEEP_USING_IT"
#define LOWBATT_POWEROFF_TITLE     "IDS_ST_BODY_LEDOT_LOW_BATTERY"
#define LOWBATT_POWEROFF_CONTENT   "IDS_VR_POP_BATTERY_CRITICALLY_LOW_DEVICE_WILL_NOW_SHUT_DOWN"
#define LOWBATT_CRITICAL_TITLE     "IDS_COM_HEADER_BATTERY_CRITICALLY_LOW"
#define LOWBATT_CRITICAL_CONTENT   "IDS_ST_POP_THE_BATTERY_POWER_IS_CRITICALLY_LOW_RECHARGE_YOUR_GEAR_NOW_TO_KEEP_USING_IT"
#define LOWBATT_EXTREME_TITLE      LOWBATT_CRITICAL_TITLE
#define LOWBATT_EXTREME_CONTENT    "IDS_ST_POP_THE_BATTERY_POWER_IS_CRITICALLY_LOW_MSG"
#define LOWBATT_NO_TOUCH_TITLE     LOWBATT_CRITICAL_TITLE
#define LOWBATT_NO_TOUCH_CONTENT   "IDS_ST_POP_THE_BATTERY_POWER_IS_CRITICALLY_LOW_YOU_CAN_ONLY_USE_THE_DEFAULT_CLOCK_RECHARGE_YOUR_GEAR_NOW_TO_KEEP_USING_IT"
#define CHARGE_ERR_TITLE           "IDS_COM_POP_ERROR"
#define CHARGE_ERR_CONTENT         "IDS_COM_POP_CHARGING_PAUSED_BATTERY_TEMPERATURE_TOO_HIGH_OR_LOW"
#define CHARGE_ERR_LOW_TITLE       CHARGE_ERR_TITLE
#define CHARGE_ERR_LOW_CONTENT     "IDS_QP_BODY_CHARGING_PAUSED_BATTERY_TEMPERATURE_TOO_LOW"
#define CHARGE_ERR_HIGH_TITLE      CHARGE_ERR_TITLE
#define CHARGE_ERR_HIGH_CONTENT    "IDS_QP_BODY_CHARGING_PAUSED_BATTERY_TEMPERATURE_TOO_HIGH"
#define CHARGE_ERR_OVP_TITLE       CHARGE_ERR_TITLE
#define CHARGE_ERR_OVP_CONTENT     "IDS_COM_POP_CHARGING_PAUSED_VOLTAGE_TOO_HIGH"
#define BATT_DISCONNECTED_TITLE    CHARGE_ERR_TITLE
#define BATT_DISCONNECTED_CONTENT  "IDS_COM_POP_BATTERY_DISCONNECTED_ABB"

#define DBUS_PATH_HOME_RAISE  "/Org/Tizen/Coreapps/home/raise"
#define DBUS_IFACE_HOME_RAISE "org.tizen.coreapps.home.raise"
#define HOME_RAISE_SIGNAL     "homeraise"

#define VCONFKEY_DO_NOT_DISTURB "memory/shealth/sleep/do_not_disturb"

#define EDJ_PATH "/usr/apps/org.tizen.lowbat-syspopup/res/edje/lowbatt"
#define EDJ_NAME EDJ_PATH"/lowbatt.edj"

#ifdef SYSTEM_APPS_MICRO_3
#define LAYOUT_STYLE "micro_3_title_content_button"
#else
#define LAYOUT_STYLE NULL
#endif

enum lowbat_options {
	LOWBAT_NONE,
	LOWBAT_WARNING,
	LOWBAT_POWEROFF,
	LOWBAT_CRITICAL,
	LOWBAT_EXTREME,
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
	{ "extreme"           , LOWBAT_EXTREME            },
	{ "notouch"           , LOWBAT_NO_TOUCH           },
	{ "chargeerr"         , LOWBAT_CHARGE_ERR         },
	{ "chargeerrlow"      , LOWBAT_CHARGE_ERR_LOW     },
	{ "chargeerrhigh"     , LOWBAT_CHARGE_ERR_HIGH    },
	{ "chargeerrovp"      , LOWBAT_CHARGE_ERR_OVP     },
	{ "battdisconnect"    , LOWBAT_BATT_DISCONNECT    },
};

enum lowbat_threshold {
	THRESHOLD_WARNING  = VCONFKEY_SYSMAN_BAT_WARNING_LOW, /* Below 15% */
	THRESHOLD_CRITICAL = VCONFKEY_SYSMAN_BAT_CRITICAL_LOW, /* Below 5% */
	THRESHOLD_EXTREME  = VCONFKEY_SYSMAN_BAT_POWER_OFF, /* Below 3% */
};

enum lowbat_extreme_state {
	LOWBAT_STATE_UNKNOWN     = -1,
	LOWBAT_STATE_NOT_EXTREME = 0,
	LOWBAT_STATE_EXTREME     = 1,
};

static int type = LOWBAT_NONE;
static Ecore_Event_Handler *mouse_up_handler = NULL;
static E_DBus_Signal_Handler *powerkey_handler = NULL;

static E_DBus_Signal_Handler *temp_handler = NULL;

static Eina_Bool mouse_up_response(void *data, int type, void *event);
static void unregister_main_handlers(struct appdata *ad);
static void register_mouse_up_handler(struct appdata *ad);

static bool (*change_sound_to_vibration)(void) = NULL;

void register_sound_to_vibration(bool (*func)(void))
{
	change_sound_to_vibration = func;
}

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

static bool get_do_not_disturb_state(void)
{
	int state, blockmode;

	if (vconf_get_int(VCONFKEY_DO_NOT_DISTURB, &state) == 0
			&& state == 1) {
		_I("Do not disturb on");
		return true;
	}

	if (vconf_get_bool(VCONFKEY_SETAPPL_BLOCKMODE_WEARABLE_BOOL, &blockmode) == 0
			&& blockmode == 1) {
		_I("Do not disturb on");
		return true;
	}

	_I("Do not disturb off");
	return false;
}

static void play_notification_feedback(void)
{
	if (get_do_not_disturb_state())
		return;
	play_feedback(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_LOWBATT);
}

static void play_silence(void)
{
	if (!change_sound_to_vibration)
		return;
	if (change_sound_to_vibration())
		play_notification_feedback();
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

	if (get_call_state() || get_voice_recording_state()) {
		play_silence();
		return;
	}

	if (get_do_not_disturb_state())
		return;

	if (!session) {
		ret = sound_manager_set_session_type(SOUND_SESSION_TYPE_NOTIFICATION);
		if (ret != SOUND_MANAGER_ERROR_NONE) {
			_E("Failed to set session to play sound(%d)", ret);
			return;
		}
		session = true;
	}

	if (access(LOWBAT_SOUND_PATH, F_OK) != 0) {
		_E("The sound file does not exist (%d)", LOWBAT_SOUND_PATH);
		return;
	}


	ret = wav_player_start(LOWBAT_SOUND_PATH, SOUND_TYPE_NOTIFICATION, NULL, NULL, NULL);
	if (ret != WAV_PLAYER_ERROR_NONE)
		_E("Failed to play sound file (%d, %s)", ret, LOWBAT_SOUND_PATH);
}

static void send_lowbat_extreme_signal(int extreme)
{
	static int state = LOWBAT_STATE_UNKNOWN;
	char *signal;

	if (state == extreme)
		return;

	switch (extreme) {
	case LOWBAT_STATE_NOT_EXTREME:
		signal = SIGNAL_LOWBAT_NOT_EXTREME;
		break;
	case LOWBAT_STATE_EXTREME:
		signal = SIGNAL_LOWBAT_EXTREME;
		break;
	default:
		_E("Unknown extreme status(%d)", extreme);
		return;
	}

	state = extreme;

	if (broadcast_dbus_signal(POPUP_PATH_LOWBAT,
				POPUP_IFACE_LOWBAT,
				signal, NULL, NULL) < 0)
		_E("Failed to send signal for battery extremely low");
}

static void show_invisible_layer(struct appdata *ad)
{
	if (!ad)
		return;

	type = LOWBAT_NONE;

	unregister_main_handlers(ad);
	release_evas_object(&(ad->popup));
	elm_win_role_set(ad->win_main, "no-dim");

	register_mouse_up_handler(ad);

	send_lowbat_extreme_signal(LOWBAT_STATE_EXTREME);
}

static void event_back_key_up(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	if (type == LOWBAT_NO_TOUCH)
		show_invisible_layer(ad);
	else
		popup_terminate();
}

static void register_main_handlers(struct appdata *ad)
{
	if (ad && ad->win_main)
		ea_object_event_callback_add(ad->win_main, EA_CALLBACK_BACK, event_back_key_up, ad);
}

static void unregister_main_handlers(struct appdata *ad)
{
	if (ad && ad->win_main)
		ea_object_event_callback_del(ad->win_main, EA_CALLBACK_BACK, event_back_key_up);
}

static bool get_charge_state(void)
{
	int val;
	if (vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, &val) == 0
			&& val == 1)
		return true;
	return false;
}

static void lowbat_terminate_popup(void *data, Evas_Object *obj, void *event_info)
{
	_I("Terminate low battery popup");
	object_cleanup(data);
	popup_terminate();
}

static void lowbatt_error_response(void *data, Evas_Object *obj, void *event_info)
{
	static bool terminate = false;
	if (terminate)
		return;
	terminate = true;
	_I("Terminate battery error popup");
	if (broadcast_dbus_signal(DEVICED_PATH_SYSNOTI,
				DEVICED_INTERFACE_SYSNOTI,
				SIGNAL_CHARGEERR_RESPONSE,
				NULL, NULL) < 0)
		_E("Failed to send signal for error popup button");

	object_cleanup(data);
	popup_terminate();
}

static Evas_Object *load_popup(struct appdata *ad,
		char *title, char *content,
		char *lbtn_text,
		void (*lbtn)(void *data, Evas_Object *obj, void *event_info),
		char *rbtn_text,
		void (*rbtn)(void *data, Evas_Object *obj, void *event_info))
{
#ifdef SYSTEM_APPS_CIRCLE
	char *licon, *ricon;
	if (rbtn) {
		licon = "circle-cancel.png";
		ricon = "circle-ok.png";
	} else {
		licon = "circle-ok.png";
		ricon = NULL;
	}

	return load_normal_popup(ad,
			EDJ_NAME,
			title, content,
			lbtn_text, licon, lbtn,
			rbtn_text, ricon, rbtn);
#else
	return load_normal_popup(ad,
			title, content,
			lbtn_text, lbtn,
			rbtn_text, rbtn);
#endif
}

static int load_charge_error_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	evas_object_show(ad->win_main);

	ad->popup = load_popup(ad,
			_(CHARGE_ERR_TITLE),
			_(CHARGE_ERR_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbatt_error_response,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_popup()");
		return -ENOMEM;
	}

	play_notification_feedback();
	play_notification_sound();

	return 0;
}

static void unregister_edbus_signal_handler(void)
{
	static E_DBus_Connection *conn = NULL;

	conn = get_dbus_connection();
	if (conn && temp_handler) {
		e_dbus_signal_handler_del(conn, temp_handler);
		temp_handler = NULL;
	}
}

static void temp_status_changed(void *data, DBusMessage *msg)
{
	int r;

	_I("edbus signal Received");

	r = dbus_message_is_signal(msg, DEVICED_INTERFACE_BATTERY, SIGNAL_TEMP_GOOD);
	if (!r) {
		_E("dbus_message_is_signal error");
		return;
	}

	unregister_edbus_signal_handler();

	object_cleanup(data);
	popup_terminate();
	_I("%s - %s", DEVICED_INTERFACE_BATTERY, SIGNAL_TEMP_GOOD);
}

static int register_edbus_signal_handler(struct appdata *ad)
{
	int ret;
	static E_DBus_Connection *conn = NULL;

	conn = get_dbus_connection();
	if (!conn) {
		_E("Failed to get dbus connection");
		return -ENOMEM;
	}

	temp_handler = e_dbus_signal_handler_add(conn, NULL, DEVICED_PATH_BATTERY,
			DEVICED_INTERFACE_BATTERY, SIGNAL_TEMP_GOOD, temp_status_changed, ad);
	if (!temp_handler) {
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

static int load_charge_error_low_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	reset_window_priority(ad->win_main, 2);
	evas_object_show(ad->win_main);

	ad->popup = load_popup(ad,
			_(CHARGE_ERR_LOW_TITLE),
			_(CHARGE_ERR_LOW_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbatt_error_response,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_popup()");
		return -ENOMEM;
	}

	play_notification_feedback();
	play_notification_sound();

	register_edbus_signal_handler(ad);
	return 0;
}

static int load_charge_error_high_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	reset_window_priority(ad->win_main, 2);
	evas_object_show(ad->win_main);

	ad->popup = load_popup(ad,
			_(CHARGE_ERR_HIGH_TITLE),
			_(CHARGE_ERR_HIGH_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbatt_error_response,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_popup()");
		return -ENOMEM;
	}

	play_notification_feedback();
	play_notification_sound();

	register_edbus_signal_handler(ad);
	return 0;
}

static void charger_changed(keynode_t *key, void *data)
{
	int status;

	status = vconf_keynode_get_int(key);
	if (status != VCONFKEY_SYSMAN_CHARGER_DISCONNECTED)
		return;

	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
				charger_changed) < 0)
		_E("Failed to release vconf key handler");
	object_cleanup(data);
	popup_terminate();
}

static void charge_now_changed(keynode_t *key, void *data)
{
	int status;

	status = vconf_keynode_get_int(key);
	if (status != 1)
		return;

	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW,
				charge_now_changed) < 0)
		_E("Failed to release vconf key handler");
	object_cleanup(data);
	popup_terminate();
}

static int load_charge_error_ovp_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	evas_object_show(ad->win_main);

	ad->popup = load_popup(ad,
			_(CHARGE_ERR_OVP_TITLE),
			_(CHARGE_ERR_OVP_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbat_terminate_popup,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_popup()");
		return -ENOMEM;
	}

	play_notification_feedback();
	play_notification_sound();

	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
			charger_changed, ad) < 0)
		_E("Failed to register vconf key handler");
	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW,
			charge_now_changed, ad) < 0)
		_E("Failed to register vconf key handler");
	return 0;
}

static int load_battery_disconnected_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	reset_window_priority(ad->win_main, 2);
	evas_object_show(ad->win_main);

	ad->popup = load_popup(ad,
			_(BATT_DISCONNECTED_TITLE),
			_(BATT_DISCONNECTED_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbat_terminate_popup,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_popup()");
		return -ENOMEM;
	}

	play_notification_feedback();
	play_notification_sound();

	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_CHARGER_STATUS,
			charger_changed, ad) < 0)
		_E("Failed to register vconf key handler");
	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW,
			charge_now_changed, ad) < 0)
		_E("Failed to register vconf key handler");
	return 0;
}

static int load_low_battery_warning_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	type = LOWBAT_WARNING;

	evas_object_show(ad->win_main);

	ad->popup = load_popup(ad,
			NULL,
			_(LOWBATT_WARNING_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbat_terminate_popup,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_popup()");
		return -ENOMEM;
	}

	send_lowbat_extreme_signal(LOWBAT_STATE_NOT_EXTREME);

	play_notification_feedback();
	play_notification_sound();

	return 0;
}

static void unregister_mouse_up_handler(struct appdata *ad)
{
	if (mouse_up_handler) {
		ecore_event_handler_del(mouse_up_handler);
		mouse_up_handler = NULL;
	}
}

static void register_mouse_up_handler(struct appdata *ad)
{
	unregister_mouse_up_handler(ad);
	mouse_up_handler = ecore_event_handler_add(
			ECORE_EVENT_MOUSE_BUTTON_UP, mouse_up_response, ad);
	if (!mouse_up_handler)
		_E("Failed to register mouse up handler");
}

static void lowbatt_extreme_response(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	show_invisible_layer(ad);
}

static int load_low_battery_critical_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	type = LOWBAT_CRITICAL;

	evas_object_show(ad->win_main);

	ad->popup = load_popup(ad,
			NULL,
			_(LOWBATT_CRITICAL_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbat_terminate_popup,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_popup()");
		return -ENOMEM;
	}

	send_lowbat_extreme_signal(LOWBAT_STATE_NOT_EXTREME);

	play_notification_feedback();
	play_notification_sound();

	return 0;
}

static int load_low_battery_extreme_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	type = LOWBAT_EXTREME;

	evas_object_show(ad->win_main);

	ad->popup = load_popup(ad,
			NULL,
			_(LOWBATT_EXTREME_CONTENT),
			_("IDS_COM_SK_CANCEL"),
			lowbat_terminate_popup,
			_("IDS_COM_SK_OK"),
			lowbatt_extreme_response);
	if (!(ad->popup)) {
		_E("FAIL: load_popup()");
		return -ENOMEM;
	}

	send_lowbat_extreme_signal(LOWBAT_STATE_NOT_EXTREME);

	play_notification_feedback();
	play_notification_sound();

	return 0;
}

static int load_low_battery_no_touch_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	type = LOWBAT_NO_TOUCH;

	evas_object_show(ad->win_main);

	ad->popup = load_popup(ad,
			NULL,
			_(LOWBATT_NO_TOUCH_CONTENT),
			_("IDS_COM_SK_OK"),
			lowbatt_extreme_response,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_popup()");
		return -ENOMEM;
	}

	send_lowbat_extreme_signal(LOWBAT_STATE_EXTREME);

	play_notification_feedback();
	play_notification_sound();

	return 0;
}

static Eina_Bool mouse_up_response(void *data, int e_type, void *event)
{
	struct appdata *ad = data;
	int ret;

	if (type != LOWBAT_NONE)
		return ECORE_CALLBACK_PASS_ON;

	register_main_handlers(ad);
	unregister_mouse_up_handler(ad);

	ret = load_low_battery_no_touch_popup(ad);
	if (ret < 0) {
		_E("Failed to load no touch popup");
		popup_terminate();
	}

	return ECORE_CALLBACK_DONE;
}

static int get_low_battery_status(void)
{
	int status;
	if (vconf_get_int(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW, &status) != 0)
		return -ENOMEM;
	return status;
}

static int load_low_battery_popup(struct appdata *ad)
{
	int status;

	if (!ad)
		return -EINVAL;

	if (get_charge_state()) {
		popup_terminate();
		return 0;
	}

	status = get_low_battery_status();

	switch (status) {
	case THRESHOLD_EXTREME:
		if (type == LOWBAT_EXTREME || type == LOWBAT_POWEROFF)
			goto same_popup_out;

		return load_low_battery_extreme_popup(ad);

	case THRESHOLD_CRITICAL:
		if (type == LOWBAT_CRITICAL)
			goto same_popup_out;

		return load_low_battery_critical_popup(ad);

	case THRESHOLD_WARNING:
		if (type == LOWBAT_WARNING)
			goto same_popup_out;

		return load_low_battery_warning_popup(ad);

	default:
		_I("Low battery status: (%d)", status);
		popup_terminate();
		return 0;
	}

same_popup_out:
	_I("Low battery status is same with previous status (%d)", type);
	return 0;
}

static void low_battery_status_changed(keynode_t *key, void *data)
{
	int ret;
	struct appdata *ad = data;

	if (!ad)
		return;

	unregister_main_handlers(ad);
	register_main_handlers(ad);

	release_evas_object(&(ad->popup));

	ret = load_low_battery_popup(ad);
	if (ret < 0) {
		_E("Failed to load low battery popup (%d)", ret);
		popup_terminate();
	}
}

static void unregister_low_battery_status_handler(void)
{
	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW,
			low_battery_status_changed) < 0)
		_E("Falied to release vconf key handler");
}

static void register_low_battery_status_handler(struct appdata *ad)
{
	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW,
			low_battery_status_changed, ad) < 0)
		_E("Failed to register vconf key handler");
}

static void charger_status_changed(keynode_t *key, void *data)
{
	/* Charging: 1, not charging: 0 */
	if (vconf_keynode_get_int(key) == 1) {
		popup_terminate();
		return;
	}
}

static void unregister_charger_status_handler(void)
{
	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW,
			charger_status_changed) < 0)
		_E("Falied to release vconf key handler");
}

static void register_charger_status_handler(struct appdata *ad)
{
	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW,
			charger_status_changed, ad) < 0)
		_E("Failed to register vconf key handler");
}

static void powerkey_pushed(void *data, DBusMessage *msg)
{
	int ret;
	struct appdata *ad = data;

	ret = dbus_message_is_signal(msg, DBUS_IFACE_HOME_RAISE, HOME_RAISE_SIGNAL);
	if (ret == 0)
		return;

	switch (type) {
	case LOWBAT_NONE:
		return;
	case LOWBAT_NO_TOUCH:
		show_invisible_layer(ad);
		return;
	default:
		break;
	}

	popup_terminate();
}

static void unregister_power_key_handler(void)
{
	E_DBus_Connection *conn;

	conn = get_dbus_connection();
	if (conn && powerkey_handler) {
		e_dbus_signal_handler_del(conn, powerkey_handler);
		powerkey_handler = NULL;
	}
}

static int register_power_key_handler(struct appdata *ad)
{
	E_DBus_Connection *conn;
	int ret;

	conn = get_dbus_connection();
	if (!conn) {
		_E("Failed to get dbus connection");
		ret = -ENOMEM;
		goto out;
	}

	powerkey_handler = e_dbus_signal_handler_add(conn, NULL, DBUS_PATH_HOME_RAISE,
			DBUS_IFACE_HOME_RAISE, HOME_RAISE_SIGNAL, powerkey_pushed, ad);
	if (!powerkey_handler) {
		_E("Failed to register handler");
		ret = -ENOMEM;
		goto out;
	}

	ret = 0;

out:
	if (ret < 0)
		unregister_power_key_handler();
	return ret;
}

/* App init */
int app_create(void *data)
{
	Evas_Object *win;
	struct appdata *ad = data;
	int ret;

	/* create window */
	win = create_win(PACKAGE);
	if (win == NULL)
		return -1;

	ad->win_main = win;

	ecore_x_netwm_window_type_set(elm_win_xwindow_get(ad->win_main),
			ECORE_X_WINDOW_TYPE_NOTIFICATION);

	elm_theme_overlay_add(NULL,EDJ_NAME);

	ret = appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR);
	if (ret != 0)
		_E("FAIL: appcore_set_i18n()");

	ret = set_dbus_connection();
	if (ret < 0)
		_E("Failed to set dbus connection(%d)", ret);

	ret = register_power_key_handler(ad);
	if (ret < 0)
		_E("Failed to register power key handler(%d)", ret);

	return 0;
}

/* Terminate noti handler */
static int app_terminate(void *data)
{
	struct appdata *ad = data;

	switch (type) {
	case LOWBAT_NONE:
	case LOWBAT_WARNING:
	case LOWBAT_POWEROFF:
	case LOWBAT_CRITICAL:
	case LOWBAT_EXTREME:
	case LOWBAT_NO_TOUCH:
		unregister_charger_status_handler();
		unregister_low_battery_status_handler();
		send_lowbat_extreme_signal(LOWBAT_STATE_NOT_EXTREME);
		break;
	case LOWBAT_CHARGE_ERR:
	case LOWBAT_CHARGE_ERR_LOW:
	case LOWBAT_CHARGE_ERR_HIGH:
		lowbatt_error_response(data, NULL, NULL);
		break;
	default:
		break;
	}

	unregister_mouse_up_handler(ad);
	unregister_main_handlers(ad);
	unregister_power_key_handler();
	unset_dbus_connection();

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

	if (ad->popup) {
		_I("low battery popup already exists");
		return 0;
	}

	opt = bundle_get_val(b, "_SYSPOPUP_CONTENT_");
	if (!opt) {
		_E("FAIL: bundle_get_val()");
		ret = -EINVAL;
		goto lowbatt_reset_out;
	}

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

	register_main_handlers(ad);

	switch (type) {
	case LOWBAT_WARNING:
	case LOWBAT_CRITICAL:
	case LOWBAT_POWEROFF:
	case LOWBAT_EXTREME:
		type = LOWBAT_NONE;
		register_low_battery_status_handler(ad);
		register_charger_status_handler(ad);
		ret = load_low_battery_popup(ad);
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

	return 0;

lowbatt_reset_out:
	popup_terminate();
	return ret;
}

int main(int argc, char *argv[])
{
	struct appdata ad;
	int val = -1, ret = -1;

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

	ret = vconf_get_int(VCONFKEY_TESTMODE_LOW_BATT_POPUP, &val);
	if(ret == 0 && val == 1) {
		_D("Testmode without launching popup");
		return 0;
	}
	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
