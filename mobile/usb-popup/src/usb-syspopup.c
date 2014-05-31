/*
 * usb-syspopup
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
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
#include <vconf.h>
#include <ail.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <appcore-efl.h>
#include <Ecore_X.h>
#include <assert.h>
#include "usb-syspopup.h"

#define SOCK_PATH "/tmp/usb_server_sock"
#define SOCK_STR_LEN 1542

#define SYSPOPUP_TYPE    "_SYSPOPUP_TYPE"

#define ACC_MANUFACTURER "_ACC_MANUFACTURER"
#define ACC_MODEL        "_ACC_MODEL"
#define ACC_DESCRIPTION  "_ACC_DESCRIPTION"
#define ACC_VERSION      "_ACC_VERSION"
#define ACC_URI          "_ACC_URI"
#define ACC_SERIAL       "_ACC_SERIAL"

#define HOST_CLASS       "_HOST_CLASS"
#define HOST_SUBCLASS    "_HOST_SUBCLASS"
#define HOST_PROTOCOL    "_HOST_PROTOCOL"
#define HOST_IDVENDOR    "_HOST_IDVENDOR"
#define HOST_IDPRODUCT   "_HOST_IDPRODUCT"

static Eina_Bool exit_idler_cb(void *data)
{
	elm_exit();
	return ECORE_CALLBACK_CANCEL;
}

static void popup_terminate(void)
{
	if (ecore_idler_add(exit_idler_cb, NULL))
		return;

	exit_idler_cb(NULL);
}

static int ipc_socket_client_init()
{
	__USB_FUNC_ENTER__ ;
	int sock;
	struct sockaddr_un remote;

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		USB_LOG("FAIL: socket(AF_UNIX, SOCK_STREAM, 0)");
		return -1;
	}
	remote.sun_family = AF_UNIX;
	strncpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path));

	if (connect(sock, (struct sockaddr *)&remote, sizeof(remote)) == -1) {
		USB_LOG("FAIL: connect(sock, (struct sockaddr *)&remote, len)");
		close(sock);
		return -1;
	}
	__USB_FUNC_EXIT__ ;
	return sock;
}

static void __win_del(void *data, Evas_Object * obj, void *event)
{
	__USB_FUNC_ENTER__ ;
	popup_terminate();
	__USB_FUNC_EXIT__ ;
}

static Evas_Object *__create_win(const char *name)
{
	__USB_FUNC_ENTER__ ;
	assert(name);
	Evas_Object *eo;
	int w;
	int h;

	eo = elm_win_add(NULL, name, ELM_WIN_DIALOG_BASIC);
	if (!eo) {
		USB_LOG("FAIL: elm_win_add()");
		return NULL;
	}

	elm_win_title_set(eo, name);
	elm_win_borderless_set(eo, EINA_TRUE);
	elm_win_alpha_set(eo, EINA_TRUE);
	elm_win_raise(eo);
	evas_object_smart_callback_add(eo, "delete,request", __win_del, NULL);
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
	evas_object_resize(eo, w, h);

	__USB_FUNC_EXIT__ ;

	return eo;
}

static void usp_usbclient_chgdet_cb(keynode_t *key, void *data)
{
	__USB_FUNC_ENTER__ ;
	int ret;
	int usb_status;

	ret = vconf_get_int(VCONFKEY_SYSMAN_USB_STATUS, &usb_status);
	if (0 == ret && usb_status == VCONFKEY_SYSMAN_USB_DISCONNECTED) {
		USB_LOG("USB cable is not connected");
		popup_terminate();
	}
	__USB_FUNC_EXIT__ ;
}

static void usp_usbhost_chgdet_cb(keynode_t *key, void *data)
{
	__USB_FUNC_ENTER__ ;
	int ret;
	int usb_status;

	ret = vconf_get_int(VCONFKEY_SYSMAN_USB_HOST_STATUS, &usb_status);
	if (0 == ret && usb_status == VCONFKEY_SYSMAN_USB_HOST_DISCONNECTED) {
		USB_LOG("USB host is not connected");
		popup_terminate();
	}
	__USB_FUNC_EXIT__ ;
}

static int usp_vconf_key_notify(void)
{
	__USB_FUNC_ENTER__;
	int ret;

	/* Event for USB cable */
	ret = vconf_notify_key_changed(VCONFKEY_SYSMAN_USB_STATUS,
					usp_usbclient_chgdet_cb, NULL);
	if (0 != ret) {
		USB_LOG("FAIL: vconf_notify_key_changed(VCONFKEY_SYSMAN_USB_STATUS)");
		return -1;
	}

	/* Event for USB host */
	ret = vconf_notify_key_changed(VCONFKEY_SYSMAN_USB_HOST_STATUS,
					usp_usbhost_chgdet_cb, NULL);
	if (0 != ret) {
		USB_LOG("FAIL: vconf_notify_key_changed(VCONFKEY_SYSMAN_USB_HOST_STATUS)");
		return -1;
	}

	__USB_FUNC_EXIT__ ;
	return 0;
}

