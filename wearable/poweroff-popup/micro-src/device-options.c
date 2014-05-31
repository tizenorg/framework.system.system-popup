/*
 *  system-popup
 *
 * Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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


#include <stdio.h>
#include <E_DBus.h>
#include <efl_assist.h>
#include "device-options.h"
#include "launcher.h"

#define POWEROFF_BUS_NAME BUS_NAME".Poweroff"

#define DBUS_PATH_HOME_RAISE  "/Org/Tizen/Coreapps/home/raise"
#define DBUS_IFACE_HOME_RAISE "org.tizen.coreapps.home.raise"
#define HOME_RAISE_SIGNAL     "homeraise"

static E_DBus_Connection *edbus_conn;
static DBusPendingCall *edbus_request_name;

/* List to store items on the popup list */
static GList *option_list = NULL;

static E_DBus_Signal_Handler *powerkey_handler = NULL;

static gint compare_options(gconstpointer first, gconstpointer second)
{
	struct device_option *opt1 = (struct device_option *)first;
	struct device_option *opt2 = (struct device_option *)second;

	return (opt1->get_id() - opt2->get_id());
}

/* Register an item on the popup list */
void register_option(const struct device_option *opt)
{
	struct device_option *option;

	if (!opt) {
		_E("Invalid parameter");
		return;
	}

	option = (struct device_option *)malloc(sizeof(struct device_option));
	if (!option) {
		_E("FAIL: malloc()");
		return;
	}

	option->get_id              = opt->get_id;
	option->is_enabled          = opt->is_enabled;
	option->init_itc            = opt->init_itc;
	option->deinit_itc          = opt->deinit_itc;
	option->get_itc             = opt->get_itc;
	option->store_item          = opt->store_item;
	option->response_clicked    = opt->response_clicked;
	option->register_handlers   = opt->register_handlers;
	option->unregister_handlers = opt->unregister_handlers;

	option_list = g_list_insert_sorted(option_list, option, compare_options);
}

/* Unregister an item on the popup list */
void unregister_option(const struct device_option *opt)
{
	GList *l;
	struct device_option *option;

	if (!opt) {
		_E("Invalid parameter");
		return;
	}

	for (l = option_list ; l ; l = g_list_next(l)) {
		option = (struct device_option *)(l->data);
		if (!option)
			continue;

		if (option->get_id != opt->get_id)
			continue;

		option_list = g_list_delete_link(option_list, l);
		break;
	}
}

static void option_list_free_func(gpointer data)
{
	struct device_option *option = (struct device_option *)data;
	FREE(option);
}

/* Unregister all items on the popup list */
static void unregister_option_full(void)
{
	if (!option_list)
		return;

	g_list_free_full(option_list, option_list_free_func);
}

static int get_genlist_item_class(struct device_option *opt,
		Elm_Genlist_Item_Class **itc)
{
	if (!opt || !itc)
		return -EINVAL;

	if(opt->init_itc) {
		if (opt->init_itc() < 0)
			return -ENOMEM;
		if (opt->get_itc(itc) < 0)
			return -ENOMEM;
		if (*itc == NULL)
			return -ENOMEM;
	}
	return 0;
}

static void deinit_genlist_item_class(void)
{
	GList *l;
	struct device_option *opt;

	for (l = option_list ; l ; l = g_list_next(l)) {
		opt = (struct device_option *)(l->data);
		if (!opt)
			continue;
		if (opt->deinit_itc)
			opt->deinit_itc();
	}
}

static void item_realized(void *data, Evas_Object *obj, void *event_info)
{
	if (!event_info || !option_list)
		return;

	if (elm_genlist_item_index_get(event_info)
			== g_list_length(option_list) - 1)
		elm_object_item_signal_emit(event_info, "elm,state,bottomline,hide", "");
}

static int create_device_options_popup(struct appdata *ad)
{
	GList *l;
	struct device_option *opt;
	Elm_Object_Item *item = NULL;
	Elm_Genlist_Item_Class *itc = NULL;
	int ret;

	if (!ad || !(ad->win_main))
		return -EINVAL;

	if (!option_list) {
		_I("No device options");
		return 0;
	}

	ad->popup = elm_popup_add(ad->win_main);
	if (ad->popup == NULL) {
		_E("FAIL: elm_popup_add()");
		return -ENOMEM;
	}

	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	ad->list = elm_genlist_add(ad->popup);
	if (ad->list == NULL) {
		_E("Failed to create genlist");
		return -ENOMEM;
	}
	elm_object_style_set(ad->list, "popup");
	elm_genlist_mode_set(ad->list, ELM_LIST_COMPRESS);

	evas_object_smart_callback_add(ad->list, "realized", item_realized, NULL);

	for (l = option_list ; l ; l = g_list_next(l)) {
		opt = (struct device_option *)(l->data);
		if (!opt)
			continue;
		ret = get_genlist_item_class(opt, &itc);
		if (ret < 0 || !itc) {
			_E("Faile to get itc");
			continue;
		}

		item = elm_genlist_item_append(
				ad->list,
				itc,
				ad,
				NULL,
				ELM_GENLIST_ITEM_NONE,
				opt->response_clicked,
				ad);
		itc = NULL;
		if (opt->is_enabled() == false) /* Emergency mode */
			elm_object_item_disabled_set(item, EINA_TRUE);
		if (opt->store_item)
			opt->store_item(item);
	}

	elm_object_content_set(ad->popup, ad->list);
	evas_object_show(ad->list);
	deinit_genlist_item_class();

	evas_object_show(ad->popup);

	return 0;
}

