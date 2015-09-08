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

#include <E_DBus.h>
#include <pthread.h>
#include <dd-display.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "common.h"

#define RETRY_MAX 10
#define DBUS_REPLY_TIMEOUT  (-1)

/* Terminate popup */
static Eina_Bool exit_idler_cb(void *data)
{
	elm_exit();
	return ECORE_CALLBACK_CANCEL;
}

void popup_terminate(void)
{
	if (ecore_idler_add(exit_idler_cb, NULL))
		return;

	exit_idler_cb(NULL);
}

/* Release evas object */
void release_evas_object(Evas_Object **obj)
{
	if (!obj || !(*obj))
		return;
	evas_object_del(*obj);
	*obj = NULL;
}

void object_cleanup(struct appdata *ad)
{
	if (!ad)
		return;
	release_evas_object(&(ad->popup));
	release_evas_object(&(ad->layout_main));
	release_evas_object(&(ad->win_main));
}

/* Create main window */
static void win_del(void *data, Evas_Object * obj, void *event)
{
	popup_terminate();
}

Evas_Object *create_win(const char *name)
{
	Evas_Object *eo;
	Ecore_X_Window xwin;
	unsigned int val = 1;
	int w;
	int h;

	if (!name)
		return NULL;

	eo = elm_win_add(NULL, name, ELM_WIN_DIALOG_BASIC);
	if (!eo) {
		_E("FAIL: elm_win_add()");
		return NULL;
	}

	elm_win_title_set(eo, name);
	elm_win_borderless_set(eo, EINA_TRUE);
	elm_win_alpha_set(eo, EINA_TRUE);
	elm_win_raise(eo);
	evas_object_smart_callback_add(eo, "delete,request", win_del, NULL);
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
	evas_object_resize(eo, w, h);

	xwin = elm_win_xwindow_get(eo);
	ecore_x_window_prop_card32_set(xwin, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL, &val, 1);

	return eo;
}

Evas_Object *load_scrollable_popup(struct appdata *ad,
			char *title,
			char *content,
			char *lbtnText,
			Evas_Smart_Cb lbtn_cb,
			char *rbtnText,
			Evas_Smart_Cb rbtn_cb)
{
	Evas_Object *lbtn;
	Evas_Object *rbtn;
	Evas_Object *popup;
	Evas_Object *label;
	Evas_Object *scroller;
	char *text;

	if (!ad || !(ad->win_main) || !content)
		return NULL;

	evas_object_show(ad->win_main);
	popup = elm_popup_add(ad->win_main);
	if (!popup)
		return NULL;
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title)
		elm_object_part_text_set(popup, "title,text", title);

	scroller = elm_scroller_add(popup);
	if (!scroller)
		return NULL;
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(popup, scroller);
	evas_object_show(scroller);

	label = elm_label_add(scroller);
	if (!label)
		return NULL;
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);

	text = elm_entry_utf8_to_markup(content);
	if (!text)
		return NULL;
	elm_object_text_set(label, text);
	free(text);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(scroller, label);
	evas_object_show(label);

	if (lbtnText && lbtn_cb) {
		/* Left button */
		lbtn = elm_button_add(popup);
		if (lbtn) {
			elm_object_text_set(lbtn, lbtnText);
			elm_object_style_set(lbtn, "popup");
			elm_object_part_content_set(popup, "button1", lbtn);
			evas_object_smart_callback_add(lbtn, "clicked", lbtn_cb, ad);
		}
	}

	if (rbtnText && rbtn_cb) {
		/* Right button */
		rbtn = elm_button_add(popup);
		if (rbtn) {
			elm_object_text_set(rbtn, rbtnText);
			elm_object_style_set(rbtn, "popup");
			elm_object_part_content_set(popup, "button2", rbtn);
			evas_object_smart_callback_add(rbtn, "clicked", rbtn_cb, ad);
		}
	}

	evas_object_show(popup);

	return popup;
}