static int usp_vconf_key_ignore(void)
{
	__USB_FUNC_ENTER__;
	int ret;

	/* Event for USB cable */
	ret = vconf_ignore_key_changed(VCONFKEY_SYSMAN_USB_STATUS, usp_usbclient_chgdet_cb);
	if (0 != ret) {
		USB_LOG("FAIL: vconf_ignore_key_changed(VCONFKEY_SYSMAN_USB_STATUS)");
		return -1;
	}

	/* Event for USB host */
	ret = vconf_ignore_key_changed(VCONFKEY_SYSMAN_USB_HOST_STATUS, usp_usbhost_chgdet_cb);
	if (0 != ret) {
		USB_LOG("FAIL: vconf_ignore_key_changed(VCONFKEY_SYSMAN_USB_HOST_STATUS)");
		return -1;
	}

	__USB_FUNC_EXIT__ ;
	return 0;
}

static int __app_create(void *data)
{
	__USB_FUNC_ENTER__ ;
	assert(data);
	struct appdata *ad = (struct appdata *)data;
	int ret;

	ad->handler.def_term_fn = NULL;
	ad->handler.def_timeout_fn = NULL;
	ad->usbAcc = NULL;
	ad->usbHost = NULL;
	ad->mApps = NULL;

	ret = usp_vconf_key_notify();
	if (ret != 0) {
		USB_LOG("FAIL: usp_vconf_key_notify()");
		return -1;
	}

	/* init internationalization */
	ret = appcore_set_i18n(PACKAGE, LOCALEDIR);
	if (ret != 0) {
		USB_LOG("FAIL: appcore_set_i18n(PACKAGE, LOCALEDIR)");
		return -1;
	}

	__USB_FUNC_EXIT__ ;

	return 0;
}

static void unload_popup(struct appdata *ad)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);

	if (ad->win) {
		evas_object_del(ad->win);
		ad->win = NULL;
	}

	__USB_FUNC_EXIT__ ;
}

static int __app_terminate(void *data)
{
	__USB_FUNC_ENTER__ ;
	assert(data);
	struct appdata *ad = (struct appdata *)data;
	int ret;

	ret = usp_vconf_key_ignore();
	if (ret != 0) USB_LOG("FAIL: usp_vconf_key_ignore()");

	unload_popup(ad);

	if (ad->b) {
		ret = bundle_free(ad->b);
		if (ret != 0) {
			USB_LOG("FAIL: bundle_free(ad->b)");
		}
		ad->b = NULL;
	}

	if (ad->usbAcc) {
		FREE(ad->usbAcc->manufacturer);
		FREE(ad->usbAcc->model);
		FREE(ad->usbAcc->description);
		FREE(ad->usbAcc->version);
		FREE(ad->usbAcc->uri);
		FREE(ad->usbAcc->serial);
	}

	FREE(ad->usbAcc);
	FREE(ad->usbHost);

	if (ad->mApps) {
		g_list_free(ad->mApps);
	}

	__USB_FUNC_EXIT__ ;
	return 0;
}

