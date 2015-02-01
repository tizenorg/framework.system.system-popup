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

#include "popup-ui.h"

int load_normal_popup(const struct popup_ops *ops)
{
	Evas_Object *lbtn;
	Evas_Object *rbtn;
	Evas_Object *popup;
	Evas_Object *label;
	Evas_Object *scroller;
	Evas_Object *win;
	char *text;
	struct object_ops *obj;
	int ret;

	if (!ops)
		return -EINVAL;

	ret = get_object_by_ops(ops, &obj);
	if (ret < 0) {
		_E("Failed to get object (%d)", ret);
		return -EINVAL;
	}

	win = get_window();
	if (!win)
		return -ENOMEM;

	evas_object_show(win);

	popup = elm_popup_add(win);
	if (!popup)
		return -ENOMEM;
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (ops->title)
		elm_object_part_text_set(popup, "title,text", _(ops->title));

	scroller = elm_scroller_add(popup);
	if (!scroller)
		return -ENOMEM;
	elm_object_style_set(scroller, "effect");
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(popup, scroller);
	evas_object_show(scroller);

	label = elm_label_add(scroller);
	if (!label)
		return -ENOMEM;
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);

	if (ops->content) {
		text = elm_entry_utf8_to_markup(_(ops->content));
		if (!text)
			return -ENOMEM;
		elm_object_text_set(label, text);
		free(text);
	}

	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(scroller, label);
	evas_object_show(label);

	if (ops->left_text) {
		/* Left button */
		lbtn = elm_button_add(popup);
		if (lbtn) {
			elm_object_text_set(lbtn, _(ops->left_text));
			elm_object_style_set(lbtn, "popup");
			elm_object_part_content_set(popup, "button1", lbtn);
			evas_object_smart_callback_add(lbtn, "clicked", left_clicked, ops);
		}
	}

	if (ops->right_text) {
		/* Right button */
		rbtn = elm_button_add(popup);
		if (rbtn) {
			elm_object_text_set(rbtn, _(ops->right_text));
			elm_object_style_set(rbtn, "popup");
			elm_object_part_content_set(popup, "button2", rbtn);
			evas_object_smart_callback_add(rbtn, "clicked", right_clicked, ops);
		}
	}

	evas_object_show(popup);

	obj->popup = popup;

	return 0;
}
