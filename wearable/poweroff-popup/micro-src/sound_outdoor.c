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
#include <dd-display.h>
#include <sound_manager.h>
#include "device-options.h"

#define SND_OD_ID                700
#define SND_OD_STYLE             "device_option.2icon.divider"

#define SOUND_ICON_SOUND         "micro-sound.png"
#define SOUND_ICON_VIBRATION     "micro-vibration.png"
#define SOUND_ICON_MUTE          "micro-mute.png"
#define SOUND_TEXT_SOUND         "IDS_ST_MBODY_SOUND_ABB"
#define SOUND_TEXT_VIBRATION     "IDS_COM_POP_VIBRATION"
#define SOUND_TEXT_MUTE          "IDS_ST_BODY_MUTE"

#define OUTDOOR_ON_ICON          "micro-outdooron.png"
#define OUTDOOR_OFF_ICON         "micro-outdooroff.png"
#define OUTDOOR_TEXT             "IDS_ST_BODY_OUTDOOR_MODE_ABB"
#define OUTDOOR_TIMEOUT          300

#define TOAST_TIMEOUT            3 /* need to confirm by UX guys */
#define DEFAULT_BRIGHTNESS       4
#define DEFAULT_RINGTONE_LEVEL   1
#define RETRY_MAX                5

#define DEVICED_PATH_DISPLAY     "/Org/Tizen/System/DeviceD/Display"
#define DEVICED_IFACE_DISPLAY    "org.tizen.system.deviced.display"
#define HBM_OFF_SIGNAL           "HBMOff"

#define VCONFKEY_LOW_BATTERY_EXTREME VCONFKEY_PM_KEY_IGNORE

static Elm_Genlist_Item_Class *snd_od_itc = NULL;

enum sound_status {
	SOUND,
	VIBRATION,
	MUTE,
};

static Evas_Object *sound_btn = NULL, *outdoor_btn = NULL;
static Evas_Object *toast_popup;
static bool outdoor_mode = false;
static Elm_Object_Item *gl_item = NULL;
static Ecore_Timer *toast_timer;
static Ecore_Event_Handler *mouse_up_handler = NULL;

static E_DBus_Connection *edbus_conn = NULL;
static E_DBus_Signal_Handler *outdoor_handler = NULL;

static void update_item(void);

static void hbm_state_changed(void *data, DBusMessage *msg)
{
	if (!msg)
		return;

	if (dbus_message_is_signal(msg, DEVICED_IFACE_DISPLAY, HBM_OFF_SIGNAL) == 0)
		return;

	update_item();
}

