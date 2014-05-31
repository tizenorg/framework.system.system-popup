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


#ifndef __DEF_poweroff_H_
#define __DEF_poweroff_H_

#include <Elementary.h>
#include <bundle.h>

#ifndef PREFIX
#define PREFIX "/usr"
#endif
#define TEMP_DIR "/tmp"
#define PACKAGE "poweroff-popup"
#define APPNAME "poweroff-popup"
#define ICON_DIR PREFIX"/apps/org.tizen.poweroff-syspopup/res/images"
#define BG_IMAGE TEMP_DIR"/01_popup_bg.jpg"
#define EDJ_PATH PREFIX"/apps/org.tizen.poweroff-syspopup/res/edje/poweroff"
#define EDJ_NAME EDJ_PATH"/poweroff.edj"
#define GRP_MAIN "main"
#define APPLICATION_BG	1
#define INDICATOR_HEIGHT (38)
#define NEW_INDI
#define ACCT_PROF
#define PREDEF_POWEROFF "poweroff"
#define PREDEF_ENTERSLEEP "entersleep"
#define MAIN_W	(480)
#define MAIN_H	(800)
#define BEAT

#define NAME_BUF_LEN	128
#define TITLE_BUF_LEN	128
#define CONTENT_BUF_LEN	256

/* Popup Response */
enum {
	POPUP_RESPONSE_YES = 0,
	POPUP_RESPONSE_NO,
	POPUP_RESPONSE_SLEEP
} response;

/* Text layout */
struct text_part {
	char *part;
	char *msgid;
};

/* Text part */
static struct text_part main_txt[] = {
	{"txt_title", N_("Poweroff"),},
	{"txt_mesg", N_(""),},
};

/* Main UI structure */
struct appdata {
	Evas *evas;
	Evas_Object *win_main;
	Evas_Object *layout_main;
	Evas_Object *popup;
	Evas_Object *popup_poweroff;
	Evas_Object *popup_access;
	Evas_Object *genlist;
	Evas_Object *genlist_access;

	Evas_Object *root_w;
	Evas_Object *root_h;
	Evas_Object *bg;
	Evas_Object *indicator;
	bundle *b;

	/* Added for syspopup */
	Evas_Object *title;
	Evas_Object *content;

	Elm_Genlist_Item_Class itc;
	Elm_Genlist_Item_Class itc1;

	double w_ratio;
	double h_ratio;

};

#endif				/* __DEF_poweroff_H__ */