static int __app_pause(void *data)
{
	__USB_FUNC_ENTER__ ;
	__USB_FUNC_EXIT__ ;
	return 0;
}

static int __app_resume(void *data)
{
	__USB_FUNC_ENTER__ ;
	__USB_FUNC_EXIT__ ;
	return 0;
}

static int request_to_usb_server(int request, char *pkgName, char *answer, int answerLen)
{
	__USB_FUNC_ENTER__ ;
	assert(answer);
	int sock_remote;
	char str[SOCK_STR_LEN];
	int t;
	sock_remote = ipc_socket_client_init();
	if (0 > sock_remote) {
		USB_LOG("FAIL: ipc_socket_client_init()");
		return -1;
	}

	switch (request) {
	case LAUNCH_APP_FOR_ACC:
	case LAUNCH_APP_FOR_HOST:
		snprintf(str, sizeof(str), "%d|%s", request, pkgName);
		break;
	default:
		snprintf(str, sizeof(str), "%d|", request);
		break;
	}
	USB_LOG("request: %s", str);

	if (send (sock_remote, str, strlen(str)+1, 0) == -1) {
		USB_LOG("FAIL: send (sock_remote, str, strlen(str), 0)");
		close(sock_remote);
		return -1;
	}

	if ((t = recv(sock_remote, answer, answerLen, 0)) > 0) {
		if (t < answerLen) {
			answer[t] = '\0';
		} else { /* t == answerLen */
			answer[answerLen-1] = '\0';
		}
		USB_LOG("[CLIENT] Received value: %s", answer);
	} else {
		USB_LOG("FAIL: recv(sock_remote, answer, answerLen, 0)");
		close(sock_remote);
		return -1;
	}

	close(sock_remote);
	__USB_FUNC_EXIT__ ;
	return 0;
}

static int load_normal_popup(struct appdata *ad,
			char *mainText,
			char *lbtnText,
			Evas_Smart_Cb lbtn_cb,
			char *rbtnText,
			Evas_Smart_Cb rbtn_cb)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);
	assert(mainText);
	assert(lbtnText);
	assert(lbtn_cb);

	Evas_Object *win;
	int ret = -1;
	Evas_Object *lbtn;
	Evas_Object *rbtn;

	unload_popup(ad);

	/* create window */
	win = __create_win(PACKAGE);
	if (win == NULL)
		return -1;
	ad->win = win;

	ret = syspopup_create(ad->b, &(ad->handler), ad->win, ad);
	if (0 != ret) {
		USB_LOG("FAIL: syspopup_create(): %d", ret);
		return -1;
	}

	evas_object_show(ad->win);
	ad->popup = elm_popup_add(ad->win);
	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(ad->popup, mainText);

	if (lbtnText && lbtn_cb) {
		/* Left button */
		lbtn = elm_button_add(ad->popup);
		elm_object_text_set(lbtn, lbtnText);
		elm_object_part_content_set(ad->popup, "button1", lbtn);
		evas_object_smart_callback_add(lbtn, "clicked", lbtn_cb, ad);
	}

	if (rbtnText && rbtn_cb) {
		/* Right button */
		rbtn = elm_button_add(ad->popup);
		elm_object_text_set(rbtn, rbtnText);
		elm_object_part_content_set(ad->popup, "button2", rbtn);
		evas_object_smart_callback_add(rbtn, "clicked", rbtn_cb, ad);
	}

	evas_object_show(ad->popup);

	__USB_FUNC_EXIT__ ;
	return 0;
}

static void load_connection_failed_popup_ok_response_cb(void *data, Evas_Object *obj, void *info)
{
	__USB_FUNC_ENTER__;
	assert(data);
	struct appdata *ad = (struct appdata *)data;
	char buf[SOCK_STR_LEN];
	int ret;

	unload_popup(ad);

	ret = request_to_usb_server(ERROR_POPUP_OK_BTN, NULL, buf, sizeof(buf));
	if (ret < 0) {
		USB_LOG("FAIL: request_to_usb_server(ERROR_POPUP_OK_BTN)");
		return;
	}

	popup_terminate();

	__USB_FUNC_EXIT__ ;
}