/* normal popup with title(optional), main text, and two buttons(optional) */
Evas_Object *load_popup_by_style(struct appdata *ad,
			char *style,
			char *title,
			char *content,
			char *lbtnText,
			Evas_Smart_Cb lbtn_cb,
			char *rbtnText,
			Evas_Smart_Cb rbtn_cb)
{
	Evas_Object *lbtn;
	Evas_Object *rbtn;
	Evas_Object *popup;
	Ecore_X_Window xwin;

	if (!ad || !(ad->win_main) || !content)
		return NULL;

	evas_object_show(ad->win_main);
	popup = elm_popup_add(ad->win_main);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (style) {
		elm_object_style_set(popup, style);
		if (!strncmp(style, "verticalbuttonstyle", strlen(style)))
			elm_layout_signal_emit(popup, "elm,state,2button", "elm");
	}

	elm_object_text_set(popup, content);

	if (title) {
		/* Popup title */
		elm_object_part_text_set(popup, "title,text", title);
	}

	if (lbtnText && lbtn_cb) {
		/* Left button */
		lbtn = elm_button_add(popup);
		elm_object_text_set(lbtn, lbtnText);
		elm_object_style_set(lbtn, "popup");
		elm_object_part_content_set(popup, "button1", lbtn);
		evas_object_smart_callback_add(lbtn, "clicked", lbtn_cb, ad);
	}

	if (rbtnText && rbtn_cb) {
		/* Right button */
		rbtn = elm_button_add(popup);
		elm_object_text_set(rbtn, rbtnText);
		elm_object_style_set(rbtn, "popup");
		elm_object_part_content_set(popup, "button2", rbtn);
		evas_object_smart_callback_add(rbtn, "clicked", rbtn_cb, ad);
	}

	xwin = elm_win_xwindow_get(popup);
	ecore_x_netwm_window_type_set(xwin, ECORE_X_WINDOW_TYPE_NOTIFICATION);

	evas_object_show(popup);

	return popup;
}

Evas_Object *load_normal_popup(struct appdata *ad,
			char *title,
			char *content,
			char *lbtnText,
			Evas_Smart_Cb lbtn_cb,
			char *rbtnText,
			Evas_Smart_Cb rbtn_cb)
{
	return load_popup_by_style(ad,
			"transparent",
			title, content,
			lbtnText, lbtn_cb,
			rbtnText, rbtn_cb);
}

Evas_Object *load_popup_with_vertical_buttons(struct appdata *ad,
			char *title,
			char *content,
			char *ubtnText,
			Evas_Smart_Cb ubtn_cb,
			char *dbtnText,
			Evas_Smart_Cb dbtn_cb)
{
	return load_popup_by_style(ad,
			"verticalbuttonstyle",
			title, content,
			ubtnText, ubtn_cb,
			dbtnText, dbtn_cb);
}

void play_feedback(int type, int pattern)
{
	int ret;

	ret = feedback_initialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		_E("Cannot initialize feedback");
		return;
	}

	switch (type) {
	case PLAY_SOUND_ONLY:
	case PLAY_LED_ONLY:
	case PLAY_VIBRATION_ONLY:
		ret = feedback_play_type(type, pattern);
		break;
	case PLAY_ALL:
		ret = feedback_play(pattern);
		break;
	default:
		_E("Play type is unknown");
		ret = 0;
	}

	if (ret != FEEDBACK_ERROR_NONE)
		_E("Cannot play feedback: %d", pattern);

	ret = feedback_deinitialize();
	if (ret != FEEDBACK_ERROR_NONE)
		_E("Cannot deinitialize feedback");
}

