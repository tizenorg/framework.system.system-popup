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

#include <syspopup_caller.h>
#include "launcher.h"

#define POPUP_CONTENT		"_SYSPOPUP_CONTENT_"
#define POPUP_NAME_POWEROFF	"poweroff"

DBusMessage *launch_popup(E_DBus_Object *obj,
				DBusMessage *msg, char *name)
{
	DBusMessageIter iter;
	DBusMessageIter aiter, piter;
	DBusMessage *reply;
	int ret;
	char *key, *value;
	bundle *b = NULL;

	if (!name) {
		ret = -EINVAL;
		goto out;
	}

	_I("launch popup (%s)", name);

	b = bundle_create();
	if (!b) {
		ret = -ENOMEM;
		goto out;
	}

	dbus_message_iter_init(msg, &iter);
	dbus_message_iter_recurse(&iter, &aiter);

	while (dbus_message_iter_get_arg_type(&aiter) != DBUS_TYPE_INVALID) {
		dbus_message_iter_recurse(&aiter, &piter);
		dbus_message_iter_get_basic(&piter, &key);
		dbus_message_iter_next(&piter);
		dbus_message_iter_get_basic(&piter, &value);
		dbus_message_iter_next(&aiter);

		_I("key(%s), value(%s)", key, value);

		ret = bundle_add(b, key, value);
		if (ret < 0) {
			_E("Failed to add bundle (%s,%s) (ret:%d)", key, value, ret);
			goto out;
		}
	}

	ret = syspopup_launch(name, b);
	if (ret < 0)
		_E("Failed to launch popup(%d)", ret);

out:
	if (b)
		bundle_free(b);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *launch_poweroff_popup(E_DBus_Object *obj,
				DBusMessage *msg, char *name)
{
	DBusMessage *reply;
	DBusMessageIter iter;
	int ret;
	bundle *b = NULL;

	if (!name) {
		ret = -EINVAL;
		goto out;
	}

	_I("launch popup (%s)", name);

	b = bundle_create();
	if (!b) {
		ret = -ENOMEM;
		goto out;
	}

	ret = bundle_add(b, POPUP_CONTENT, POPUP_NAME_POWEROFF);
	if (ret < 0) {
		_E("Failed to add bundle (%s,%s) (ret:%d)", POPUP_CONTENT, POPUP_NAME_POWEROFF, ret);
		goto out;
	}

	ret = syspopup_launch(name, b);
	if (ret < 0)
		_E("Failed to launch popup(%d)", ret);

out:
	if (b)
		bundle_free(b);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);
	return reply;
}
