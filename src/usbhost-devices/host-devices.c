/*
 *
 * Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
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
 */

#include <stdio.h>
#include <efl_extension.h>
#include <vconf.h>
#include <ui-gadget.h>
#include <appcore-efl.h>
#include <Elementary.h>
#include <Ecore_X.h>
#include <dlog.h>
#include <bundle_internal.h>
#include <glib.h>


#undef LOG_TAG
#define LOG_TAG "SYSTEM_APPS"
#define _D(fmt, args...)   SLOGD(fmt, ##args)
#define _E(fmt, args...)   SLOGE(fmt, ##args)
#define _I(fmt, args...)   SLOGI(fmt, ##args)

#define GENLIST_STYLE "2text.1icon.4"

#define APPOPER_TYPE "_TYPE_"
#define APPOPER_TYPE_DEVICE_LIST   "DEVICE_LIST"

/* IPC to receive device info from usb-manager */
#define DEVICE_OBJECT_PATH         "/Org/Tizen/System/DeviceD/Usbhost"
#define DEVICE_INTERFACE_NAME      "org.tizen.system.deviced.Usbhost"
#define DEVICE_HOST_ALL            "host_device_all"
#define DEVICE_ADD_SIGNAL_NAME     "host_device_add"
#define DEVICE_REMOVE_SIGNAL_NAME  "host_device_remove"

#define RETRY_MAX 10
#define BUF_MAX 256

#define MOUSE_RIGHT_BUTTON 3

#define EDJ_PATH "/usr/apps/org.tizen.host-devices/res/edje/host-devices"
#define EDJ_NAME EDJ_PATH"/host-devices.edj"
#define USB_ICON_BLUE "usb_icon_blue.png"

struct appdata {
	Evas_Object *win_main;
};

static Evas_Object *nf;
static Evas_Object *gl; /* genlist */
static GList *devs;     /* devices to show on the genlist  */

struct dev_noti {
	char title[BUF_MAX];
	char content[BUF_MAX];
	Elm_Object_Item *item;
};

static Elm_Genlist_Item_Class itc;
static E_DBus_Signal_Handler *handler_add;
static E_DBus_Signal_Handler *handler_remove;
static E_DBus_Connection *edbus_conn;

static Ecore_Event_Handler *mouse_event_handler;

static void release_handlers(struct appdata *ad);

/* Terminate app */
static Eina_Bool exit_idler_cb(void *data)
{
	elm_exit();
	return ECORE_CALLBACK_CANCEL;
}

static void terminate_app(void)
{
	if (ecore_idler_add(exit_idler_cb, NULL))
		return;
	exit_idler_cb(NULL);
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

static int broadcast_dbus_signal(const char *path, const char *interface,
		const char *name, const char *sig, char *param[])
{
	DBusPendingCall *pc;
	DBusMessageIter iter;
	DBusMessage *msg;
	int ret;

	if (!path || !interface || !name)
		return -EINVAL;

	if (!edbus_conn)
		return -EIO;

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

	pc = e_dbus_message_send(edbus_conn, msg, NULL, -1, NULL);
	if (!pc) {
		_E("FAIL: e_dbus_message_send()");
		ret = -ECONNREFUSED;
		goto out;
	}

	ret = 0;

out:
	if (msg)
		dbus_message_unref(msg);
	return ret;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item;

	if (!event_info)
		return ;

	item = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);
}

static int request_all_host_device_info(void)
{
	return broadcast_dbus_signal(DEVICE_OBJECT_PATH, DEVICE_INTERFACE_NAME,
			DEVICE_HOST_ALL, NULL, NULL);
}

static void show_dev_list(GList *l)
{
	GList *t;
	struct dev_noti *noti;
	_D("***** DEVS_LIST ******");
	if (!l) {
		_D("** DEVS EMPTY *******");
		return;
	}

	for (t = l ; t ; t = g_list_next(t)) {
		noti = (struct dev_noti*)(t->data);
		_D("** title: %s", noti->title);
		_D("** content :%s", noti->content);
		_D("****************************");
	}
}

static char *get_title_id(char *type)
{
	if (!strncmp(type, "keyboard", strlen(type)))
		return strdup("IDS_COM_POP_KEYBOARD_CONNECTED_ABB2");

	else if (!strncmp(type, "mouse", strlen(type)))
		return strdup("IDS_COM_POP_MOUSE_CONNECTED_ABB2");

	else if (!strncmp(type, "camera", strlen(type)))
		return strdup("IDS_COM_POP_CAMERA_CONNECTED_ABB2");

	else if (!strncmp(type, "printer", strlen(type)))
		return strdup("IDS_COM_POP_PRINTER_CONNECTED_ABB2");

	else if (!strncmp(type, "unknown", strlen(type)))
		return strdup("IDS_COM_POP_UNKNOWN_USB_DEVICE_CONNECTED");

	else
		return NULL;
}

