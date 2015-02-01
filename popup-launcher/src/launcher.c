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

#include "launcher.h"

#ifdef TIZEN_MICRO
#define activate_notification_no_param(x,y,z)     0
#define activate_notification_single_param(x,y,z) 0
#define activate_notification_double_param(x,y,z) 0
#define activate_notification_triple_param(x,y,z) 0
#define update_notification_double_param(x,y,z)   0
#define progress_update_notification(x,y)         0
#define deactivate_notification(x,y)              0
#endif

#define TERMINATE_TIMEOUT  5

static E_DBus_Connection *edbus_conn;
static DBusPendingCall *edbus_request_name;
static E_DBus_Object *obj;
static E_DBus_Interface *iface;
static Ecore_Timer *term_timer = NULL;

static Eina_Bool exit_idler_cb(void *data)
{
	e_dbus_connection_close(edbus_conn);
	e_dbus_shutdown();

	ecore_main_loop_quit();
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool terminate_launcher(void *data)
{
	if (term_timer)
		ecore_timer_del(term_timer);

	if (ecore_idler_add(exit_idler_cb, NULL))
		return ECORE_CALLBACK_CANCEL;

	exit_idler_cb(NULL);
	return ECORE_CALLBACK_CANCEL;
}

static void set_timer_to_terminate(void)
{
	if (term_timer)
		ecore_timer_reset(term_timer);
	else {
		term_timer = ecore_timer_add(TERMINATE_TIMEOUT, terminate_launcher, NULL);
		if (!term_timer)
			terminate_launcher(NULL);
	}
}

/* Legacy datausage popup and notification */
static DBusMessage *edbus_datausage(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_double_param(obj, msg, DATAUSAGE_SYSPOPUP);
}

static DBusMessage *edbus_noti_off(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return deactivate_notification(obj, msg);
}

static DBusMessage *edbus_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_no_param(obj, msg, DATAUSAGE_WARNING);
}

static const struct edbus_method
edbus_methods[] = {
	{ "DataUsage"          , "ssss",  "i", edbus_datausage },
	{ "NotiOn"             ,    "s",  "i", edbus_noti_on   },
	{ "NotiOff"            ,    "i",  "i", edbus_noti_off  },
	/* Add methods here */
};

/* Poweroff popup */
static DBusMessage *edbus_poweroff_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_no_param(obj, msg, POWEROFF_SYSPOPUP);
}

static const struct edbus_method
edbus_poweroff_methods[] = {
	{ "PopupLaunch" ,   NULL,  "i", edbus_poweroff_popup      },
	/* Add methods here */
};

/* Lowbat popup */
static DBusMessage *edbus_lowbat_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, LOWBAT_SYSPOPUP);
}

static const struct edbus_method
edbus_lowbat_methods[] = {
	{ "PopupLaunch" ,   "ss",  "i", edbus_lowbat_popup      },
	/* Add methods here */
};

/* Lowmem popup */
static DBusMessage *edbus_lowmem_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, LOWMEM_SYSPOPUP);
}

static const struct edbus_method
edbus_lowmem_methods[] = {
	{ "PopupLaunch" ,   "ss",  "i", edbus_lowmem_popup      },
	/* Add methods here */
};

/* Mmc popup */
static DBusMessage *edbus_mmc_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, MMC_SYSPOPUP);
}

static const struct edbus_method
edbus_mmc_methods[] = {
	{ "PopupLaunch" ,   "ss",  "i", edbus_mmc_popup      },
	/* Add methods here */
};

/* Usb popup */
static DBusMessage *edbus_usb_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, USB_SYSPOPUP);
}

static const struct edbus_method
edbus_usb_methods[] = {
	{ "PopupLaunch"        ,   "ss",  "i", edbus_usb_popup         },
	/* Add methods here */
};

/* Usbotg popup */
static DBusMessage *edbus_usbstorage_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_double_param(obj, msg, USBOTG_SYSPOPUP);
}

static DBusMessage *edbus_usbcamera_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, USBOTG_SYSPOPUP);
}

static DBusMessage *edbus_usbstorage_unmount_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_double_param(obj, msg, USBOTG_SYSPOPUP);
}

static const struct edbus_method
edbus_usbotg_methods[] = {
	{ "StoragePopupLaunch"        , "ssss",  "i", edbus_usbstorage_popup         },
	{ "CameraPopupLaunch"         ,   "ss",  "i", edbus_usbcamera_popup          },
	{ "StorageUnmountPopupLaunch" , "ssss",  "i", edbus_usbstorage_unmount_popup },
	/* Add methods here */
};

