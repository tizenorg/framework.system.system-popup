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

#define MOUNT_FAILED_POPUP       "usbotg_mount_failed"

static int show_mount_failed_popup(void *data, bundle *b);

struct popup_ops usbotg_mount_failed_ops = {
	.name = MOUNT_FAILED_POPUP,
	.popup = NULL,
	.show_popup = show_mount_failed_popup
};

static void ok_clicked(void *data, Evas_Object *obj, void *event_info)
{
	release_evas_object(&(usbotg_mount_failed_ops.popup));
	terminate_if_no_popup();
}

static int show_mount_failed_popup(void *data, bundle *b)
{
	struct appdata *ad = data;

	if (!ad || !(ad->win_main))
		return -EINVAL;

	if (usbotg_mount_failed_ops.popup) {
		_E("Popup already exists");
		return 0;
	}

	evas_object_show(ad->win_main);

	usbotg_mount_failed_ops.popup = load_normal_popup(ad,
			_("IDS_COM_HEADER_ATTENTION"),
			_("IDS_COM_BODY_USB_STORAGE_BLANK_OR_HAS_UNSUPPORTED_FILE_SYSTEM"),
			_("IDS_COM_SK_OK"),
			ok_clicked,
			NULL, NULL);
	if (!(usbotg_mount_failed_ops.popup)) {
		_E("FAIL: load_normal_popup()");
		terminate_if_no_popup();
		return -ENOMEM;
	}

	if (set_display_feedback(PATTERN_MMC) < 0)
		_E("Failed to set display and feedback");

	return 0;
}

static __attribute__ ((constructor)) void register_usbotg_mount_failed_popup(void)
{
	register_popup(&usbotg_mount_failed_ops);
}
