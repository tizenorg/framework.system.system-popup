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

#define SETTING_MMC_ENCRYPTION_UG  "setting-mmc-encryption-efl"

#define DD_BUS_NAME             "org.tizen.system.deviced"
#define DD_OBJECT_PATH_ODE      "/Org/Tizen/System/DeviceD/Ode"
#define DD_INTERFACE_NAME_ODE   DD_BUS_NAME".ode"
#define DD_SIGNAL_GENERAL_MOUNT "RequestGeneralMount"
#define DD_SIGNAL_ODE_MOUNT     "RequestOdeMount"
#define DD_SIGNAL_REMOVE_MMC    "RemoveMmc"

static const struct popup_ops mount_error_ops;
static const struct popup_ops mount_read_only_ops;
static const struct popup_ops check_smack_ops;
static const struct popup_ops ode_encrypt_ops;
static const struct popup_ops ode_decrypt_ops;

static void remove_other_mmc_popups(const struct popup_ops *ops)
{
	if (ops != &mount_error_ops)
		unload_simple_popup(&mount_error_ops);

	if (ops != &mount_read_only_ops)
		unload_simple_popup(&mount_read_only_ops);

	if (ops != &check_smack_ops)
		unload_simple_popup(&check_smack_ops);

	if (ops != &ode_encrypt_ops)
		unload_simple_popup(&ode_encrypt_ops);

	if (ops != &ode_decrypt_ops)
		unload_simple_popup(&ode_decrypt_ops);
}

static bool mmc_inserted(void)
{
	int val;
	if (vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &val) == 0
			&& val != VCONFKEY_SYSMAN_MMC_REMOVED)
		return true;
	return false;
}

static bool mmc_mounted(void)
{
	int val;
	if (vconf_get_int(VCONFKEY_SYSMAN_MMC_MOUNT, &val) == 0
			&& val == VCONFKEY_SYSMAN_MMC_MOUNT_FAILED)
		return false;
	return true;
}

static void send_mount_signal(char *signal)
{
	int ret;

	if (!signal)
		return;

	ret = broadcast_dbus_signal(
			DD_OBJECT_PATH_ODE,
			DD_INTERFACE_NAME_ODE,
			signal,
			NULL, NULL);
	if (ret < 0)
		_E("FAIL: broadcast_dbus_signal(%s:%d)", signal, ret);
}

static void launch_app(char *appname)
{
	bundle *b;
	int ret;

	if (!appname)
		return;

	b = bundle_create();
	if (b) {
		ret = aul_launch_app(appname, b);
		if (ret < 0)
			_E("FAIL: aul_launch_app(%d)", ret);
		if (bundle_free(b) != 0)
			_E("FAIL: bundle_free(b);");
	} else {
		_E("Failed to create bundle");
	}
}

static void send_general_mount_signal(const struct popup_ops *ops)
{
	unload_simple_popup(ops);
	send_mount_signal(DD_SIGNAL_GENERAL_MOUNT);
	terminate_if_no_popup();
}

static void send_ode_mount_signal(const struct popup_ops *ops)
{
	unload_simple_popup(ops);
	send_mount_signal(DD_SIGNAL_ODE_MOUNT);
	terminate_if_no_popup();
}

static void ode_launch_settings(const struct popup_ops *ops)
{
	unload_simple_popup(ops);
	launch_app(SETTING_MMC_ENCRYPTION_UG);
	terminate_if_no_popup();
}

static bool skip_mount_error_popup(const struct popup_ops *ops)
{
	return mmc_mounted();
}

static bool skip_ode_popup(const struct popup_ops *ops)
{
	return !mmc_inserted();
}

static E_DBus_Signal_Handler *mmc_removed_handler= NULL;

static void unregister_ode_handler(const struct popup_ops *ops)
{
	if (mmc_removed_handler) {
		unregister_dbus_signal_handler(mmc_removed_handler);
		mmc_removed_handler = NULL;
	}
}

static void mmc_removed(void *data, DBusMessage *msg)
{
	const struct popup_ops *ops = data;

	unregister_ode_handler(ops);
	unload_simple_popup(ops);
	terminate_if_no_popup();
}

static void register_ode_handler(const struct popup_ops *ops)
{
	int ret;

	unregister_ode_handler(ops);

	ret = register_dbus_signal_handler(&mmc_removed_handler,
			DD_OBJECT_PATH_ODE,
			DD_INTERFACE_NAME_ODE,
			DD_SIGNAL_REMOVE_MMC,
			mmc_removed,
			(void*)ops);
	if (ret < 0)
		_E("Failed to register mmc removed signal handler(%d)", ret);
}

