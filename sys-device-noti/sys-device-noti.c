/*
 *  sys-device-noti
 *
 * Copyright (c) 2010 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <stdlib.h>
#include <dd-display.h>
#include <notification.h>
#include <libintl.h>
#include <locale.h>
#include <vconf.h>
#include "sys-device-noti.h"
#include "common.h"

#define BATTERY_FULL_ICON_PATH				SYSTEM_ICONDIR"/batt_full_icon.png"
#define BATTERY_FULL_INDICATOR_ICON_PATH	SYSTEM_ICONDIR"/batt_full_indicator.png"

static void set_locale(void)
{
	char *lang, *r;

	lang = vconf_get_str(VCONFKEY_LANGSET);
	if (!lang)
		goto next;

	setenv("LANG", lang, 1);
	setenv("LC_MESSAGES", lang, 1);
	r = setlocale(LC_ALL, "");
	if (!r)
		setlocale(LC_ALL, lang);
	free(lang);

next:
	bindtextdomain(LANG_DOMAIN,LOCALE_DIR);
	textdomain(LANG_DOMAIN);
}

static void delete_noti(void)
{
	/* delete previous notification */
	notification_delete_all_by_type(NULL, NOTIFICATION_TYPE_ONGOING);
}

static int create_noti(void)
{
	notification_h noti;
	notification_error_e err;

	noti = notification_create(NOTIFICATION_TYPE_ONGOING);
	if (!noti)
		return -1;

	err = notification_set_text_domain(noti,
			LANG_DOMAIN, LOCALE_DIR);
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	err = notification_set_layout(noti,
			NOTIFICATION_LY_ONGOING_EVENT);
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	err = notification_set_text(noti,
			NOTIFICATION_TEXT_TYPE_TITLE,
			_("IDS_IDLE_POP_BATTERY_FULLY_CAHRGED"),
			"IDS_IDLE_POP_BATTERY_FULLY_CAHRGED",
			NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	err = notification_set_text(noti,
			NOTIFICATION_TEXT_TYPE_CONTENT,
			_("IDS_SYNCML_POP_DM_REMOVE_CHARGER"),
			"IDS_SYNCML_POP_DM_REMOVE_CHARGER",
			NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	err = notification_set_text(noti,
			NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF,
			_("IDS_SYNCML_POP_DM_REMOVE_CHARGER"),
			"IDS_SYNCML_POP_DM_REMOVE_CHARGER",
			NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	err = notification_set_image(noti,
			NOTIFICATION_IMAGE_TYPE_ICON,
			BATTERY_FULL_ICON_PATH);
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	err = notification_set_image(noti,
			NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
			BATTERY_FULL_INDICATOR_ICON_PATH);
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	err = notification_set_time(noti, time(NULL));
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	err = notification_set_property(noti,
			NOTIFICATION_PROP_DISABLE_APP_LAUNCH);
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	err = notification_set_display_applist(noti,
			NOTIFICATION_DISPLAY_APP_INDICATOR
			| NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	err = notification_insert(noti, NULL);
	if (err != NOTIFICATION_ERROR_NONE)
		goto exit;

	notification_free(noti);
	return 0;

exit:
	notification_free(noti);
	return -1;
}

int main(int argc, char *argv[])
{
	int bNoti = -1;
	cb_noti_type cb_type = -1;

	if (argc == 3)
		bNoti = atoi(argv[2]);

	cb_type = (cb_noti_type)atoi(argv[1]);

	/* set locale */
	set_locale();

	switch (cb_type) {
	case DEVICE_NOTI_BATT_CHARGE:
		play_feedback(PLAY_ALL, PATTERN_CHARGERCONN);
		break;
	case DEVICE_NOTI_BATT_FULL:
		delete_noti();
		if (bNoti) {
			create_noti();
			play_feedback(PLAY_ALL, PATTERN_FULLCHARGED);
		}
		break;
	default:
		break;
	}

	return 0;
}
