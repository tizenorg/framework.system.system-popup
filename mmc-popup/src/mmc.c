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
#include <appcore-efl.h>
#include "mmc.h"
#include <vconf.h>
#include <vconf-keys.h>
#include <Ecore_X.h>
#include <utilX.h>
#include <notification.h>
#include <syspopup.h>
#include <dd-deviced.h>
#include <dd-display.h>
#include <dd-mmc.h>
#include <aul.h>
#include "common.h"

#define CHECK_ACT			0
#define MOUNT_ERROR_ACT		1
#define MOUNT_RDONLY_ACT	2
#define MOUNT_CHECK_SMACK_ACT	3
#define MMC_ODE_ENCRYPT		4
#define MMC_ODE_DECRYPT		5

#define RETRY_MAX 10

#define SETTING_MMC_ENCRYPTION_UG  "setting-mmc-encryption-efl"

/* IPC to continue mounting mmc without encription/decryption */
#define DD_BUS_NAME             "org.tizen.system.deviced"
#define DD_OBJECT_PATH_ODE      "/Org/Tizen/System/DeviceD/Ode"
#define DD_INTERFACE_NAME_ODE   DD_BUS_NAME".ode"
#define DD_SIGNAL_GENERAL_MOUNT "RequestGeneralMount"
#define DD_SIGNAL_ODE_MOUNT     "RequestOdeMount"
#define DD_SIGNAL_REMOVE_MMC    "RemoveMmc"

static int option = -1;

static bool (*is_encryption_restricted)(void) = NULL;

void register_encryption_restricted_function(bool (*func)(void))
{
	if (func)
		is_encryption_restricted = func;
}

static bool is_mmc_inserted(void)
{
	int status;
	if (vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &status) == 0
			&& status != VCONFKEY_SYSMAN_MMC_REMOVED) {
		return true;
	}
	return false;
}

static bool is_mmc_mounted(void)
{
	int status;
	if (vconf_get_int(VCONFKEY_SYSMAN_MMC_MOUNT, &status) == 0
			&& status == VCONFKEY_SYSMAN_MMC_MOUNT_FAILED) {
		return false;
	}
	return true;
}

static void unregister_edbus_signal_handler(struct appdata *ad)
{
	if (!ad)
		return;

	e_dbus_signal_handler_del(ad->edbus_conn, ad->edbus_handler);
	e_dbus_connection_close(ad->edbus_conn);
	e_dbus_shutdown();
}

static void mmc_removed_signal_cb(void *data, DBusMessage *msg)
{
	unregister_edbus_signal_handler(data);
	popup_terminate();
}

static int register_edbus_signal_handler(struct appdata *ad)
{
	int retry;
	int ret;

	if (!ad)
		return -EINVAL;

	retry = 0;
	while (e_dbus_init() == 0) {
		retry++;
		if (retry >= RETRY_MAX) {
			return -ECONNREFUSED;
		}
	}

	ad->edbus_conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
	if (!(ad->edbus_conn)) {
		ret = -ECONNREFUSED;
		goto edbus_handler_out;
	}

	ad->edbus_handler = e_dbus_signal_handler_add(ad->edbus_conn, NULL, DD_OBJECT_PATH_ODE,
			DD_INTERFACE_NAME_ODE, DD_SIGNAL_REMOVE_MMC, mmc_removed_signal_cb, ad);
	if (!(ad->edbus_handler)) {
		ret = -ECONNREFUSED;
		goto edbus_handler_connection_out;
	}

	return 0;

edbus_handler_connection_out:
	e_dbus_connection_close(ad->edbus_conn);
edbus_handler_out:
	e_dbus_shutdown();
	return ret;
}

static void mmc_mount_status_changed_cb(keynode_t *in_key, void *data)
{
	if (vconf_keynode_get_int(in_key) != VCONFKEY_SYSMAN_MMC_MOUNT_FAILED)
		popup_terminate();
}

