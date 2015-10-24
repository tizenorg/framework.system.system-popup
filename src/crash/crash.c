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
#include <pkgmgr-info.h>

#define BUF_MAX 256

static int appinfo_get_appname_func(pkgmgrinfo_appinfo_h handle, void *data)
{
	char *str = NULL;
	char *label;
	int ret;

	if (!data)
		return PMINFO_R_ERROR;

	ret = pkgmgrinfo_appinfo_get_label(handle, &str);
	if (ret != PMINFO_R_OK)
		return ret;

	if (!str)
		return PMINFO_R_ERROR;

	label = strdup(str);
	if (!label)
		return PMINFO_R_ERROR;

	(*(char**)data) = label;

	return PMINFO_R_OK;
}

static int get_app_name(char *exepath, char *tname, unsigned int len)
{
	pkgmgrinfo_appinfo_filter_h handle = NULL;
	int count, ret;
	char *name = NULL;

	ret = pkgmgrinfo_appinfo_filter_create(&handle);
	if (ret != PMINFO_R_OK)
		goto out;

	ret = pkgmgrinfo_appinfo_filter_add_string(handle,
			PMINFO_APPINFO_PROP_APP_EXEC, exepath);
	if (ret != PMINFO_R_OK)
		goto out;

	ret = pkgmgrinfo_appinfo_filter_count(handle, &count);
	if (ret != PMINFO_R_OK)
		goto out;

	if (count < 1)
		goto out;

	ret = pkgmgrinfo_appinfo_filter_foreach_appinfo(handle,
			appinfo_get_appname_func, &name);
	if (ret != PMINFO_R_OK) {
		name = NULL;
		goto out;
	}

out:
	if (handle)
		pkgmgrinfo_appinfo_filter_destroy(handle);
	if (!name)
		return -ENOENT;

	snprintf(tname, len, "%s", name);
	free(name);
	return 0;
}

static int crash_get_content(const struct popup_ops *ops, char *content, unsigned int len)
{
	int ret;
	unsigned int l;
	struct object_ops *obj;
	char *text, *name, *path;
	char tname[BUF_MAX];

	if (!ops || !content)
		return -EINVAL;

	ret = get_object_by_ops(ops, &obj);
	if (ret < 0) {
		_E("Failed to get object (%d)", ret);
		return -ENOENT;
	}

	name = (char *)bundle_get_val(obj->b, "_PROCESS_NAME_");
	if (!name) {
		_E("Failed to get process name");
		return -ENOENT;
	}

	path = (char *)bundle_get_val(obj->b, "_EXEPATH_");
	if (!path) {
		_E("Failed to get execution path");
		return -ENOENT;
	}

	l = sizeof(tname);
	ret = get_app_name(path, tname, l);
	if (ret < 0)
		snprintf(tname, l, "%s", name);

	_I("name(%s), path(%s), tname(%s)", name, path, tname);

	text = _("IDS_ST_BODY_PS_HAS_CLOSED_UNEXPECTEDLY");
	snprintf(content, len, text, tname);

	return 0;
}

static const struct popup_ops crash_ops = {
	.name			= "crash",
	.show_popup		= load_simple_popup,
	.title			= "IDS_COM_HEADER_ATTENTION",
	.get_content	= crash_get_content,
	.left_text		= "IDS_COM_SK_OK",
	.flags			= SCROLLABLE,
};

static __attribute__ ((constructor)) void crash_register_popup(void)
{
	register_popup(&crash_ops);
}
