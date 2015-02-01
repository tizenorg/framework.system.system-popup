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

#include "accessibility.h"

#define ACCESSIBILITY_POPUP     "accessibility"
#define ACCESSIBILITY_CONTENT   "IDS_ST_HEADER_ACCESSIBILITY"

/* List to store items on the popup list */
static GList *acc_option_list = NULL;
static Evas_Object *item_list = NULL;

static int show_accessibility_popup(void *data, bundle *b);

struct popup_ops accessibility_ops = {
	.name = ACCESSIBILITY_POPUP,
	.popup = NULL,
	.show_popup = show_accessibility_popup
};

static gint compare_acc_options(gconstpointer first, gconstpointer second)
{
	struct acc_option *opt1 = (struct acc_option *)first;
	struct acc_option *opt2 = (struct acc_option *)second;

	return (opt1->get_id() - opt2->get_id());
}

/* Register an item on the popup list */
void register_acc_option(const struct acc_option *opt)
{
	struct acc_option *option;

	if (!opt) {
		_E("Invalid parameter");
		return;
	}

	option = (struct acc_option *)malloc(sizeof(struct acc_option));
	if (!option) {
		_E("FAIL: malloc()");
		return;
	}

	option->get_id              = opt->get_id;
	option->get_icon            = opt->get_icon;
	option->get_content         = opt->get_content;
	option->response_clicked    = opt->response_clicked;
	option->register_handlers   = opt->register_handlers;
	option->unregister_handlers = opt->unregister_handlers;

	acc_option_list = g_list_insert_sorted(acc_option_list, option, compare_acc_options);
}

/* Unregister an item on the popup list */
void unregister_acc_option(const struct acc_option *opt)
{
	GList *l;
	struct acc_option *option;

	if (!opt) {
		_E("Invalid parameter");
		return;
	}

	for (l = acc_option_list ; l ; l = g_list_next(l)) {
		option = (struct acc_option *)(l->data);
		if (!option)
			continue;

		if (option->get_id != opt->get_id)
			continue;

		acc_option_list = g_list_delete_link(acc_option_list, l);
		break;
	}
}

static void acc_option_list_free_func(gpointer data)
{
	struct acc_option *option = (struct acc_option *)data;
	FREE(option);
}

/* Unregister all items on the popup list */
void unregister_acc_option_full(void)
{
	if (!acc_option_list)
		return;

	g_list_free_full(acc_option_list, acc_option_list_free_func);
}

void remove_accessibility_popup(void)
{
	release_evas_object(&(accessibility_ops.popup));
}

/* Basic popup widget */
static Evas_Object *load_accessibility_popup(struct appdata *ad)
{
	char content[BUF_MAX];
	Ecore_X_Window xwin;
	GList *l;
	struct acc_option *opt;
	char item_content[BUF_MAX];
	Evas_Object *popup;

	if (!ad || !(ad->win_main))
		return NULL;

	if (!acc_option_list) {
		_I("No device options");
		return NULL;
	}

	popup = elm_popup_add(ad->win_main);
	if (popup == NULL) {
		_E("FAIL: elm_popup_add()");
		return NULL;
	}

	/* title */
	elm_object_part_text_set(popup, "title,text", _(ACCESSIBILITY_CONTENT));
	elm_object_style_set (popup, "content_no_vhpad_transparent");

	/* list */
	item_list = elm_list_add(popup);
	elm_list_mode_set(item_list,ELM_LIST_EXPAND);
	elm_object_style_set (item_list, "popup");

	/* Add items to list on the popup */
	for (l = acc_option_list ; l ; l = g_list_next(l)) {
		opt = (struct acc_option *)(l->data);
		if (!opt)
			continue;

		if (opt->get_content(item_content, sizeof(item_content)) < 0)
			continue;

		snprintf(content, sizeof(content), "%s", _(item_content));
		elm_list_item_append(item_list, content, NULL, NULL, opt->response_clicked, ad);
	}

	elm_list_go(item_list);

	xwin = elm_win_xwindow_get(popup);
	ecore_x_netwm_window_type_set(xwin, ECORE_X_WINDOW_TYPE_NOTIFICATION);
	utilx_grab_key(ecore_x_display_get(), xwin, KEY_SELECT, SHARED_GRAB);
	elm_object_content_set(popup,item_list);

	evas_object_show(popup);

	return popup;
}

static int show_accessibility_popup(void *data, bundle *b)
{
	struct appdata *ad = (struct appdata *)data;

	if (accessibility_ops.popup) {
		_E("Popup already exists");
		return 0;
	}

	evas_object_show(ad->win_main);

	accessibility_ops.popup = load_accessibility_popup(ad);
	if (!(accessibility_ops.popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static __attribute__ ((constructor)) void register_accessibility_popup(void)
{
	register_popup(&accessibility_ops);
}
