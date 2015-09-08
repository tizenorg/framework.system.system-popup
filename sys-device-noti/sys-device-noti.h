/*
 *  system-server
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Giyeol Ok <giyeol.ok@samsung.com>
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
#ifndef __SYS_DEVICE_NOTI_H__
#define __SYS_DEVICE_NOTI_H__

#ifndef _
#define _(str) gettext(str)
#endif

#ifndef gettext_noop
#define gettext_noop(str) (str)
#endif

#ifndef N_
#define N_(str) gettext_noop(str)
#endif

typedef enum {
	DEVICE_NOTI_BATT_CHARGE = 0,
	DEVICE_NOTI_BATT_LOW,
	DEVICE_NOTI_BATT_FULL,
	DEVICE_NOTI_MAX
} cb_noti_type;

typedef enum {
	DEVICE_NOTI_OFF	= 0,
	DEVICE_NOTI_ON	= 1
} cb_noti_onoff_type;

#endif /* __SYS_DEVICE__NOTI_H__ */
