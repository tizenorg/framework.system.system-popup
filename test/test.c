/*
 * systemfw-app-test
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

/* Notice:
 * User can test to launch only lowbat, mmc, usb, and datausage popup
 * using this test app since the test app receives just 2 parameters */

#include <stdio.h>
#include <stdlib.h>
#include <E_DBus.h>
#include "launcher.h"
#include "common.h"

#define DBUS_REPLY_TIMEOUT  (120 * 1000)
#define RETRY_MAX 10

#define FIN_MENU 0

struct test_menu {
	const char *text;
	void (*func)(void);
};

static int append_variant(DBusMessageIter *iter, const char *sig, char *param[])
{
	char *ch;
	int i, int_type;

	if (!sig || !param)
		return 0;

	for (ch = (char*)sig, i = 0; *ch != '\0'; ++i, ++ch) {
		switch (*ch) {
		case 's':
			dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &param[i]);
			break;
		case 'i':
			int_type = atoi(param[i]);
			dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &int_type);
			break;
		default:
			_E("ERROR: %s %c", sig, *ch);
			return -EINVAL;
		}
	}
	return 0;
}

DBusMessage *call_dbus_method(const char *dest, const char *path,
		const char *interface, const char *method,
		const char *sig, char *param[])
{
	DBusConnection *conn;
	DBusMessage *msg = NULL;
	DBusMessageIter iter;
	DBusMessage *ret;
	DBusError err;
	int r;

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (!conn) {
		ret = NULL;
		goto out;
	}

	msg = dbus_message_new_method_call(dest, path, interface, method);
	if (!msg) {
		ret = NULL;
		goto out;
	}

	dbus_message_iter_init_append(msg, &iter);
	r = append_variant(&iter, sig, param);
	if (r < 0) {
		ret = NULL;
		goto out;
	}

	/*This function is for synchronous dbus method call */
	dbus_error_init(&err);
	ret = dbus_connection_send_with_reply_and_block(conn, msg, DBUS_REPLY_TIMEOUT, &err);
	dbus_error_free(&err);

out:
	dbus_message_unref(msg);

	return ret;
}

int request_to_launch_by_dbus(char *bus, char *path, char *iface,
		char *method, char *ptype, char *param[])
{
	DBusMessage *msg;
	DBusError err;
	int i, r, ret_val;

	i = 0;
	do {
		msg = call_dbus_method(bus, path, iface, method, ptype, param);
		if (msg)
			break;
		i++;
	} while (i < RETRY_MAX);
	if (!msg) {
		_E("fail to call dbus method");
		return -ECONNREFUSED;
	}

	dbus_error_init(&err);
	r = dbus_message_get_args(msg, &err, DBUS_TYPE_INT32, &ret_val, DBUS_TYPE_INVALID);
	if (!r) {
		_E("no message : [%s:%s]", err.name, err.message);
		ret_val = -EBADMSG;
	}

	dbus_message_unref(msg);
	dbus_error_free(&err);

	return ret_val;
}

static int list_and_select_menu(const struct test_menu menu[], int size, char *title)
{
	int i, sel = -1;

	if (!menu || !title)
		return -EINVAL;

	while (1) {
		printf("\n** %s\n", title);
		for (i = 1 ; i < size ; i++) {
			printf("%3d. %s\n", i, menu[i].text);
		}
		printf("%3d. %s\n", 0, menu[0].text);
		printf("Select a menu to test: ");
		if (scanf("%d", &sel) != 1) {
			_E("FAIL: scanf()");
			return 0;
		}

		if (sel < FIN_MENU || sel > size - 1) {
			printf("[ERROR] The input number is wrong (%d)!!\n", sel);
			continue;
		}
		return sel;
	}
}

/* Notification menu */
static const struct test_menu
noti_remove_menu[] = {
	{ "Remove previous notification", NULL },
};

