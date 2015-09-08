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
#include <vconf.h>
#include <efl_assist.h>
#include <unicode/udat.h>
#include <unicode/udatpg.h>
#include <unicode/ustring.h>

#define RETRY_MAX 10
#define DBUS_REPLY_TIMEOUT  (-1)
#define BUF_MAX 64

#ifdef SYSTEM_APPS_MICRO_3
#define TIZEN_MICRO_WIDTH  360
#define TIZEN_MICRO_HEIGHT 480
#define LAYOUT_CHECKVIEW   "micro_3_title_content_button"
#else
#define TIZEN_MICRO_WIDTH  320
#define TIZEN_MICRO_HEIGHT 320
#define LAYOUT_CHECKVIEW   "popup_checkview"
#endif

#define TABLE_COLOR "system-color.xml"
#define TABLE_FONT  "system-font.xml"

static E_DBus_Connection *edbus_conn = NULL;

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
	Ea_Theme_Color_Table *color;
	Ea_Theme_Font_Table *font;

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
			TIZEN_MICRO_WIDTH * elm_config_scale_get(),
			TIZEN_MICRO_HEIGHT * elm_config_scale_get());

	ea_theme_changeable_ui_enabled_set(EINA_TRUE);

	color = ea_theme_color_table_new(TABLE_COLOR);
	if (color) {
		ea_theme_colors_set(color, EA_THEME_STYLE_DEFAULT);
		ea_theme_color_table_free(color);
	}

	font = ea_theme_font_table_new(TABLE_FONT);
	if (font) {
		ea_theme_fonts_set(font);
		ea_theme_font_table_free(font);
	}

	return eo;
}

static Evas_Object *load_scrollable_style_popup(struct appdata *ad,
			char *ly_style,
			char *edj_name,
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
	char *text;

	if (!ad || !(ad->win_main) || !content || !ly_style || !edj_name)
		return NULL;

	popup = elm_popup_add(ad->win_main);
	if (!popup)
		return NULL;
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, ELM_NOTIFY_ALIGN_FILL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	if (title)
		elm_object_part_text_set(popup, "title,text", title);

	text = elm_entry_utf8_to_markup(content);
	if (!text)
		return NULL;
	elm_object_text_set(popup, text);
	free(text);

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

static Evas_Object *load_scrollable_no_style_popup(struct appdata *ad,
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
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, ELM_NOTIFY_ALIGN_FILL);
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

Evas_Object *load_scrollable_popup(struct appdata *ad,
			char *ly_style,
			char *edj_name,
			char *title,
			char *content,
			char *lbtnText,
			Evas_Smart_Cb lbtn_cb,
			char *rbtnText,
			Evas_Smart_Cb rbtn_cb)
{
	if (ly_style && edj_name)
		return load_scrollable_style_popup(ad,
				ly_style, edj_name, title, content,
				lbtnText, lbtn_cb, rbtnText, rbtn_cb);
	else
		return load_scrollable_no_style_popup(ad,
				title, content,
				lbtnText, lbtn_cb, rbtnText, rbtn_cb);
}

bool get_check_state(Evas_Object *check)
{
	if (check && elm_check_state_get(check))
		return true;
	return false;
}

Evas_Object *load_scrollable_check_popup(struct appdata *ad,
			char *edj_name,
			char *title,
			char *content,
			char *text_check,
			Evas_Object **check,
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
	Evas_Object *layout, *layout_inner;
	char *text;

	if (!ad || !(ad->win_main) || !content || !edj_name || !text_check || !check)
		return NULL;

	evas_object_show(ad->win_main);
	popup = elm_popup_add(ad->win_main);
	if (!popup)
		return NULL;
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, ELM_NOTIFY_ALIGN_FILL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title)
		elm_object_part_text_set(popup, "title,text", title);

	layout = elm_layout_add(popup);
	if (!layout)
		return NULL;
	elm_layout_file_set(layout, edj_name, LAYOUT_CHECKVIEW);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(popup, layout);

	scroller = elm_scroller_add(popup);
	if (!scroller)
		return NULL;
	elm_object_style_set(scroller, "effect");
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "elm.swallow.content", scroller);
	evas_object_show(scroller);

	layout_inner = elm_layout_add(layout);
	elm_layout_file_set(layout_inner, edj_name, "popup_checkview_internal");
	evas_object_size_hint_weight_set(layout_inner, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(scroller, layout_inner);

	label = elm_label_add(layout);
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

	elm_object_part_content_set(layout_inner, "label", label);
	elm_object_part_content_set(layout, "elm.swallow.content", scroller);

	*check = elm_check_add(popup);
	if (!(*check))
		return NULL;
	elm_object_style_set(*check, "popup");
	elm_object_text_set(*check, text_check);
	elm_object_part_content_set(layout_inner, "elm.swallow.end", *check);
	evas_object_show(*check);

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
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, ELM_NOTIFY_ALIGN_FILL);
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

