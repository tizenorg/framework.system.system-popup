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
#include <appsvc.h>

#define MYFILES_APPNAME	"org.tizen.myfile"
#define GALLERY_APPNAME	"org.tizen.gallery"

#define USB_MOUNT_ROOT_PATH	"/opt/storage"
#define USB_MOUNT_PATH		"_DEVICE_PATH_"

#define BUF_MAX 128

#define DEVICED_PATH			"/Org/Tizen/System/DeviceD"
#define DEVICED_IFACE			"org.tizen.system.deviced"
#define DEVICED_PATH_USBHOST	DEVICED_PATH"/Usbhost"
#define DEVICED_IFACE_USBHOST	DEVICED_IFACE".Usbhost"
#define SIGNAL_NAME_UNMOUNT		"unmount_storage"

static char added_path[BUF_MAX];
static char removed_path[BUF_MAX];

enum ext_app {
	EXT_MYFILES,
	EXT_GALLERY,
};

static const struct popup_ops storage_mounted_ops;
static const struct popup_ops unmount_storage_ops;
static const struct popup_ops camera_added_ops;
static const struct popup_ops storage_mount_failed_ops;
static const struct popup_ops storage_removed_unsafe_ops;

static void remove_otg_popup(const struct popup_ops *ops, char *path)
{
	int len;
	char *popup_path;

	if (ops == &storage_mounted_ops)
		popup_path = added_path;
	else if (ops == &unmount_storage_ops)
		popup_path = removed_path;
	else
		popup_path = NULL;

	if (popup_path) {
		if (!path)
			return;
		len = strlen(popup_path);
		if (len != strlen(path))
			return;
		if (strncmp(popup_path, path, len))
			return;
	}

	unload_simple_popup(ops);
}

static void set_myfiles_param(bundle *b)
{
	if (!b)
		return;

	if (strlen(added_path) > 0)
		appsvc_add_data(b, "path", added_path);
	else
		appsvc_add_data(b, "path", USB_MOUNT_ROOT_PATH);
	appsvc_set_pkgname(b, MYFILES_APPNAME);
}

static void set_gallery_param(bundle *b)
{
	if (!b)
		return;
	appsvc_add_data(b, "album-id", "GALLERY_ALBUM_PTP_ID");
	appsvc_set_pkgname(b, GALLERY_APPNAME);
}

static struct ext_app_type {
	int type;
	void (*set_param)(bundle *b);
} app_type[] = {
	{ EXT_MYFILES	, set_myfiles_param	},
	{ EXT_GALLERY	, set_gallery_param	},
};

static void launch_app(int type)
{
	bundle *b;
	int ret, i, type_len;

	type_len = ARRAY_SIZE(app_type);
	for (i = 0 ; i < ARRAY_SIZE(app_type) ; i++) {
		if (type == app_type[i].type)
			break;
	}
	if (i == type_len) {
		_E("Invalid type (%d)", type);
		return;
	}

	b = bundle_create();
	if (b) {
		appsvc_set_operation(b, APPSVC_OPERATION_VIEW);
		if (app_type[i].set_param)
			app_type[i].set_param(b);
		ret = appsvc_run_service(b, 0, NULL, NULL);
		if (ret < 0)
			_E("Failed to launch (%d) app(%d)", type, ret);
		bundle_free(b);
	}
}

static void storage_browse(const struct popup_ops *ops)
{
	unload_simple_popup(ops);
	launch_app(EXT_MYFILES);
	terminate_if_no_popup();
}

static void camera_browse(const struct popup_ops *ops)
{
	unload_simple_popup(ops);
	launch_app(EXT_GALLERY);
	terminate_if_no_popup();
}

static void storage_unmount(const struct popup_ops *ops)
{
	char *param[1];
	int ret;

	unload_simple_popup(ops);

	param[0] = removed_path;

	ret = broadcast_dbus_signal(DEVICED_PATH_USBHOST,
			DEVICED_IFACE_USBHOST,
			SIGNAL_NAME_UNMOUNT, "s", param);
	if (ret < 0)
		_E("FAIL: broadcast_dbus_signal()");

	memset(removed_path, 0, sizeof(removed_path));

	terminate_if_no_popup();
}

static void storage_mounted_launch(const struct popup_ops *ops)
{
	int ret;
	struct object_ops *obj;
	char *path;

	if (!ops)
		return;

	ret = get_object_by_ops(ops, &obj);
	if (ret < 0) {
		_E("Failed to get object (%d)", ret);
		return;
	}

	path = (char *)bundle_get_val(obj->b, USB_MOUNT_PATH);
	if (!path) {
		_E("Failed to get mount path");
		return;
	}

	_I("USB storage mount path (%s)", path);
	snprintf(added_path, sizeof(added_path), "%s", path);
}

