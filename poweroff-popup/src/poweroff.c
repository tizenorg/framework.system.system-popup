/*
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
#include <appcore-efl.h>
#include <sensor.h>
#include <devman.h>
#include <devman_haptic.h>
#include <pmapi.h>
#include <sysman.h>
#include "poweroff.h"

#include <Ecore_X.h>
#include <Ecore_Input.h>
#include <utilX.h>

/* Time profiling support */
#ifdef ACCT_PROF
#include <sys/acct.h>
#endif /* ACCT_PROF */

#include <syspopup.h>
#include <vconf.h>

int create_and_show_basic_popup_min(struct appdata *ad);
void poweroff_response_yes_cb_min(void *data, Evas_Object * obj, void *event_info);
void poweroff_response_no_cb_min(void *data, Evas_Object * obj, void *event_info);


int myterm(bundle *b, void *data)
{
	return 0;
}

int mytimeout(bundle *b, void *data)
{
	return 0;
}

syspopup_handler handler = {
	.def_term_fn = myterm,
	.def_timeout_fn = mytimeout
};

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

/* App Life cycle funtions */
static void win_del(void *data, Evas_Object * obj, void *event)
{
	popup_terminate();
}

/* Quit  */
static void main_quit_cb(void *data, Evas_Object * obj, const char *emission,
			 const char *source)
{
	popup_terminate();
}

/* Update text font */
static void update_ts(Evas_Object * eo, struct text_part *tp, int size)
{
	int i;

	if (eo == NULL || tp == NULL || size < 0)
		return;

	for (i = 0; i < size; i++) {
		if (tp[i].part && tp[i].msgid)
			edje_object_part_text_set(eo, tp[i].part,
						  _(tp[i].msgid));
	}
}

/* Language changed noti handler */
static int lang_changed(void *data)
{
	struct appdata *ad = data;

	if (ad->layout_main == NULL)
		return 0;

	update_ts(elm_layout_edje_get(ad->layout_main), main_txt,
		  sizeof(main_txt) / sizeof(main_txt[0]));
	return 0;
}

/* Create main window */
static Evas_Object *create_win(const char *name)
{
	Evas_Object *eo;
	int w, h;

	eo = elm_win_add(NULL, name, ELM_WIN_DIALOG_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		elm_win_borderless_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request", win_del, NULL);
		elm_win_alpha_set(eo, EINA_TRUE);
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &w,
					&h);
		evas_object_resize(eo, w, h);
	}

	return eo;
}

/* Read from EDJ file */
static Evas_Object *load_edj(Evas_Object * parent, const char *file,
			     const char *group)
{
	Evas_Object *eo;
	int r;

	eo = elm_layout_add(parent);
	if (eo) {
		r = elm_layout_file_set(eo, file, group);
		if (!r) {
			evas_object_del(eo);
			return NULL;
		}

		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND,
						 EVAS_HINT_EXPAND);
	}

	return eo;
}

/* Terminate noti handler */
static int app_terminate(void *data)
{
	struct appdata *ad = data;

	if (ad->layout_main)
		evas_object_del(ad->layout_main);

	if (ad->win_main)
		evas_object_del(ad->win_main);

	return 0;
}

/* Pause/background */
static int app_pause(void *data)
{
	return 0;
}

/* Resume */
static int app_resume(void *data)
{
	return 0;
}


/* Reset */
static int app_reset(bundle *b, void *data)
{
	struct appdata *ad = data;

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
	} else {
		syspopup_create(b, &handler, ad->win_main, ad);
		evas_object_show(ad->win_main);

		/* Start Main UI */
		poweroff_start((void *)ad);
	}

	return 0;
}

/* Customized print */
void system_print(const char *format, ...)
{
	/* Un-comment return to disable logs */

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

/* Cleanup objects to avoid mem-leak */
void poweroff_cleanup(struct appdata *ad)
{
	if (ad->popup)
		evas_object_del(ad->popup);
	if (ad->layout_main)
		evas_object_del(ad->layout_main);
}

/* Background clicked noti */
static void bg_clicked_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	system_print("\n system-popup : In BG Noti \n");
	fflush(stdout);
	popup_terminate();
}

static void poweroff_popup_direct_cancel(keynode_t *key, void *data)
{
	int val;
	struct appdata *ad = data;
	if ( vconf_get_int(VCONFKEY_PM_STATE, &val) == 0
	&& val == VCONFKEY_PM_STATE_LCDOFF ) {
		vconf_ignore_key_changed(VCONFKEY_PM_STATE,(void*)poweroff_popup_direct_cancel);
		if (ad != NULL)
			poweroff_cleanup(ad);
		popup_terminate();
	}
	else
		return;
}
void poweroff_response_yes_cb_min(void *data, Evas_Object * obj, void *event_info)
{
	static int bPowerOff = 0;
	if (1 == bPowerOff)
		return;
	bPowerOff = 1;
	system_print("System-popup : Switching off phone !! Bye Bye \n");
	vconf_ignore_key_changed(VCONFKEY_PM_STATE,(void*)poweroff_popup_direct_cancel);
	/* This will cleanup the memory */
	if (data != NULL)
		poweroff_cleanup(data);

	if (sysman_call_predef_action(PREDEF_POWEROFF, 0) == -1) {
		system_print("System-popup : failed to request poweroff to system_server \n");
		system("poweroff");
	}
}

