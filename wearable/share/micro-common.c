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

#include "micro-common.h"
#include <E_DBus.h>
#include <efl_assist.h>

#define RETRY_MAX 10
#define DBUS_REPLY_TIMEOUT  (-1)

#define TIZEN_MICRO_SIZE 320

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
	evas_object_resize(eo,
			TIZEN_MICRO_SIZE * elm_config_scale_get(),
			TIZEN_MICRO_SIZE * elm_config_scale_get());

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
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title)
		elm_object_part_text_set(popup, "title,text", title);

	scroller = elm_scroller_add(popup);
	if (!scroller)
		return NULL;
	elm_object_style_set(scroller, "effect");
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

	if (!ad || !(ad->win_main) || !content)
		return NULL;

	evas_object_show(ad->win_main);
	popup = elm_popup_add(ad->win_main);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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

Evas_Object *load_popup_toast(struct appdata *ad,
		char *content)
{
	Evas_Object *popup;

	if (!ad || !(ad->win_main) || !content)
		return NULL;

	popup = elm_popup_add(ad->win_main);
	elm_object_style_set(popup, "toast");
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	elm_object_part_text_set(popup, "elm.text", content);

	evas_object_show(popup);

	return popup;
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

void play_feedback(int type, int pattern)
{
	int ret;

	ret = feedback_initialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		_E("Cannot initialize feedback");
		return;
	}

	switch (type) {
	case FEEDBACK_TYPE_LED:
	case FEEDBACK_TYPE_SOUND:
	case FEEDBACK_TYPE_VIBRATION:
		ret = feedback_play_type(type, pattern);
		break;
	case FEEDBACK_TYPE_NONE:
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
