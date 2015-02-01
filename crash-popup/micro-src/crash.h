/*
 * CRASH-POPUP
 * Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CRASH_POPUP_H__
#define __CRASH_POPUP_H__

#include <dlog.h>

#undef LOG_TAG
#define LOG_TAG "CRASH_POPUP"

#undef _D
#undef _I
#undef _W
#undef _E
#undef _SD
#undef _SI
#undef _SW
#undef _SE

#define _D(fmt, arg...) LOGD(fmt, ##arg)
#define _I(fmt, arg...) LOGI(fmt, ##arg)
#define _W(fmt, arg...) LOGW(fmt, ##arg)
#define _E(fmt, arg...) LOGE(fmt, ##arg)
#define _SD(fmt, arg...) SECURE_LOGD(fmt, ##arg)
#define _SI(fmt, arg...) SECURE_LOGI(fmt, ##arg)
#define _SW(fmt, arg...) SECURE_LOGW(fmt, ##arg)
#define _SE(fmt, arg...) SECURE_LOGE(fmt, ##arg)

#endif
/* __CRASH_POPUP_H__ */
