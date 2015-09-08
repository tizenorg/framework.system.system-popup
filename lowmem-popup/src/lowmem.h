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


#ifndef __DEF_lowmem_H_
#define __DEF_lowmem_H_

#include <Elementary.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef PREFIX
#define PREFIX "/usr"
#endif /* PREFIX */

#define TEMP_DIR		"/tmp"
#define PACKAGE			"lowmem-popup"
#define APPNAME			"lowmem-popup"
#define MAIN_W			(480)
#define MAIN_H			(800)
#define EDJ_PATH		"/usr/apps/org.tizen.lowmem-syspopup/res/edje/lowmem"
#define EDJ_NAME		EDJ_PATH"/lowmem.edj"
#define EDJ_POPUP_NAME		EDJ_PATH"/lowmem_popup.edj"
#define GRP_MAIN		"main"
#define GRP_POPUP		"popup"
#define MAX_PROCESS_NAME	512
#define PROCESS_NAME_FILE	"/tmp/processname.txt"
#define BEAT

#endif				/* __DEF_lowmem_H__ */