static void unregister_edbus_signal_handler(void)
{
	if (edbus_conn) {
		if (outdoor_handler) {
			e_dbus_signal_handler_del(edbus_conn, outdoor_handler);
			outdoor_handler = NULL;
		}
		e_dbus_connection_close(edbus_conn);
		edbus_conn = NULL;
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

	outdoor_handler = e_dbus_signal_handler_add(edbus_conn, NULL, DEVICED_PATH_DISPLAY,
			DEVICED_IFACE_DISPLAY, HBM_OFF_SIGNAL, hbm_state_changed, ad);
	if (!outdoor_handler) {
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

static void response_snd_od_clicked(void *data, Evas_Object * obj, void *event_info)
{
	_I("Sound and Outdoor is selected");
	return;
}

static int store_item(Elm_Object_Item *item)
{
	if (!item)
		return -EINVAL;
	gl_item = item;
	return 0;
}

static void update_item(void)
{
	if (gl_item)
		elm_genlist_item_update(gl_item);
}

static int get_snd_od_id(void)
{
	return SND_OD_ID;
}

static int get_sound_status(void)
{
	int snd, vib;

	if (vconf_get_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &snd) != 0)
		return -ENOMEM;

	if (snd == 1)
		return SOUND;

	if (vconf_get_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, &vib) != 0)
		return -ENOMEM;

	if (vib == 1)
		return VIBRATION;

	return MUTE;
}

static int get_sound_text(int status, char *text, int len)
{
	char *title;

	if (!text || len <= 0)
		return -EINVAL;

	switch(status) {
	case SOUND:
		title = SOUND_TEXT_SOUND;
		break;
	case VIBRATION:
		title = SOUND_TEXT_VIBRATION;
		break;
	case MUTE:
		title = SOUND_TEXT_MUTE;
		break;
	default:
		return -EINVAL;
	}
	snprintf(text, len, "%s", title);

	return 0;
}

static int get_sound_icon(int status, char *icon, int len)
{
	char *image;

	if (!icon || len <= 0)
		return -EINVAL;

	switch(status) {
	case SOUND:
		image = SOUND_ICON_SOUND;
		break;
	case VIBRATION:
		image = SOUND_ICON_VIBRATION;
		break;
	case MUTE:
		image = SOUND_ICON_MUTE;
		break;
	default:
		return -EINVAL;
	}
	snprintf(icon, len, "%s", image);

	return 0;
}

static int change_sound_status(void)
{
	int current = get_sound_status();
	int ret, ringtone;

	switch (current) {
	case SOUND: /* Change to Vibration */
		vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, 0);
		vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, 1);
		play_feedback(FEEDBACK_TYPE_VIBRATION,
				FEEDBACK_PATTERN_VIBRATION_ON);
		break;
	case VIBRATION: /* Change to Mute */
		vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, 0);
		vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, 0);
		break;
	case MUTE: /* Change to Sound */
		vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, 1);
		vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, 0);
		play_feedback(FEEDBACK_TYPE_SOUND,
				FEEDBACK_PATTERN_SILENT_OFF);
		ret = sound_manager_get_volume(SOUND_TYPE_RINGTONE, &ringtone);
		if (ret == SOUND_MANAGER_ERROR_NONE && ringtone == 0) {
			ret = sound_manager_set_volume(SOUND_TYPE_RINGTONE, DEFAULT_RINGTONE_LEVEL);
			if (ret != SOUND_MANAGER_ERROR_NONE)
				_E("Failed to set ringtone volume");
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static bool is_outdoor_mode_on(void)
{
	int ret;

	ret = display_get_hbm();
	_I("hbm: %d", ret);

	/* if failed, it returns original state */
	if (ret < 0)
		return outdoor_mode;

	outdoor_mode = ret;
	return outdoor_mode;
}

static int turn_on_outdoor_mode(void)
{
	return display_enable_hbm(OUTDOOR_ON, OUTDOOR_TIMEOUT);
}

static int turn_off_outdoor_mode(void)
{
	return display_enable_hbm(OUTDOOR_OFF, 0);
}

static void sound_clicked(void *data, Evas_Object * obj, void *event_info)
{
	_I("Sound clicked");

	if (change_sound_status() < 0)
		_E("Failed to change sound status");

	update_item();
}

static void unregister_toast_handlers(struct appdata *ad)
{
	if (toast_timer) {
		ecore_timer_del(toast_timer);
		toast_timer = NULL;
	}

	if (mouse_up_handler) {
		ecore_event_handler_del(mouse_up_handler);
		mouse_up_handler = NULL;
	}
}

static Eina_Bool mouse_up_response(void *data, int type, void *event)
{
	unregister_toast_handlers(data);
	release_evas_object(&toast_popup);

	return ECORE_CALLBACK_DONE;
}

static Eina_Bool outdoor_toast_timeout(void *data)
{
	unregister_toast_handlers(data);
	release_evas_object(&toast_popup);

	return ECORE_CALLBACK_CANCEL;
}

static void register_toast_handlers(struct appdata *ad)
{
	unregister_toast_handlers(ad);

	toast_timer = ecore_timer_add(TOAST_TIMEOUT, outdoor_toast_timeout, NULL);
	if (!toast_timer)
		_E("Failed to add timer");

	mouse_up_handler = ecore_event_handler_add(
			ECORE_EVENT_MOUSE_BUTTON_UP, mouse_up_response, ad);
	if (!mouse_up_handler)
		_E("Failed to register mouse up handler");

}

static int show_outdoor_toast(struct appdata *ad)
{
	char content[BUF_MAX];
	char *text;

	if (!ad)
		return -EINVAL;

	text = _("IDS_IDLE_POP_AFTER_P1SD_MINS_BRIGHTNESS_WILL_BE_RESET_TO_DEFAULT_LEVEL_HP2SD");

	snprintf(content, sizeof(content), text, OUTDOOR_TIMEOUT/60, DEFAULT_BRIGHTNESS);

	release_evas_object(&toast_popup);

	toast_popup = load_popup_toast(ad, content);
	if (!toast_popup) {
		_E("FAIL: load_popup_toast()");
		return -ENOMEM;
	}

	register_toast_handlers(ad);

	return 0;
}

static void outdoor_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata*)data;
	int state;

	_I("Outdoor clicked");

	if (vconf_get_int(VCONFKEY_LOW_BATTERY_EXTREME, &state) == 0
			&& state == 1) {
		update_item();
		return;
	}

	if (outdoor_mode) {
		if (turn_off_outdoor_mode() < 0)
			_E("Failed to turn off outdoor mode");
	} else {
		show_outdoor_toast(ad);
		if (turn_on_outdoor_mode() < 0)
			_E("Failed to turn on outdoor mode");
	}

	update_item();
}