void poweroff_response_no_cb_min(void *data, Evas_Object * obj, void *event_info)
{
	system_print("\nSystem-popup: Option is Wrong");
	vconf_ignore_key_changed(VCONFKEY_PM_STATE,(void*)poweroff_popup_direct_cancel);
	if(data != NULL)
		poweroff_cleanup(data);
	popup_terminate();
}

int create_and_show_basic_popup_min(struct appdata *ad)
{
	Evas_Object *btn1;
	Evas_Object *btn2;

	ad->popup_poweroff = elm_popup_add(ad->win_main);
	if (ad->popup_poweroff == NULL) {
		system_print("\n System-popup : Add popup failed \n");
		return -1;
	}

	evas_object_size_hint_weight_set(ad->popup_poweroff, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_style_set(ad->popup_poweroff, "transparent");
	elm_object_text_set(ad->popup_poweroff, _("IDS_ST_BODY_POWER_OFF"));
	elm_object_part_text_set(ad->popup_poweroff, "title,text", _("IDS_COM_BODY_SYSTEM_INFO_ABB"));

	btn1 = elm_button_add(ad->popup_poweroff);
	elm_object_text_set(btn1, _("IDS_COM_SK_OK"));
	elm_object_part_content_set(ad->popup_poweroff, "button1", btn1);
	elm_object_style_set (btn1,"popup_button/default");
	evas_object_smart_callback_add(btn1, "clicked", poweroff_response_yes_cb_min, ad);
	btn2 = elm_button_add(ad->popup_poweroff);
	elm_object_text_set(btn2, _("IDS_COM_SK_CANCEL"));
	elm_object_part_content_set(ad->popup_poweroff, "button2", btn2);
	elm_object_style_set (btn2,"popup_button/default");
	evas_object_smart_callback_add(btn2, "clicked", poweroff_response_no_cb_min, ad);

	Ecore_X_Window xwin;
	xwin = elm_win_xwindow_get(ad->popup_poweroff);
	ecore_x_netwm_window_type_set(xwin, ECORE_X_WINDOW_TYPE_NOTIFICATION);
	utilx_grab_key(ecore_x_display_get(), xwin, KEY_SELECT, SHARED_GRAB);
	evas_object_show(ad->popup_poweroff);
	vconf_notify_key_changed(VCONFKEY_PM_STATE, poweroff_popup_direct_cancel, ad);
	
	return 0;
	
}

static void bg_noti_cb(void *data)
{
	ui_bgimg_reload((Evas_Object *) data);
}

/* Create indicator bar */
static int poweroff_create_indicator(struct appdata *ad)
{

	elm_win_indicator_mode_set(ad->win_main, ELM_WIN_INDICATOR_HIDE);
	return 0;
}

/* Play vibration */
static int poweroff_play_vibration()
{

	int ret_val = 0;
	int dev_handle = 0;
	int mode = 0;

	/* Open the haptic device */
	dev_handle = device_haptic_open(DEV_IDX_0, mode);
	if (dev_handle < 0)
		return -1;

	/* Play a monotone pattern for 1s */
	ret_val = device_haptic_play_monotone(dev_handle, 1000);
	device_haptic_close(dev_handle);
	if (ret_val < 0)
		return -1;

	return 0;

}

/* Start UI */
int poweroff_start(void *data)
{
	struct appdata *ad = data;
	int ret_val = 0;

	/* Create and show popup */
	ret_val = create_and_show_basic_popup_min(ad);
	if (ret_val != 0)
		return -1;

	/* Change LCD brightness */
	ret_val = pm_change_state(LCD_NORMAL);
	if (ret_val != 0)
		return -1;

	/* Play a vibration for 1 sec */
	ret_val = poweroff_play_vibration();
	if (ret_val == -1)
		system_print("\n Poweroff : Play vibration Failed \n");

	return 0;
}

/* App init */
int app_create(void *data)
{

	Evas_Object *win;
	struct appdata *ad = data;

	/* Create window (Reqd for sys-popup) */
	win = create_win(PACKAGE);
	if (win == NULL)
		return -1;

	ad->win_main = win;

	elm_theme_overlay_add(NULL,EDJ_NAME); 

	return 0;
}

int main(int argc, char *argv[])
{
	struct appdata ad;

	/* App life cycle management */
	struct appcore_ops ops = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.reset = app_reset,
	};

	memset(&ad, 0x0, sizeof(struct appdata));
	ops.data = &ad;

	/* Go into loop */
	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