static void notice_to_usb_server_perm_response(struct appdata *ad, int info)
{
	__USB_FUNC_ENTER__;
	assert(ad);
	int ret;
	char buf[SOCK_STR_LEN];

	unload_popup(ad);

	ret = request_to_usb_server(info, NULL, buf, sizeof(buf));
	if (ret < 0) {
		USB_LOG("FAIL: request_to_usb_server()");
	}
	__USB_FUNC_EXIT__ ;
}

static void request_perm_popup_yes_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	__USB_FUNC_ENTER__;
	assert(data);
	struct appdata *ad = (struct appdata *)data;

	switch (ad->type) {
	case REQ_ACC_PERM_POPUP:
		notice_to_usb_server_perm_response(ad, REQ_ACC_PERM_NOTI_YES_BTN);
		break;
	case REQ_HOST_PERM_POPUP:
		notice_to_usb_server_perm_response(ad, REQ_HOST_PERM_NOTI_YES_BTN);
		break;
	default:
		USB_LOG("ERROR: ad->type is improper: %d", ad->type);
		break;
	}

	popup_terminate();
	__USB_FUNC_EXIT__ ;
}

static void request_perm_popup_no_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	__USB_FUNC_ENTER__;
	assert(data);
	struct appdata *ad = (struct appdata *)data;

	switch (ad->type) {
	case REQ_ACC_PERM_POPUP:
		notice_to_usb_server_perm_response(ad, REQ_ACC_PERM_NOTI_NO_BTN);
		break;
	case REQ_HOST_PERM_POPUP:
		notice_to_usb_server_perm_response(ad, REQ_HOST_PERM_NOTI_NO_BTN);
		break;
	default:
		USB_LOG("ERROR: ad->type is improper: %d", ad->type);
		break;
	}

	popup_terminate();
	__USB_FUNC_EXIT__ ;
}

static void load_connection_failed_popup(void *data)
{
	__USB_FUNC_ENTER__ ;
	assert(data);
	struct appdata *ad = (struct appdata *)data;
	int ret;

	ret = load_normal_popup(ad,
			gettext("IDS_USB_POP_USB_CONNECTION_FAILED"),
			dgettext("sys_string","IDS_COM_SK_OK"),
			load_connection_failed_popup_ok_response_cb,
			NULL,
			NULL);
	if (0 > ret) {
		USB_LOG("FAIL: load_normal_popup(): %d", ret);
		popup_terminate();
	}

	__USB_FUNC_EXIT__ ;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	__USB_FUNC_ENTER__ ;
	assert(data);
	char *text = (char *)data;
	__USB_FUNC_EXIT__ ;
	return strdup(text);
}

static void select_app_popup_cancel_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	__USB_FUNC_ENTER__ ;
	assert(data);
	struct appdata *ad = (struct appdata *)data;
	unload_popup(ad);
	popup_terminate();
	__USB_FUNC_EXIT__ ;
}

static int launch_app_for_device(struct appdata *ad, char *appId)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);
	assert(appId);
	int ret;
	char answer[SOCK_STR_LEN];

	switch (ad->type) {
	case SELECT_PKG_FOR_ACC_POPUP:
		ret = request_to_usb_server(LAUNCH_APP_FOR_ACC, appId,
							answer, sizeof(answer));
		break;
	case SELECT_PKG_FOR_HOST_POPUP:
		ret = request_to_usb_server(LAUNCH_APP_FOR_HOST, appId,
							answer, sizeof(answer));
		break;
	default:
		ret = -1;
		break;
	}
	if (0 != ret) {
		USB_LOG("FAIL: request_to_usb_server(LAUNCH_APP, ad->selPkg, answer)");
		return -1;
	}

	USB_LOG("Launching app result is %s", answer);
	__USB_FUNC_EXIT__ ;
	return 0;
}

