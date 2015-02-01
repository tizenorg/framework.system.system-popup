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

#include <dd-display.h>
#include "core.h"
#include "common.h"

#define SYSPOPUP_CONTENT "_SYSPOPUP_CONTENT_"

static GList *popup_list = NULL;

void register_popup(struct popup_ops *ops)
{
	if (!ops) {
		_E("Invalid parameter");
		return;
	}

	popup_list = g_list_append(popup_list, ops);
}

void unregister_all_popup(void)
{
	if (!popup_list)
		return;

	g_list_free(popup_list);
}

void terminate_if_no_popup(void)
{
	GList *l;
	struct popup_ops *ops;

	for (l = popup_list ; l ; l = g_list_next(l)) {
		ops = (struct popup_ops *)(l->data);
		if (ops->popup)
			return;
	}
	popup_terminate();
}

static int load_popup_by_type(struct appdata *ad, bundle *b)
{
	char *type;
	GList *l;
	struct popup_ops *ops;

	if (!ad || !b)
		return -EINVAL;

	type = (char *)bundle_get_val(b, SYSPOPUP_CONTENT);
	if (!type) {
		_E("FAIL: bundle_get_val()");
		return -ENOMEM;
	}

	for (l = popup_list ; l ; l = g_list_next(l)) {
		ops = (struct popup_ops *)(l->data);
		if (!ops || !(ops->name) || !(ops->show_popup))
			continue;
		if (strncmp (ops->name, type, strlen(type)))
			continue;
		return ops->show_popup(ad, b);
	}
	return -EINVAL;
}

static int app_create(void *data)
{
	Evas_Object *win;
	struct appdata *ad = data;

	ad->handler.def_term_fn = NULL;
	ad->handler.def_timeout_fn = NULL;

	/* create window */
	win = create_win(PACKAGE);
	if (!win)
		return -ENOMEM;

	ad->win_main = win;

	if (appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR) != 0)
		_E("FAIL: appcore_set_i18n()");

	return 0;
}

static int app_terminate(void *data)
{
	struct appdata *ad = (struct appdata *)data;

	release_evas_object(&(ad->layout_main));
	release_evas_object(&(ad->win_main));

	unregister_all_popup();

	return 0;
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
	struct appdata *ad = (struct appdata *)data;
	int ret;

	if (!ad || !b) {
		ret = -EINVAL;
		goto out;
	}

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
	} else {
		ret = syspopup_create(b, &(ad->handler), ad->win_main, ad);
		if (ret < 0) {
			_E("FAIL: syspopup_create(): %d", ret);
			goto out;
		}
	}

	/* change window priority to normal */
	reset_window_priority(ad->win_main, WIN_PRIORITY_NORMAL);

	ret = load_popup_by_type(ad, b);
	if (ret < 0)
		goto out;

	return 0;

out:
	popup_terminate();
	return ret;
}

int main(int argc, char *argv[])
{
	struct appdata ad;
	struct appcore_ops ops = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.reset = app_reset,
	};

	memset(&ad, 0x0, sizeof(struct appdata));

	ops.data = &ad;

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
