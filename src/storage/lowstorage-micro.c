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

static const struct popup_ops lowstorage_warning_ops;
static const struct popup_ops lowstorage_critical_ops;
static const struct popup_ops lowstorage_full_ops;

static void remove_other_lowstorage_popups(const struct popup_ops *ops)
{
	if (ops != &lowstorage_warning_ops)
		unload_simple_popup(&lowstorage_warning_ops);

	if (ops != &lowstorage_critical_ops)
		unload_simple_popup(&lowstorage_critical_ops);

	if (ops != &lowstorage_full_ops)
		unload_simple_popup(&lowstorage_full_ops);
}

static const struct popup_ops lowstorage_warning_ops = {
	.name		= "lowstorage_warning",
	.show_popup	= load_simple_popup,
	.content	= "IDS_DAV_BODY_LOW_MEMORY_LEFT_ORANGE",
	.left_text	= "IDS_COM_SK_OK",
	.launch		= remove_other_lowstorage_popups,
	.flags		= SCROLLABLE,
};

static const struct popup_ops lowstorage_critical_ops = {
	.name		= "lowstorage_critical",
	.show_popup	= load_simple_popup,
	.content	= "IDS_DAV_BODY_LOW_MEMORY_LEFT_ORANGE",
	.left_text	= "IDS_COM_SK_OK",
	.launch		= remove_other_lowstorage_popups,
	.flags		= SCROLLABLE,
};

static const struct popup_ops lowstorage_full_ops = {
	.name		= "lowstorage_full",
	.show_popup	= load_simple_popup,
	.content	= "IDS_ST_POP_UNABLE_TO_RECORD_THERE_IS_NOT_ENOUGH_SPACE_IN_YOUR_GEAR_STORAGE",
	.left_text	= "IDS_COM_SK_OK",
	.launch		= remove_other_lowstorage_popups,
	.flags		= SCROLLABLE,
};

static __attribute__ ((constructor)) void lowstorage_register_popup(void)
{
	register_popup(&lowstorage_warning_ops);
	register_popup(&lowstorage_critical_ops);
	register_popup(&lowstorage_full_ops);
}
