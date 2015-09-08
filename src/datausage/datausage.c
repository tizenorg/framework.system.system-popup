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
#include <appsvc.h>

#define DATAUSAGE_LIMIT "_DATAUSAGE_LIMIT_"
#define DATAUSAGE_APP   "org.tizen.clocksetting.datausage"

static const struct popup_ops datausage_warning_ops;
static const struct popup_ops datausage_disabled_ops;

static void datausage_launch(const struct popup_ops *ops)
{
	const struct popup_ops *remove;

	if (ops == &datausage_warning_ops)
		remove = &datausage_disabled_ops;
	else if (ops == &datausage_disabled_ops)
		remove = &datausage_warning_ops;
	else
		remove = NULL;

	if (remove)
		unload_simple_popup(remove);
}

static void launch_datausage_app(void)
{
	bundle *b;
	int ret;

	b = bundle_create();
	if (!b) {
		_E("Failed to make bundle");
		return;
	}

	appsvc_set_operation(b, APPSVC_OPERATION_VIEW);
	appsvc_set_pkgname(b, DATAUSAGE_APP);

	ret = appsvc_run_service(b, 0, NULL, NULL);
	if (ret < 0)
		_E("Failed to launch datausage app(%d)", ret);
	bundle_free(b);
}

static void datausage_setting(const struct popup_ops *ops)
{
	_I("Setting is selected");

	unload_simple_popup(ops);

	launch_datausage_app();

	terminate_if_no_popup();
}

static int datausage_get_content(const struct popup_ops *ops, char *content, unsigned int len)
{
	char *text, *limit;
	char buf[32];
	struct object_ops *obj;
	int ret;

	if (!ops || !content)
		return -EINVAL;

	ret = get_object_by_ops(ops, &obj);
	if (ret < 0) {
		_E("Failed to get object (%d)", ret);
		return -ENOENT;
	}

	limit = (char *)bundle_get_val(obj->b, DATAUSAGE_LIMIT);
	if (!limit) {
		_E("Failed to get data limit");
		return -ENOENT;
	}

	text = _("IDS_ST_POP_THE_MAXIMUM_DATA_USAGE_LIMIT_HPS_HAS_ALMOST_BEEN_REACHED_TAP_SETTINGS_TO_CHECK_YOUR_DATA_USAGE");

	snprintf(buf, sizeof(buf), "%sMB", limit);
	snprintf(content, len, text, buf);

	return 0;
}

static const struct popup_ops datausage_warning_ops = {
	.name			= "datausage_warning",
	.show_popup		= load_simple_popup,
	.title			= "IDS_COM_BODY_DATA_USAGE_WARNING",
	.get_content	= datausage_get_content,
	.left_text		= "IDS_COM_BODY_SETTINGS",
	.left_icon		= "circle-setting.png",
	.left			= datausage_setting,
	.right_text		= "IDS_COM_SK_OK",
	.right_icon		= "circle-ok.png",
	.launch			= datausage_launch,
	.flags			= SCROLLABLE,
};

static const struct popup_ops datausage_disabled_ops = {
	.name			= "datausage_disabled",
	.show_popup		= load_simple_popup,
	.title			= "IDS_ST_BODY_LIMIT_DATA_USAGE",
	.content		= "IDS_COM_BODY_SPECIFIED_DATA_USAGE_LIMIT_REACHED_ADDITIONAL_DATA_USE_MAY_INCUR_CHARGES_FROM_SERVICE_PROVIDER",
	.left_text      = "IDS_COM_BODY_SETTINGS",
	.left_icon		= "circle-setting.png",
	.left           = datausage_setting,
	.right_text		= "IDS_COM_SK_OK",
	.right_icon		= "circle-ok.png",
	.launch			= datausage_launch,
	.flags			= SCROLLABLE,
};

/* Constructor to register datausage button */
static __attribute__ ((constructor)) void datausage_register_popup(void)
{
	register_popup(&datausage_warning_ops);
	register_popup(&datausage_disabled_ops);
}
