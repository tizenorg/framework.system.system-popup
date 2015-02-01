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
#include "double-data.h"

#define DOUBLE_ID                800
#define DOUBLE_STYLE             "device_option.2icon.divider"

static Elm_Genlist_Item_Class *double_itc;
static Elm_Object_Item *gl_item;

static struct double_item_option *first_option;
static struct double_item_option *second_option;

int register_first_option(struct double_item_option *opt)
{
	if (!opt)
		return -EINVAL;

	if (first_option) {
		_E("First button is already registerd by (%s)", first_option->name);
		return -EEXIST;
	}

	first_option = opt;
	return 0;
}

int register_second_option(struct double_item_option *opt)
{
	if (!opt)
		return -EINVAL;

	if (second_option) {
		_E("Second button is already registerd by (%s)", second_option->name);
		return -EEXIST;
	}

	second_option = opt;
	return 0;
}

static int store_item(Elm_Object_Item *item)
{
	if (!item)
		return -EINVAL;
	gl_item = item;
	return 0;
}

void update_item(void)
{
	if (gl_item)
		elm_genlist_item_update(gl_item);
}

static int get_id(void)
{
	return DOUBLE_ID;
}

static void register_handlers(void *data)
{
	if (first_option && first_option->register_handler)
		if (first_option->register_handler(data) < 0)
			_E("Failed to register handler for first option");

	if (second_option && second_option->register_handler)
		if (second_option->register_handler(data) < 0)
			_E("Failed to register handler for second option");
}

static void unregister_handlers(void *data)
{
	if (first_option && first_option->unregister_handler)
		first_option->unregister_handler(data);

	if (second_option && second_option->unregister_handler)
		second_option->unregister_handler(data);
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

static Evas_Object *get_button(Evas_Object *obj, struct appdata *ad, struct double_item_option *opt)
{
	char icon[BUF_MAX], text[BUF_MAX];

	if (opt && opt->get_text && opt->get_icon) {
		if (opt->get_text(text, sizeof(text)) < 0)
			return NULL;
		if (opt->get_icon(icon, sizeof(icon)) < 0)
			return NULL;
		opt->btn = make_button(obj, text, icon, opt->response_clicked, ad);
		return opt->btn;
	}

	return NULL;
}

static Evas_Object *get_icon(void *data, Evas_Object *obj, const char *part)
{
	struct appdata *ad = data;

	if (!ad)
		return NULL;

	/* first button */
	if (!strcmp(part, "elm.icon"))
		return get_button(obj, ad, first_option);

	/* second button */
	if (!strcmp(part, "elm.icon.1"))
		return get_button(obj, ad, second_option);

	return NULL;
}

static bool is_item_enabled(void)
{
	return true;
}

static int init_itc(void)
{
	double_itc = elm_genlist_item_class_new();
	if (!double_itc)
		return -ENOMEM;
	return 0;
}

static void deinit_itc(void)
{
	if (double_itc) {
		elm_genlist_item_class_free(double_itc);
		double_itc = NULL;
	}
}

static int get_itc(Elm_Genlist_Item_Class **itc)
{
	double_itc->item_style = DOUBLE_STYLE;
	double_itc->func.text_get = NULL;
	double_itc->func.content_get = get_icon;
	*itc = double_itc;
	return 0;
}

static void response_clicked(void *data, Evas_Object *obj, void *event_info)
{
	/* Do nothing */
}

static const struct device_option double_ops = {
	.name                = "double-data",
	.get_id              = get_id,
	.is_enabled          = is_item_enabled,
	.init_itc            = init_itc,
	.deinit_itc          = deinit_itc,
	.get_itc             = get_itc,
	.store_item          = store_item,
	.response_clicked    = response_clicked,
	.register_handlers   = register_handlers,
	.unregister_handlers = unregister_handlers
};

/* Constructor to register snd_od item */
static __attribute__ ((constructor)) void register_device_option_double_data(void)
{
	int state;

	if (vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &state) != 0)
		return;

	register_option(&double_ops);
}
