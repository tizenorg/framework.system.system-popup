/*
 *  system-popup
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
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

#include "popup-ui.h"

static Evas_Object *window = NULL;

/* Common */
void release_evas_object(Evas_Object **obj)
{
	if (!obj || !(*obj))
		return;
	if (*obj == window)
		return;
	evas_object_del(*obj);
	*obj = NULL;
}

/* Windows */
Evas_Object *get_window(void)
{
	return window;
}

void remove_window(void)
{
	if (window) {
		evas_object_del(window);
		window = NULL;
	}
}

static void win_del(void *data, Evas_Object * obj, void *event)
{
	popup_terminate();
}

int create_window(const char *name)
{
	Evas_Object *eo;
	int w, h, len;

	if (!name)
		return -EINVAL;

	eo = elm_win_add(NULL, name, ELM_WIN_DIALOG_BASIC);
	if (!eo) {
		_E("FAIL: elm_win_add()");
		return -ENOMEM;;
	}

	elm_win_title_set(eo, name);
	elm_win_borderless_set(eo, EINA_TRUE);
	elm_win_alpha_set(eo, EINA_TRUE);
	elm_win_raise(eo);
	evas_object_smart_callback_add(eo, "delete,request", win_del, NULL);
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
	len = max(w,h);
	evas_object_resize(eo, len, len);

	window = eo;

	return 0;
}

int reset_window_priority(int priority)
{
	Ecore_X_Window xwin;
	Display *dpy;
	Evas_Object *win;

	win = get_window();
	if (!win)
		return -ENOMEM;

	if (priority < WIN_PRIORITY_LOW || priority > WIN_PRIORITY_HIGH)
		return -EINVAL;

	xwin = elm_win_xwindow_get(win);
	dpy = ecore_x_display_get();

	ecore_x_netwm_window_type_set(xwin, ECORE_X_WINDOW_TYPE_NOTIFICATION);
	utilx_set_system_notification_level(dpy, xwin, priority);

	return 0;
}

/* Popups */
static void default_button_action(const struct popup_ops *ops)
{
	if (ops)
		unload_simple_popup(ops);
	terminate_if_no_popup();
}

void left_clicked(void *data, Evas_Object * obj, void *event_info)
{
	const struct popup_ops *ops = data;

	if (ops && ops->left) {
		ops->left(ops);
		return;
	}

	default_button_action(ops);
}

void right_clicked(void *data, Evas_Object * obj, void *event_info)
{
	const struct popup_ops *ops = data;

	if (ops && ops->right) {
		ops->right(ops);
		return;
	}

	default_button_action(ops);
}

int get_object_by_ops(const struct popup_ops *ops, struct object_ops **obj)
{
	GList *popup_list, *l;
	struct object_ops *o;

	if (!ops || !obj)
		return -EINVAL;

	popup_list = get_popup_list();
	if (!popup_list)
		return -ENOMEM;

	for (l = popup_list ; l ; l = g_list_next(l)) {
		o = (struct object_ops *)(l->data);
		if (!o || (o->ops != ops))
			continue;
		*obj = o;
		return 0;
	}

	return -ENOENT;
}

bool get_check_state(const struct popup_ops *ops)
{
	struct object_ops *obj;
	int ret;

	ret = get_object_by_ops(ops, &obj);
	if (ret < 0) {
		_E("Failed to get object (%d)", ret);
		return false;
	}

	if (obj->check && elm_check_state_get(obj->check))
		return true;
	return false;
}

void unload_simple_popup(const struct popup_ops *ops)
{
	struct object_ops *obj;
	int ret;

	ret = get_object_by_ops(ops, &obj);
	if (ret < 0)
		return;

	release_evas_object(&(obj->popup));
}

int load_simple_popup(bundle *b, const struct popup_ops *ops)
{
	struct object_ops *obj;
	int ret;

	if (!ops)
		return -EINVAL;

	unload_simple_popup(ops);

	ret = get_object_by_ops(ops, &obj);
	if (ret < 0) {
		_E("Failed to get object (%d)", ret);
		return -ENOENT;
	}
	obj->b = bundle_dup(b);

	return load_normal_popup(ops);
}