/* DataUsage popups/notifications */
static DBusMessage *edbus_datausage_blocked_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_double_param(obj, msg, DATAUSAGE_SYSPOPUP);
}

static DBusMessage *edbus_datausage_warning_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_no_param(obj, msg, DATAUSAGE_WARNING);
}

static DBusMessage *edbus_datausage_disabled_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_no_param(obj, msg, DATAUSAGE_DISABLED);
}

static const struct edbus_method
edbus_datausage_methods[] = {
	{ "BlockedPopupLaunch" , "ssss",  "i", edbus_datausage_blocked_popup      },
	{ "WarningNotiOn"      ,   NULL,  "i", edbus_datausage_warning_noti_on    },
	{ "WarningNotiOff"     ,    "i",  "i", edbus_noti_off                     },
	{ "DisabledNotiOn"     ,   NULL,  "i", edbus_datausage_disabled_noti_on   },
	{ "DisabledNotiOff"    ,    "i",  "i", edbus_noti_off                     },
	/* Add methods here */
};

/* LED torch notifications */
static DBusMessage *edbus_led_torch_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_no_param(obj, msg, LED_TORCH);
}

static const struct edbus_method
edbus_led_methods[] = {
	{ "TorchNotiOn"        ,   NULL,  "i", edbus_led_torch_noti_on            },
	{ "TorchNotiOff"       ,    "i",  "i", edbus_noti_off                     },
	/* Add methods here */
};

/* Ode complete/progress notifications */
static DBusMessage *edbus_ode_complete_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_single_param(obj, msg, ODE_COMPLETE);
}

static DBusMessage *edbus_ode_progress_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_single_param(obj, msg, ODE_PROGRESS);
}

static DBusMessage *edbus_ode_progress_noti_update(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return progress_update_notification(obj, msg);
}

static DBusMessage *edbus_ode_error_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_triple_param(obj, msg, ODE_ERROR);
}

static const struct edbus_method
edbus_ode_methods[] = {
	{ "CompNotiOn"      ,    "s",  "i", edbus_ode_complete_noti_on     },
	{ "CompNotiOff"     ,    "i",  "i", edbus_noti_off                 },
	{ "ProgNotiOn"      ,    "s",  "i", edbus_ode_progress_noti_on     },
	{ "ProgNotiUpdate"  ,   "ii",  "i", edbus_ode_progress_noti_update },
	{ "ProgNotiOff"     ,    "i",  "i", edbus_noti_off                 },
	{ "ErrorNotiOn"     ,  "sii",  "i", edbus_ode_error_noti_on        },
	{ "ErrorNotiOff"    ,    "i",  "i", edbus_noti_off                 },
	/* Add methods here */
};

/* System popup */
static DBusMessage *edbus_recovery_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, SYSTEM_SYSPOPUP);
}

static DBusMessage *edbus_watchdog_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_double_param(obj, msg, SYSTEM_SYSPOPUP);
}

static DBusMessage *edbus_usbotg_warning_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, SYSTEM_SYSPOPUP);
}

static DBusMessage *edbus_brightness_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_double_param(obj, msg, SYSTEM_SYSPOPUP);
}

static DBusMessage *edbus_ode_error_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_triple_param(obj, msg, SYSTEM_SYSPOPUP);
}

static DBusMessage *edbus_accessibility_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, SYSTEM_SYSPOPUP);
}

static DBusMessage *edbus_cooldown_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_single_param(obj, msg, SYSTEM_SYSPOPUP);
}

static const struct edbus_method
edbus_system_methods[] = {
	{ "RecoveryPopupLaunch"           ,     "ss",  "i", edbus_recovery_popup            },
	{ "WatchdogPopupLaunch"           ,   "ssss",  "i", edbus_watchdog_popup            },
	{ "UsbotgWarningPopupLaunch"      ,     "ss",  "i", edbus_usbotg_warning_popup      },
	{ "BrightnessPopupLaunch"         ,   "ssss",  "i", edbus_brightness_popup          },
	{ "OdeErrorPopupLaunch"           , "ssssss",  "i", edbus_ode_error_popup           },
	{ "AccessibilityPopupLaunch"      ,     "ss",  "i", edbus_accessibility_popup       },
	{ "CooldownPopupLaunch"           ,     "ss",  "i", edbus_cooldown_popup            },
	/* Add methods here */
};