static void unregister_vconf_mmc_mount(void)
{
	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_MMC_MOUNT, mmc_mount_status_changed_cb) != 0)
		_E("FAIL: vconf_ignore_key_changed()");
}

static int register_vconf_mmc_mount(struct appdata *ad)
{
	int ret;

	if (!ad)
		return -EINVAL;

	ret = vconf_notify_key_changed(VCONFKEY_SYSMAN_MMC_MOUNT, mmc_mount_status_changed_cb, ad);
	if (ret < 0) {
		_E("FAIL: vconf_notify_key_changed()");
		return ret;
	}

	return 0;
}

void mmc_response(void *data, Evas_Object *obj, void *event_info)
{
	if (option == MOUNT_CHECK_SMACK_ACT)
		deviced_call_predef_action(PREDEF_CHECK_SMACK_MMC, 0);

	if (data != NULL)
		object_cleanup(data);

	popup_terminate();
}

static void mmc_err_response (void *data, Evas_Object *obj, void *event_info)
{
	unregister_vconf_mmc_mount();
	object_cleanup(data);
	popup_terminate();
}

static void launch_setting_encryption_ug(void)
{
	int ret;
	bundle *b;

	b = bundle_create();
	if (!b) {
		_E("FAIL: bundle_create()");
		return;
	}

	ret = aul_launch_app(SETTING_MMC_ENCRYPTION_UG, b);
	if (ret != AUL_R_OK) {
		_E("FAIL: aul_launch_app()");
	}

	if (bundle_free(b) != 0)
		_E("FAIL: bundle_free(b);");
}

static void ode_launch_setting_cb(void *data, Evas_Object * obj, void *event_info)
{
	if (data) {
		unregister_edbus_signal_handler(data);
		object_cleanup(data);
	}

	launch_setting_encryption_ug();

	popup_terminate();
}

static void send_general_mount_signal_cb(void *data, Evas_Object * obj, void *event_info)
{
	int ret;

	if (data) {
		unregister_edbus_signal_handler(data);
		object_cleanup(data);
	}

	ret = broadcast_dbus_signal(DD_OBJECT_PATH_ODE,
			DD_INTERFACE_NAME_ODE,
			DD_SIGNAL_GENERAL_MOUNT,
			NULL, NULL);
	if (ret < 0)
		_E("FAIL: broadcast_dbus_signal()");

	popup_terminate();
}

static void send_ode_mount_signal_cb(void *data, Evas_Object * obj, void *event_info)
{
	int ret;

	if (data) {
		unregister_edbus_signal_handler(data);
		object_cleanup(data);
	}

	ret = broadcast_dbus_signal(DD_OBJECT_PATH_ODE,
			DD_INTERFACE_NAME_ODE,
			DD_SIGNAL_ODE_MOUNT,
			NULL, NULL);
	if (ret < 0)
		_E("FAIL: broadcast_dbus_signal()");

	popup_terminate();
}

static void ode_later_clicked(void *data, Evas_Object * obj, void *event_info)
{
	if (data) {
		unregister_edbus_signal_handler(data);
		object_cleanup(data);
	}

	/* remove the mounted sdcard */
	deviced_request_unmount_mmc(NULL, true);

	/* Turn on the "Encrypt SD card" option on the Settings app */
	if (vconf_set_bool(VCONFKEY_SETAPPL_MMC_ENCRYPTION_STATUS_BOOL, 1) != 0)
		_E("Failed to set MMC encryption status to true");

	popup_terminate();
}

static Evas_Object *load_encrypt_popup(struct appdata *ad)
{
	return load_popup_with_vertical_buttons(ad,
			_("IDS_DN_BODY_ENCRYPT_SD_CARD"),
			_("IDS_ST_POP_TO_USE_YOUR_SD_CARD_IT_MUST_BE_ENCRYPTED_ENCRYPT_SD_CARD_OR_DISABLE_DEVICE_ENCRYPTION_Q"),
			_("IDS_ST_BUTTON_ENCRYPT_SD_CARD_ABB"),
			ode_launch_setting_cb,
			_("IDS_ST_BUTTON_DISABLE_ENCRYPTION_ABB"),
			send_general_mount_signal_cb);
}

