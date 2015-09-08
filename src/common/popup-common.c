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
#include <dd-display.h>

#define RETRY_MAX 10
#define DBUS_REPLY_TIMEOUT  (-1)
#define BUF_MAX 64

static E_DBus_Connection *edbus_conn = NULL;

/* Terminate popup */
static Eina_Bool exit_idler_cb(void *data)
{
	elm_exit();
	return ECORE_CALLBACK_CANCEL;
}

void popup_terminate(void)
{
	if (ecore_idler_add(exit_idler_cb, NULL))
		return;

	exit_idler_cb(NULL);
}

/* Feedback */
void play_feedback(int type, int pattern)
{
	int ret;

	ret = feedback_initialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		_E("Cannot initialize feedback");
		return;
	}

	switch (type) {
	case FEEDBACK_TYPE_LED:
	case FEEDBACK_TYPE_SOUND:
	case FEEDBACK_TYPE_VIBRATION:
		ret = feedback_play_type(type, pattern);
		break;
	case FEEDBACK_TYPE_NONE:
		ret = feedback_play(pattern);
		break;
	default:
		_E("Play type is unknown");
		ret = 0;
	}
	if (ret != FEEDBACK_ERROR_NONE)
		_E("Cannot play feedback: %d", pattern);

	ret = feedback_deinitialize();
	if (ret != FEEDBACK_ERROR_NONE)
		_E("Cannot deinitialize feedback");
}

static void *thread_feedback(void* data)
{
	int type = (int)data;
	if (type < 0)
		type = FEEDBACK_PATTERN_LOWBATT; /* Warning */
	play_feedback(FEEDBACK_TYPE_NONE, type);
	return NULL;
}

static void *thread_display(void* data)
{
	if (display_change_state(LCD_NORMAL) < 0)
		_E("FAIL: display_change_state()");
	return NULL;
}

static void start_thread(void *(*operation)(void *), void *data)
{
	pthread_t th;
	int ret;

	ret = pthread_create(&th, NULL, operation, data);
	if (ret < 0) {
		_E("Failed to create pthread(%d)", ret);
		return;
	}
	pthread_detach(th);
	return;
}

void change_display_state(void)
{
	start_thread(thread_display, NULL);
}

void notify_feedback(int pattern)
{
	start_thread(thread_feedback, (void *)pattern);
}

/* dbus */
int set_dbus_connection(void)
{
	int retry;

	if (edbus_conn)
		return 0;

	retry = 0;
	while (e_dbus_init() == 0) {
		if (retry++ >= RETRY_MAX)
			return -ENOMEM;
	}

	edbus_conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
	if (!edbus_conn) {
		_E("Failed to get dbus bus");
		e_dbus_shutdown();
		return -ENOMEM;
	}

	return 0;
}

E_DBus_Connection *get_dbus_connection(void)
{
	return edbus_conn;
}

void unset_dbus_connection(void)
{
	if (edbus_conn) {
		e_dbus_connection_close(edbus_conn);
		e_dbus_shutdown();
		edbus_conn = NULL;
	}
}

static int append_variant(DBusMessageIter *iter, const char *sig, char *param[])
{
	char *ch;
	int i;
	int iValue;

	if (!sig || !param)
		return 0;

	for (ch = (char*)sig, i = 0; *ch != '\0'; ++i, ++ch) {
		switch (*ch) {
		case 'i':
			iValue = atoi(param[i]);
			dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &iValue);
			break;
		case 's':
			dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &param[i]);
			break;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

int broadcast_dbus_signal(const char *path, const char *interface,
		const char *name, const char *sig, char *param[])
{
	E_DBus_Connection *conn = NULL;
	DBusPendingCall *pc;
	DBusMessageIter iter;
	DBusMessage *msg;
	int ret;

	if (!path || !interface || !name)
		return -EINVAL;

	conn = get_dbus_connection();
	if (!conn) {
		_E("Failed to get dbus connection");
		return -ENOMEM;
	}

	msg = dbus_message_new_signal(path, interface, name);
	if (!msg) {
		_E("FAIL: dbus_message_new_signal()");
		return -ENOMEM;
	}

	dbus_message_iter_init_append(msg, &iter);
	ret = append_variant(&iter, sig, param);
	if (ret < 0) {
		_E("append_variant error(%d)", ret);
		goto out;
	}

	pc = e_dbus_message_send(conn, msg, NULL, -1, NULL);
	if (!pc) {
		_E("FAIL: e_dbus_message_send()");
		ret = -ECONNREFUSED;
		goto out;
	}

	ret = 0;

out:
	dbus_message_unref(msg);
	return ret;
}

int dbus_method_sync(const char *dest, const char *path,
		const char *interface, const char *method,
		const char *sig, char *param[])
{
	DBusConnection *conn = NULL;
	DBusMessage *msg = NULL;
	DBusMessageIter iter;
	DBusMessage *reply = NULL;
	DBusError err;
	int ret, result;

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (!conn) {
		_E("dbus_bus_get error");
		return -EPERM;
	}

	msg = dbus_message_new_method_call(dest, path, interface, method);
	if (!msg) {
		_E("dbus_message_new_method_call(%s:%s-%s)", path, interface, method);
		ret = -EBADMSG;
		goto out;
	}

	dbus_message_iter_init_append(msg, &iter);
	ret = append_variant(&iter, sig, param);
	if (ret < 0) {
		_E("append_variant error(%d)", ret);
		goto out;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, DBUS_REPLY_TIMEOUT, &err);
	if (!reply) {
		_E("dbus_connection_send error(%s:%s)", err.name, err.message);
		dbus_error_free(&err);
		ret = -ECOMM;
		goto out;
	}

	ret = dbus_message_get_args(reply, &err, DBUS_TYPE_INT32, &result, DBUS_TYPE_INVALID);
	if (!ret) {
		_E("no message : [%s:%s]", err.name, err.message);
		dbus_error_free(&err);
		ret = -ENOMSG;
		goto out;
	}

	ret = result;

out:
	if (msg)
		dbus_message_unref(msg);
	if (reply)
		dbus_message_unref(reply);
	if (conn)
		dbus_connection_unref(conn);
	return ret;
}