static void powerkey_pushed(void *data, DBusMessage *msg)
{
	int ret;

	ret = dbus_message_is_signal(msg, DBUS_IFACE_HOME_RAISE, HOME_RAISE_SIGNAL);
	if (ret == 0)
		return;

	popup_terminate();
}

static void unregister_power_key_handler(void)
{
	if (powerkey_handler && edbus_conn) {
		e_dbus_signal_handler_del(edbus_conn, powerkey_handler);
		powerkey_handler = NULL;
	}
}

static int register_power_key_handler(void)
{
	int retry, ret;

	if (!edbus_conn) {
		retry = 0;
		while (e_dbus_init() == 0) {
			if (retry++ >= RETRY_MAX)
				return -ENOMEM;
		}

		edbus_conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
		if (!edbus_conn) {
			_E("Failed to get dbus bus");
			ret = -ENOMEM;
			goto out;
		}
	}

	powerkey_handler = e_dbus_signal_handler_add(edbus_conn, NULL, DBUS_PATH_HOME_RAISE,
			DBUS_IFACE_HOME_RAISE, HOME_RAISE_SIGNAL, powerkey_pushed, NULL);
	if (!powerkey_handler) {
		_E("Failed to register handler");
		ret = -ENOMEM;
		goto out;
	}

	ret = 0;

out:
	if (ret < 0)
		unregister_power_key_handler();
	return ret;
}

static void pm_state_changed(keynode_t *key, void *data)
{
	int state;
	struct appdata *ad = data;

	if (!key || !ad)
		return;

	state = vconf_keynode_get_int(key);
	if (state != VCONFKEY_PM_STATE_LCDOFF)
		return;

	if (vconf_ignore_key_changed(VCONFKEY_PM_STATE, pm_state_changed) != 0)
		_E("vconf key ignore failed");

	popup_terminate();
}

void register_main_list_handlers(struct appdata *ad)
{
	GList *l;
	struct device_option *opt;

	if (!ad)
		return;

	if (vconf_notify_key_changed(VCONFKEY_PM_STATE, pm_state_changed, ad) != 0)
		_E("vconf key notify failed");

	for (l = option_list ; l ; l = g_list_next(l)) {
		opt = (struct device_option *)(l->data);
		if (!opt)
			continue;
		if (opt->register_handlers)
			opt->register_handlers(ad);
	}
}

void unregister_main_list_handlers(struct appdata *ad)
{
	GList *l;
	struct device_option *opt;

	if (!ad)
		return;

	if (vconf_ignore_key_changed(VCONFKEY_PM_STATE, pm_state_changed) != 0)
		_E("vconf key ignore failed");

	for (l = option_list ; l ; l = g_list_next(l)) {
		opt = (struct device_option *)(l->data);
		if (!opt)
			continue;
		if (opt->unregister_handlers)
			opt->unregister_handlers(ad);
	}
}

static void event_back_key_up(void *data, Evas_Object *obj, void *event_info)
{
	popup_terminate();
}

static void register_main_handlers(struct appdata *ad)
{
	if (register_power_key_handler() < 0)
		_E("Failed to register power key handler");

	if (ad && ad->win_main)
		ea_object_event_callback_add(ad->win_main, EA_CALLBACK_BACK, event_back_key_up, ad);
}

static void unregister_main_handlers(struct appdata *ad)
{
	unregister_power_key_handler();

	if (ad && ad->win_main)
		ea_object_event_callback_del(ad->win_main, EA_CALLBACK_BACK, event_back_key_up);
}

static int app_create(void *data)
{
	Evas_Object *win;
	struct appdata *ad = data;
	int ret;

	/* Create window (Reqd for sys-popup) */
	win = create_win(PACKAGE);
	if (win == NULL)
		return -1;

	ad->win_main = win;

	ecore_x_netwm_window_type_set(elm_win_xwindow_get(ad->win_main),
			ECORE_X_WINDOW_TYPE_NOTIFICATION);

	reset_window_priority(ad->win_main, WIN_PRIORITY_HIGH);

	evas_object_show(ad->win_main);

	ad->options = option_list;

	elm_theme_overlay_add(NULL,EDJ_NAME);

	ret = appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR);
	if (ret != 0)
		_E("FAIL: appcore_set_i18n()");

	register_main_list_handlers(ad);
	register_main_handlers(ad);

	return 0;
}

