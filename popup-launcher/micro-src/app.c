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

#include <unistd.h>
#include "launcher.h"

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

