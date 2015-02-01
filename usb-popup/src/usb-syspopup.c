/*
 * usb-syspopup
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
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

#include <stdio.h>
#include <vconf.h>
#include <ail.h>
#include <string.h>
#include <appcore-efl.h>
#include <Ecore_X.h>
#include <assert.h>
#include "usb-syspopup.h"
#include "common.h"

#define BUF_MAX 1024

#define SYSPOPUP_CONTENT   "_SYSPOPUP_CONTENT_"
#define ERROR_POPUP        "error"
#define RESTRICT_POPUP     "restrict"

static char popup_type[BUF_MAX] = {0, };

static bool is_usb_connected(void)
{
	int state;

	if (vconf_get_int(VCONFKEY_SYSMAN_USB_STATUS, &state) != 0)
		return false;

	switch (state) {
	case VCONFKEY_SYSMAN_USB_CONNECTED:
	case VCONFKEY_SYSMAN_USB_AVAILABLE:
		return true;
	case VCONFKEY_SYSMAN_USB_DISCONNECTED:
	default:
		return false;
	}
}

static void usb_state_changed(keynode_t *key, void *data)
{
	if (!is_usb_connected())
		popup_terminate();
}

static int usp_vconf_key_notify(void)
{
	int ret;

	/* Event for USB cable */
	ret = vconf_notify_key_changed(VCONFKEY_SYSMAN_USB_STATUS,
					usb_state_changed, NULL);
	if (0 != ret) {
		_E("FAIL: vconf_notify_key_changed(VCONFKEY_SYSMAN_USB_STATUS)");
		return -1;
	}

	return 0;
}

static int usp_vconf_key_ignore(void)
{
	int ret;

	/* Event for USB cable */
	ret = vconf_ignore_key_changed(VCONFKEY_SYSMAN_USB_STATUS, usb_state_changed);
	if (0 != ret) {
		_E("FAIL: vconf_ignore_key_changed(VCONFKEY_SYSMAN_USB_STATUS)");
		return -1;
	}

	return 0;
}

static int __app_create(void *data)
{
	assert(data);
	struct appdata *ad = (struct appdata *)data;
	int ret;

	ad->handler.def_term_fn = NULL;
	ad->handler.def_timeout_fn = NULL;

	/* init internationalization */
	ret = appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR);
	if (ret != 0) {
		_E("FAIL: appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR)");
		return -1;
	}

	return 0;
}

static void unload_popup(struct appdata *ad)
{
	assert(ad);

	if (ad->win_main) {
		evas_object_del(ad->win_main);
		ad->win_main = NULL;
	}
}

static int __app_terminate(void *data)
{
	assert(data);
	struct appdata *ad = (struct appdata *)data;
	int ret;

	if (!strncmp(popup_type, ERROR_POPUP, strlen(popup_type))) {
		ret = usp_vconf_key_ignore();
		if (ret != 0) _E("FAIL: usp_vconf_key_ignore()");
	}

	unload_popup(ad);

	if (ad->b) {
		ret = bundle_free(ad->b);
		if (ret != 0) {
			_E("FAIL: bundle_free(ad->b)");
		}
		ad->b = NULL;
	}

	return 0;
}

static int __app_pause(void *data)
{
	return 0;
}

static int __app_resume(void *data)
{
	return 0;
}

static void usb_popup_response(void *data, Evas_Object * obj, void *event_info)
{
	assert(data);
	struct appdata *ad = (struct appdata *)data;

	unload_popup(ad);

	popup_terminate();
}

static int load_connection_failed_popup(void *data)
{
	assert(data);
	struct appdata *ad = (struct appdata *)data;

	if (usp_vconf_key_notify() < 0)
		_E("Failed to notify vconf key");

	if (!is_usb_connected())
		return -ECANCELED;

	ad->popup = load_normal_popup(ad,
			NULL,
			_("IDS_USB_POP_USB_CONNECTION_FAILED"),
			_("IDS_COM_SK_OK"),
			usb_popup_response,
			NULL,
			NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int load_usb_restrict_popup(void *data)
{
	assert(data);
	struct appdata *ad = (struct appdata *)data;

	ad->popup = load_normal_popup(ad,
		NULL,
		_("IDS_ST_POP_SECURITY_POLICY_PREVENTS_USE_OF_DESKTOP_SYNC"),
		_("IDS_COM_SK_OK"),
		usb_popup_response,
		NULL,
		NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int __app_reset(bundle *b, void *data)
{
	assert(data);
	struct appdata *ad = data;
	char *type;
	int ret;

	ad->b = bundle_dup(b);

	if (syspopup_has_popup(b)) {
		_D("usb-syspopup is already loaded");
		return 0;
	}

	type = (char *)bundle_get_val(b, SYSPOPUP_CONTENT);
	if (!type) {
		_E("ERROR: Non existing type of popup");
		ret = -EINVAL;
		goto out;
	}

	snprintf(popup_type, sizeof(popup_type), "%s", type);

	/* create window */
	ad->win_main = create_win(PACKAGE);
	if (!(ad->win_main)) {
		ret = -ENOMEM;
		goto out;
	}

	if (syspopup_create(ad->b, &(ad->handler), ad->win_main, ad) != 0) {
		_E("FAIL: syspopup_create()");
		ret = -ENOMEM;
		goto out;
	}

	if (!strncmp(type, ERROR_POPUP, strlen(ERROR_POPUP))) {
		_D("Connection failed popup is loaded");
		ret = load_connection_failed_popup(ad);
		goto out;
	}

	if (!strncmp(type, RESTRICT_POPUP, strlen(RESTRICT_POPUP))) {
		_D("USB restrict popup is loaded");
		ret = load_usb_restrict_popup(ad);
		goto out;
	}

	ret = -EINVAL;

out:
	if (ret < 0)
		popup_terminate();
	return ret;
}

int main(int argc, char *argv[])
{

	struct appdata ad;
	struct appcore_ops ops = {
		.create = __app_create,
		.terminate = __app_terminate,
		.pause = __app_pause,
		.resume = __app_resume,
		.reset = __app_reset,
	};

	memset(&ad, 0x0, sizeof(struct appdata));

	ops.data = &ad;

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