static void register_snd_od_handlers(void *data)
{
	struct appdata *ad = data;
	if (register_edbus_signal_handler(ad) < 0)
		_E("Failed to register dbus signal");
}

static void unregister_snd_od_handlers(void *data)
{
	if (sound_btn) {
		evas_object_smart_callback_del(sound_btn, "clicked", sound_clicked);
		sound_btn = NULL;
	}

	if (outdoor_btn) {
		evas_object_smart_callback_del(outdoor_btn, "clicked", outdoor_clicked);
		outdoor_btn = NULL;
	}

	unregister_edbus_signal_handler();

	unregister_toast_handlers(data);
}

static Evas_Object *make_button(Evas_Object *obj, char *text, char *icon,
		void (*btn_clicked)(void *data, Evas_Object * obj, void *event_info), void *data)
{
	Evas_Object *btn = NULL;
	Evas_Object *ic = NULL;

	if (!obj || !text || !icon)
		return NULL;

	btn = elm_button_add(obj);
	if (!btn)
		goto out;
	elm_object_style_set(btn, "device_option");
	elm_object_text_set(btn, _(text));
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);

	ic = elm_image_add(btn);
	if (!ic)
		goto out;
	elm_image_file_set(ic, EDJ_NAME,  icon);
	evas_object_size_hint_min_set(ic, IMAGE_SIZE, IMAGE_MIN_SIZE);
	evas_object_size_hint_max_set(ic, IMAGE_SIZE, IMAGE_MIN_SIZE);
	elm_object_content_set(btn, ic);
	evas_object_propagate_events_set(btn, EINA_FALSE);

	evas_object_smart_callback_add(btn, "clicked", btn_clicked, data);

	return btn;
out:
	if (btn)
		evas_object_del(btn);
	return NULL;
}

static Evas_Object *get_snd_od_icon(void *data, Evas_Object *obj, const char *part)
{
	struct appdata *ad = data;
	int status;
	char icon[BUF_MAX], text[BUF_MAX];

	if (!ad)
		return NULL;

	/* Sound */
	if (!strcmp(part, "elm.icon")) {
		status = get_sound_status();
		if (get_sound_icon(status, icon, sizeof(icon)) < 0)
			return NULL;
		if (get_sound_text(status, text, sizeof(text)) < 0)
			return NULL;
		sound_btn = make_button(obj, text, icon, sound_clicked, ad);
		return sound_btn;
	}

	/* Outdoor */
	if (!strcmp(part, "elm.icon.1")) {
		if (is_outdoor_mode_on()) {
			snprintf(icon, sizeof(icon), "%s", OUTDOOR_ON_ICON);
		} else {
			snprintf(icon, sizeof(icon), "%s", OUTDOOR_OFF_ICON);
		}

		outdoor_btn = make_button(obj, OUTDOOR_TEXT, icon, outdoor_clicked, ad);
		return outdoor_btn;
	}

	return NULL;
}

static bool is_snd_od_enabled(void)
{
	return true;
}

static int init_snd_od_itc(void)
{
	snd_od_itc = elm_genlist_item_class_new();
	if (!snd_od_itc)
		return -ENOMEM;
	return 0;
}

static void deinit_snd_od_itc(void)
{
	if (snd_od_itc) {
		elm_genlist_item_class_free(snd_od_itc);
		snd_od_itc = NULL;
	}
}

static int get_snd_od_itc(Elm_Genlist_Item_Class **itc)
{
	snd_od_itc->item_style = SND_OD_STYLE;
	snd_od_itc->func.text_get = NULL;
	snd_od_itc->func.content_get = get_snd_od_icon;
	*itc = snd_od_itc;
	return 0;
}

static const struct device_option snd_od_ops = {
	.name                = "snd_od",
	.get_id              = get_snd_od_id,
	.is_enabled          = is_snd_od_enabled,
	.init_itc            = init_snd_od_itc,
	.deinit_itc          = deinit_snd_od_itc,
	.get_itc             = get_snd_od_itc,
	.store_item          = store_item,
	.response_clicked    = response_snd_od_clicked,
	.register_handlers   = register_snd_od_handlers,
	.unregister_handlers = unregister_snd_od_handlers
};

/* Constructor to register snd_od item */
static __attribute__ ((constructor)) void register_device_option_snd_od(void)
{
	register_option(&snd_od_ops);
}