static int append_variant(DBusMessageIter *iter, const char *sig, char *param[])
{
	char *ch;
	int i;
	int iValue;

	if (!sig || !param)
		return 0;

	for (ch = (char*)sig, i = 0; *ch != '\0'; ++i, ++ch) {
		switch (*ch) {
		case 'i':
			iValue = atoi(param[i]);
			dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &iValue);
			break;
		case 's':
			dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &param[i]);
			break;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

int broadcast_dbus_signal(const char *path, const char *interface,
		const char *name, const char *sig, char *param[])
{
	E_DBus_Connection *conn = NULL;
	DBusPendingCall *pc;
	DBusMessageIter iter;
	DBusMessage *msg;
	int ret, retry;

	retry = 0;
	do {
		ret = e_dbus_init();
		if (ret > 0)
			break;
		if (retry == RETRY_MAX) {
			_E("FAIL: e_dbus_init()");
			return -ENOMEM;
		}
		retry++;
	} while (retry < RETRY_MAX);

	if (!path || !interface || !name)
		return -EINVAL;

	conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
	if (!conn) {
		_E("FAIL: e_dbus_bus_get()");
		return -ENOENT;
	}

	msg = dbus_message_new_signal(path, interface, name);
	if (!msg) {
		_E("FAIL: dbus_message_new_signal()");
		ret = -ENOMEM;
		goto out_conn;
	}

	dbus_message_iter_init_append(msg, &iter);
	ret = append_variant(&iter, sig, param);
	if (ret < 0) {
		_E("append_variant error(%d)", ret);
		goto out_msg_conn;
	}

	pc = e_dbus_message_send(conn, msg, NULL, -1, NULL);
	if (!pc) {
		_E("FAIL: e_dbus_message_send()");
		ret = -ECONNREFUSED;
		goto out_msg_conn;
	}

	ret = 0;

out_msg_conn:
	dbus_message_unref(msg);
out_conn:
	e_dbus_connection_close(conn);
	e_dbus_shutdown();
	return ret;
}

static void *start_thread_display_feedback(void* data)
{
	int type = (int)data;
	if (display_change_state(LCD_NORMAL) < 0)
		_E("FAIL: display_change_state()");

	if (type >= 0)
		play_feedback(PLAY_ALL, type);
	return NULL;
}

int set_display_feedback(int type)
{
	pthread_t th;
	int ret;

	ret = pthread_create(&th, NULL, start_thread_display_feedback, (void *)type);
	if (ret < 0) {
		_E("Failed to create pthread");
		return ret;
	}
	pthread_detach(th);

	return 0;
}

int dbus_method_sync(const char *dest, const char *path,
		const char *interface, const char *method,
		const char *sig, char *param[])
{
	DBusConnection *conn = NULL;
	DBusMessage *msg = NULL;
	DBusMessageIter iter;
	DBusMessage *reply = NULL;
	DBusError err;
	int ret, result;

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (!conn) {
		_E("dbus_bus_get error");
		return -EPERM;
	}

	msg = dbus_message_new_method_call(dest, path, interface, method);
	if (!msg) {
		_E("dbus_message_new_method_call(%s:%s-%s)", path, interface, method);
		ret = -EBADMSG;
		goto out;
	}

	dbus_message_iter_init_append(msg, &iter);
	ret = append_variant(&iter, sig, param);
	if (ret < 0) {
		_E("append_variant error(%d)", ret);
		goto out;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, DBUS_REPLY_TIMEOUT, &err);
	if (!reply) {
		_E("dbus_connection_send error(%s:%s)", err.name, err.message);
		dbus_error_free(&err);
		ret = -ECOMM;
		goto out;
	}

	ret = dbus_message_get_args(reply, &err, DBUS_TYPE_INT32, &result, DBUS_TYPE_INVALID);
	if (!ret) {
		_E("no message : [%s:%s]", err.name, err.message);
		dbus_error_free(&err);
		ret = -ENOMSG;
		goto out;
	}

	ret = result;

out:
	if (msg)
		dbus_message_unref(msg);
	if (reply)
		dbus_message_unref(reply);
	if (conn)
		dbus_connection_unref(conn);
	return ret;
}

int get_hallic_status(void)
{
	int ret, i;

	i = 0;
	do {
		ret = dbus_method_sync(DEVICED_BUS_NAME,
				DEVICED_PATH_HALL,
				DEVICED_IFACE_HALL,
				"getstatus", NULL, NULL);
		if (ret < 0)
			_E("Failed to request hall ic status: ret(%d), retry (%d)", ret, i);
		else
			break;
	} while (i++ < RETRY_MAX);

	return ret;
}

int reset_window_priority(Evas_Object *win, int priority)
{
	Ecore_X_Window xwin;
	Display *dpy;

	if (priority < WIN_PRIORITY_LOW || priority > WIN_PRIORITY_HIGH)
		return -EINVAL;

	xwin = elm_win_xwindow_get(win);
	dpy = ecore_x_display_get();

	utilx_set_system_notification_level(dpy, xwin, priority);

	return 0;
}

int set_popup_focus(Evas_Object *win, bool focus)
{
	XWMHints *hints;
	Display *dpy;
	Ecore_X_Window xwin;

	hints = XAllocWMHints();
	if (!hints) {
		_E("Failed to allocate WWMHints");
		return -ENOMEM;
	}

	dpy = ecore_x_display_get();
	xwin = elm_win_xwindow_get(win);

	hints->flags = InputHint | StateHint;
	if (focus)
		hints->input = 1;
	else
		hints->input = 0;
	hints->initial_state = NormalState;
	XSetWMHints(dpy, xwin, hints);
	XFree(hints);

	return 0;
}

#define DEVICED_OBJECT_PATH "/Org/Tizen/System/DeviceD"
#define DEVICED_IFACE_NAME  DEVICED_BUS_NAME
#define DEVICED_PATH_POWER  DEVICED_OBJECT_PATH"/Power"
#define DEVICED_IFACE_POWER DEVICED_IFACE_NAME".power"
#define METHOD_POWEROFF     "reboot"
#define TYPE_POWEROFF       "poweroff"

int device_poweroff(void)
{
	char *param[2];
	char opt[4];

	snprintf(opt, sizeof(opt), "0");
	param[0] = TYPE_POWEROFF;
	param[1] = opt;

	return dbus_method_sync(
			DEVICED_BUS_NAME,
			DEVICED_PATH_POWER,
			DEVICED_IFACE_POWER,
			METHOD_POWEROFF,
			"si", param);
}