static void select_app_popup_gl_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	__USB_FUNC_ENTER__ ;
	assert(event_info);
	assert(data);
	struct appdata *ad = (struct appdata *)data;
	char *appId;
	int ret;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (!item) {
		USB_LOG("ERROR: item is NULL");
		unload_popup(ad);
		return;
	}

	appId = (char *)elm_object_item_data_get(item);
	USB_LOG("Selected Item: %s", appId);

	unload_popup(ad);

	ret = launch_app_for_device(ad, appId);
	if (0 != ret) {
		USB_LOG("FAIL: send_sel_pkg_to_usb_server(ad)");
	}

	popup_terminate();

	__USB_FUNC_EXIT__ ;
}

static void load_popup_to_select_app(struct appdata *ad)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);
	int ret;
	char *text;
	GList *l;
	Evas_Object *btn;
	Evas_Object *win;
	Evas_Object *genlist;
	Elm_Object_Item *item;

	/* create window */
	win = __create_win(PACKAGE);
	if (win == NULL)
		return ;
	ad->win = win;

	ret = syspopup_create(ad->b, &(ad->handler), ad->win, ad);
	if (ret != 0) {
		USB_LOG("FAIL: syspopup_create(): %d", ret);
		return ;
	}

	evas_object_show(ad->win);
	ad->popup = elm_popup_add(ad->win);
	elm_object_style_set(ad->popup,"menustyle");
	elm_object_part_text_set(ad->popup, "title,text", "Select app to launch");
	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	btn = elm_button_add(ad->popup);
	elm_object_text_set(btn, "Cancel");
	elm_object_part_content_set(ad->popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", select_app_popup_cancel_response_cb, ad);

	ad->itc.item_style = "1text";
	ad->itc.func.text_get = _gl_text_get;
	ad->itc.func.content_get = NULL;
	ad->itc.func.state_get = NULL;
	ad->itc.func.del = NULL;
	genlist = elm_genlist_add(ad->popup);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	for (l = ad->mApps; l; l = g_list_next(l)) {
		text = (char *)(l->data);
		item = elm_genlist_item_append(genlist, &(ad->itc), (void *) text, NULL,
			ELM_GENLIST_ITEM_NONE, select_app_popup_gl_select_cb, ad);
		if (!item) {
			USB_LOG("ERROR: item == NULL");
		}
	}

	elm_object_content_set(ad->popup, genlist);
	evas_object_show(ad->popup);

	__USB_FUNC_EXIT__ ;
}

static void load_popup_to_confirm_uri(struct appdata *ad)
{
	__USB_FUNC_ENTER__ ;
	/* TODO
	 * After using Manifest for Accessory and Host,
	 * this function can be implemented*/
	__USB_FUNC_EXIT__ ;
}

static int get_accessory_info(struct appdata *ad)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);
	assert(ad->usbAcc);

	ad->usbAcc->manufacturer = strdup(bundle_get_val(ad->b, ACC_MANUFACTURER));
	if (!(ad->usbAcc->manufacturer))
		goto out_manufacturer;

	ad->usbAcc->model = strdup(bundle_get_val(ad->b, ACC_MODEL));
	if (!(ad->usbAcc->model))
		goto out_model;

	ad->usbAcc->description = strdup(bundle_get_val(ad->b, ACC_DESCRIPTION));
	if (!(ad->usbAcc->description))
		goto out_description;

	ad->usbAcc->version = strdup(bundle_get_val(ad->b, ACC_VERSION));
	if (!(ad->usbAcc->version))
		goto out_version;

	ad->usbAcc->uri = strdup(bundle_get_val(ad->b, ACC_URI));
	if (!(ad->usbAcc->uri))
		goto out_uri;

	ad->usbAcc->serial = strdup(bundle_get_val(ad->b, ACC_SERIAL));
	if (!(ad->usbAcc->serial))
		goto out_serial;

	__USB_FUNC_EXIT__ ;
	return 0;

