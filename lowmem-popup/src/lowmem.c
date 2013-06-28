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
#include <devman_haptic.h>
#include <mmf/mm_sound.h>
#include <pmapi.h>
#include <sysman.h>
#include "lowmem.h"
#include <Ecore_X.h>
#include <utilX.h>

#define APPLICATION_BG		1
#define INDICATOR_HEIGHT	(38)	/* the case of 480*800 */
#define SOUND_PATH		PREFIX"/apps/org.tizen.lowmem-syspopup/res/keysound/02_Warning.wav"
#define NEW_INDI

#define ACCT_PROF
#ifdef ACCT_PROF
#include <sys/acct.h>
#endif /* ACCT_PROF */

static const char *process_name = NULL;

#include <syspopup.h>

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

struct text_part {
	char *part;
	char *msgid;
};

static struct text_part main_txt[] = {
	{"txt_title", N_("Low memory popup"),},
	{"txt_mesg", N_(""),},
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
static void win_del(void *data, Evas_Object *obj, void *event)
{
	popup_terminate();
}

/* Quit  */
static void main_quit_cb(void *data, Evas_Object *obj, const char *emission,
	     const char *source)
{
	popup_terminate();
}

/* Update text font */
static void update_ts(Evas_Object *eo, struct text_part *tp, int size)
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
	int ret = 0;

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
	} else {
		ret = syspopup_create(b, &handler, ad->win_main, ad);
		evas_object_show(ad->win_main);
		process_name = bundle_get_val(b, "_APP_NAME_"); 
		if (process_name == NULL)
			process_name = "unknown_app";

		/* Start Main UI */
		lowmem_start((void *)ad);
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
void lowmem_cleanup(struct appdata *ad)
{
	if (ad == NULL)
		return;

	if (ad->popup)
		evas_object_del(ad->popup);
	if (ad->layout_main)
		evas_object_del(ad->layout_main);
}

/* Background clicked noti */
void bg_clicked_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	system_print("\n system-popup : In BG Noti \n");
	fflush(stdout);
	popup_terminate();
}

void lowmem_clicked_cb(void *data, Evas * e, Evas_Object * obj,
		       void *event_info)
{
	system_print("\n system-popup : Screen clicked \n");
	fflush(stdout);
	popup_terminate();
}

/* Create indicator bar */
int lowmem_create_indicator(struct appdata *ad)
{
	elm_win_indicator_mode_set(ad->win_main, ELM_WIN_INDICATOR_HIDE);
	return 0;
}

/* Play vibration */
int lowmem_play_vibration()
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

void lowmem_timeout_func(void *data)
{
	system_print("\n System-popup : In Lowmem timeout\n");

	/* Cleanup */
	lowmem_cleanup(data);

	/* Now get lost */
	popup_terminate();
}

/* Basic popup widget */
int lowmem_create_and_show_basic_popup(struct appdata *ad)
{
	Evas_Object *btn1;

	/* Initialization */
	char *note = (char *)malloc(MAX_PROCESS_NAME * (sizeof(char)));
	char note_buf[MAX_PROCESS_NAME] = {0, };
	int ret_val = 0;

	if (!note) {
		system_print("\n System-popup : can not malloc \n");
		return -1;
	}
	system_print("\n System-popup : process name is %s \n", process_name);
	snprintf(note_buf, MAX_PROCESS_NAME, _("IDS_IDLE_POP_PS_CLOSED"), process_name);
	snprintf(note, MAX_PROCESS_NAME, "%s %s", _("IDS_COM_POP_NOT_ENOUGH_MEMORY"),note_buf);

	/* Add notify */
	/* No need to give main window, it will create internally */
	ad->popup = elm_popup_add(ad->win_main);
	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_object_style_set(ad->popup, "transparent");
	elm_popup_timeout_set(ad->layout_main, 3);
	elm_object_text_set(ad->popup, note);
	elm_object_part_text_set(ad->popup, "title,text", _("IDS_COM_BODY_SYSTEM_INFO_ABB"));

	btn1 = elm_button_add(ad->popup);
	elm_object_text_set(btn1, _("IDS_COM_SK_OK"));
	elm_object_part_content_set(ad->popup, "button1", btn1);
	elm_object_style_set(btn1, "popup_button/default");
	evas_object_smart_callback_add(btn1, "clicked", bg_clicked_cb, ad);

	Ecore_X_Window xwin;
	xwin = elm_win_xwindow_get(ad->popup);
	ecore_x_netwm_window_type_set(xwin, ECORE_X_WINDOW_TYPE_NOTIFICATION);
	evas_object_show(ad->popup);

	free(note);

	return 0;
}

int lowmem_start(void *data)
{
	struct appdata *ad = data;
	int ret_val = 0;

	/* Create and show popup */
	ret_val = lowmem_create_and_show_basic_popup(ad);
	if (ret_val != 0)
		return -1;

	/* Change LCD brightness */
	ret_val = pm_change_state(LCD_NORMAL);
	if (ret_val != 0)
		return -1;

	/* Play vibration */
	ret_val = lowmem_play_vibration();
	if (ret_val == -1)
		system_print("\n Lowmem : Play vibration failed \n");

	/* Play the sound alert */
	ret_val = mm_sound_play_keysound(SOUND_PATH, 1);
	if (ret_val != 0)
		system_print("\n Lowmem : Play vibration failed \n");

	return 0;
}

/* App init */
int app_create(void *data)
{
	Evas_Object *win;
	struct appdata *ad = data;

	/* create window */
	win = create_win(PACKAGE);
	if (win == NULL)
		return -1;

	ad->win_main = win;

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

	sysconf_set_mempolicy(OOM_IGNORE);

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