static char *get_content_text(char *vendor, char *model)
{
	char content[BUF_MAX];

	if (vendor && model)
		snprintf(content, sizeof(content), "%s %s", vendor, model);
	else if (vendor)
		snprintf(content, sizeof(content), "%s", vendor);
	else if (model)
		snprintf(content, sizeof(content), "%s", model);
	else
		snprintf(content, sizeof(content), " ");

	return strdup(content);
}

static void add_dev_noti(struct appdata *ad, char *type, char *vendor, char *model)
{
	struct dev_noti *noti;
	char *title;
	char *content;

	if (!ad || !type)
		return;

	title = get_title_id(type);
	if (!title) {
		return;
	}

	content = get_content_text(vendor, model);
	if (!content) {
		free(title);
		return;
	}

	noti = (struct dev_noti *)malloc(sizeof(struct dev_noti));
	if (!noti) {
		_E("FAIL: malloc()");
		free(title);
		free(content);
		return;
	}

	snprintf(noti->title, sizeof(noti->title), "%s", title);
	snprintf(noti->content, sizeof(noti->content), "%s", content);
	free(title);
	free(content);

	noti->item = elm_genlist_item_append(gl, &itc, noti, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);

	devs = g_list_append(devs, noti);
	show_dev_list(devs);
}

static void remove_dev_noti(struct appdata *ad, char *type, char *vendor, char *model)
{
	GList *l;
	struct dev_noti *noti;
	char *title;
	char *content;

	if (!ad || !type)
		return;

	title = get_title_id(type);
	if (!title)
		return;

	content = get_content_text(vendor, model);
	if (!content) {
		free(title);
		return;
	}

	for (l = devs; l ; l = g_list_next(l)) {
		noti = (struct dev_noti *)(l->data);
		if (strncmp(noti->title, title, strlen(title)))
			continue;
		if (strncmp(noti->content, content, strlen(content)))
			continue;

		elm_object_item_del(noti->item);
		noti->item = NULL;
		devs = g_list_delete_link(devs, l);
		break;
	}
	show_dev_list(devs);
	free(title);
	free(content);
}

static void host_device_add_signal_cb(void *data, DBusMessage *msg)
{
	DBusError err;
	char *type;
	char *vendor;
	char *model;
	struct appdata *ad;

	if (!msg || !data) {
		return;
	}

	ad = (struct appdata *)data;
	if (dbus_message_is_signal(msg, DEVICE_INTERFACE_NAME, DEVICE_ADD_SIGNAL_NAME) == 0) {
		return;
	}

	dbus_error_init(&err);
	if (dbus_message_get_args(msg, &err,
				DBUS_TYPE_STRING, &type,
				DBUS_TYPE_STRING, &vendor,
				DBUS_TYPE_STRING, &model,
				DBUS_TYPE_INVALID) == 0) {
		return;
	}

	add_dev_noti(ad, type, vendor, model);
}

static void host_device_remove_signal_cb(void *data, DBusMessage *msg)
{
	DBusError err;
	char *type;
	char *vendor;
	char *model;
	struct appdata *ad;

	if (!msg || !data) {
		return;
	}

	ad = (struct appdata *)data;
	if (dbus_message_is_signal(msg, DEVICE_INTERFACE_NAME, DEVICE_REMOVE_SIGNAL_NAME) == 0) {
		return;
	}

	dbus_error_init(&err);
	if (dbus_message_get_args(msg, &err,
				DBUS_TYPE_STRING, &type,
				DBUS_TYPE_STRING, &vendor,
				DBUS_TYPE_STRING, &model,
				DBUS_TYPE_INVALID) == 0) {
		return;
	}

	remove_dev_noti(ad, type, vendor, model);
}

static void unregister_edbus_signal_handler()
{
    e_dbus_signal_handler_del(edbus_conn, handler_add);
	e_dbus_signal_handler_del(edbus_conn, handler_remove);
	e_dbus_connection_close(edbus_conn);
	e_dbus_shutdown();
}

static int register_edbus_signal_handler(struct appdata *ad)
{
	int retry;
	if (!ad)
		return -1;

	retry = 0;
	while (e_dbus_init() == 0) {
		if (retry++ >= RETRY_MAX)
			return -1;
	}

	edbus_conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
	if (!edbus_conn)
		return -1;

	handler_add = e_dbus_signal_handler_add(edbus_conn, NULL, DEVICE_OBJECT_PATH,
			DEVICE_INTERFACE_NAME, DEVICE_ADD_SIGNAL_NAME, host_device_add_signal_cb, ad);
	if (!handler_add)
		return -1;

	handler_remove = e_dbus_signal_handler_add(edbus_conn, NULL, DEVICE_OBJECT_PATH,
			DEVICE_INTERFACE_NAME, DEVICE_REMOVE_SIGNAL_NAME, host_device_remove_signal_cb, ad);
	if (!handler_remove)
		return -1;

	return 0;
}

