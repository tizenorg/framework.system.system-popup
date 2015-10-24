/*
 *  system-popup
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
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


#ifndef __POPUP_UI_H__
#define __POPUP_UI_H__

#include "popup-common.h"

void left_clicked(void *data, Evas_Object * obj, void *event_info);
void right_clicked(void *data, Evas_Object * obj, void *event_info);

int load_normal_popup(const struct popup_ops *ops);

#endif /* __POPU_UI_H__ */