/* Tima notification */
static DBusMessage *edbus_lkm_prevention_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_no_param(obj, msg, TIMA_LKM_PREVENTION);
}

static DBusMessage *edbus_pkm_detection_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_no_param(obj, msg, TIMA_PKM_DETECTION);
}

static const struct edbus_method
edbus_tima_methods[] = {
	{ "LKMPreventionNotiOn"  ,     NULL,  "i", edbus_lkm_prevention_noti_on    },
	{ "LKMPreventionNotiOff" ,      "i",  "i", edbus_noti_off                  },
	{ "PKMDetectionNotiOn"   ,     NULL,  "i", edbus_pkm_detection_noti_on     },
	{ "PKMDetectionNotiOff"  ,      "i",  "i", edbus_noti_off                  },
	/* Add methods here */
};

/* Crash popup */
static DBusMessage *edbus_crash_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_popup_double_param(obj, msg, CRASH_SYSPOPUP);
}

static const struct edbus_method
edbus_crash_methods[] = {
	{ "PopupLaunch"  ,     "ssss",  "i", edbus_crash_popup    },
	/* Add methods here */
};

/* Ticker noti */
static DBusMessage *edbus_ticker_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_single_param(obj, msg, TICKER);
}

static const struct edbus_method
edbus_ticker_methods[] = {
	{ "TickerNotiOn"  ,     "s",  "i", edbus_ticker_noti_on    },
	/* Add methods here */
};

/* App noti */
static DBusMessage *edbus_cradle_app_launch(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_app_no_param(obj, msg, CRADLE_APP_NAME);
}

static DBusMessage *edbus_pwlock_app_launch(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return launch_app_single_param(obj, msg, PWLOCK_APP_NAME);
}

static DBusMessage *edbus_app_terminate_by_pid(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return terminate_app_by_pid(obj, msg);
}

static const struct edbus_method
edbus_app_methods[] = {
	{ "CradleAppLaunch"  ,     NULL,  "i", edbus_cradle_app_launch    },
	{ "PWLockAppLaunch"  ,     "ss",  "i", edbus_pwlock_app_launch    },
	{ "AppTerminateByPid",      "i",  "i", edbus_app_terminate_by_pid },
	/* Add methods here */
};

/* USB host notifications */
static DBusMessage *edbus_usb_storage_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_single_param(obj, msg, USB_STORAGE);
}

static DBusMessage *edbus_usb_storage_ro_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_single_param(obj, msg, USB_STORAGE_RO);
}

static DBusMessage *edbus_usb_device_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_double_param(obj, msg, USB_DEVICE);
}

static DBusMessage *edbus_usb_device_noti_update(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return update_notification_double_param(obj, msg, USB_DEVICE);
}

static const struct edbus_method
edbus_usbhost_methods[] = {
	{ "UsbStorageNotiOn"   ,    "s",  "i", edbus_usb_storage_noti_on          },
	{ "UsbStorageRoNotiOn" ,    "s",  "i", edbus_usb_storage_ro_noti_on       },
	{ "UsbStorageNotiOff"  ,    "i",  "i", edbus_noti_off                     },
	{ "UsbDeviceNotiOn"    ,   "ss",  "i", edbus_usb_device_noti_on           },
	{ "UsbDeviceNotiUpdate", "isss",  "i", edbus_usb_device_noti_update       },
	{ "UsbDeviceNotiOff"   ,    "i",  "i", edbus_noti_off                     },

	/* Add methods here */
};

/* Battery notifications */
static DBusMessage *edbus_battery_full_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	set_timer_to_terminate();
	return activate_notification_no_param(obj, msg, BATTERY_FULL);
}

static DBusMessage *edbus_battery_charge_noti_on(E_DBus_Object *obj, DBusMessage *msg)
{
	char param[2];
	char *args[3];

	set_timer_to_terminate();

	args[0] = SERVANT_APP_NAME;
	snprintf(param, sizeof(param), "%d", CHARGER_CONNECTION);
	args[1] = param;
	args[2] = NULL;
	return launch_system_servant_app(obj, msg, args);
}

static const struct edbus_method
edbus_battery_methods[] = {
	{ "BatteryFullNotiOn"   ,   NULL,  "i", edbus_battery_full_noti_on         },
	{ "BatteryFullNotiOff"  ,    "i",  "i", edbus_noti_off                     },
	{ "BatteryChargeNotiOn" ,   NULL,  "i", edbus_battery_charge_noti_on       },
	/* Add methods here */
};