static Evas_Object *load_encrypt_popup_restricted(struct appdata *ad)
{
	return load_popup_with_vertical_buttons(ad,
			_("IDS_DN_BODY_ENCRYPT_SD_CARD"),
			_("IDS_ST_POP_THIS_SD_CARD_IS_NOT_ENCRYPTED_YOU_CANNOT_DISABLE_ENCRYPT_SD_CARD_BECAUSE_IT_HAS_BEEN_ENABLED_BY_AN_ADMINISTRATOR_ENCRYPT_Q"),
			_("IDS_HEALTH_BUTTON_LATER_ABB"),
			ode_later_clicked,
			_("IDS_ST_BUTTON_ENCRYPT_SD_CARD_ABB"),
			ode_launch_setting_cb);
}

static int launch_mmc_ode_encrypt_popup(struct appdata *ad)
{
	int ret;

	if (!ad)
		return -EINVAL;

	ret = register_edbus_signal_handler(ad);
	if (ret < 0) {
		return ret;
	}

	if (is_encryption_restricted && is_encryption_restricted()) {
		ad->popup = load_encrypt_popup_restricted(ad);
	} else {
		ad->popup = load_encrypt_popup(ad);
	}

	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		ret = -ENOMEM;
		goto mmc_ode_encrypt_out;
	}

	if (is_mmc_inserted() == false) {
		_E("mmc is not inserted");
		ret = -ENODEV;
		goto mmc_ode_encrypt_out;
	}

	return 0;

mmc_ode_encrypt_out:
	unregister_edbus_signal_handler(ad);
	return ret;
}

static int launch_mmc_ode_decrypt_popup(struct appdata *ad)
{
	int ret;

	if (!ad)
		return -EINVAL;

	ret = register_edbus_signal_handler(ad);
	if (ret < 0) {
		return ret;
	}

	ad->popup = load_popup_with_vertical_buttons(ad,
			_("IDS_DN_BODY_DECRYPT_SD_CARD"),
			_("IDS_ST_POP_TO_USE_YOUR_SD_CARD_IT_MUST_BE_DECRYPTED_DECRYPT_SD_CARD_OR_ENABLE_DEVICE_ENCRYPTION_Q"),
			_("IDS_ST_BUTTON_DECRYPT_SD_CARD_ABB"),
			ode_launch_setting_cb,
			_("IDS_ST_BUTTON_ENABLE_ENCRYPTION_ABB"),
			send_ode_mount_signal_cb);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		ret = -ENOMEM;
		goto mmc_ode_decrypt_out;
	}

	if (is_mmc_inserted() == false) {
		_E("mmc is not inserted");
		ret = -ENODEV;
		goto mmc_ode_decrypt_out;
	}

	return 0;

mmc_ode_decrypt_out:
	unregister_edbus_signal_handler(ad);
	return ret;
}

static int launch_mount_error_popup(struct appdata *ad)
{
	int ret;

	if (!ad)
		return -EINVAL;

	ret = register_vconf_mmc_mount(ad);
	if (ret < 0) {
		return ret;
	}

	if (is_mmc_mounted() != false) {
		_E("mmc is mounted successfully");
		ret = -EEXIST;
		goto mount_error_out;
	}

	ad->popup = load_normal_popup(ad,
			NULL,
			_("IDS_DN_POP_FAILED_TO_MOUNT_SD_CARD_REINSERT_OR_FORMAT_SD_CARD"),
			_("IDS_COM_SK_OK"),
			mmc_err_response,
			NULL,
			NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		ret = -ENOMEM;
		goto mount_error_out;
	}

	return 0;

mount_error_out:
	unregister_vconf_mmc_mount();
	return ret;
}

