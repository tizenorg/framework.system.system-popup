/*
 * usbotg-syspopup
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
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

#ifndef __USBOTG_SYSPOPUP_H___
#define __USBOTG_SYSPOPUP_H__

#include <Elementary.h>

typedef enum {
	POPUP_NONE = 0x0,
	STORAGE_ADDED_POPUP = 0x1,
	STORAGE_UNMOUNT_POPUP = 0x4,
	CAMERA_ADDED_POPUP = 0x8
} USBOTG_SYSPOPUP_TYPE;

typedef enum {
	STORAGE_ADDED_PATH = 0x1,
	STORAGE_REMOVED_PATH = 0x2,
	STORAGE_UNMOUNT_PATH = 0x4
} USBOTG_STORAGE_PATH;

#endif				/* __USBOTG_SYSPOPUP_H__ */
