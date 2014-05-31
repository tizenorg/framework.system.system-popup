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

#include <feedback.h>
#include <stdio.h>
#include <stdlib.h>
#include <libintl.h>
#include "sys-device-noti.h"


static void play_feedback(int pattern)
{
	feedback_initialize();
	feedback_play(pattern);
	feedback_deinitialize();
}

int main(int argc, char *argv[])
{
	int bNoti = -1;
	cb_noti_type cb_type = -1;

	if (argc == 3)
		bNoti = atoi(argv[2]);

	cb_type = (cb_noti_type)atoi(argv[1]);

	switch (cb_type) {
	case DEVICE_NOTI_BATT_CHARGE:
		play_feedback(FEEDBACK_PATTERN_CHARGERCONN);
		break;
	case DEVICE_NOTI_BATT_FULL:
		if (bNoti)
			play_feedback(FEEDBACK_PATTERN_FULLCHARGED);
		break;
	default:
		break;
	}

	return 0;
}
