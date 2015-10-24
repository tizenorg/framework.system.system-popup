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

#include "popup-common.h"
#include <aul.h>

#define ODE_ERROR_TYPE				"_ERROR_TYPE_"
#define ODE_MEMORY_SPACE			"_MEMORY_SPACE_"

#define ENCRYPT_NOT_ENOUGH_SPACE	"encrypt_not_enough_space"
#define DECRYPT_NOT_ENOUGH_SPACE	"decrypt_not_enough_space"
#define ENCRYPT_OPERATION_FAILED	"encrypt_operation_failed"
#define DECRYPT_OPERATION_FAILED	"decrypt_operation_failed"

#define DD_BUS_NAME					"org.tizen.system.deviced"
#define DD_OBJECT_PATH_ODE			"/Org/Tizen/System/DeviceD/Ode"
#define DD_INTERFACE_NAME_ODE		DD_BUS_NAME".ode"
#define DD_SIGNAL_REMOVE_ERROR_NOTI	"RequestRemoveErrorNoti"

#define SETTING_ENCRYPTION_UG		"setting-mmc-encryption-efl"

static void cancel_clicked(const struct popup_ops *ops)
{
	int ret;

	_I("Cancel is selected");
	unload_simple_popup(ops);

    ret = broadcast_dbus_signal(DD_OBJECT_PATH_ODE,
			DD_INTERFACE_NAME_ODE,
			DD_SIGNAL_REMOVE_ERROR_NOTI,
			NULL, NULL);
	if (ret < 0)
		_E("Failed to send dbus signal to remove ode error noti(%d)", ret);

	terminate_if_no_popup();
}

static void retry_clicked(const struct popup_ops *ops)
{
	bundle *b;
	int ret;

	_I("Retry is selected");
	unload_simple_popup(ops);

	b = bundle_create();
	if (b) {
		ret = aul_launch_app(SETTING_ENCRYPTION_UG, b);
		if (ret < 0)
			_E("FAIL: aul_launch_app(%d)", ret);
		if (bundle_free(b) != 0)
			_E("FAIL: bundle_free(b);");
	} else {
		_E("Failed to create bundle");
	}

	terminate_if_no_popup();
}

static int ode_error_get_content(const struct popup_ops *ops, char *content, unsigned int len)
{
	struct object_ops *obj;
	int ret, err_len;
	char *buf, *err, *cSpace;
	double dSpace;
	int iSpace;

	if (!ops || !content)
		return -EINVAL;

	ret = get_object_by_ops(ops, &obj);
	if (ret < 0) {
		_E("Failed to get object (%d)", ret);
		return -ENOENT;
	}

	err = (char *)bundle_get_val(obj->b, ODE_ERROR_TYPE);
	if (!err) {
		_E("Failed to get error type");
		return -ENOENT;
	}

	cSpace = (char *)bundle_get_val(obj->b, ODE_MEMORY_SPACE);
	if (!cSpace) {
		_E("Failed to get memory space");
		return -ENOENT;
	}

	iSpace = atoi(cSpace);
	dSpace = (double)iSpace/1024;
	_I("Space: (%s, %d, %f)", cSpace, iSpace, dSpace);

	err_len = strlen(err);
	if (!strncmp(err, ENCRYPT_NOT_ENOUGH_SPACE, err_len)) {
		buf = _("IDS_ST_BODY_UNABLE_TO_ENCRYPT_SD_CARD_NOT_ENOUGH_SPACE_ON_CARD_APPROXIMATELY_P2F_MB_NEEDED_DELETE_SOME_FILES");
		snprintf(content, len, buf, dSpace);
		return 0;
	}

	if (!strncmp(err, ENCRYPT_OPERATION_FAILED, err_len)) {
		buf = "Encrypt process has been interupted by error. Retry to encrypt SD card?";
		snprintf(content, len, "%s", buf);
		return 0;
	}

	if (!strncmp(err, DECRYPT_NOT_ENOUGH_SPACE, err_len)) {
		buf = _("IDS_ST_BODY_UNABLE_TO_DECRYPT_SD_CARD_NOT_ENOUGH_SPACE_ON_CARD_APPROXIMATELY_P2F_MB_NEEDED_DELETE_SOME_FILES");
		snprintf(content, len, buf, dSpace);
		return 0;

	}

	if (!strncmp(err, DECRYPT_OPERATION_FAILED, err_len)) {
		buf = "Decrypt process has been interupted by error. Retry to decrypt SD card?";
		snprintf(content, len, "%s", buf);
		return 0;
	}

	_E("Unknown type (%s)", err);
	return -EINVAL;
}

static const struct popup_ops ode_error_ops = {
	.name			= "ode_error",
	.show_popup		= load_simple_popup,
	.title			= "IDS_COM_HEADER_ATTENTION",
	.get_content	= ode_error_get_content,
	.left_text		= "IDS_COM_SK_CANCEL",
	.left			= cancel_clicked,
	.right_text		= "IDS_COM_SK_RETRY_A",
	.launch			= retry_clicked,
	.flags			= SCROLLABLE,
};

static __attribute__ ((constructor)) void ode_register_popup(void)
{
	register_popup(&ode_error_ops);
}
