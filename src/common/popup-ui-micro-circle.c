/*
 *  system-popup
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#define BUF_MAX 512

int load_normal_popup(const struct popup_ops *ops)
{
	Evas_Object *lbtn;
	Evas_Object *licon;
	Evas_Object *rbtn;
	Evas_Object *ricon;
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *win;
	char content[BUF_MAX];
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
	if (!popup) {
		_E("popup is NULL. eext_circle_popup_layout_add failed");
		return -ENOMEM;
	}
	elm_object_style_set(popup, "circle");
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, ELM_NOTIFY_ALIGN_FILL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	if (ops->title)
		elm_object_part_text_set(layout, "elm.text.title", _(ops->title));

	if (ops->content)
		snprintf(content, sizeof(content), "%s", _(ops->content));
	else if (ops->get_content) {
		ret = ops->get_content(ops, content, sizeof(content));
		if (ret < 0) {
			_E("Failed to get popup content");
			return ret;
		}
	} else
		return -ENOENT;

	elm_object_content_set(popup, layout);

	if (ops->left_text && !ops->right_text) {
		elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons1");
		elm_object_part_text_set(layout, "elm.text", content);
		elm_object_content_set(popup, layout);

		if (ops->left_icon) {

			/* one button */
			lbtn = elm_button_add(popup);
			if (lbtn) {
				elm_object_text_set(lbtn, _(ops->left_text));
				elm_object_style_set(lbtn, "popup/circle");
				elm_object_part_content_set(popup, "button1", lbtn);
				evas_object_smart_callback_add(lbtn, "clicked", left_clicked, ops);
			}
			licon = elm_image_add(lbtn);
			if (licon) {
				elm_image_file_set(licon, EDJ_NAME, ops->left_icon);
				evas_object_size_hint_weight_set(licon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				elm_object_part_content_set(lbtn, "elm.swallow.content", licon);
				evas_object_show(licon);
			}

		}
	} else {
		elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons2");
		elm_object_part_text_set(layout, "elm.text", _(ops->content));
		elm_object_content_set(popup, layout);

		if (ops->left_text && ops->left_icon) {
			/* Left button */
			lbtn = elm_button_add(popup);
			if (lbtn) {
				elm_object_text_set(lbtn, _(ops->left_text));
				elm_object_style_set(lbtn, "popup/circle/left");
				elm_object_part_content_set(popup, "button1", lbtn);
				evas_object_smart_callback_add(lbtn, "clicked", left_clicked, ops);
			}
			licon = elm_image_add(lbtn);
			if (licon) {
				elm_image_file_set(licon, EDJ_NAME, ops->left_icon);
				evas_object_size_hint_weight_set(licon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				elm_object_part_content_set(lbtn, "elm.swallow.content", licon);
				evas_object_show(licon);
			}

		}

		if (ops->right_text && ops->right_icon) {
			/* Right button */
			rbtn = elm_button_add(popup);
			if (rbtn) {
				elm_object_text_set(rbtn, _(ops->right_text));
				elm_object_style_set(rbtn, "popup/circle/right");
				elm_object_part_content_set(popup, "button2", rbtn);
				evas_object_smart_callback_add(rbtn, "clicked", right_clicked, ops);
			}
			ricon = elm_image_add(rbtn);
			if (ricon) {
				elm_image_file_set(ricon, EDJ_NAME, ops->right_icon);
				evas_object_size_hint_weight_set(ricon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				elm_object_part_content_set(rbtn, "elm.swallow.content", ricon);
				evas_object_show(ricon);
			}

		}
	}

	evas_object_show(popup);

	obj->popup = popup;

	return 0;
}