static void mmc_mount_status_changed(keynode_t *in_key, void *data)
{
	const struct popup_ops *ops = data;

	if (vconf_keynode_get_int(in_key) == VCONFKEY_SYSMAN_MMC_MOUNT_FAILED)
		return;

	unload_simple_popup(ops);
	terminate_if_no_popup();
}

static void register_mmc_mount_handler(const struct popup_ops *ops)
{
	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_MMC_MOUNT,
				mmc_mount_status_changed, (void *)ops) != 0)
		_E("Failed to register mmc mount handler");
}

static void unregister_mmc_mount_handler(const struct popup_ops *ops)
{
	vconf_ignore_key_changed(VCONFKEY_SYSMAN_MMC_MOUNT,
			mmc_mount_status_changed);
}

static void launch_mmc_popup(const struct popup_ops *ops)
{
	remove_other_mmc_popups(ops);
	unregister_ode_handler(ops);

	if (ops == &mount_error_ops)
		register_mmc_mount_handler(ops);

	if (ops == &ode_encrypt_ops ||
		ops == &ode_decrypt_ops)
		register_ode_handler(ops);
}

static void terminate_mmc_popup(const struct popup_ops *ops)
{
	unregister_mmc_mount_handler(ops);
	unregister_ode_handler(ops);
}

static const struct popup_ops mount_error_ops = {
	.name		= "mounterr",//"mmc_mount_error",
	.show_popup	= load_simple_popup,
	.content	= "IDS_DN_POP_FAILED_TO_MOUNT_SD_CARD_REINSERT_OR_FORMAT_SD_CARD",
	.left_text	= "IDS_COM_SK_OK",
	.skip		= skip_mount_error_popup,
	.launch		= launch_mmc_popup,
	.flags		= SCROLLABLE,
};

static const struct popup_ops mount_read_only_ops = {
	.name		= "mountrdonly",//"mmc_mount_read_only",
	.show_popup	= load_simple_popup,
	.content	= "IDS_ST_BODY_SD_CARD_MOUNTED_READ_ONLY",
	.left_text	= "IDS_COM_SK_OK",
	.launch		= launch_mmc_popup,
	.flags		= SCROLLABLE,
};

static const struct popup_ops check_smack_ops = {
	.name		= "checksmack",//"mmc_check_smack",
	.show_popup	= load_simple_popup,
	.content	= "IDS_MF_BODY_MMC_DATA_IS_INITIALIZING_ING",
	.left_text	= "IDS_COM_SK_OK",
	.launch		= launch_mmc_popup,
	.flags		= SCROLLABLE,
};

static const struct popup_ops ode_encrypt_ops = {
	.name		= "odeencrypt",//"mmc_ode_encrypt",
	.show_popup	= load_simple_popup,
	.title		= "IDS_DN_BODY_ENCRYPT_SD_CARD",
	.content	= "IDS_ST_POP_TO_USE_YOUR_SD_CARD_IT_MUST_BE_ENCRYPTED_ENCRYPT_SD_CARD_OR_DISABLE_DEVICE_ENCRYPTION_Q",
	.left_text	= "IDS_ST_BUTTON_ENCRYPT_SD_CARD_ABB",
	.left		= ode_launch_settings,
	.right_text	= "IDS_ST_BUTTON_DISABLE_ENCRYPTION_ABB",
	.right		= send_general_mount_signal,
	.skip		= skip_ode_popup,
	.launch		= launch_mmc_popup,
	.terminate	= terminate_mmc_popup,
	.flags		= SCROLLABLE,
};

static const struct popup_ops ode_decrypt_ops = {
	.name		= "odedecrypt",//"mmc_ode_decrypt",
	.show_popup	= load_simple_popup,
	.title		= "IDS_DN_BODY_DECRYPT_SD_CARD",
	.content	= "IDS_ST_POP_TO_USE_YOUR_SD_CARD_IT_MUST_BE_DECRYPTED_DECRYPT_SD_CARD_OR_ENABLE_DEVICE_ENCRYPTION_Q",
	.left_text	= "IDS_ST_BUTTON_DECRYPT_SD_CARD_ABB",
	.left		= ode_launch_settings,
	.right_text	= "IDS_ST_BUTTON_ENABLE_ENCRYPTION_ABB",
	.right		= send_ode_mount_signal,
	.skip		= skip_ode_popup,
	.launch		= launch_mmc_popup,
	.terminate	= terminate_mmc_popup,
	.flags		= SCROLLABLE,
};

/* Constructor to register mmc popup */
static __attribute__ ((constructor)) void register_mmc_popup(void)
{
	register_popup(&mount_error_ops);
	register_popup(&mount_read_only_ops);
	register_popup(&check_smack_ops);
	register_popup(&ode_encrypt_ops);
	register_popup(&ode_decrypt_ops);
}
