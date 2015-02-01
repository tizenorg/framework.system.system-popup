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


#ifndef __DEF_mmc_H_
#define __DEF_mmc_H_

#include <Elementary.h>
#include <stdbool.h>

#ifndef PREFIX
#define PREFIX "/usr"
#endif /* PREFIX */

#define TEMP_DIR			"/tmp"
#define PACKAGE				"mmc-popup"
#define APPNAME				"mmc-popup"
#define MAIN_W				(480)
#define MAIN_H				(800)
#define GRP_MAIN			"main"
#define GRP_POPUP			"popup"
#define NEW_INDI
#define APPLICATION_BG			1
#define INDICATOR_HEIGHT		(38)
#define PREDEF_CHECK_SMACK_MMC		"checksmackmmc"

#define BEAT

/* Acct profiling support */
#define  ACCT_PROF
#ifdef   ACCT_PROF
#include <sys/acct.h>
#endif /* ACCT_PROF */

void register_encryption_restricted_function(bool (*func)(void));

#endif				/* __DEF_mmc_H__ */