/* Servant application */
static DBusMessage *load_system_servant(E_DBus_Object *obj, DBusMessage *msg, int type)
{
	char param[2];
	char *args[3];

	set_timer_to_terminate();

	args[0] = SERVANT_APP_NAME;
	snprintf(param, sizeof(param), "%d", type);
	args[1] = param;
	args[2] = NULL;
	return launch_system_servant_app(obj, msg, args);
}

static DBusMessage *edbus_tts_screen_off(E_DBus_Object *obj, DBusMessage *msg)
{
	return load_system_servant(obj, msg, TTS_SCREENOFF);
}

static DBusMessage *edbus_tts_enabled(E_DBus_Object *obj, DBusMessage *msg)
{
	return load_system_servant(obj, msg, TTS_ENABLED);
}

static DBusMessage *edbus_tts_disabled(E_DBus_Object *obj, DBusMessage *msg)
{
	return load_system_servant(obj, msg, TTS_DISABLED);
}

static const struct edbus_method
edbus_servant_methods[] = {
	{ "ScreenOffTts"       ,   NULL,  "i", edbus_tts_screen_off       },
	{ "TtsEnabled"         ,   NULL,  "i", edbus_tts_enabled          },
	{ "TtsDisabled"        ,   NULL,  "i", edbus_tts_disabled         },
	/* Add methods here */
};

static struct edbus_object
edbus_objects[]= {
	{ POPUP_PATH_POWEROFF    , POPUP_IFACE_POWEROFF    , NULL, NULL,
		edbus_poweroff_methods   , ARRAY_SIZE(edbus_poweroff_methods)  },
	{ POPUP_PATH_LOWBAT      , POPUP_IFACE_LOWBAT      , NULL, NULL,
		edbus_lowbat_methods     , ARRAY_SIZE(edbus_lowbat_methods)    },
	{ POPUP_PATH_LOWMEM      , POPUP_IFACE_LOWMEM      , NULL, NULL,
		edbus_lowmem_methods     , ARRAY_SIZE(edbus_lowmem_methods)    },
	{ POPUP_PATH_MMC         , POPUP_IFACE_MMC         , NULL, NULL,
		edbus_mmc_methods        , ARRAY_SIZE(edbus_mmc_methods)       },
	{ POPUP_PATH_USB         , POPUP_IFACE_USB         , NULL, NULL,
		edbus_usb_methods        , ARRAY_SIZE(edbus_usb_methods)       },
	{ POPUP_PATH_USBOTG      , POPUP_IFACE_USBOTG      , NULL, NULL,
		edbus_usbotg_methods     , ARRAY_SIZE(edbus_usbotg_methods)    },
	{ POPUP_PATH_DATAUSAGE   , POPUP_IFACE_DATAUSAGE   , NULL, NULL,
		edbus_datausage_methods  , ARRAY_SIZE(edbus_datausage_methods) },
	{ POPUP_PATH_LED         , POPUP_IFACE_LED         , NULL, NULL,
		edbus_led_methods        , ARRAY_SIZE(edbus_led_methods)       },
	{ POPUP_PATH_ODE         , POPUP_IFACE_ODE         , NULL, NULL,
		edbus_ode_methods        , ARRAY_SIZE(edbus_ode_methods)       },
	{ POPUP_PATH_SYSTEM      , POPUP_IFACE_SYSTEM      , NULL, NULL,
		edbus_system_methods     , ARRAY_SIZE(edbus_system_methods)    },
	{ POPUP_PATH_TIMA        , POPUP_IFACE_TIMA        , NULL, NULL,
		edbus_tima_methods       , ARRAY_SIZE(edbus_tima_methods)      },
	{ POPUP_PATH_CRASH       , POPUP_IFACE_CRASH       , NULL, NULL,
		edbus_crash_methods      , ARRAY_SIZE(edbus_crash_methods)     },
	{ POPUP_PATH_TICKER      , POPUP_IFACE_TICKER      , NULL, NULL,
		edbus_ticker_methods     , ARRAY_SIZE(edbus_ticker_methods)    },
	{ POPUP_PATH_APP         , POPUP_IFACE_APP         , NULL, NULL,
		edbus_app_methods        , ARRAY_SIZE(edbus_app_methods)       },
	{ POPUP_PATH_USBHOST     , POPUP_IFACE_USBHOST     , NULL, NULL,
		edbus_usbhost_methods    , ARRAY_SIZE(edbus_usbhost_methods)   },
	{ POPUP_PATH_BATTERY     , POPUP_IFACE_BATTERY     , NULL, NULL,
		edbus_battery_methods    , ARRAY_SIZE(edbus_battery_methods)   },
	{ POPUP_PATH_SERVANT     , POPUP_IFACE_SERVANT     , NULL, NULL,
		edbus_servant_methods    , ARRAY_SIZE(edbus_servant_methods)   },
	/* Add new object & interface here*/
};

