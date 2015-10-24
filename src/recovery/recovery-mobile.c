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

#define SYSTEMD_STOP_POWER_RESTART_RECOVERY 6

static void reboot_now(const struct popup_ops *ops)
{
	_I("Reboot now selected");

	unload_simple_popup(ops);

	if (vconf_set_int(VCONFKEY_SYSMAN_POWER_OFF_STATUS,
				SYSTEMD_STOP_POWER_RESTART_RECOVERY) != 0)
		_E("Fail to request restart to deviced");

	terminate_if_no_popup();
}

static const struct popup_ops recovery_ops = {
	.name			= "recovery",
	.show_popup		= load_simple_popup,
	.title			= "Prevention information",
	.content		= "To protect your device, it is recommended to reboot it",
	.left_text		= "Reboot now",
	.left			= reboot_now,
	.right_text		= "Reboot later",
	.flags			= SCROLLABLE,
};


/* Constructor to register mount_failed button */
static __attribute__ ((constructor)) void recovery_register_popup(void)
{
	register_popup(&recovery_ops);
}