static void _quit_cb(void *data, Evas_Object* obj, void* event_info)
{
	terminate_app();
}

static int create_bg(Evas_Object *parent)
{
	Evas_Object *bg;

	if (!parent)
		return -EINVAL;

	bg = elm_bg_add(parent);
	if (!bg)
		return -ENOMEM;

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, bg);
	evas_object_show(bg);

	return 0;
}

static Evas_Object* create_conform(Evas_Object *parent)
{
	Evas_Object *conform;

	if (!parent)
		return NULL;

	conform = elm_conformant_add(parent);
	if (!conform)
		return NULL;

	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, conform);
	evas_object_show(conform);

	return conform;
}

static Evas_Object* create_layout_main(Evas_Object* parent)
{
	Evas_Object *layout;

	if (!parent)
		return NULL;

	layout = elm_layout_add(parent);
	if (!layout) {
		return NULL;
	}

	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_show(layout);

	return layout;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	/* TODO The text of list will be changed according to the usb device connected */
	struct dev_noti *noti;

	if (!data)
		return NULL;

	noti = (struct dev_noti *)data;
	if (!strcmp(part, "elm.text.main.left.top"))
		return strdup(_(noti->title));

	if (!strcmp(part, "elm.text.sub.left.bottom"))
		return strdup(_(noti->content));

	return NULL;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *layout;
	Evas_Object *image;

	if (strcmp(part, "elm.icon.1"))
		return NULL;
	layout = elm_layout_add(obj);
	if (!layout)
		return NULL;
	elm_layout_theme_set(layout, "layout", "list/B/type.3", "default");

	image = elm_image_add(obj);
	if (!image)
		return NULL;
	elm_image_file_set(image, EDJ_NAME, USB_ICON_BLUE);
	evas_object_size_hint_align_set(image, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_content_set(layout, "elm.swallow.content", image);
	return layout;
}

static int create_naviframe_layout(Evas_Object* parent)
{
	if (!parent)
		return -EINVAL;

	nf = elm_naviframe_add(parent);
	if (!nf)
		return -ENOMEM;

	elm_object_part_content_set(parent, "elm.swallow.content", nf);

	evas_object_show(nf);

	return 0;
}

static int create_view_layout(struct appdata *ad)
{
	Evas_Object *btn;

	if (!ad || !nf)
		return -EINVAL;

	itc.item_style = GENLIST_STYLE;
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = _gl_content_get;
	itc.func.state_get = NULL;
	itc.func.del = NULL;

	gl = elm_genlist_add(ad->win_main);
	if (!gl)
		return -ENOMEM;

	btn = elm_button_add(nf);
	if (!btn)
		return -ENOMEM;

	elm_object_style_set(btn, "naviframe/end_btn/default");
	evas_object_smart_callback_add(btn, "clicked", _quit_cb, ad->win_main);
	elm_naviframe_item_push(nf,
			_("IDS_USB_MBODY_USB_DEVICES"),
			btn, NULL, gl, NULL);

	return 0;
}

static int init_host_devices(struct appdata *ad)
{
	Evas_Object *conform;
	Evas_Object *layout_main;
	int ret;

	if (!ad)
		return -EINVAL;

	// Background Image
	ret = create_bg(ad->win_main);
	if (ret < 0)
		return ret;

	// Conformant
	conform = create_conform(ad->win_main);
	if (!conform)
		return -ENOMEM;

	// Base Layout
	layout_main = create_layout_main(conform);
	if (!layout_main)
		return -ENOMEM;

	elm_object_content_set(conform, layout_main);

	elm_win_conformant_set(ad->win_main, EINA_TRUE);

	// Naviframe
	ret = create_naviframe_layout(layout_main);
	if (ret < 0)
		return ret;

	// Naviframe Content
	ret = create_view_layout(ad);
	if (ret < 0)
		return ret;

	return 0;
}

int _lang_changed(void *event, void *data)
{
	char *locale = vconf_get_str(VCONFKEY_LANGSET);
	if (locale)
		elm_language_set(locale);
	return 0;
}

static void window_resize_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	struct appdata *ad;
	Evas_Coord root_w;
	Evas_Coord root_h;

	if (!data)
		return ;

	ad = (struct appdata *)data;
	evas_object_geometry_get(ad->win_main, NULL, NULL, &root_w, &root_h);
}

