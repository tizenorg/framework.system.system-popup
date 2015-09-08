/*
 * popup-launcher
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <vconf.h>
#include <appsvc.h>
#include <notification.h>
#include "launcher.h"

#define BUF_MAX 256

struct ticker_info {
	const char *name;
	const char *msgid;
};

static const struct ticker_info ticker_notifications[] = {
	{ "usb-client-default"          , "IDS_COM_BODY_CONNECTED_AS_A_MEDIA_DEVICE"           },
	{ "usb-client-ssh"              , "SSH enabled"                                        },
	{ "dock-connected"              , "IDS_QP_POP_THE_DOCK_IS_CONNECTED"                   },
	{ "hdmi-connected"              , "IDS_VPL_POP_HDMI_CABLE_CONNECTED"                   },
	{ "hdmi-disconnected"           , "IDS_VPL_POP_HDMI_CABLE_DISCONNECTED"                },
	{ "connector-connected"         , "IDS_COM_POP_USB_CONNECTOR_CONNECTED"                },
	{ "connector-disconnected"      , "IDS_COM_POP_USB_CONNECTOR_DISCONNECTED"             },
	{ "keyboard-connected"          , "IDS_COM_POP_KEYBOARD_CONNECTED_ABB2"                },
	{ "mouse-connected"             , "IDS_COM_POP_MOUSE_CONNECTED_ABB2"                   },
	{ "storage-connected"           , "IDS_COM_POP_USB_MASS_STORAGE_CONNECTED_ABB2"        },
	{ "storage-ro-connected"        , "IDS_ST_BODY_READ_ONLY_USB_DEV_CONNECTED_M_NOUN_ABB" },
	{ "storage-disconnected-safe"   , "IDS_USB_BODY_USB_MASS_STORAGE_SAFELY_REMOVED"       },
	{ "storage-disconnected-unsafe" , "IDS_COM_POP_USB_MASS_STORAGE_UNEXPECTEDLY_REMOVED"  },
	{ "camera-connected"            , "IDS_COM_POP_CAMERA_CONNECTED_ABB2"                  },
	{ "printer-connected"           , "IDS_COM_POP_PRINTER_CONNECTED_ABB2"                 },
	{ "unknown-connected"           , "IDS_COM_POP_UNKNOWN_USB_DEVICE_CONNECTED"           },
	{ "device-disconnected"         , "IDS_COM_BODY_USB_DEVICE_SAFELY_REMOVED"             },
};

static void set_language(void)
{
	char *lang;
	char *r;

	lang = vconf_get_str(VCONFKEY_LANGSET);
	if (!lang)
		return;

	setenv("LANG", lang, 1);
	setenv("LC_MESSAGES", lang, 1);

	r = setlocale(LC_ALL, "");
	if (!r) {
		setlocale(LC_ALL, lang);
	}

	free(lang);
}

static int add_notification (
		int type, int layout,
		char *title, char *content,
		char *icon, char *icon_indi,
		bundle *b, int prop,
		int applist)
{
	int priv_id;
	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	set_language();

	noti = notification_create(type);
	if (!noti) {
		_E("FAIL: notification_create()");
		return -ENOMEM;
	}

	noti_err = notification_set_text_domain(noti, LANG_DOMAIN, LOCALE_DIR);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		_E("FAIL: notification_set_text_domain()");
		priv_id = -ENOMEM;
		goto out;
	}


	noti_err = notification_set_layout(noti, layout);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		_E("FAIL: notification_set_layout()");
		priv_id = -ENOMEM;
		goto out;
	}

	if (layout == NOTIFICATION_LY_ONGOING_PROGRESS) {
		noti_err = notification_set_progress(noti, (double)0.0);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			_E("FAIL: notification_setprogress()");
			priv_id = -ENOMEM;
			goto out;
		}
	}

	if (title) {
		noti_err = notification_set_text (noti,
				NOTIFICATION_TEXT_TYPE_TITLE,
				_(title),
				title,
				NOTIFICATION_VARIABLE_TYPE_NONE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			_E("FAIL: notification_set_text()");
			priv_id = -ENOMEM;
			goto out;
		}
	}

	if (content) {
		noti_err = notification_set_text (noti,
				NOTIFICATION_TEXT_TYPE_CONTENT,
				_(content),
				content,
				NOTIFICATION_VARIABLE_TYPE_NONE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			_E("FAIL: notification_set_text()");
			priv_id = -ENOMEM;
			goto out;
		}
	}

	if (icon) {
		noti_err = notification_set_image (noti,
				NOTIFICATION_IMAGE_TYPE_ICON,
				icon);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			_E("FAIL: notification_set_image()");
			priv_id = -ENOMEM;
			goto out;
		}
	}

	if (icon_indi && (applist & NOTIFICATION_DISPLAY_APP_INDICATOR)) {
		noti_err = notification_set_image (noti,
				NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
				icon_indi);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			_E("FAIL: notification_set_image()");
			priv_id = -ENOMEM;
			goto out;
		}
	}

	noti_err = notification_set_property (noti, prop);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		_E("FAIL: notification_set_property()");
		priv_id = -ENOMEM;
		goto out;
	}

	if (b) {
		noti_err = notification_set_execute_option(noti,
				NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, b);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			_E("FAIL: notification_set_pkgname()");
			priv_id = -ENOMEM;
			goto out;
		}
	}

	noti_err = notification_set_display_applist(noti, applist);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		_E("FAIL: notification_set_display_applist()");
		priv_id = -ENOMEM;
		goto out;
	}

	noti_err = notification_set_pkgname(noti, APPNAME);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		_E("FAIL: notification_set_pkgname()");
		priv_id = -ENOMEM;
		goto out;
	}

	priv_id = -1;
	noti_err = notification_insert(noti, &priv_id);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		_E("FAIL: notification_insert()");

out:
	if (noti)
		notification_free(noti);
	return priv_id;
}

static int launch_datausage_warning_notification(void)
{
	int ret;
	bundle *b;

	b = bundle_create();
	if (b)
		appsvc_set_pkgname(b, SETTING_DATAUSAGE_UG);

	ret =  add_notification(NOTIFICATION_TYPE_NOTI,
				NOTIFICATION_LY_NOTI_EVENT_SINGLE,
				"IDS_COM_BODY_DATA_USAGE_WARNING",
				"IDS_COM_BODY_TAP_TO_VIEW_USAGE_AND_SETTINGS",
				DATAUSAGE_ICON,
				DATAUSAGE_ICON,
				b,
				NOTIFICATION_PROP_VOLATILE_DISPLAY
				| NOTIFICATION_PROP_DISABLE_AUTO_DELETE,
				NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY
				| NOTIFICATION_DISPLAY_APP_TICKER
				| NOTIFICATION_DISPLAY_APP_LOCK
				| NOTIFICATION_DISPLAY_APP_INDICATOR);

	if (b)
		bundle_free(b);
	return ret;
}

static int launch_datausage_disabled_notification(void)
{
	int ret;
	bundle *b;

	b = bundle_create();
	if (b)
		appsvc_set_pkgname(b, SETTING_DATAUSAGE_UG);

	ret =  add_notification(NOTIFICATION_TYPE_ONGOING,
				NOTIFICATION_LY_ONGOING_EVENT,
				"IDS_COM_HEADER_MOBILE_DATA_DISABLED",
				"IDS_COM_BODY_TAP_TO_VIEW_USAGE_AND_SETTINGS",
				DATAUSAGE_ICON,
				DATAUSAGE_ICON,
				b,
				NOTIFICATION_PROP_VOLATILE_DISPLAY,
				NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY
				| NOTIFICATION_DISPLAY_APP_TICKER
				| NOTIFICATION_DISPLAY_APP_LOCK
				| NOTIFICATION_DISPLAY_APP_INDICATOR);

	if (b)
		bundle_free(b);
	return ret;
}

static int launch_led_torch_notification(void)
{
	int ret;
	bundle *b;

	b = bundle_create();
	if (b)
		appsvc_set_pkgname(b, SETTING_LIGHTOFF_APP);

	ret = add_notification(NOTIFICATION_TYPE_ONGOING,
				NOTIFICATION_LY_ONGOING_EVENT,
				"IDS_QP_BODY_ASSISTIVE_LIGHT_IS_ON",
				"IDS_ST_BODY_TAP_TO_TURN_OFF_ASSISTIVE_LIGHT",
				LED_TORCH_ICON,
				NULL,
				b,
				NOTIFICATION_PROP_VOLATILE_DISPLAY,
				NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

	if (b)
		bundle_free(b);
	return ret;
}

static int launch_ode_complete_notification(void *data)
{
	char *text, *content, *input;
	int ret;

	if (!data)
		return -EINVAL;

	input = (char *)data;

	if (!strncmp(input, ODE_ENCRYPT, strlen(ODE_ENCRYPT)))
		text = "IDS_ST_BODY_SD_CARD_ENCRYPTED";
	else if (!strncmp(input, ODE_DECRYPT, strlen(ODE_DECRYPT)))
		text = "IDS_ST_BODY_SD_CARD_DECRYPTED";
	else
		return -EINVAL;

	set_language();
	content = dgettext(LANG_DOMAIN, text);
	if (!content)
		return -ENOMEM;

	ret = notification_status_message_post(content);
	if (ret != NOTIFICATION_ERROR_NONE)
		_E("FAIL: notification_status_message_post(text)");

	return ret;
}

static int launch_ode_progress_notification(void *data)
{
	char *title, *input, *icon;
	int ret;
	bundle *b;

	if (!data)
		return -EINVAL;

	input = (char *)data;

	if (!strncmp(input, ODE_ENCRYPT, strlen(ODE_ENCRYPT))) {
		title = "IDS_DN_POP_ENCRYPTING_SD_CARD_ING";
		icon = ODE_ENCRYPT_ICON;
	} else if (!strncmp(input, ODE_DECRYPT, strlen(ODE_DECRYPT))) {
		title = "IDS_DN_POP_DECRYPTING_SD_CARD_ING";
		icon = ODE_DECRYPT_ICON;
	} else {
		return -EINVAL;
	}

	b = bundle_create();
	if (b)
		appsvc_set_pkgname(b, SETTING_ENCRYPTING_APP);

	ret = add_notification(NOTIFICATION_TYPE_ONGOING,
				NOTIFICATION_LY_ONGOING_PROGRESS,
				title,
				NULL,
				icon,
				icon,
				b,
				NOTIFICATION_PROP_VOLATILE_DISPLAY,
				NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY
				| NOTIFICATION_DISPLAY_APP_INDICATOR);

	if (b)
		bundle_free(b);
	return ret;
}

static int launch_ode_error_notification(void *data1, void *data2, void *data3)
{
	char *enc_type = (char *)data1;
	char err_type[BUF_MAX];
	char space[BUF_MAX];
	char *icon = NULL;
	char *content = NULL;
	int ret;
	bundle *b;

	if (!enc_type)
		return -EINVAL;

	b = bundle_create();
	if (b) {
		snprintf(err_type, sizeof(err_type), "%d", (int)data2);
		snprintf(space, sizeof(space), "%d", (int)data3);
		appsvc_set_pkgname(b, SYSTEM_SIGNAL_SENDER);
		appsvc_add_data(b, SIGNAL_SENDER_TYPE, enc_type);
		appsvc_add_data(b, SIGNAL_SENDER_ERROR_TYPE, err_type);
		appsvc_add_data(b, SIGNAL_SENDER_MEMORY_SPACE, space);

		if (!strncmp(enc_type, ODE_ENCRYPT, strlen(ODE_ENCRYPT))) {
			icon = ODE_ENCRYPT_ERROR_ICON;
			content = "IDS_ST_BODY_SD_CARD_ENCRYPTION_ERROR_OCCURRED";
		} else if (!strncmp(enc_type, ODE_DECRYPT, strlen(ODE_DECRYPT))) {
			icon = ODE_DECRYPT_ERROR_ICON;
			content = "IDS_ST_BODY_SD_CARD_DECRYPTION_ERROR_OCCURRED";
		}
	}

	ret = add_notification(NOTIFICATION_TYPE_ONGOING,
				NOTIFICATION_LY_ONGOING_EVENT,
				content,
				NULL,
				icon,
				icon,
				b,
				NOTIFICATION_PROP_VOLATILE_DISPLAY,
				NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY
				| NOTIFICATION_DISPLAY_APP_INDICATOR);

	if(b)
		bundle_free(b);
	return ret;
}

static int launch_tima_lkm_prevention_notification(void)
{
	int ret;
	bundle *b;

	b = bundle_create();
	if (b) {
		appsvc_set_pkgname(b, SYSTEM_SIGNAL_SENDER);
		appsvc_add_data(b, SIGNAL_SENDER_TYPE, SIGNAL_SENDER_TYPE_RECOVERY);
	}

	ret = add_notification(NOTIFICATION_TYPE_ONGOING,
				NOTIFICATION_LY_ONGOING_EVENT,
				"Prevention Information",
				"The device has detected an application attempting "
				"unpermitted actions and has stopped loading. To "
				"protect your device, it is recommended you reboot.",
				TIMA_ICON,
				NULL,
				b,
				NOTIFICATION_PROP_VOLATILE_DISPLAY,
				NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

	if (b)
		bundle_free(b);
	return ret;
}

static int launch_tima_pkm_detection_notification(void)
{
	int ret;
	bundle *b;

	b = bundle_create();
	if (b) {
		appsvc_set_pkgname(b, SYSTEM_SIGNAL_SENDER);
		appsvc_add_data(b, SIGNAL_SENDER_TYPE, SIGNAL_SENDER_TYPE_RECOVERY);
	}

	ret = add_notification(NOTIFICATION_TYPE_ONGOING,
				NOTIFICATION_LY_ONGOING_EVENT,
				"Detection Information",
				"The device has detected an application attempting "
				"unpermitted actions. To protect your device, it is "
				"recommended you reboot.",
				TIMA_ICON,
				NULL,
				b,
				NOTIFICATION_PROP_VOLATILE_DISPLAY,
				NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

	if (b)
		bundle_free(b);
	return ret;
}

static int launch_usb_storage_notification(void *data, int type)
{
	int ret;
	bundle *b;
	char *path = data;
	char *name;
	char *title;

	if (!path)
		return -EINVAL;

	switch(type) {
	case USB_STORAGE:
		title = "IDS_COM_POP_USB_MASS_STORAGE_CONNECTED_ABB2";
		break;
	case USB_STORAGE_RO:
		title = "IDS_ST_BODY_READ_ONLY_USB_DEV_CONNECTED_M_NOUN_ABB";
		break;
	default:
		_E("Unknown type(%d)", type);
		return -EINVAL;
	}

	name = strrchr(path, '/');
	if (name)
		name = name + 1;
	else
		name = path;

	b = bundle_create();
	if (b) {
		appsvc_set_pkgname(b, SYSTEM_SIGNAL_SENDER);
		appsvc_add_data(b, SIGNAL_SENDER_TYPE, SIGNAL_SENDER_TYPE_USBSTORAGE_UNMOUNT);
		appsvc_add_data(b, SIGNAL_SENDER_DEVICE_PATH, path);
	}

	ret = add_notification(NOTIFICATION_TYPE_ONGOING,
				NOTIFICATION_LY_ONGOING_EVENT,
				title,
				name,
				USB_ICON,
				NULL,
				b,
				NOTIFICATION_PROP_VOLATILE_DISPLAY,
				NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

	if (b)
		bundle_free(b);
	return ret;
}

static int launch_usb_device_notification(char *dev, char *name)
{
	int ret;
	bundle *b;
	char *title;

	if (!dev)
		return -EINVAL;

	if (!strncmp(dev, "keyboard", strlen(dev))) {
		title = "IDS_COM_POP_KEYBOARD_CONNECTED_ABB2";
	} else if (!strncmp(dev, "mouse", strlen(dev))) {
		title = "IDS_COM_POP_MOUSE_CONNECTED_ABB2";
	} else if (!strncmp(dev, "camera", strlen(dev))) {
		title = "IDS_COM_POP_CAMERA_CONNECTED_ABB2";
	} else if (!strncmp(dev, "printer", strlen(dev))) {
		title = "IDS_COM_POP_PRINTER_CONNECTED_ABB2";
	} else if (!strncmp(dev, "unknown", strlen(dev))) {
		title = "IDS_COM_POP_UNKNOWN_USB_DEVICE_CONNECTED";
	} else {
		_E("Invalid device");
		return -EINVAL;
	}

	b = bundle_create();
	if (b) {
		appsvc_set_pkgname(b, HOST_DEVICES);
		appsvc_add_data(b, APPOPER_TYPE, APPOPER_TYPE_DEVICE_LIST);
	}

	ret = add_notification(NOTIFICATION_TYPE_ONGOING,
				NOTIFICATION_LY_ONGOING_EVENT,
				title,
				name,
				USB_ICON,
				NULL,
				b,
				NOTIFICATION_PROP_VOLATILE_DISPLAY,
				NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

	if (b)
		bundle_free(b);
	return ret;
}

static int launch_ticker_notification(void *data)
{
	char *content, *input;
	int ret, i;

	if (!data)
		return -EINVAL;

	input = (char *)data;

	content = NULL;
	for (i = 0; i < ARRAY_SIZE(ticker_notifications); i++) {
		if (strncmp(input, ticker_notifications[i].name,
					strlen(ticker_notifications[i].name)))
			continue;

		set_language();
		content = dgettext(LANG_DOMAIN, ticker_notifications[i].msgid);
		break;
	}

	if (!content)
		return -EINVAL;

	ret = notification_status_message_post(content);
	if (ret != NOTIFICATION_ERROR_NONE)
		_E("FAIL: notification_status_message_post(text)");

	return ret;
}

static int launch_battery_full_notification(void)
{
	int ret;

	ret =  add_notification(NOTIFICATION_TYPE_ONGOING,
				NOTIFICATION_LY_ONGOING_EVENT,
				"IDS_IDLE_POP_BATTERY_FULLY_CAHRGED",
				"IDS_SYNCML_POP_DM_REMOVE_CHARGER",
				BATT_NOTI_ICON,
				BATT_INDI_ICON,
				NULL,
				NOTIFICATION_PROP_DISABLE_APP_LAUNCH,
				NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY
				| NOTIFICATION_DISPLAY_APP_INDICATOR);

	return ret;
}

static int launch_notification_no_param_by_type(int type)
{
	switch (type) {
	case DATAUSAGE_WARNING:
		return launch_datausage_warning_notification();
	case DATAUSAGE_DISABLED:
		return launch_datausage_disabled_notification();
	case LED_TORCH:
		return launch_led_torch_notification();
	case TIMA_LKM_PREVENTION:
		return launch_tima_lkm_prevention_notification();
	case TIMA_PKM_DETECTION:
		return launch_tima_pkm_detection_notification();
	case BATTERY_FULL:
		return launch_battery_full_notification();
	default:
		_E("Noti type is unknown");
		return -EINVAL;
	}
}

static int launch_notification_single_param_by_type(int type, void *data)
{
	switch (type) {
	case ODE_COMPLETE:
		return launch_ode_complete_notification(data);
	case ODE_PROGRESS:
		return launch_ode_progress_notification(data);
	case TICKER:
		return launch_ticker_notification(data);
	case USB_STORAGE:
	case USB_STORAGE_RO:
		return launch_usb_storage_notification(data, type);
	default:
		_E("Noti type is unknown");
		return -EINVAL;
	}
}

static int launch_notification_double_param_by_type(int type, char *data1, char *data2)
{
	switch (type) {
	case USB_DEVICE:
		return launch_usb_device_notification(data1, data2);
	default:
		_E("Noti type is unknown");
		return -EINVAL;
	}
}

static int launch_notification_triple_param_by_type(int type, void *data1, void *data2, void *data3)
{
	switch (type) {
	case ODE_ERROR:
		return launch_ode_error_notification(data1, data2, data3);
	default:
		_E("Noti type is unknown");
		return -EINVAL;
	}
}


DBusMessage *activate_notification_no_param(E_DBus_Object *obj, DBusMessage *msg, int type)
{
	DBusMessageIter iter;
	DBusMessage *reply;
	int priv_id;

	_D("Load notification");

	priv_id = launch_notification_no_param_by_type(type);
	if (priv_id < 0)
		_E("FAIL: launch_noti_by_type()");

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &priv_id);

	return reply;
}

DBusMessage *activate_notification_single_param(E_DBus_Object *obj, DBusMessage *msg, int type)
{
	DBusError err;
	DBusMessageIter iter;
	DBusMessage *reply;
	int priv_id;
	char *value;

	_D("Load notification");

	dbus_error_init(&err);

	if (!dbus_message_get_args(msg, &err,
		    DBUS_TYPE_STRING, &value,
			DBUS_TYPE_INVALID)) {
		_E("there is no message");
		priv_id = -EINVAL;
		goto out;
	}

	priv_id = launch_notification_single_param_by_type(type, value);
	if (priv_id < 0)
		_E("FAIL: launch_noti_single_param_by_type()");

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &priv_id);

	return reply;
}

DBusMessage *activate_notification_double_param(E_DBus_Object *obj, DBusMessage *msg, int type)
{
	DBusError err;
	DBusMessageIter iter;
	DBusMessage *reply;
	int priv_id;
	char *str1;
	char *str2;

	dbus_error_init(&err);

	if (!dbus_message_get_args(msg, &err,
		    DBUS_TYPE_STRING, &str1,
			DBUS_TYPE_STRING, &str2,
			DBUS_TYPE_INVALID)) {
		_E("there is no message");
		priv_id = -EINVAL;
		goto out;
	}

	priv_id = launch_notification_double_param_by_type(type, str1, str2);
	if (priv_id < 0)
		_E("FAIL: launch_noti_single_param_by_type()");

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &priv_id);

	return reply;
}

DBusMessage *activate_notification_triple_param(E_DBus_Object *obj, DBusMessage *msg, int type)
{
	DBusError err;
	DBusMessageIter iter;
	DBusMessage *reply;
	int priv_id;
	char *data1;
	int data2, data3;

	dbus_error_init(&err);

	if (!dbus_message_get_args(msg, &err,
		    DBUS_TYPE_STRING, &data1,
			DBUS_TYPE_INT32, &data2,
			DBUS_TYPE_INT32, &data3,
			DBUS_TYPE_INVALID)) {
		_E("there is no message");
		priv_id = -EINVAL;
		goto out;
	}

	priv_id = launch_notification_triple_param_by_type(type, (void *)data1, (void *)data2, (void *)data3);
	if (priv_id < 0)
		_E("FAIL: launch_noti_single_param_by_type()");

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &priv_id);

	return reply;
}

DBusMessage *deactivate_notification(E_DBus_Object *obj, DBusMessage *msg)
{
	DBusError err;
	DBusMessageIter iter;
	DBusMessage *reply;
	int value;
	int ret;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	dbus_error_init(&err);

	if (!dbus_message_get_args(msg, &err,
		    DBUS_TYPE_INT32, &value,
			DBUS_TYPE_INVALID)) {
		_E("there is no message");
		ret = -EINVAL;
		goto out;
	}

	_D("Remove notification (noti id: %d)", value);

	noti_err = notification_delete_by_priv_id(APPNAME,
			NOTIFICATION_TYPE_NOTI, value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		_E("FAIL: notification_delete_by_priv_id(): %d", noti_err);
		ret = -EBADF;
		goto out;
	}

	ret = 0;

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}

static int progress_update_notification_by_progress_rate(int priv_id, int rate)
{
	int ret;
	notification_h noti;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	if (rate < 0 || rate > 100) {
		_E("Improper rate: %d", rate);
		return -EINVAL;
	}

	noti = notification_load(APPNAME, priv_id);
	if (!noti) {
		_E("FAIL: notification_load()");
		return -ENOMEM;
	}

	noti_err = notification_set_progress(noti, rate/100.f);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		_E("FAIL: notification_set_progress()");
		ret = -ENOMEM;
		goto out;
	}

	noti_err = notification_update(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		_E("FAIL: notification_update()");
		ret = -ENOMEM;
		goto out;
	}

	ret = 0;

out:
	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		_E("FAIL: notification_free()");
	return ret;
}

DBusMessage *progress_update_notification(E_DBus_Object *obj, DBusMessage *msg)
{
	DBusError err;
	DBusMessageIter iter;
	DBusMessage *reply;
	int priv_id;
	int rate;
	int ret;

	dbus_error_init(&err);

	if (!dbus_message_get_args(msg, &err,
		    DBUS_TYPE_INT32, &priv_id,
			DBUS_TYPE_INT32, &rate,
			DBUS_TYPE_INVALID)) {
		_E("there is no message");
		ret = -EINVAL;
		goto out;
	}

	_D("Progress update notification (noti id: %d, rate: %d\%)", priv_id, rate);

	ret = progress_update_notification_by_progress_rate(priv_id, rate);
	if (ret < 0)
		_E("FAIL: progress_update_notification()");

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}

static int update_usb_device_notification(int id, int num, char *str1, char *str2)
{
	int ret;
	notification_h noti;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	char *title, *content;

	if (num <= 0)
		return -EINVAL;

	if (num == 1) {
		content = str2;
		if (!strncmp(str1, "keyboard", strlen(str1))) {
			title = "IDS_COM_POP_KEYBOARD_CONNECTED_ABB2";
		} else if (!strncmp(str1, "mouse", strlen(str1))) {
			title = "IDS_COM_POP_MOUSE_CONNECTED_ABB2";
		} else if (!strncmp(str1, "camera", strlen(str1))) {
			title = "IDS_COM_POP_CAMERA_CONNECTED_ABB2";
		} else if (!strncmp(str1, "printer", strlen(str1))) {
			title = "IDS_COM_POP_PRINTER_CONNECTED_ABB2";
		} else if (!strncmp(str1, "unknown", strlen(str1))) {
			title = "IDS_COM_POP_UNKNOWN_USB_DEVICE_CONNECTED";
		} else {
			_E("Invalid device");
			return -EINVAL;
		}

	} else {
		title = "IDS_USB_MBODY_PD_USB_DEVICES_CONNECTED";
		content = "Tap to show devices"; /* TODO: Translation */
	}

	set_language();

	noti = notification_load(APPNAME, id);
	if (!noti) {
		_E("FAIL: notification_load()");
		return -ENOMEM;
	}

	noti_err = notification_set_text_domain(noti, LANG_DOMAIN, LOCALE_DIR);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		_E("FAIL: notification_set_text_domain()");
		ret = -ENOMEM;
		goto out;
	}

	if (title) {
		if (num > 1)
			noti_err = notification_set_text (noti,
					NOTIFICATION_TEXT_TYPE_TITLE,
					_(title),
					title,
					NOTIFICATION_VARIABLE_TYPE_INT,
					num,
					NOTIFICATION_VARIABLE_TYPE_NONE);
		else
			noti_err = notification_set_text (noti,
					NOTIFICATION_TEXT_TYPE_TITLE,
					_(title),
					title,
					NOTIFICATION_VARIABLE_TYPE_NONE);

		if (noti_err != NOTIFICATION_ERROR_NONE) {
			_E("FAIL: notification_set_text()");
			ret = -ENOMEM;
			goto out;
		}
	}

	if (content) {
		noti_err = notification_set_text (noti,
				NOTIFICATION_TEXT_TYPE_CONTENT,
				_(content),
				content,
				NOTIFICATION_VARIABLE_TYPE_NONE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			_E("FAIL: notification_set_text()");
			ret = -ENOMEM;
			goto out;
		}
	}
	noti_err = notification_update(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		_E("FAIL: notification_update()");
		ret = -ENOMEM;
		goto out;
	}

	ret = 0;

out:
	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		_E("FAIL: notification_free()");
	return ret;
}

static int text_update_notification(int type, int id, char *data, char *str1, char *str2)
{
	int num;
	switch (type) {
	case USB_DEVICE:
		num = atoi(data);
		return update_usb_device_notification(id, num, str1, str2);
	default:
		return -EINVAL;
	}
}

DBusMessage *update_notification_double_param(E_DBus_Object *obj, DBusMessage *msg, int type)
{
	DBusError err;
	DBusMessageIter iter;
	DBusMessage *reply;
	int id;
	char *data, *str1, *str2;
	int ret;

	dbus_error_init(&err);

	if (!dbus_message_get_args(msg, &err,
		    DBUS_TYPE_INT32, &id,
			DBUS_TYPE_STRING, &data,
			DBUS_TYPE_STRING, &str1,
			DBUS_TYPE_STRING, &str2,
			DBUS_TYPE_INVALID)) {
		_E("there is no message");
		ret = -EINVAL;
		goto out;
	}

	ret = text_update_notification(type, id, data, str1, str2);
	if (ret < 0)
		_E("FAIL: progress_update_notification()");

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}