static void activate_normal_noti(char *bus,
		char *path,
		char *iface,
		char *method_on,
		char *type,
		char *param[],
		char *method_off)
{
	int ret;
	char str[8];
	char *noti_id[1];

	ret = request_to_launch_by_dbus(bus, path, iface,
			method_on, type, param);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);

	if (!method_off)
		return;

	/* Deactivate notification */
	snprintf(str, sizeof(str), "%d", ret);
	noti_id[0] = str;
	ret = list_and_select_menu(noti_remove_menu,
			ARRAY_SIZE(noti_remove_menu), "Remove notification");
	ret = request_to_launch_by_dbus(bus, path, iface,
			method_off, "i", noti_id);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus()");
}

static const struct test_menu
noti_update_menu[] = {
	{ "Remove progress notification", NULL },
	{ "Update progress notification", NULL },
};

static int get_progress_rate(char *title)
{
	int rate;
	if (!title)
		return -EINVAL;

	printf("\n** %s (0 ~ 100) : ", title);
	if (scanf("%d", &rate) != 1) {
		_E("FAIL: scanf()");
		return -ENOMEM;
	}

	return rate;
}

static int update_progress_noti(char *bus, char *path,
		char *iface, char *method_update, char *noti_id)
{
	int sel, ret;
	int i_rate;
	char c_rate[8];
	char *param[2];

	do {
		sel = list_and_select_menu(noti_update_menu,
				ARRAY_SIZE(noti_update_menu), "Update notification");
		if (sel == FIN_MENU)
			return sel;
		if (sel >= 0 && sel < ARRAY_SIZE(noti_update_menu))
			break;
		printf("[ERROR] The input number is wrong (%d)!!\n\n", sel);
	} while(1);

	do{
		i_rate = get_progress_rate(
				"Input progress rate to update the notification");
		if (i_rate >= 0 && i_rate <= 100)
			break;
		printf("\n[ERROR] (%d) is invalid value.\n", i_rate);
	} while(1);

	snprintf(c_rate, sizeof(c_rate), "%d", i_rate);
	param[0] = noti_id;
	param[1] = c_rate;
	ret = request_to_launch_by_dbus(bus, path, iface,
			method_update, "ii", param);
	if (ret != 0)
		return -ENOMEM;

	return 1;
}

static void activate_progress_noti(char *bus,
		char *path,
		char *iface,
		char *method_on,
		char *type_on,
		char *param_on[],
		char *method_update,
		char *method_off)
{
	int ret, sel;
	char noti_id[8];
	char *param_off[1];

	ret = request_to_launch_by_dbus(bus, path, iface,
			method_on, type_on, param_on);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);

	snprintf(noti_id, sizeof(noti_id), "%d", ret);

	do {
		sel = update_progress_noti(bus, path, iface,
				method_update, noti_id);
		if (sel == FIN_MENU)
			break;
		if (sel < 0)
			_E("FAIL: update_progress_noti()");
	} while (1);

	param_off[0] = noti_id;
	ret = request_to_launch_by_dbus(bus, path, iface,
			method_off, "i", param_off);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus()");
}

static void test_data_warning_noti(void)
{
	activate_normal_noti(BUS_NAME, POPUP_PATH_DATAUSAGE, POPUP_IFACE_DATAUSAGE,
			"WarningNotiOn", NULL, NULL, "WarningNotiOff");
}

static void test_data_disabled_noti(void)
{
	activate_normal_noti(BUS_NAME, POPUP_PATH_DATAUSAGE, POPUP_IFACE_DATAUSAGE,
			"DisabledNotiOn", NULL, NULL, "DisabledNotiOff");
}

static void test_led_torch_noti(void)
{
	activate_normal_noti(BUS_NAME, POPUP_PATH_LED, POPUP_IFACE_LED,
			"TorchNotiOn", NULL, NULL, "TorchNotiOff");
}

static void test_encrypt_complete_noti(void)
{
	char *param[1];

	param[0] = "encrypt";
	activate_normal_noti(BUS_NAME, POPUP_PATH_ODE, POPUP_IFACE_ODE,
			"CompNotiOn", "s", param, "CompNotiOff");
}

static void test_decrypt_complete_noti(void)
{
	char *param[1];

	param[0] = "decrypt";
	activate_normal_noti(BUS_NAME, POPUP_PATH_ODE, POPUP_IFACE_ODE,
			"CompNotiOn", "s", param, "CompNotiOff");
}

