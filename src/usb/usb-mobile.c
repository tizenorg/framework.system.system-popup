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

static const struct popup_ops usb_error_ops;
static const struct popup_ops usb_restrict_ops;

static int state_handler = -1;

static void usb_state_changed(keynode_t *key, void *data)
{
	int state;
	const struct popup_ops *ops = data;

	if (vconf_get_int(VCONFKEY_SYSMAN_USB_STATUS, &state) != 0)
		return;

	switch (state) {
	case VCONFKEY_SYSMAN_USB_DISCONNECTED:
		_I("USB cabel is disconnected");
		unload_simple_popup(ops);
		if (state_handler == 0) {
			vconf_ignore_key_changed(
					VCONFKEY_SYSMAN_USB_STATUS,
					usb_state_changed);
			state_handler = -1;
		}
		terminate_if_no_popup();
		break;
	case VCONFKEY_SYSMAN_USB_CONNECTED:
	case VCONFKEY_SYSMAN_USB_AVAILABLE:
	default:
		break;
	}
}

static void usb_launch(const struct popup_ops *ops)
{
	const struct popup_ops *remove;

	if (ops == &usb_error_ops)
		remove = &usb_restrict_ops;
	else if (ops == &usb_restrict_ops)
		remove = &usb_error_ops;
	else
		remove = NULL;

	if (remove)
		unload_simple_popup(remove);

	if (state_handler != 0) {
		state_handler = vconf_notify_key_changed(
				VCONFKEY_SYSMAN_USB_STATUS,
				usb_state_changed, (void *)ops);
		if (state_handler != 0)
			_E("Failed to register usb state change event()");
	}
}

static void usb_terminate(const struct popup_ops *ops)
{
	unload_simple_popup(ops);
	if (state_handler == 0) {
		vconf_ignore_key_changed(
				VCONFKEY_SYSMAN_USB_STATUS,
				usb_state_changed);
		state_handler = -1;
	}
	terminate_if_no_popup();
}

static const struct popup_ops usb_error_ops = {
	.name			= "usb_error",
	.show_popup		= load_simple_popup,
	.content		= "IDS_USB_POP_USB_CONNECTION_FAILED",
	.left_text		= "IDS_COM_SK_OK",
	.launch			= usb_launch,
	.terminate		= usb_terminate,
	.flags			= SCROLLABLE,
};

static const struct popup_ops usb_restrict_ops = {
	.name			= "usb_restrict",
	.show_popup		= load_simple_popup,
	.content		= "IDS_ST_POP_SECURITY_POLICY_PREVENTS_USE_OF_DESKTOP_SYNC",
	.left_text		= "IDS_COM_SK_OK",
	.launch			= usb_launch,
	.terminate		= usb_terminate,
	.flags			= SCROLLABLE,
};


/* Constructor to register mount_failed button */
static __attribute__ ((constructor)) void usb_register_popup(void)
{
	register_popup(&usb_error_ops);
	register_popup(&usb_restrict_ops);
}
