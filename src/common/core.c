/*
 * system-popup
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include "popup-common.h"

#define SYSPOPUP_CONTENT "_SYSPOPUP_CONTENT_"

static syspopup_handler handler;
static GList *popup_list = NULL;

GList *get_popup_list(void)
{
	return popup_list;
}

void register_popup(const struct popup_ops *ops)
{
	struct object_ops *obj;
	if (!ops) {
		_E("Invalid parameter");
		return;
	}

	obj = (struct object_ops *)malloc(sizeof(struct object_ops));
	if (!obj) {
		_E("malloc() fialed");
		return;
	}

	memset(obj, 0, sizeof(struct object_ops));

	obj->ops = ops;

	popup_list = g_list_append(popup_list, obj);
}

static void free_obj(gpointer data)
{
	struct object_ops *obj = data;
	FREE(obj);
}

void unregister_all_popup(void)
{
	if (!popup_list)
		return;

	g_list_free_full(popup_list, free_obj);
}

void terminate_if_no_popup(void)
{
	GList *l;
	struct object_ops *obj;

	for (l = popup_list ; l ; l = g_list_next(l)) {
		obj = (struct object_ops *)(l->data);
		if (obj->popup) {
			_I("popup exists(%s)", obj->ops->name);
			return;
		}
	}
	popup_terminate();
}

static int load_popup_by_type(bundle *b)
{
	char *type;
	GList *l;
	struct object_ops *obj;
	int ret;

	if (!b)
		return -EINVAL;

	type = (char *)bundle_get_val(b, SYSPOPUP_CONTENT);
	if (!type) {
		_E("FAIL: bundle_get_val()");
		return -ENOMEM;
	}

	for (l = popup_list ; l ; l = g_list_next(l)) {
		obj = (struct object_ops *)(l->data);
		if (!obj || !(obj->ops) || !(obj->ops->name) || !(obj->ops->show_popup))
			continue;
		if (strncmp (type, obj->ops->name, strlen(type)))
			continue;

		ret = obj->ops->show_popup(b, obj->ops);

		if (obj->ops->launch)
			obj->ops->launch(obj->ops);

		return ret;
	}
	return -EINVAL;
}

static int release_all_handlers(void *data)
{
	GList *l;
	struct object_ops *obj;
	static bool already = false;

	if (already)
		return 0;
	already = true;

	for (l = popup_list ; l ; l = g_list_next(l)) {
		obj = (struct object_ops *)(l->data);
		if (obj && obj->popup) {
			if (obj->ops->terminate)
				obj->ops->terminate(obj->ops);
			release_evas_object(&(obj->popup));
		}
	}

	unset_dbus_connection();
	remove_window();
	unregister_all_popup();
	return 0;
}

static int terminate_by_syspopup(bundle *b, void *data)
{
	return release_all_handlers(data);
}

static int app_create(void *data)
{
	int ret;

	handler.def_term_fn = terminate_by_syspopup;
	handler.def_timeout_fn = NULL;

	/* create window */
	ret = create_window(PACKAGE);
	if (ret < 0)
		return ret;

	if (appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR) != 0)
		_E("FAIL: appcore_set_i18n()");

	ret = set_dbus_connection();
	if (ret < 0)
		_E("Failed to set dbus connection (%d)", ret);

	return 0;
}

static int app_terminate(void *data)
{
	return release_all_handlers(data);
}

static int app_pause(void *data)
{
	popup_terminate();
	return 0;
}

static int app_resume(void *data)
{
	return 0;
}

static int app_reset(bundle *b, void *data)
{
	int ret;
	Evas_Object *win;

	if (!b) {
		ret = -EINVAL;
		goto out;
	}

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
	} else {
		win = get_window();
		if (!win)
			return -ENOMEM;

		ret = syspopup_create(b, &handler, win, NULL);
		if (ret < 0) {
			_E("FAIL: syspopup_create(): %d", ret);
			goto out;
		}

		/* change window priority to normal */
		reset_window_priority(WIN_PRIORITY_HIGH);
	}

	ret = load_popup_by_type(b);
	if (ret < 0)
		goto out;

	return 0;

out:
	popup_terminate();
	return ret;
}

int main(int argc, char *argv[])
{
	struct appcore_ops ops = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.reset = app_reset,
	};

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
