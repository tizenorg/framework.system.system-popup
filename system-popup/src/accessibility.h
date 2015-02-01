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


#ifndef __ACCESSIBILITY_H_
#define __ACCESSIBILITY_H_

#include "core.h"
#include "common.h"

#define BUF_MAX 256

struct acc_option{
	const char *name;
	int (*get_id)(void);
	int (*get_icon)(char *icon, int size);
	int (*get_content)(char *content, int size);
	void (*response_clicked)(void *data, Evas_Object *obj, void *event_info);
	void (*register_handlers) (void *data);
	void (*unregister_handlers) (void *data);
};

void register_acc_option(const struct acc_option *opt);
void unregister_acc_option_full(void);
void remove_accessibility_popup(void);

#endif  /* __ACCESSIBILITY_H__ */
