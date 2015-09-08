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
#include <dd-deviced.h>
#include <dd-display.h>
#include <dd-led.h>
#include <aul.h>
#include "accessibility.h"
#include "launcher.h"

#include <Ecore_X.h>
#include <Ecore_Input.h>
#include <utilX.h>

#define TTS_ID        10

static char *tts_content[] = {
	"IDS_TPLATFORM_OPT_DISABLE_SCREEN_READER_ABB",
	"IDS_TPLATFORM_OPT_ENABLE_SCREEN_READER_ABB",
};

static char *tts_servant_method[] = {
	"TtsDisabled",
	"TtsEnabled",
};

static int tts_state;

static int play_tts_by_servant(void)
{
	return dbus_method_sync(BUS_NAME,
			POPUP_PATH_SERVANT,
			POPUP_IFACE_SERVANT,
			tts_servant_method[tts_state],
			NULL, NULL);
}

static void run_tts(void)
{
	if (vconf_set_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, tts_state) != 0)
		_E("Failed to set tts");
}

static void response_tts_clicked(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	int ret;

	if (!ad)
		return;

	remove_accessibility_popup();
	unregister_acc_option_full();

	run_tts();

	ret = play_tts_by_servant();
	if (ret < 0)
		_E("Failed to play tts (%d)", ret);

	terminate_if_no_popup();
}

static int get_tts_id(void)
{
	return TTS_ID;
}

static int get_tts_content(char *content, int size)
{
	if (!content || size <= 0)
		return -EINVAL;

	if (elm_config_access_get() == EINA_TRUE)
		tts_state = 0;
	else
		tts_state = 1;

	snprintf(content, size, "%s", tts_content[tts_state]);
	return 0;
}

static const struct acc_option tts_ops = {
	.name                = "tts",
	.get_id              = get_tts_id,
	.get_icon            = NULL,
	.get_content         = get_tts_content,
	.response_clicked    = response_tts_clicked,
	.register_handlers   = NULL,
	.unregister_handlers = NULL
};

/* Constructor to register tts item */
static __attribute__ ((constructor)) void register_acc_option_tts(void)
{
	register_acc_option(&tts_ops);
}
