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


#ifndef __DEF_poweroff_H_
#define __DEF_poweroff_H_

#include <vconf.h>
#include "micro-common.h"

#define PACKAGE "poweroff-popup"
#define EDJ_PATH "/usr/apps/org.tizen.poweroff-syspopup/res/edje/poweroff"
#define EDJ_NAME EDJ_PATH"/poweroff.edj"

#define IMAGE_SIZE     (50*elm_config_scale_get())
#define IMAGE_MIN_SIZE (36*elm_config_scale_get())
#define BUF_MAX 256

struct device_option{
	const char *name;
	int (*get_id)(void);
	bool (*is_enabled)(void);
	int (*init_itc)(void);
	void (*deinit_itc)(void);
	int (*get_itc)(Elm_Genlist_Item_Class **itc);
	int (*store_item)(Elm_Object_Item *item);
	void (*response_clicked)(void *data, Evas_Object *obj, void *event_info);
	void (*register_handlers) (void *data);
	void (*unregister_handlers) (void *data);
};

void register_option(const struct device_option *opt);
void unregister_option(const struct device_option *opt);
void unregister_main_list_handlers(struct appdata *ad);

#endif				/* __DEF_poweroff_H__ */