static Evas_Object* create_window(const char *name)
{
	Evas_Object *eo;
	int rots[4] = {0, 90, 180, 270};
	int w, h;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (!eo)
		return NULL;

	elm_win_title_set(eo, name);
	elm_win_autodel_set(eo, EINA_TRUE);
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
	evas_object_resize(eo, w, h);

	if (elm_win_wm_rotation_supported_get(eo)) {
		elm_win_wm_rotation_available_rotations_set(eo, rots, 4);
	}

	evas_object_smart_callback_add(eo, "delete,request", _quit_cb, NULL);

	return eo;
}

static void event_back_key_up(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	if (!ad)
		return;

	eext_object_event_callback_del(ad->win_main, EEXT_CALLBACK_BACK, event_back_key_up);

	if (mouse_event_handler) {
		ecore_event_handler_del(mouse_event_handler);
		mouse_event_handler = NULL;
	}

	terminate_app();
}

static Eina_Bool event_mouse_button_up(void *data, int type, void *event)
{
	struct appdata *ad = data;
	Ecore_Event_Mouse_Button *move = event;

	if (!ad || !move)
		return ECORE_CALLBACK_RENEW;

	if (move->buttons == MOUSE_RIGHT_BUTTON) {
		release_handlers(ad);
		terminate_app();
	}

	return ECORE_CALLBACK_RENEW;
}

static void release_handlers(struct appdata *ad)
{
	if (!ad)
		return;

	eext_object_event_callback_del(ad->win_main, EEXT_CALLBACK_BACK, event_back_key_up);

	if (mouse_event_handler) {
		ecore_event_handler_del(mouse_event_handler);
		mouse_event_handler = NULL;
	}
}

static int create_usb_devices_window(struct appdata *ad)
{
	int ret;
	Evas *evas;

	if (!ad)
		return -EINVAL;

	ad->win_main = create_window(PKGNAME);
	if (!(ad->win_main))
		return -ENOMEM;

	evas_object_event_callback_add(ad->win_main, EVAS_CALLBACK_RESIZE, window_resize_cb, ad);

	eext_object_event_callback_add(ad->win_main, EEXT_CALLBACK_BACK, event_back_key_up, ad);

	mouse_event_handler = ecore_event_handler_add(
			ECORE_EVENT_MOUSE_BUTTON_UP, event_mouse_button_up, ad);
	if (!mouse_event_handler)
		return -ENOMEM;


	evas_object_show(ad->win_main);

	// Indicator
	if (UG_INIT_EFL(ad->win_main, UG_OPT_INDICATOR_ENABLE) != 0) {
		return -ENOMEM;
	}
	elm_win_indicator_mode_set(ad->win_main, ELM_WIN_INDICATOR_SHOW);


	evas = evas_object_evas_get(ad->win_main);
	if (!evas)
		return -ENOMEM;

	ret = init_host_devices(ad);

	return ret;
}

int app_create(void *data)
{
	struct appdata *ad;
	int ret;

	if (!data)
		return -EINVAL;

	ad = (struct appdata *) data;

	appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR);
	appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, _lang_changed, index);

	elm_theme_overlay_add(NULL,EDJ_NAME);

	ret = register_edbus_signal_handler(ad);
	if (ret < 0) {
		_E("FAIL: register_edbus_signal_handler()");
		return ret;
	}

	return 0;
}

int app_terminate(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if (ad->win_main) {
		evas_object_del(ad->win_main);
		ad->win_main = NULL;
	}

	unregister_edbus_signal_handler();
	return 0;
}

int app_pause(void *data)
{
	return 0;
}

int app_resume(void *data)
{
	return 0;
}

int app_reset(bundle *b, void *data)
{
	struct appdata *ad = data;
	int ret;
	char *val;

	if (!b) {
		_E("ERROR: bundle is NULL");
		ret = -EINVAL;
		goto reset_out;
	}

	val = (char *)bundle_get_val(b, APPOPER_TYPE);
	if (!val) {
		_E("FAIL: bundle_get_val()");
		ret = -ENOMEM;
		goto reset_out;
	}

	ret = strcmp(val, APPOPER_TYPE_DEVICE_LIST);
	if (ret != 0) {
		goto reset_out;
	}

	ret = create_usb_devices_window(ad);
	if (ret < 0) {
		_E("FAIL: create_usb_devices_window()");
		goto reset_cb_out;
	}

	ret = request_all_host_device_info();
	if (ret < 0) {
		_E("FAIL: request_all_host_device_info()");
		goto reset_cb_out;
	}

	if (ad->win_main)
		elm_win_activate(ad->win_main);

	return 0;

reset_cb_out:
	release_handlers(ad);
reset_out:
	terminate_app();
	return ret;
}

int main(int argc, char *argv[])
{
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

	return appcore_efl_main(PKGNAME, &argc, &argv, &ops);
}