static int launch_mount_rdonly_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	ad->popup = load_normal_popup(ad,
			NULL,
			_("IDS_ST_BODY_SD_CARD_MOUNTED_READ_ONLY"),
			_("IDS_COM_SK_OK"),
			mmc_response,
			NULL,
			NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int launch_mount_check_smack_popup(struct appdata *ad)
{
	if (!ad)
		return -EINVAL;

	ad->popup = load_normal_popup(ad,
			NULL,
			_("IDS_MF_BODY_MMC_DATA_IS_INITIALIZING_ING"),
			NULL,
			NULL,
			NULL,
			NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

int mmc_popup_start(void *data)
{
	struct appdata *ad = data;
	int ret;

	switch (option) {
	case MMC_ODE_ENCRYPT:
		ret = launch_mmc_ode_encrypt_popup(ad);
		break;
	case MMC_ODE_DECRYPT:
		ret = launch_mmc_ode_decrypt_popup(ad);
		break;
	case MOUNT_ERROR_ACT:
		ret = launch_mount_error_popup(ad);
		break;
	case MOUNT_RDONLY_ACT:
		ret = launch_mount_rdonly_popup(ad);
		break;
	case MOUNT_CHECK_SMACK_ACT:
		ret = launch_mount_check_smack_popup(ad);
		break;
	default:
		_E("Option is unknown");
		ret = -EINVAL;
		break;
	}

	if (ret < 0)
		goto mmc_popup_start_out;

	if (option == MOUNT_ERROR_ACT || option == MOUNT_RDONLY_ACT) {
		if (set_display_feedback(PATTERN_LOWBAT) < 0)
			_E("Failed to set display and feedback");
	} else {
		if (set_display_feedback(-1) < 0)
			_E("Failed to set display");
	}

	return 0;

mmc_popup_start_out:
	popup_terminate();
	return ret;
}

/* App init */
int app_create(void *data)
{
	Evas_Object *win;
	struct appdata *ad = data;
	int ret;

	ad->handler.def_term_fn = NULL;
	ad->handler.def_timeout_fn = NULL;

	/* create window */
	win = create_win(PACKAGE);
	if (win == NULL)
		return -1;

	ad->win_main = win;

	ret = appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR);
	if (ret != 0)
		_E("FAIL: appcore_set_i18n()");

	return 0;
}

/* Terminate noti handler */
static int app_terminate(void *data)
{
	struct appdata *ad = data;

	if (ad->layout_main)
		evas_object_del(ad->layout_main);

	if (ad->win_main)
		evas_object_del(ad->win_main);

	return 0;
}

/* Pause/background */
static int app_pause(void *data)
{
	return 0;
}

/* Resume */
static int app_resume(void *data)
{
	return 0;
}


/* Reset */
static int app_reset(bundle *b, void *data)
{
	struct appdata *ad = data;
	const char *opt;
	int ret;

	opt = bundle_get_val(b, "_SYSPOPUP_CONTENT_");
	if (!opt) {
		_E("Failed to get conent");
		popup_terminate();
		return -ENOENT;
	}

	if (!strcmp(opt,"mounterr"))
		option = MOUNT_ERROR_ACT;
	else if (!strcmp(opt, "mountrdonly"))
		option = MOUNT_RDONLY_ACT;
	else if (!strcmp(opt, "checksmack"))
		option = MOUNT_CHECK_SMACK_ACT;
	else if (!strcmp(opt, "odeencrypt"))
		option = MMC_ODE_ENCRYPT;
	else if (!strcmp(opt, "odedecrypt"))
		option = MMC_ODE_DECRYPT;

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
	} else {
		ret = syspopup_create(b, &(ad->handler), ad->win_main, ad);
		if (ret < 0) {
			_E("Failed to create syspopup");
			popup_terminate();
			return ret;
		}

		evas_object_show(ad->win_main);
		/* Start Main UI */
		mmc_popup_start((void *)ad);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct appdata ad;

	/* App life cycle management */
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