static int init_methods(void)
{
	int ret;
	int i, j;

	/***************************************************************************/
	/* TODO                                                                    */
	/* These codes are old and will be removed after other notis are developed */
	/***************************************************************************/
	for (i = 0; i < ARRAY_SIZE(edbus_methods); i++) {
		ret = e_dbus_interface_method_add(iface,
				edbus_methods[i].member,
				edbus_methods[i].signature,
				edbus_methods[i].reply_signature,
				edbus_methods[i].func);
		if (!ret) {
			_E("fail to add method %s!", edbus_methods[i].member);
			return -ECONNREFUSED;
		}
	}
	/***************************************************************************/

	for (i = 0; i < ARRAY_SIZE(edbus_objects); i++) {
		for (j = 0; j < edbus_objects[i].methods_len; j++) {
			ret = e_dbus_interface_method_add(edbus_objects[i].iface,
					edbus_objects[i].methods[j].member,
					edbus_objects[i].methods[j].signature,
					edbus_objects[i].methods[j].reply_signature,
					edbus_objects[i].methods[j].func);
			if (!ret) {
				_E("fail to add method %s!", edbus_objects[i].methods[j].member);
				return -ECONNREFUSED;
			}
		}
	}
	return 0;
}

static int register_dbus(void)
{
	DBusError error;
	int retry, ret, i;

	dbus_error_init(&error);

	retry = 0;
	do {
		if (e_dbus_init())
			break;
		if (++retry == RETRY_MAX) {
			_E("fail to init edbus");
			return -ECONNREFUSED;
		}
	} while (retry <= RETRY_MAX);

	retry = 0;
	do {
		edbus_conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
		if (edbus_conn)
			break;
		if (++retry == RETRY_MAX) {
			_E("fail to get edbus");
			ret = -ECONNREFUSED;
			goto out1;
		}
	} while (retry <= RETRY_MAX);

	retry = 0;
	do {
		edbus_request_name = e_dbus_request_name(edbus_conn, BUS_NAME, 0, NULL, NULL);
		if (edbus_request_name)
			break;
		if (++retry == RETRY_MAX) {
			_E("fail to request edbus name");
			ret = -ECONNREFUSED;
			goto out2;
		}
	} while (retry <= RETRY_MAX);

	/***************************************************************************/
	/* TODO                                                                    */
	/* These codes are old and will be removed after other notis are developed */
	/***************************************************************************/
	obj = e_dbus_object_add(edbus_conn, POPUP_DBUS_PATH, NULL);
	if (!obj) {
		_E("fail to add edbus obj");
		ret = -ECONNREFUSED;
		goto out2;
	}

	iface = e_dbus_interface_new(POPUP_DBUS_IFACE);
	if (!iface) {
		_E("fail to add edbus interface");
		ret = -ECONNREFUSED;
		goto out2;
	}

	e_dbus_object_interface_attach(obj, iface);
	/***************************************************************************/

	for (i = 0; i < ARRAY_SIZE(edbus_objects); i++) {
		edbus_objects[i].obj = e_dbus_object_add(edbus_conn, edbus_objects[i].path, NULL);
		if (!(edbus_objects[i].obj)) {
			_E("fail to add edbus obj");
			ret = -ECONNREFUSED;
			goto out2;
		}

		edbus_objects[i].iface = e_dbus_interface_new(edbus_objects[i].interface);
		if (!(edbus_objects[i].iface)) {
			_E("fail to add edbus interface");
			ret = -ECONNREFUSED;
			goto out2;
		}

		e_dbus_object_interface_attach(edbus_objects[i].obj, edbus_objects[i].iface);
	}

	return 0;

out2:
	e_dbus_connection_close(edbus_conn);
out1:
	e_dbus_shutdown();

	return ret;
}

int main (int argc, char *argv[])
{
	int ret;

	ecore_init();

	ret = register_dbus();
	if (ret < 0)
		return ret;

	ret= init_methods();
	if (ret < 0)
		return ret;

	ecore_main_loop_begin();
	ecore_shutdown();
	return 0;
}