out_serial:
	FREE(ad->usbAcc->serial);
out_uri:
	FREE(ad->usbAcc->uri);
out_version:
	FREE(ad->usbAcc->version);
out_description:
	FREE(ad->usbAcc->description);
out_model:
	FREE(ad->usbAcc->model);
out_manufacturer:
	FREE(ad->usbAcc->manufacturer);

	__USB_FUNC_EXIT__ ;
	return -1;
}

static int get_host_info(struct appdata *ad)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);
	assert(ad->usbHost);

	char *text;

	text = (char *)bundle_get_val(ad->b, HOST_CLASS);
	if (!text) return -1;
	ad->usbHost->devClass = atoi(text);

	text = (char *)bundle_get_val(ad->b, HOST_SUBCLASS);
	if (!text) return -1;
	ad->usbHost->devSubClass = atoi(text);

	text = (char *)bundle_get_val(ad->b, HOST_PROTOCOL);
	if (!text) return -1;
	ad->usbHost->devProtocol = atoi(text);

	text = (char *)bundle_get_val(ad->b, HOST_IDVENDOR);
	if (!text) return -1;
	ad->usbHost->idVendor = atoi(text);

	text = (char *)bundle_get_val(ad->b, HOST_IDPRODUCT);
	if (!text) return -1;
	ad->usbHost->idProduct = atoi(text);

	__USB_FUNC_EXIT__ ;
	return 0;
}

static int get_accessory_matched(struct appdata* ad)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);
	if (ad->mApps) {
		g_list_free(ad->mApps);
		ad->mApps = NULL;
	}

	/* TODO ************************************************
	 * Find apps which is matched with the connected device
	 * and store the apps to the list (ad->mApps) */
	ad->mApps = g_list_append(ad->mApps, "acc_test");
	/*******************************************************/

	__USB_FUNC_EXIT__ ;
	return g_list_length(ad->mApps);
}

static void load_select_pkg_for_acc_popup(struct appdata *ad)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);
	int ret;
	int numOfApps;

	ad->usbAcc = (struct UsbAccessory *)malloc(sizeof(struct UsbAccessory));

	ret = get_accessory_info(ad);
	if (0 > ret) {
		USB_LOG("FAIL: get_accessory_info(ad)");
		popup_terminate();
		return ;
	}

	numOfApps = get_accessory_matched(ad);
	USB_LOG("number of apps matched: %d", numOfApps);
	if (numOfApps > 0) {
		load_popup_to_select_app(ad);
	} else {
		load_popup_to_confirm_uri(ad);
	}
	__USB_FUNC_EXIT__ ;
}

static int get_host_matched(struct appdata* ad)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);
	if (ad->mApps) {
		g_list_free(ad->mApps);
		ad->mApps = NULL;
	}

	/* TODO ************************************************
	 * Find apps which is matched with the connected device
	 * and store the apps to the list (ad->mApps) */
	ad->mApps = g_list_append(ad->mApps, "host_test");
	/*******************************************************/

	__USB_FUNC_EXIT__ ;
	return g_list_length(ad->mApps);
}

static void load_select_pkg_for_host_popup(struct appdata *ad)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);
	int ret;
	int numOfApps;

	ad->usbHost = (struct UsbHost *)malloc(sizeof(struct UsbHost));

	ret = get_host_info(ad);
	if (0 > ret) {
		USB_LOG("FAIL: get_host_info(ad)");
		popup_terminate();
		return ;
	}

	numOfApps = get_host_matched(ad);
	USB_LOG("number of apps matched: %d\n", numOfApps);
	if (numOfApps > 0) {
		load_popup_to_select_app(ad);
	} else {
		load_popup_to_confirm_uri(ad);
	}
	__USB_FUNC_EXIT__ ;
}

