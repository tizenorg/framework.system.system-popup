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

#include "launcher.h"

static int launch_app(char *appname,
		char *key1, char *value1,
		char *key2, char *value2,
		char *key3, char *value3)
{
	int ret;
	bundle *b;

	if (!appname)
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

	ret = aul_launch_app(appname, b);
	if (ret <= 0)
		_E("FAIL: aul_launch_app(%s)", appname);

out:
	bundle_free(b);
	return ret;
}

static int launch_app_raw(char *path, char **argv)
{
	pid_t pid;
	int ret;

	if (!path || access(path, X_OK) != 0) {
		_E("Cannot execute (%s)", path);
		return -EINVAL;
	}

	pid = fork();
	if (pid < 0) {
		_E("Failed to fork (%d)", errno);
		return -errno;
	}

	if (pid > 0) { /* Parent */
		return pid;
	}

	/* Child */
	ret = execvp(path, argv);

	/* Failed */
	_E("Failed execvp(ret: %d, errno: %d)", ret, errno);
	return -errno;
}

DBusMessage *launch_app_no_param(E_DBus_Object *obj,
				DBusMessage *msg, char *appname)
{
	DBusMessageIter iter;
	DBusMessage *reply;
	int ret;

	if (!appname) {
		ret = -EINVAL;
		goto out;
	}

	_I("launch app (%s)", appname);

	ret = launch_app(appname, NULL, NULL, NULL, NULL, NULL, NULL);
	if (ret < 0)
		_E("FAIL: launch_app(): %d", ret);

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *launch_app_single_param(E_DBus_Object *obj,
				DBusMessage *msg, char *appname)
{
	DBusError err;
	DBusMessageIter iter;
	DBusMessage *reply;
	char *key;
	char *value;
	int ret;

	if (!appname) {
		ret = -EINVAL;
		goto out;
	}

	_I("launch app (%s)", appname);

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

	ret = launch_app(appname, key, value, NULL, NULL, NULL, NULL);
	if (ret < 0)
		_E("FAIL: launch_app(): %d", ret);

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *terminate_app_by_pid(E_DBus_Object *obj, DBusMessage *msg)
{
	DBusError err;
	DBusMessageIter iter;
	DBusMessage *reply;
	pid_t pid;
	int ret;

	dbus_error_init(&err);

	if (!dbus_message_get_args(msg, &err,
		    DBUS_TYPE_INT32, &pid,
			DBUS_TYPE_INVALID)) {
		_E("there is no message");
		ret = -EINVAL;
		goto out;
	}

	if (pid < 0) {
		_E("message is invalid!");
		ret = -EINVAL;
		goto out;
	}

	ret = aul_terminate_pid(pid);
	if (ret < 0)
		_E("FAIL: aul_terminate_pid(): %d", ret);

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *launch_system_servant_app(E_DBus_Object *obj,
				DBusMessage *msg, char **argv)
{
	DBusMessageIter iter;
	DBusMessage *reply;
	int ret;

	if (!argv) {
		ret = -EINVAL;
		goto out;
	}

	_I("launch app raw(%s)", argv[0]);

	ret = launch_app_raw(argv[0], argv);
	if (ret < 0)
		_E("FAIL: launch_app_raw: %d", ret);

out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &ret);

	return reply;
}