static void test_encrypt_progress_noti(void)
{
	char *param[1];

	param[0] = "encrypt";
	activate_progress_noti(BUS_NAME, POPUP_PATH_ODE, POPUP_IFACE_ODE,
			"ProgNotiOn", "s", param, "ProgNotiUpdate", "ProgNotiOff");
}

static void test_decrypt_progress_noti(void)
{
	char *param[1];

	param[0] = "decrypt";
	activate_progress_noti(BUS_NAME, POPUP_PATH_ODE, POPUP_IFACE_ODE,
			"ProgNotiOn", "s", param, "ProgNotiUpdate", "ProgNotiOff");
}

static void test_encrypt_error_noti(void)
{
	char *param[3];
	char type[2];
	char num[16];

	param[0] = "encrypt";
	snprintf(type, sizeof(type), "%d", 0);
	param[1] = type;
	snprintf(num, sizeof(num), "%d", 456);
	param[2] = num;

	activate_normal_noti(BUS_NAME, POPUP_PATH_ODE, POPUP_IFACE_ODE,
			"ErrorNotiOn", "sii", param, "ErrorNotiOff");
}

static void test_tima_lkm_prevention_noti(void)
{
	activate_normal_noti(BUS_NAME, POPUP_PATH_TIMA, POPUP_IFACE_TIMA,
			"LKMPreventionNotiOn", NULL, NULL, "LKMPreventionNotiOff");
}

static void test_tima_pkm_detection_noti(void)
{
	activate_normal_noti(BUS_NAME, POPUP_PATH_TIMA, POPUP_IFACE_TIMA,
			"PKMDetectionNotiOn", NULL, NULL, "PKMDetectionNotiOff");
}

static void test_usb_device_noti(void)
{
	char *param[2];

	param[0] = "keyboard";
	param[1] = "Samsung keyboard";
	activate_normal_noti(BUS_NAME, POPUP_PATH_USBHOST, POPUP_IFACE_USBHOST,
			"UsbDeviceNotiOn", "ss", param, "UsbDeviceNotiOff");
}

static void test_ticker_noti(void)
{
	char *param[1];

	param[0] = "usb-client-default";
	activate_normal_noti(BUS_NAME, POPUP_PATH_TICKER, POPUP_IFACE_TICKER,
			"TickerNotiOn", "s", param, NULL);
}

static void test_battery_full_noti(void)
{
	activate_normal_noti(BUS_NAME, POPUP_PATH_BATTERY, POPUP_IFACE_BATTERY,
			"BatteryFullNotiOn", NULL, NULL, "BatteryFullNotiOff");
}

static void test_battery_charge_noti(void)
{
	activate_normal_noti(BUS_NAME, POPUP_PATH_BATTERY, POPUP_IFACE_BATTERY,
			"BatteryChargeNotiOn", NULL, NULL, NULL);
}

static const struct test_menu
notifications_menu[] = {
	{ "Go to previous menu"       , NULL                           },
	{ "Data warning noti"         , test_data_warning_noti         },
	{ "Data disabled noti"        , test_data_disabled_noti        },
	{ "LED torch noti"            , test_led_torch_noti            },
	{ "Encryption complete noti"  , test_encrypt_complete_noti     },
	{ "Decryption complete noti"  , test_decrypt_complete_noti     },
	{ "Encryption Progress noti"  , test_encrypt_progress_noti     },
	{ "Decryption Progress noti"  , test_decrypt_progress_noti     },
	{ "Encryption error noti"     , test_encrypt_error_noti        },
	{ "TIMA LKM prevention noti"  , test_tima_lkm_prevention_noti  },
	{ "TIMA PKM detection noti"   , test_tima_pkm_detection_noti   },
	{ "USB device noti"           , test_usb_device_noti           },
	{ "Battery full noti"         , test_battery_full_noti         },
	{ "Battery charge noti"       , test_battery_charge_noti       },
	{ "Ticker noti"               , test_ticker_noti               },
	/* Add additional menus here */
};

static void test_notifications(void)
{
	int sel;

	/* Acticate notification */
	sel = list_and_select_menu(notifications_menu,
			ARRAY_SIZE(notifications_menu), "Notification Menu");
	if (sel == FIN_MENU)
		return ;

	if (sel < 0 || sel >= ARRAY_SIZE(notifications_menu))
		return;

	notifications_menu[sel].func();
}
/**************/
/* Popup menu */
/**************/

