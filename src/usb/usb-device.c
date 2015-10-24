/*
 *  system-popup
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

enum button_selected_e {
	USB_DEVICE_CONFIRM_OK,
	USB_DEVICE_CONFIRM_NOK,
};

#define USD_PATH				"/Org/Tizen/System/USD"
#define USD_INTERFACE				"org.tizen.system.usd"
#define USD_USB_DEVICE_CONFIRM_SIGNAL		"USBDeviceOpenResult"

static const struct popup_ops usb_device_confirm_ops;

static void send_result_dbus_signal(int result)
{
	int ret;
	char buf[8];
	char *param[1];

	snprintf(buf, sizeof(buf), "%d", result);
	param[0] = buf;
	ret = broadcast_dbus_signal(USD_PATH,
			USD_INTERFACE,
			USD_USB_DEVICE_CONFIRM_SIGNAL,
			"i", param);
	if (ret < 0)
		_E("FAIL: broadcast_dbus_signal(%d)", ret);
}

static void usb_device_confirm_ok_clicked(const struct popup_ops *ops)
{
	_I("OK is selected");

	unload_simple_popup(ops);

	send_result_dbus_signal(USB_DEVICE_CONFIRM_OK);

	terminate_if_no_popup();
}

static void usb_device_confirm_cancel_clicked(const struct popup_ops *ops)
{
	_I("CANCEL is selected");

	unload_simple_popup(ops);

	send_result_dbus_signal(USB_DEVICE_CONFIRM_NOK);

	terminate_if_no_popup();
}

static void usb_device_confirm_terminate(const struct popup_ops *ops)
{
	_I("terminate usb device confirm popup");

	unload_simple_popup(ops);

	send_result_dbus_signal(USB_DEVICE_CONFIRM_NOK);

	terminate_if_no_popup();
}

static const struct popup_ops usb_device_confirm_ops = {
	.name		= "usb_device_confirm",
	.show_popup	= load_simple_popup,
	.content	= "Do you use this usb device?", /* TODO */
	.left_text	= "IDS_COM_SK_CANCEL",
	.left		= usb_device_confirm_cancel_clicked,
	.right_text	= "IDS_COM_SK_OK",
	.right		= usb_device_confirm_ok_clicked,
	.terminate	= usb_device_confirm_terminate,
	.flags		= SCROLLABLE,
};

/* Constructor to register usb_device popup */
static __attribute__ ((constructor)) void usb_device_register_popup(void)
{
	register_popup(&usb_device_confirm_ops);

}
