/*
 * system-popup
 * Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
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
 */

#include "core.h"
#include "common.h"

#define BRIGHTNESS_POPUP       "brightness"
#define BRIGHTNESS_FULL_ICON   "brightness_full.png"
#define BRIGHTNESS_EMPTY_ICON  "brightness_empty.png"

#define EDJ_PATH "/usr/apps/org.tizen.system-syspopup/res/edje/system"
#define EDJ_NAME EDJ_PATH"/system.edj"

#define BRIGHTNESS_MIN  0.0
#define BRIGHTNESS_MAX  100.0
#define TERM_TIMEOUT    10
#define RETRY_MAX       5

#define DEVICED_PATH_DISPLAY    "/Org/Tizen/System/DeviceD/Display"
#define DEVICED_IFACE_DISPLAY   "org.tizen.system.deviced.display"
#define DISPLAY_READY_SIGNAL    "BrightnessReady"
#define DISPLAY_CHANGED_SIGNAL  "BrightnessChanged"

static Evas_Object *vbar = NULL;
static Ecore_Timer *term_timer = NULL;
static E_DBus_Connection *edbus_conn = NULL;
static E_DBus_Signal_Handler *handler = NULL;

static int show_brightness_popup(void *data, bundle *b);
static Eina_Bool timer_expired(void *data);

struct popup_ops brightness_ops = {
	.name = BRIGHTNESS_POPUP,
	.popup = NULL,
	.show_popup = show_brightness_popup,
};

static int get_auto_brt(void)
{
	int state;

	if (vconf_get_int(VCONFKEY_SETAPPL_BRIGHTNESS_AUTOMATIC_INT, &state) != 0)
		state = 0;
	return state;
}

static int get_current_brt(void)
{
	int state;
	char *vconf_key;

	if (get_auto_brt() == 0)
		vconf_key = VCONFKEY_SETAPPL_LCD_BRIGHTNESS;
	else
		vconf_key = VCONFKEY_SETAPPL_LCD_AUTOMATIC_BRIGHTNESS;

	if (vconf_get_int(vconf_key, &state) == 0) {
		_E("brightness_state (%d)", state);
		return state;
	} else
		return 0;
}

static Evas_Object *make_brightness_ui(struct appdata *ad)
{
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *licon;
	Evas_Object *ricon;

	elm_theme_overlay_add(NULL,EDJ_NAME);

	popup = elm_popup_add(ad->win_main);
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_TOP);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_style_set(popup, "transparent");

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, EDJ_NAME, "popup_brightnessbar");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	vbar = elm_slider_add(popup);
	elm_slider_min_max_set(vbar, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
	elm_slider_value_set(vbar, get_current_brt());

	licon = elm_image_add(popup);
	elm_image_file_set(licon, EDJ_NAME, BRIGHTNESS_EMPTY_ICON);
	evas_object_size_hint_aspect_set(licon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(licon, EINA_FALSE, EINA_FALSE);
	elm_object_part_content_set(vbar, "icon", licon);

	ricon = elm_image_add(popup);
	elm_image_file_set(ricon, EDJ_NAME, BRIGHTNESS_FULL_ICON);
	evas_object_size_hint_aspect_set(ricon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ricon, EINA_FALSE, EINA_FALSE);

	elm_object_part_content_set(layout, "elm.swallow.content", vbar);
	elm_object_part_content_set(layout, "elm.swallow.icon", ricon);

	elm_object_content_set(popup, layout);
	evas_object_show(popup);

	elm_object_disabled_set(vbar, EINA_TRUE);

	return popup;
}

static void brightness_changed(void *data, DBusMessage *msg)
{
	DBusError err;
	int state;

	if (dbus_message_is_signal(msg, DEVICED_IFACE_DISPLAY, DISPLAY_CHANGED_SIGNAL) == 0)
		return;

	dbus_error_init(&err);
	if (dbus_message_get_args(msg, &err,
				DBUS_TYPE_INT32, &state,
				DBUS_TYPE_INVALID) == 0) {
		dbus_error_free(&err);
		return;
	}
	dbus_error_free(&err);

	_I("Brightness(%d)", state);
	elm_slider_value_set(vbar, state);

	if (term_timer) {
		ecore_timer_reset(term_timer);
	} else {
		term_timer = ecore_timer_add(TERM_TIMEOUT, timer_expired, NULL);
		if (!term_timer)
			_E("Failed to register timer");
	}
}

static void unregister_edbus_signal_handler(void)
{
	if (edbus_conn) {
		if (handler)
			e_dbus_signal_handler_del(edbus_conn, handler);
		e_dbus_connection_close(edbus_conn);
	}
	e_dbus_shutdown();
}

static int register_edbus_signal_handler(void)
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

	handler = e_dbus_signal_handler_add(edbus_conn, NULL, DEVICED_PATH_DISPLAY,
			DEVICED_IFACE_DISPLAY, DISPLAY_CHANGED_SIGNAL, brightness_changed, NULL);
	if (!handler) {
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

static Eina_Bool timer_expired(void *data)
{
	char *pa[1];
	char popup_off[2];

	unregister_edbus_signal_handler();

	if (term_timer) {
		ecore_timer_del(term_timer);
		term_timer = NULL;
	}

	snprintf(popup_off, sizeof(popup_off), "%d", 0);
	pa[0] = popup_off;
	if (broadcast_dbus_signal(DEVICED_PATH_DISPLAY,
			DEVICED_IFACE_DISPLAY,
			DISPLAY_READY_SIGNAL,
			"i", pa) < 0)
		_E("Failed to send ready signal to deviced");

	release_evas_object(&(brightness_ops.popup));
	terminate_if_no_popup();
	return ECORE_CALLBACK_CANCEL;
}

static int show_brightness_popup(void *data, bundle *b)
{
	struct appdata *ad = data;
	char *type;
	char *pa[1];
	char popup_on[2];

	if (!ad || !b)
		return -EINVAL;

	type = (char *)bundle_get_val(b, "_TYPE_");
	if (!type) {
		_E("Failed to get type");
		return -ENOMEM;
	}

	if (!strncmp(type, "terminate", strlen(type))) {
		timer_expired(NULL);
		return 0;
	}

	if (brightness_ops.popup)
		return 0;

	if (!(ad->win_main))
		return -EINVAL;

	if (register_edbus_signal_handler() != 0) {
		_E("Failed to init brightness popup");
		return -ENOMEM;
	}

	if (set_popup_focus(ad->win_main, false) < 0)
		_E("Failed to disable focus");
	elm_win_role_set(ad->win_main, "no-dim");
	evas_object_show(ad->win_main);

	brightness_ops.popup = make_brightness_ui(ad);

	if (set_display_feedback(-1) < 0)
		_E("Failed to set display and feedback");

	term_timer = ecore_timer_add(TERM_TIMEOUT, timer_expired, NULL);
	if (!term_timer)
		_E("Failed to register timer");

	snprintf(popup_on, sizeof(popup_on), "%d", 1);
	pa[0] = popup_on;
	if (broadcast_dbus_signal(DEVICED_PATH_DISPLAY,
			DEVICED_IFACE_DISPLAY,
			DISPLAY_READY_SIGNAL,
			"i", pa) < 0)
		_E("Failed to send ready signal to deviced");

	return 0;
}

static __attribute__ ((constructor)) void register_brightness_popup(void)
{
	register_popup(&brightness_ops);
}