static int app_terminate(void *data)
{
	struct appdata *ad = data;

	unregister_main_list_handlers(ad);
	unregister_main_handlers(ad);
	unregister_option_full();

	if (ad->win_main)
		evas_object_del(ad->win_main);

	return 0;
}

static int app_pause(void *data)
{
	return 0;
}

static int app_resume(void *data)
{
	return 0;
}

static int app_reset(bundle *b, void *data)
{
	struct appdata *ad = data;
	int ret;

	if (ad->popup) {
		_E("Device options popup already exists");
		return 0;
	}

	ret = create_device_options_popup(ad);
	if (ret < 0) {
		_E("Failed to create popup(%d)", ret);
		popup_terminate();
	}

	play_feedback(FEEDBACK_TYPE_NONE, FEEDBACK_PATTERN_HOLD);

	return ret;
}

/* Poweroff popup */
static DBusMessage *edbus_poweroff_popup(E_DBus_Object *obj, DBusMessage *msg)
{
	/* Dummy function not to block dbus method call*/
	return 0;
}

static const struct edbus_method
edbus_poweroff_methods[] = {
	{ "PopupLaunch" ,   NULL,  "i", edbus_poweroff_popup      },
};

static struct edbus_object
edbus_objects[]= {
	{ POPUP_PATH_POWEROFF    , POPUP_IFACE_POWEROFF    , NULL, NULL,
		edbus_poweroff_methods   , ARRAY_SIZE(edbus_poweroff_methods)  },
};

static int init_methods(void)
{
	int ret;
	int i, j;

	for (i = 0; i < ARRAY_SIZE(edbus_objects); i++) {
		for (j = 0; j < edbus_objects[i].methods_len; j++) {
			ret = e_dbus_interface_method_add(edbus_objects[i].iface,
					edbus_objects[i].methods[j].member,
					edbus_objects[i].methods[j].signature,
					edbus_objects[i].methods[j].reply_signature,
					edbus_objects[i].methods[j].func);
			if (!ret) {
				_E("fail to add method %s!", edbus_objects[i].methods[j].member);
				return -ECONNREFUSED;
			}
		}
	}
	return 0;
}

static int register_dbus(void)
{
	DBusError error;
	int retry, ret, i;

	dbus_error_init(&error);

	retry = 0;
	do {
		if (e_dbus_init())
			break;
		if (++retry == RETRY_MAX) {
			_E("fail to init edbus");
			return -ECONNREFUSED;
		}
	} while (retry <= RETRY_MAX);

	retry = 0;
	do {
		edbus_conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
		if (edbus_conn)
			break;
		if (++retry == RETRY_MAX) {
			_E("fail to get edbus");
			ret = -ECONNREFUSED;
			goto out1;
		}
	} while (retry <= RETRY_MAX);

	retry = 0;
	do {
		edbus_request_name = e_dbus_request_name(edbus_conn, POWEROFF_BUS_NAME, 0, NULL, NULL);
		if (edbus_request_name)
			break;
		if (++retry == RETRY_MAX) {
			_E("fail to request edbus name");
			ret = -ECONNREFUSED;
			goto out2;
		}
	} while (retry <= RETRY_MAX);

	for (i = 0; i < ARRAY_SIZE(edbus_objects); i++) {
		edbus_objects[i].obj = e_dbus_object_add(edbus_conn, edbus_objects[i].path, NULL);
		if (!(edbus_objects[i].obj)) {
			_E("fail to add edbus obj");
			ret = -ECONNREFUSED;
			goto out2;
		}

		edbus_objects[i].iface = e_dbus_interface_new(edbus_objects[i].interface);
		if (!(edbus_objects[i].iface)) {
			_E("fail to add edbus interface");
			ret = -ECONNREFUSED;
			goto out2;
		}

		e_dbus_object_interface_attach(edbus_objects[i].obj, edbus_objects[i].iface);
	}

	return 0;

out2:
	e_dbus_connection_close(edbus_conn);
out1:
	e_dbus_shutdown();

	return ret;
}

int main(int argc, char *argv[])
{
	int ret;
	struct appdata ad;

	struct appcore_ops ops = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.reset = app_reset,
	};

	memset(&ad, 0x0, sizeof(struct appdata));
	ops.data = &ad;

	ret = register_dbus();
	if (ret < 0) {
		_E("Failed to register dbus method()");
		return ret;
	}

	ret = init_methods();
	if (ret < 0) {
		_E("Failed to init method()");
		return ret;
	}

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
