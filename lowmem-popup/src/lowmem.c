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
#include <appcore-efl.h>
#include "lowmem.h"
#include <Ecore_X.h>
#include <utilX.h>
#include <syspopup.h>
#include <dd-display.h>
#include <dd-deviced.h>
#include <vconf.h>
#include "common.h"

#define LOWMEM_LEVEL_WARNING	"warning"
#define LOWMEM_LEVEL_CRITICAL	"critical"

static const char *process_name = NULL;
static const char *memnoti_level = NULL;
static const char *memnoti_size = NULL;

/* Background clicked noti */
void bg_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	_D("system-popup : In BG Noti ");
	fflush(stdout);
	popup_terminate();
}

void lowmem_clicked_cb(void *data, Evas * e, Evas_Object * obj,
		       void *event_info)
{
	_D("system-popup : Screen clicked ");
	fflush(stdout);
	popup_terminate();
}

/* Create indicator bar */
int lowmem_create_indicator(struct appdata *ad)
{
	elm_win_indicator_mode_set(ad->win_main, ELM_WIN_INDICATOR_HIDE);
	return 0;
}

void lowmem_timeout_func(void *data)
{
	_D(" System-popup : In Lowmem timeout");

	/* Cleanup */
	object_cleanup(data);

	/* Now get lost */
	popup_terminate();
}

static void get_lowmem_storage_popup_title_text(char **title, char **text)
{
	int ret;
	int operator;

	_I("lowmem noti is %s ", memnoti_level);

	ret = vconf_get_int(VCONFKEY_CSC_OPERATOR, &operator);
	if (ret != 0)
		operator = -1;

	if (!strncmp(memnoti_level, LOWMEM_LEVEL_WARNING, strlen(LOWMEM_LEVEL_WARNING))) {
		switch (operator) {
		case VCONFKEY_CSC_OPERATOR_DOCOMO:
			*title = _("IDS_PN_FREE_SPACE_DECREASES");
			*text = _("IDS_PN_FREE_SPACE_OF_THE_DEVICE_IS_LOW");
			break;
		default:
			*title = NULL;
			*text = _("IDS_DAV_BODY_LOW_MEMORY_LEFT_ORANGE");
			break;
		}

		return;
	}

	if (!strncmp(memnoti_level, LOWMEM_LEVEL_CRITICAL, strlen(LOWMEM_LEVEL_CRITICAL))) {
		switch (operator) {
		case VCONFKEY_CSC_OPERATOR_DOCOMO:
			*title = _("IDS_PN_BODY_THE_DEVICE_MEMORY_IS_FULL_JPN_DCM");
			*text = _("IDS_PN_BODY_NOT_ENOUGH_STORAGE_AVAILABLE_SOME_FUNCTIONS_"
					"AND_APPLICATIONS_MAY_NOT_BE_AVAILABLE_NOTI_MSG_JPN_DCM");
			break;
		default:
			*title = NULL;
			*text = _("IDS_COM_POP_NOT_ENOUGH_MEMORY");
			break;
		}

		return;
	}

	_E("Low memory level is unknown (%s)", memnoti_level);

	*title = NULL;
	*text = _("IDS_COM_POP_NOT_ENOUGH_MEMORY");
}

static int get_lowmem_process_popup_text(char **text)
{
	char *note;
	char note_buf[MAX_PROCESS_NAME] = {0, };
	char *subtext1;
	char *subtext2;
	int noteLen;

	subtext1 = _("IDS_IDLE_POP_PS_CLOSED");
	subtext2 = _("IDS_COM_POP_NOT_ENOUGH_MEMORY");
	snprintf(note_buf, sizeof(note_buf), subtext1, process_name);

	noteLen = strlen(note_buf) + strlen(subtext2) + 3; /* 3: one period, one space, and one '\0' */

	note = (char *)malloc(noteLen);
	if (!note) {
		_E("FAIL: malloc()");
		return -ENOMEM;
	}

	snprintf(note, noteLen, "%s. %s", subtext2, note_buf);
	_I("Popup content: %s", note);

	*text = strdup(note);
	FREE(note);

	return 0;
}


static int load_lowmem_storage_popup(struct appdata *ad)
{
	char *text = NULL;
	char *title = NULL;

	if (!ad)
		return -EINVAL;

	get_lowmem_storage_popup_title_text(&title, &text);

	evas_object_show(ad->win_main);

	ad->popup = load_normal_popup(ad,
			title,
			text,
			_("IDS_COM_SK_OK"),
			bg_clicked_cb,
			NULL, NULL);
	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

static int load_lowmem_process_popup(struct appdata *ad)
{
	char *text = NULL;
	int ret;

	if (!ad)
		return -EINVAL;

	ret = get_lowmem_process_popup_text(&text);
	if (ret < 0) {
		_E("FAIL: get_lowmem_process_popup_text()");
		return ret;
	}

	if (!text)
		return -ENOMEM;

	evas_object_show(ad->win_main);

	ad->popup = load_normal_popup(ad,
			_("IDS_IDLE_BODY_LOW_MEMORY"),
			text,
			_("IDS_COM_SK_OK"),
			bg_clicked_cb,
			NULL, NULL);

	FREE(text);

	if (!(ad->popup)) {
		_E("FAIL: load_normal_popup()");
		return -ENOMEM;
	}

	return 0;
}

/* App init */
int app_create(void *data)
{
	Evas_Object *win;
	struct appdata *ad = data;
	int ret;

	ad->handler.def_term_fn = NULL;
	ad->handler.def_timeout_fn = NULL;

	/* create window */
	win = create_win(PACKAGE);
	if (win == NULL)
		return -1;

	ad->win_main = win;

	ret = appcore_set_i18n(LANG_DOMAIN, LOCALE_DIR);
	if (ret != 0)
		_E("FAIL: appcore_set_i18n()");

	return 0;

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
	int ret;

	if (syspopup_has_popup(b)) {
		syspopup_reset(b);
		return 0;
	}

	if (syspopup_create(b, &(ad->handler), ad->win_main, ad) < 0) {
		_E("FAIL: syspopup_create()");
		ret = -ENOMEM;
		goto lowmem_reset_out;
	}

	memnoti_level = bundle_get_val(b, "_MEM_NOTI_");
	if (memnoti_level) {
		memnoti_size = bundle_get_val(b, "_MEM_SIZE_");
		ret = load_lowmem_storage_popup(ad);
		if (ret < 0)
			goto lowmem_reset_out;

		syspopup_reset_timeout(b, -1);

	} else {
		process_name = bundle_get_val(b, "_APP_NAME_");
		if (process_name == NULL)
			process_name = "unknown_app";
		ret = load_lowmem_process_popup(ad);
		if (ret < 0)
			goto lowmem_reset_out;
	}

	if (set_display_feedback(PATTERN_LOWMEM) < 0)
		_E("Failed to set display and feedback");

	return 0;

lowmem_reset_out:
	popup_terminate();
	return ret;
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

	deviced_conf_set_mempolicy(OOM_IGNORE);

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