static void unmount_storage_launch(const struct popup_ops *ops)
{
	int ret;
	struct object_ops *obj;
	char *path;

	if (!ops)
		return;

	ret = get_object_by_ops(ops, &obj);
	if (ret < 0) {
		_E("Failed to get object (%d)", ret);
		return;
	}

	path = (char *)bundle_get_val(obj->b, USB_MOUNT_PATH);
	if (!path) {
		_E("Failed to get mount path");
		return;
	}

	remove_otg_popup(&storage_mounted_ops, path);

	snprintf(removed_path, sizeof(removed_path), "%s", path);
}

static int storage_unmounted(bundle *b, const struct popup_ops *ops)
{
	char *path;

	if (!b || !ops)
		return -EINVAL;

	path = (char *)bundle_get_val(b, USB_MOUNT_PATH);
	if (!path) {
		_E("Failed to get mount path");
		return -ENOENT;
	}

	remove_otg_popup(&storage_mounted_ops, path);
	remove_otg_popup(&unmount_storage_ops, path);

	terminate_if_no_popup();

	return 0;
}

static int camera_removed(bundle *b, const struct popup_ops *ops)
{
	remove_otg_popup(&camera_added_ops, NULL);
	terminate_if_no_popup();
	return 0;
}

static const struct popup_ops storage_mounted_ops = {
	.name			= "usbotg_storage_mounted",
	.show_popup		= load_simple_popup,
	.title			= "IDS_ST_BODY_USB_STORAGE_ABB",
	.content		= "IDS_USB_BODY_BROWSE_STORAGE_CONNECTED_VIA_USB_Q",
	.left_text		= "IDS_COM_SK_CANCEL",
	.right_text		= "IDS_BT_SK_BROWSE",
	.right			= storage_browse,
	.launch			= storage_mounted_launch,
	.flags			= SCROLLABLE,
};

static const struct popup_ops storage_unmounted_ops = {
	.name			= "usbotg_storage_unmounted",
	.show_popup		= storage_unmounted,
};

static const struct popup_ops unmount_storage_ops = {
	.name			= "usbotg_unmount_storage",
	.show_popup		= load_simple_popup,
	.title			= "IDS_ST_BODY_USB_STORAGE_ABB",
	.content		= "IDS_COM_POP_UNMOUNT_USB_MASS_STORAGE_BEFORE_REMOVING_TO_AVOID_DATA_LOSS",
	.left_text		= "IDS_COM_SK_CANCEL",
	.right_text		= "IDS_USB_BUTTON_UNMOUNT",
	.right			= storage_unmount,
	.launch			= unmount_storage_launch,
	.flags			= SCROLLABLE,
};

static const struct popup_ops camera_added_ops = {
	.name			= "usbotg_camera_added",
	.show_popup		= load_simple_popup,
	.title			= "IDS_CAM_HEADER_CAMERA_M_APPLICATION",
	.content		= "IDS_USB_BODY_BROWSE_CAMERA_CONNECTED_VIA_USB_Q",
	.left_text		= "IDS_COM_SK_CANCEL",
	.right_text		= "IDS_BT_SK_BROWSE",
	.right			= camera_browse,
	.flags			= SCROLLABLE,
};

static const struct popup_ops camera_removed_ops = {
	.name			= "usbotg_camera_removed",
	.show_popup		= camera_removed,
};

static const struct popup_ops storage_mount_failed_ops = {
	.name			= "usbotg_storage_mount_failed",
	.show_popup		= load_simple_popup,
	.title			= "IDS_COM_HEADER_ATTENTION",
	.content		= "IDS_COM_BODY_USB_STORAGE_BLANK_OR_HAS_UNSUPPORTED_FILE_SYSTEM",
	.left_text		= "IDS_COM_SK_OK",
	.flags			= SCROLLABLE,
};

static const struct popup_ops storage_removed_unsafe_ops = {
	.name			= "usbotg_storage_removed_unsafe",
	.show_popup		= load_simple_popup,
	.title			= "IDS_COM_HEADER_ATTENTION",
	.content		= "IDS_COM_POP_USB_MASS_STORAGE_UNEXPECTEDLY_REMOVED",
	.left_text		= "IDS_COM_SK_OK",
	.flags			= SCROLLABLE,
};


/* Constructor to register mount_failed button */
static __attribute__ ((constructor)) void usbotg_register_popup(void)
{
	register_popup(&storage_mounted_ops);
	register_popup(&storage_unmounted_ops);
	register_popup(&unmount_storage_ops);
	register_popup(&camera_added_ops);
	register_popup(&camera_removed_ops);
	register_popup(&storage_mount_failed_ops);
	register_popup(&storage_removed_unsafe_ops);
}
