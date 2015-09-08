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
#include "launcher.h"

#define ODE_ERROR_POPUP        "ode_error"
#define SETTING_ENCRYPTION_UG  "setting-mmc-encryption-efl"

#define ERROR_ENCRYPT_NOT_ENOUGH_SPACE "encrypt_not_enough_space"
#define ERROR_DECRYPT_NOT_ENOUGH_SPACE "decrypt_not_enough_space"
#define ERROR_ENCRYPT_OPERATION_FAILED "encrypt_operation_failed"
#define ERROR_DECRYPT_OPERATION_FAILED "decrypt_operation_failed"

#define BUF_MAX 256

#define DD_BUS_NAME                 "org.tizen.system.deviced"
#define DD_OBJECT_PATH_ODE          "/Org/Tizen/System/DeviceD/Ode"
#define DD_INTERFACE_NAME_ODE       DD_BUS_NAME".ode"
#define DD_SIGNAL_REMOVE_ERROR_NOTI "RequestRemoveErrorNoti"

static int show_ode_error_popup(void *data, bundle *b);

struct popup_ops ode_error_ops = {
	.name = ODE_ERROR_POPUP,
	.popup = NULL,
	.show_popup = show_ode_error_popup
};

static void cancel_clicked(void *data, Evas_Object *obj, void *event_info)
{
	int ret;
	release_evas_object(&(ode_error_ops.popup));
	ret = broadcast_dbus_signal(DD_OBJECT_PATH_ODE,
			DD_INTERFACE_NAME_ODE,
			DD_SIGNAL_REMOVE_ERROR_NOTI,
			NULL, NULL);
	if (ret < 0)
		_E("Failed to send dbus signal to remove ode error noti(%d)", ret);
	terminate_if_no_popup();
}

static void retry_clicked(void *data, Evas_Object *obj, void *event_info)
{
	bundle *b;
	int ret;

	release_evas_object(&(ode_error_ops.popup));

	b = bundle_create();
	if (!b) {
		_E("FAIL: bundle_create()");
		return;
	}

	ret = aul_launch_app(SETTING_ENCRYPTION_UG, b);
	if (ret < 0)
		_E("FAIL: aul_launch_app()");

	if (bundle_free(b) != 0)
		_E("FAIL: bundle_free(b);");

	terminate_if_no_popup();
}

static int get_popup_content(bundle *b, char *content, int len)
{
	char *err, *cSpace;
	int iSpace;
	double dSpace;
	char *buf;

	if (!b || !content || len <= 0)
		return -EINVAL;

	err = (char *)bundle_get_val(b, SIGNAL_SENDER_ERROR_TYPE);
	if (!err) {
		_E("Failed to get error type");
		return -EINVAL;
	}

	cSpace = (char *)bundle_get_val(b, SIGNAL_SENDER_MEMORY_SPACE);
	if (!cSpace) {
		_E("Failed to get space needed");
		return -EINVAL;
	}
	iSpace = atoi(cSpace);
	dSpace = (double)iSpace/1024;
	_I("Space: (%s, %d, %f)", cSpace, iSpace, dSpace);

	if (!strncmp(err, ERROR_ENCRYPT_NOT_ENOUGH_SPACE, strlen(ERROR_ENCRYPT_NOT_ENOUGH_SPACE))) {
		buf = _("IDS_ST_BODY_UNABLE_TO_ENCRYPT_SD_CARD_NOT_ENOUGH_SPACE_ON_CARD_APPROXIMATELY_P2F_MB_NEEDED_DELETE_SOME_FILES");
		snprintf(content, len, buf, dSpace);
		return 0;
	}

	if (!strncmp(err, ERROR_ENCRYPT_OPERATION_FAILED, strlen(ERROR_ENCRYPT_OPERATION_FAILED))) {
		/* TODO: Translation */
		buf = "Encrypt process has been interupted by error. Retry to encrypt SD card?";
		snprintf(content, len, "%s", buf);
		return 0;
	}

	if (!strncmp(err, ERROR_DECRYPT_NOT_ENOUGH_SPACE, strlen(ERROR_DECRYPT_NOT_ENOUGH_SPACE))) {
		buf = _("IDS_ST_BODY_UNABLE_TO_DECRYPT_SD_CARD_NOT_ENOUGH_SPACE_ON_CARD_APPROXIMATELY_P2F_MB_NEEDED_DELETE_SOME_FILES");
		snprintf(content, len, buf, dSpace);
		return 0;
	}

	if (!strncmp(err, ERROR_DECRYPT_OPERATION_FAILED, strlen(ERROR_DECRYPT_OPERATION_FAILED))) {
		buf = "Decrypt process has been interupted by error. Retry to decrypt SD card?";
		snprintf(content, len, "%s", buf);
		return 0;
	}

	_E("Unknown type (%s)", err);
	return -EINVAL;
}

static int show_ode_error_popup(void *data, bundle *b)
{
	struct appdata *ad = data;
	char content[BUF_MAX];
	int ret;

	if (!ad || !(ad->win_main))
		return -EINVAL;

	if (ode_error_ops.popup) {
		_E("Popup already exists");
		return 0;
	}

	ret = get_popup_content(b, content, sizeof(content));
	if (ret < 0) {
		_E("Failed to get popup content");
		terminate_if_no_popup();
		return 0;
	}

	evas_object_show(ad->win_main);

	ode_error_ops.popup = load_normal_popup(ad,
			_("IDS_COM_HEADER_ATTENTION"),
			content,
			_("IDS_COM_SK_CANCEL"),
			cancel_clicked,
			_("IDS_COM_SK_RETRY_A"),
			retry_clicked);
	if (!(ode_error_ops.popup)) {
		_E("FAIL: load_normal_popup()");
		terminate_if_no_popup();
		return -ENOMEM;
	}

	if (set_display_feedback(PATTERN_WARNING) < 0)
		_E("Failed to set display and feedback");

	return 0;
}

static __attribute__ ((constructor)) void register_ode_error_popup(void)
{
	register_popup(&ode_error_ops);
}