#ifdef SYSTEM_APPS_CIRCLE
Evas_Object *load_normal_popup(struct appdata *ad,
		char *edj,
		char *title,
		char *content,
		char *lbtn_text,
		char *lbtn_icon,
		Evas_Smart_Cb lbtn_clicked,
		char *rbtn_text,
		char *rbtn_icon,
		Evas_Smart_Cb rbtn_clicked)
{
	Evas_Object *lbtn;
	Evas_Object *licon;
	Evas_Object *rbtn;
	Evas_Object *ricon;
	Evas_Object *popup;
	Evas_Object *layout;

	if (!ad || !(ad->win_main) || !content)
		return NULL;

	evas_object_show(ad->win_main);

	popup = elm_popup_add(ad->win_main);
	if (!popup)
		return NULL;
	elm_object_style_set(popup, "circle");
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, ELM_NOTIFY_ALIGN_FILL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	if (title)
		elm_object_part_text_set(layout, "elm.text.title", _(title));

	elm_object_content_set(popup, layout);

	if (lbtn_text && !rbtn_text) {
		/* one button */
		elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons1");
		elm_object_part_text_set(layout, "elm.text", content);
		elm_object_content_set(popup, layout);

		if (lbtn_icon) {
			lbtn = elm_button_add(popup);
			if (lbtn) {
				elm_object_text_set(lbtn, _(lbtn_text));
				elm_object_style_set(lbtn, "popup/circle");
				elm_object_part_content_set(popup, "button1", lbtn);
				evas_object_smart_callback_add(lbtn, "clicked", lbtn_clicked, ad);
			}
			licon = elm_image_add(lbtn);
			if (licon) {
				elm_image_file_set(licon, edj, lbtn_icon);
				evas_object_size_hint_weight_set(licon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				elm_object_part_content_set(lbtn, "elm.swallow.content", licon);
				evas_object_show(licon);
			}
		}
	} else {
		/* Two button */
		elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons2");
		elm_object_part_text_set(layout, "elm.text", content);
		elm_object_content_set(popup, layout);

		if (lbtn_text && lbtn_icon) {
			/* Left button */
			lbtn = elm_button_add(popup);
			if (lbtn) {
				elm_object_text_set(lbtn, _(lbtn_text));
				elm_object_style_set(lbtn, "popup/circle/left");
				elm_object_part_content_set(popup, "button1", lbtn);
				evas_object_smart_callback_add(lbtn, "clicked", lbtn_clicked, ad);
			}
			licon = elm_image_add(lbtn);
			if (licon) {
				elm_image_file_set(licon, edj, lbtn_icon);
				evas_object_size_hint_weight_set(licon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				elm_object_part_content_set(lbtn, "elm.swallow.content", licon);
				evas_object_show(licon);
			}
		}

		if (rbtn_text && rbtn_icon) {
			/* Right button */
			rbtn = elm_button_add(popup);
			if (rbtn) {
				elm_object_text_set(rbtn, _(rbtn_text));
				elm_object_style_set(rbtn, "popup/circle/right");
				elm_object_part_content_set(popup, "button2", rbtn);
				evas_object_smart_callback_add(rbtn, "clicked", rbtn_clicked, ad);
			}
			ricon = elm_image_add(rbtn);
			if (ricon) {
				elm_image_file_set(ricon, edj, rbtn_icon);
				evas_object_size_hint_weight_set(ricon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				elm_object_part_content_set(rbtn, "elm.swallow.content", ricon);
				evas_object_show(ricon);
			}
		}
	}

	evas_object_show(popup);

	return popup;

}
#else
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
#endif

Evas_Object *load_popup_toast(struct appdata *ad,
		char *content)
{
	Evas_Object *popup;

	if (!ad || !(ad->win_main) || !content)
		return NULL;

	popup = elm_popup_add(ad->win_main);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, ELM_NOTIFY_ALIGN_FILL);
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

int set_dbus_connection(void)
{
	int retry;

	if (edbus_conn)
		return 0;

	retry = 0;
	while (e_dbus_init() == 0) {
		if (retry++ >= RETRY_MAX)
			return -ENOMEM;
	}

	edbus_conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
	if (!edbus_conn) {
		_E("Failed to get dbus bus");
		e_dbus_shutdown();
		return -ENOMEM;
	}

	return 0;
}

E_DBus_Connection *get_dbus_connection(void)
{
	return edbus_conn;
}

void unset_dbus_connection(void)
{
	if (edbus_conn) {
		e_dbus_connection_close(edbus_conn);
		e_dbus_shutdown();
		edbus_conn = NULL;
	}
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
	int ret;

	if (!path || !interface || !name)
		return -EINVAL;

	conn = get_dbus_connection();
	if (!conn) {
		_E("Failed to get dbus connection");
		return -ENOMEM;
	}

	msg = dbus_message_new_signal(path, interface, name);
	if (!msg) {
		_E("FAIL: dbus_message_new_signal()");
		return -ENOMEM;
	}

	dbus_message_iter_init_append(msg, &iter);
	ret = append_variant(&iter, sig, param);
	if (ret < 0) {
		_E("append_variant error(%d)", ret);
		goto out;
	}

	pc = e_dbus_message_send(conn, msg, NULL, -1, NULL);
	if (!pc) {
		_E("FAIL: e_dbus_message_send()");
		ret = -ECONNREFUSED;
		goto out;
	}

	ret = 0;

out:
	dbus_message_unref(msg);
	return ret;
}

int change_pd_to_ps(char *text)
{
	char *p;

	if (!text || strlen(text) == 0)
		return -EINVAL;

	_I("text: (%s)", text);

	p = text;
	do {
		p = strstr(p, "$d");
		if (!p)
			break;
		*(p+1) = 's';
	} while(p);

	_I("text: (%s)", text);

	return 0;
}

int get_str_from_icu(int number, char *buf, int buf_len)
{
	char locale[BUF_MAX];
	char *p;
	UErrorCode status = U_ZERO_ERROR;
	UNumberFormat *num_fmt;
	UChar result[BUF_MAX] = {0, };
	char res[BUF_MAX] = {0, };
	int32_t len = (int32_t) (sizeof(result) / sizeof((result)[0]));

	if (!buf || buf_len <= 0)
		return -EINVAL;

	snprintf(locale, sizeof(locale), "%s", vconf_get_str(VCONFKEY_REGIONFORMAT));

	if (locale[0] != '\0') {
		p = strstr(locale, ".UTF-8");
		if (p)
			*p = 0;
	}

	num_fmt = unum_open(UNUM_DEFAULT, NULL, -1, locale, NULL, &status);
	unum_format(num_fmt, number, result, len, NULL, &status);

	u_austrcpy(res, result);

	unum_close(num_fmt);

	_I("Number (%d) from ICU: (%s)", number, res);

	snprintf(buf, buf_len, "%s", res);

	return 0;
}