/* Poweroff popup */
static void launch_poweroff_popup(void)
{
	int ret;

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_POWEROFF,
			POPUP_IFACE_POWEROFF, "PopupLaunch", NULL, NULL);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

/* Lowbat popup */
static void launch_lowbat_popup(char *opt)
{
	int ret;
	char *pa[2];
	if (!opt)
		return;
	pa[0] = "_SYSPOPUP_CONTENT_";
	pa[1] = opt;

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_LOWBAT,
			POPUP_IFACE_LOWBAT, "PopupLaunch", "ss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

static void launch_lowbat_warning_popup(void)
{
	launch_lowbat_popup("warning");
}

static void launch_lowbat_poweroff_popup(void)
{
	launch_lowbat_popup("poweroff");
}

static void launch_lowbat_chargeerr_popup(void)
{
	launch_lowbat_popup("chargeerr");
}

static void launch_lowbat_battdisconnect_popup(void)
{
	launch_lowbat_popup("battdisconnect");
}

static const struct test_menu
lowbat_popups_menu[] = {
	{ "Go to previous menu"          , NULL                               },
	{ "Warning popup"                , launch_lowbat_warning_popup        },
	{ "Poweroff popup"               , launch_lowbat_poweroff_popup       },
	{ "Charging error popup"         , launch_lowbat_chargeerr_popup      },
	{ "Battery disconnected popup"   , launch_lowbat_battdisconnect_popup },
	/* Add additional menus here */
};

static void test_lowbat_popup(void)
{
	int sel;
	sel = list_and_select_menu(lowbat_popups_menu,
			ARRAY_SIZE(lowbat_popups_menu), "Low Battery Popup Menu");
	if (sel == FIN_MENU)
		return ;

	if (sel < 0 || sel >= ARRAY_SIZE(lowbat_popups_menu))
		return;

	lowbat_popups_menu[sel].func();
}

/* Lowmem popup */
static void launch_lowmem_popup(char *type, char *opt)
{
	int ret;
	char *pa[2];
	if (!opt || !type)
		return;
	pa[0] = type;
	pa[1] = opt;

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_LOWMEM,
			POPUP_IFACE_LOWMEM, "PopupLaunch", "ss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

static void launch_lowmem_memsize_warning_popup(void)
{
	launch_lowmem_popup("_MEM_NOTI_", "warning");
}

static void launch_lowmem_memsize_critical_popup(void)
{
	launch_lowmem_popup("_MEM_NOTI_", "critical");
}

static void launch_lowmem_appname_popup(void)
{
	launch_lowmem_popup("_APP_NAME_", "test app");
}

static const struct test_menu
lowmem_popups_menu[] = {
	{ "Go to previous menu"          , NULL                                 },
	{ "Memory size warning popup"    , launch_lowmem_memsize_warning_popup  },
	{ "Memory size critical popup"   , launch_lowmem_memsize_critical_popup },
	{ "Process name popup"           , launch_lowmem_appname_popup          },
	/* Add additional menus here */
};

static void test_lowmem_popup(void)
{
	int sel;
	sel = list_and_select_menu(lowmem_popups_menu,
			ARRAY_SIZE(lowmem_popups_menu), "Low Memory Popup Menu");
	if (sel == FIN_MENU)
		return ;

	if (sel < 0 || sel >= ARRAY_SIZE(lowmem_popups_menu))
		return;

	lowmem_popups_menu[sel].func();
}

/* Usb popup */
static void launch_usb_popup(char *type, char *opt)
{
	int ret;
	char *pa[2];
	if (!opt || !type)
		return;
	pa[0] = type;
	pa[1] = opt;

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_USB,
			POPUP_IFACE_USB, "PopupLaunch", "ss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

static void launch_usb_error_popup(void)
{
	launch_usb_popup("_SYSPOPUP_CONTENT_", "error");
}

static void launch_usb_restrict_popup(void)
{
	launch_usb_popup("_SYSPOPUP_CONTENT_", "restrict");
}

static const struct test_menu
usb_popups_menu[] = {
	{ "Go to previous menu"   , NULL                        },
	{ "USB error popup"       , launch_usb_error_popup      },
	{ "USB restrict popup"    , launch_usb_restrict_popup   },
	/* Add additional menus here */
};

static void test_usb_popup(void)
{
	int sel;
	sel = list_and_select_menu(usb_popups_menu,
			ARRAY_SIZE(usb_popups_menu), "Low Memory Popup Menu");
	if (sel == FIN_MENU)
		return ;

	if (sel < 0 || sel >= ARRAY_SIZE(usb_popups_menu))
		return;

	usb_popups_menu[sel].func();
}

/* Recovery popup */
static void launch_recovery_popup(void)
{
	int ret;
	char *pa[2];
	pa[0] = "_SYSPOPUP_CONTENT_";
	pa[1] = "recovery";

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_SYSTEM,
			POPUP_IFACE_SYSTEM, "RecoveryPopupLaunch", "ss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

/* Watchdog popup */
static void launch_watchdog_popup(void)
{
	int ret;
	char *pa[4];
	pa[0] = "_SYSPOPUP_CONTENT_";
	pa[1] = "watchdog";
	pa[2] = "_APP_NAME_";
	pa[3] = "test_app";

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_SYSTEM,
			POPUP_IFACE_SYSTEM, "WatchdogPopupLaunch", "ssss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

/* Data blocked popup */
static void launch_data_blocked_popup(void)
{
	int ret;
	char *pa[4];

	pa[0] = "_SYSPOPUP_CONTENT_";
	pa[1] = "data_blocked";
	pa[2] = "_DATAUSAGE_LIMIT_";
	pa[3] = "20";

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_DATAUSAGE,
			POPUP_IFACE_DATAUSAGE, "BlockedPopupLaunch", "ssss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

/* Crash popup */
static void launch_crash_popup(void)
{
	int ret;
	char *pa[4];

	pa[0] = "_PROCESS_NAME_";
	pa[1] = "clock";
	pa[2] = "_EXEPATH_";
	pa[3] = "/usr/apps/org.tizen.clock/bin/clock";

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_CRASH,
			POPUP_IFACE_CRASH, "PopupLaunch", "ssss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

/* Usb otg mount failed popup */
static void launch_usbotg_mount_failed_popup(void)
{
	int ret;
	char *pa[2];
	pa[0] = "_SYSPOPUP_CONTENT_";
	pa[1] = "usbotg_mount_failed";

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_SYSTEM,
			POPUP_IFACE_SYSTEM, "UsbotgWarningPopupLaunch", "ss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

/* Usb otg removed unsafely popup */
static void launch_usbotg_removed_unsafe_popup(void)
{
	int ret;
	char *pa[2];
	pa[0] = "_SYSPOPUP_CONTENT_";
	pa[1] = "usbotg_removed_unsafe";

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_SYSTEM,
			POPUP_IFACE_SYSTEM, "UsbotgWarningPopupLaunch", "ss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

/* Brightness popup */
static void launch_brightness_popup_launch(void)
{
	int ret;
	char *pa[4];
	pa[0] = "_SYSPOPUP_CONTENT_";
	pa[1] = "brightness";
	pa[2] = "_TYPE_";
	pa[3] = "launch";

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_SYSTEM,
			POPUP_IFACE_SYSTEM, "BrightnessPopupLaunch", "ssss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

static void launch_brightness_popup_terminate(void)
{
	int ret;
	char *pa[4];
	pa[0] = "_SYSPOPUP_CONTENT_";
	pa[1] = "brightness";
	pa[2] = "_TYPE_";
	pa[3] = "terminate";

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_SYSTEM,
			POPUP_IFACE_SYSTEM, "BrightnessPopupLaunch", "ssss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

static const struct test_menu
popups_menu[] = {
	{ "Go to previous menu"          , NULL                              },
	{ "Poweroff popup"               , launch_poweroff_popup             },
	{ "Lowbat popup"                 , test_lowbat_popup                 },
	{ "Lowmem popup"                 , test_lowmem_popup                 },
	{ "Usb popup"                    , test_usb_popup                    },
	{ "Recovery popup"               , launch_recovery_popup             },
	{ "Watchdog popup"               , launch_watchdog_popup             },
	{ "Data blocked popup"           , launch_data_blocked_popup         },
	{ "Crash popup"                  , launch_crash_popup                },
	{ "Usbotg mount failed popup"    , launch_usbotg_mount_failed_popup  },
	{ "Usbotg removed unsafe popup"  , launch_usbotg_removed_unsafe_popup},
	{ "Brightness popup launch"      , launch_brightness_popup_launch    },
	{ "Brightness popup terminate"   , launch_brightness_popup_terminate },
	/* Add additional menus here */
};

static void test_popups(void)
{
	int sel;

	sel = list_and_select_menu(popups_menu,
			ARRAY_SIZE(popups_menu), "Popup Menu");
	if (sel == FIN_MENU)
		return ;

	if (sel < 0 || sel >= ARRAY_SIZE(popups_menu))
		return;

	popups_menu[sel].func();
}

/* Cradle app */
static void launch_cradle_app(void)
{
	int ret;

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_APP,
			POPUP_IFACE_APP, "CradleAppLaunch", NULL, NULL);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

/* pwlock app */
static void launch_pwlock_app(void)
{
	int ret;
	char *pa[2];
	pa[0] = "after_bootup";
	pa[1] = "1";

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_APP,
			POPUP_IFACE_APP, "PWLockAppLaunch", "ss", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

/* Terminate app */
static int get_integer_value(char *text)
{
	int val;

	if (!text)
		return -1;

	printf("%s: ", text);
	if (scanf("%d", &val) != 1) {
		_E("FAIL: scanf()");
		val = -1;
	}

	return val;
}

static void terminate_app_pid(void)
{
	int pid;
	int ret;
	char *pa[1];
	char buf[64];

	printf("\n");

	pid = get_integer_value("Input pid of an app to terminate");
	if (pid < 0) {
		printf("\nInput value is invalid !!\n");
		return;
	}

	snprintf(buf, sizeof(buf), "%d", pid);
	pa[0] = buf;

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_APP,
			POPUP_IFACE_APP, "AppTerminateByPid", "i", pa);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

/* Screen off tts */
static void launch_screen_off_tts(void)
{
	int ret;

	ret = request_to_launch_by_dbus(BUS_NAME, POPUP_PATH_SERVANT,
			POPUP_IFACE_SERVANT, "ScreenOffTts", NULL, NULL);
	if (ret < 0)
		_E("FAIL: request_to_launch_by_dbus(): %d", ret);
}

static const struct test_menu
apps_menu[] = {
	{ "Go to previous menu"          , NULL                    },
	{ "Launch cradle app"            , launch_cradle_app       },
	{ "Launch pwlock app"            , launch_pwlock_app       },
	{ "Terminate app by pid"         , terminate_app_pid       },
	{ "Screen Off TTS"               , launch_screen_off_tts   },
	/* Add additional menus here */
};

static void test_apps(void)
{
	int sel;

	sel = list_and_select_menu(apps_menu,
			ARRAY_SIZE(apps_menu), "App Menu");
	if (sel == FIN_MENU)
		return ;

	if (sel < 0 || sel >= ARRAY_SIZE(apps_menu))
		return;

	apps_menu[sel].func();
}


/* Main Menu */
static const struct test_menu
main_menu[] = {
	{ "Finish to test"  , NULL               },
	{ "Popups"          , test_popups        },
	{ "Notifications"   , test_notifications },
	{ "Apps"            , test_apps },
	/* Add additional menus here */
};

int main(int argc, char *argv[])
{
	int sel;

	printf("###############################\n");
	printf("## System Framework test app ##\n");
	printf("###############################\n");

	while(1) {
		sel = list_and_select_menu(main_menu,
			ARRAY_SIZE(main_menu), "Main Menu");

		if (sel == FIN_MENU) {
			printf("\nBye ~~ !!\n\n");
			break;
		}

		if (sel < 0 || sel >= ARRAY_SIZE(main_menu))
			return -EINVAL;

		main_menu[sel].func();
	}

	return 0;
}
