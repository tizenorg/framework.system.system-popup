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

static int launch_popup(char *popup_name,
				char *key1, char *value1,
				char *key2, char *value2,
				char *key3, char *value3)
{
	int ret;
	bundle *b;

	if (!popup_name)
		return -EINVAL;

	b = bundle_create();
	if (!b) {
		return -ENOMEM;
	}

	if (key1 && value1) {
		ret = bundle_add(b, key1, value1);
		if (ret < 0)
			goto out;
	}

	if (key2 && value2) {
		ret = bundle_add(b, key2, value2);
		if (ret < 0)
			goto out;
	}

	if (key3 && value3) {
		ret = bundle_add(b, key3, value3);
		if (ret < 0)
			goto out;
	}

	ret = syspopup_launch(popup_name, b);
	if (ret < 0)
		_E("FAIL: syspopup_launch(): %d", ret);

out:
	bundle_free(b);
	return ret;
}

DBusMessage *launch_popup_no_param(E_DBus_Object *obj,
				DBusMessage *msg, char *popup_name)
{
	DBusMessageIter iter;
	DBusMessage *reply;
	int ret;

	if (!popup_name) {
		ret = -EINVAL;
		goto out;
	}

	_I("launch popup (%s)", popup_name);

	ret = launch_popup(popup_name, NULL, NULL, NULL, NULL, NULL, NULL);
	if (ret < 0)
		_E("FAIL: launch_popup(): %d", ret);

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}


DBusMessage *launch_popup_single_param(E_DBus_Object *obj,
				DBusMessage *msg, char *popup_name)
{
	DBusError err;
	DBusMessageIter iter;
	DBusMessage *reply;
	char *key;
	char *value;
	int ret;

	if (!popup_name) {
		ret = -EINVAL;
		goto out;
	}

	_I("launch popup (%s)", popup_name);

	dbus_error_init(&err);

	if (!dbus_message_get_args(msg, &err,
		    DBUS_TYPE_STRING, &key,
		    DBUS_TYPE_STRING, &value,
			DBUS_TYPE_INVALID)) {
		_E("there is no message");
		ret = -EINVAL;
		goto out;
	}

	if (!key || !value) {
		_E("message is invalid!");
		ret = -EINVAL;
		goto out;
	}

	ret = launch_popup(popup_name, key, value, NULL, NULL, NULL, NULL);
	if (ret < 0)
		_E("FAIL: launch_popup(): %d", ret);

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *launch_popup_double_param(E_DBus_Object *obj,
				DBusMessage *msg, char *popup_name)
{
	DBusError err;
	DBusMessageIter iter;
	DBusMessage *reply;
	char *key1, *key2;
	char *value1, *value2;
	int ret;

	if (!popup_name) {
		ret = -EINVAL;
		goto out;
	}

	_I("launch popup (%s)", popup_name);

	dbus_error_init(&err);

	if (!dbus_message_get_args(msg, &err,
		    DBUS_TYPE_STRING, &key1,
		    DBUS_TYPE_STRING, &value1,
		    DBUS_TYPE_STRING, &key2,
		    DBUS_TYPE_STRING, &value2,
			DBUS_TYPE_INVALID)) {
		_E("there is no message");
		ret = -EINVAL;
		goto out;
	}

	if (!key1 || !key2 || !value1 || !value2) {
		_E("message is invalid!");
		ret = -EINVAL;
		goto out;
	}

	ret = launch_popup(popup_name, key1, value1, key2, value2, NULL, NULL);
	if (ret < 0)
		_E("FAIL: launch_popup(): %d", ret);

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}