void load_request_perm_popup(struct appdata *ad)
{
	__USB_FUNC_ENTER__ ;
	assert(ad);
	int ret;
	char *text;

	switch (ad->type) {
	case REQ_ACC_PERM_POPUP:
		text = gettext("IDS_COM_POP_ALLOW_APPLICATION_P1SS_TO_ACCESS_USB_ACCESSORY_Q_ABB");
		break;
	case REQ_HOST_PERM_POPUP:
		text = "Allow application to access USB host?";
		break;
	default:
		USB_LOG("ERROR: ad->type is improper");
		return;
	}
	USB_LOG("Main text is (%s)\n", text);

	ret = load_normal_popup(ad,
				text,
				dgettext("sys_string","IDS_COM_SK_YES"),
				request_perm_popup_yes_response_cb,
				dgettext("sys_string","IDS_COM_SK_NO"),
				request_perm_popup_no_response_cb);
	if (0 > ret) {
		USB_LOG("FAIL: load_normal_popup(): %d", ret);
		popup_terminate();
	}

	__USB_FUNC_EXIT__ ;
}

static int __app_reset(bundle *b, void *data)
{
	__USB_FUNC_ENTER__ ;
	assert(data);
	struct appdata *ad = data;
	char *type;

	ad->b = bundle_dup(b);

	/* When syspopup is already loaded, remove the popup and load new popup */
	if (syspopup_has_popup(b)) {
		USB_LOG("usb-syspopup is already loaded");
		unload_popup(ad);
		/* Resetting all proporties of syspopup */
		syspopup_reset(b);
	}

	type = (char *)bundle_get_val(b, SYSPOPUP_TYPE);
	if (!type) {
		USB_LOG("ERROR: Non existing type of popup\n");
		popup_terminate();
		return -1;
	}

	ad->type = atoi(type);
	USB_LOG("ad->type is (%d)\n", ad->type);

	/* In case that USB cable/device is disconnected before launching popup,
	 * the connection status are checked according to the popup type */
	switch(ad->type) {
	case ERROR_POPUP:
	case SELECT_PKG_FOR_ACC_POPUP:
	case REQ_ACC_PERM_POPUP:
		ad->isClientOrHost = USB_DEVICE_CLIENT;
		usp_usbclient_chgdet_cb(NULL, NULL);
		break;
	case SELECT_PKG_FOR_HOST_POPUP:
	case REQ_HOST_PERM_POPUP:
		ad->isClientOrHost = USB_DEVICE_HOST;
		usp_usbhost_chgdet_cb(NULL, NULL);
		break;
	default:
		USB_LOG("ERROR: The popup type(%d) does not exist\n", ad->type);
		ad->isClientOrHost = USB_DEVICE_UNKNOWN;
		popup_terminate();
		return -1;
	}

	switch(ad->type) {
	case ERROR_POPUP:
		USB_LOG("Connection failed popup is loaded");
		load_connection_failed_popup(ad);
		break;
	case SELECT_PKG_FOR_ACC_POPUP:
		USB_LOG("Select pkg for acc popup is loaded");
		load_select_pkg_for_acc_popup(ad);
		break;
	case SELECT_PKG_FOR_HOST_POPUP:
		USB_LOG("Select pkg for host popup is loaded");
		load_select_pkg_for_host_popup(ad);
		break;
	case REQ_ACC_PERM_POPUP:
	case REQ_HOST_PERM_POPUP:
		USB_LOG("Request Permission popup is loaded");
		load_request_perm_popup(ad);
		break;
	default:
		USB_LOG("ERROR: The popup type(%d) does not exist", ad->type);
		return -1;
	}

	__USB_FUNC_EXIT__ ;
	return 0;
}

int main(int argc, char *argv[])
{
	__USB_FUNC_ENTER__ ;

	struct appdata ad;
	struct appcore_ops ops = {
		.create = __app_create,
		.terminate = __app_terminate,
		.pause = __app_pause,
		.resume = __app_resume,
		.reset = __app_reset,
	};

	memset(&ad, 0x0, sizeof(struct appdata));

	ops.data = &ad;

	__USB_FUNC_EXIT__ ;
	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
