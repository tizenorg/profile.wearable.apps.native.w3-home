/*
 * Samsung API
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Elementary.h>
#include <Evas.h>
#include <stdbool.h>
#include <vconf.h>
#include <bundle.h>
#include <aul.h>
#include <efl_assist.h>
#include <efl_extension.h>
#include <dlog.h>

#include "conf.h"
#include "layout.h"
#include "log.h"
#include "util.h"
#include "main.h"
#include "key.h"
#include "effect.h"
#include "page_indicator.h"
//#include "util_time.h"
#include "index.h"

#define PAGE_INDICATOR_MAX 31
#define PAGE_INDICATOR_EDJ EDJEDIR"/page_indicator.edj"
#define PRIVATE_DATA_KEY_PAGE_INDICATOR_SELECTED_INDEX "pd_p_i_s_i"
#define PRIVATE_DATA_KEY_PAGE_INDICATOR_ITEM_ID "pd_p_i_i_id"

static const char g_slot_name[PAGE_INDICATOR_MAX][32] = {
	"item_0",

	"item_1", /*notification start */
	"item_2",
	"item_3",
	"item_4",
	"item_5",
	"item_6",
	"item_7",
	"item_8",
	"item_9",
	"item_10",
	"item_11",
	"item_12",
	"item_13",
	"item_14",
	"item_15",

	"item_16", /* widget start */
	"item_17",
	"item_18",
	"item_19",
	"item_20",
	"item_21",
	"item_22",
	"item_23",
	"item_24",
	"item_25",
	"item_26",
	"item_27",
	"item_28",
	"item_29",
	"item_30",
};



/*
 * Item
 */
inline static void _item_activate(Evas_Object *item)
{
	elm_object_signal_emit(item, "elm,state,active", "elm");
}

inline static void _item_inactivate(Evas_Object *item)
{
	elm_object_signal_emit(item, "elm,state,inactive", "elm");
}

inline static void _item_id_set(Evas_Object *item, int id)
{
	evas_object_data_set(item, PRIVATE_DATA_KEY_PAGE_INDICATOR_ITEM_ID, (void *)id);
}

inline static int _item_id_get(Evas_Object *item)
{
	return (int)evas_object_data_get(item, PRIVATE_DATA_KEY_PAGE_INDICATOR_ITEM_ID);
}

#define INDEX_ICON_W 19
#define INDEX_ICON_H 19
#define INDEX_ICON_PAGE IMAGEDIR"/home_indicator_horizontal_dot.png"
#define INDEX_ICON_CLOCK IMAGEDIR"/home_indicator_clock_02.png"
#define INDEX_ICON_MORE IMAGEDIR"/home_indicator_clock_02.png"
static Evas_Object *_item_icon_create(Evas_Object *parent, int type)
{
	Evas_Object *icon = elm_image_add(parent);
	retv_if(icon == NULL, NULL);

	const char *file = NULL;
	if (type == 1) {
		file = INDEX_ICON_CLOCK;
	} else if (type == 2) {
		file = INDEX_ICON_MORE;
	} else {
		file = INDEX_ICON_PAGE;
	}
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	if (elm_image_file_set(icon, file, NULL) == EINA_FALSE) {
		_E("Failed to set image file:%s", file);
	}
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_resize(icon, INDEX_ICON_W, INDEX_ICON_H);
	evas_object_show(icon);

	return icon;
}

HAPI void page_indicator_clock_update(Evas_Object *indicator, char *time_str)
{
	/*ret_if(indicator == NULL);
	ret_if(time_str == NULL);

	Evas_Object *item = page_indicator_item_at(indicator, PAGE_INDICATOR_ID_PIVOT);
	if (item) {
		elm_object_part_text_set(item, "elm.text", time_str);
	}*/
}

HAPI Evas_Object *page_indicator_item_create(Evas_Object *parent, int layout_type)
{
	retv_if(parent == NULL, NULL);

	Eina_Bool ret = EINA_TRUE;
	retv_if(parent == NULL, NULL);

	Evas_Object *layout = elm_layout_add(parent);
	retv_if(layout == NULL, NULL);

	Evas_Object *icon = NULL;

	switch (layout_type) {
	case PAGE_INDICATOR_ITEM_TYPE_PAGE:
		ret = elm_layout_file_set(layout, PAGE_INDICATOR_EDJ, "page_item/icon");
		retv_if(ret == EINA_FALSE, NULL);

		icon = _item_icon_create(parent, 3);
		if (icon) {
			elm_object_part_content_set(layout, "elm.swallow.icon", icon);
		}
	break;

	case PAGE_INDICATOR_ITEM_TYPE_MORE:
		ret = elm_layout_file_set(layout, PAGE_INDICATOR_EDJ, "page_item/icon");
		retv_if(ret == EINA_FALSE, NULL);

		icon = _item_icon_create(parent, 2);
		if (icon) {
			elm_object_part_content_set(layout, "elm.swallow.icon", icon);
		}
	break;

	case PAGE_INDICATOR_ITEM_TYPE_CLOCK:
		ret = elm_layout_file_set(layout, PAGE_INDICATOR_EDJ, "page_item/text");
		retv_if(ret == EINA_FALSE, NULL);

		/*char *time_str = util_time_get_time();
		if (time_str) {
			elm_object_part_text_set(layout, "elm.text", time_str);
			free(time_str);
		}*/
	break;
	}

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);

	return layout;
}



/*
 * Page Indicator
 */
inline static void _selected_index_set(Evas_Object *indicator, int index)
{
	evas_object_data_set(indicator, PRIVATE_DATA_KEY_PAGE_INDICATOR_SELECTED_INDEX, (void *)index);
}

inline static int _selected_index_get(Evas_Object *indicator)
{
	return (int)evas_object_data_get(indicator, PRIVATE_DATA_KEY_PAGE_INDICATOR_SELECTED_INDEX);
}

static int _item_index_find(Evas_Object *indicator, Evas_Object *item)
{
	retv_if(indicator == NULL, -1);

	int i;
	for (i = 0; i < PAGE_INDICATOR_MAX; i++) {
		Evas_Object *temp = elm_object_part_content_get(indicator, g_slot_name[i]);
		if (temp == item) {
			return i;
		}
	}

	return -1;
}

HAPI Evas_Object *page_indicator_item_find(Evas_Object *indicator, int id)
{
	retv_if(indicator == NULL, NULL);

	int i;
	for (i = 0; i < PAGE_INDICATOR_MAX; i++) {
		Evas_Object *item = elm_object_part_content_get(indicator, g_slot_name[i]);
		if (item) {
			if (id == _item_id_get(item)) {
				return item;
			}
		}
	}

	return NULL;
}

HAPI Evas_Object *page_indicator_item_at(Evas_Object *indicator, int index)
{
	_D("index:%d", index);
	retv_if(indicator == NULL, NULL);
	retv_if(index < 0 ||  index >= PAGE_INDICATOR_MAX, NULL);

	return elm_object_part_content_get(indicator, g_slot_name[index]);
}

HAPI int page_indicator_item_append(Evas_Object *indicator, Evas_Object *item, int type)
{
	retv_if(indicator == NULL, -1);
	retv_if(item == NULL, -1);

	int i, index = -1;

	if (type == PAGE_INDICATOR_ID_PIVOT) {
		index = PAGE_INDICATOR_ID_PIVOT;
	}
	else if (type >= PAGE_INDICATOR_ID_BASE_RIGHT) {
		for (i = PAGE_INDICATOR_ID_BASE_RIGHT; i < PAGE_INDICATOR_MAX; i++) {
			if (elm_object_part_content_get(indicator, g_slot_name[i]) == NULL) {
				index = i;
				break;
			}
		}
	}
	else if (type >= PAGE_INDICATOR_ID_BASE_LEFT) {
		for (i = PAGE_INDICATOR_ID_BASE_LEFT; i < PAGE_INDICATOR_ID_BASE_RIGHT; i++) {
			if (elm_object_part_content_get(indicator, g_slot_name[i]) == NULL) {
				index = i;
				break;
			}
		}
	}

	if (index >= 0) {
		_item_id_set(item, index);
		elm_object_part_content_set(indicator, g_slot_name[index], item);
	} else {
		_E("index direction:%d is fulled", type);
	}

	return index;
}

HAPI void page_indicator_item_clear(Evas_Object *indicator)
{
	ret_if(indicator == NULL);

	int i;
	for (i = 0; i < PAGE_INDICATOR_MAX; i++) {
		Evas_Object *item = elm_object_part_content_unset(indicator, g_slot_name[i]);
		if (item) {
			evas_object_del(item);
		}
	}
	_selected_index_set(indicator, -1);
}

HAPI void page_indicator_level_go(Evas_Object *indicator, int index)
{
	ret_if(indicator == NULL);
	ret_if(index < 0 ||  index >= PAGE_INDICATOR_MAX);
	int selected_index = _selected_index_get(indicator);
	if (index == selected_index) {
		return;
	}

	Evas_Object *temp = page_indicator_item_at(indicator, selected_index);
	if (temp) {
		_item_inactivate(temp);
	}

	Evas_Object *item = page_indicator_item_at(indicator, index);
	if (item) {
		_item_activate(item);
	}

	_selected_index_set(indicator, index);
#ifdef HOME_TBD
	page_indicator_focus_activate(indicator, index);
#endif

	if (index > 0) {
		elm_object_signal_emit(indicator, "show", "focus,icon");
	} else {
		elm_object_signal_emit(indicator, "hide", "focus,icon");
	}
	edje_object_message_signal_process(_EDJ(indicator));
}

HAPI void page_indicator_item_activate(Evas_Object *indicator, Evas_Object *item)
{
	ret_if(indicator == NULL);
	ret_if(item == NULL);

	int index = _item_index_find(indicator, item);
	int selected_index = _selected_index_get(indicator);

	if (index == selected_index) {
		return;
	}

	Evas_Object *temp = page_indicator_item_at(indicator, selected_index);
	if (temp) {
		_item_inactivate(temp);
	}
	_item_activate(item);

	_selected_index_set(indicator, index);
#ifdef HOME_TBD
	page_indicator_focus_activate(indicator, index);
#endif

	if (index > 0) {
		elm_object_signal_emit(indicator, "show", "focus,icon");
	} else {
		elm_object_signal_emit(indicator, "hide", "focus,icon");
	}
	edje_object_message_signal_process(_EDJ(indicator));
}

HAPI Evas_Object *page_indicator_add(Evas_Object *parent)
{
	Eina_Bool ret = EINA_TRUE;
	retv_if(parent == NULL, NULL);

	Evas_Object *layout = elm_layout_add(parent);
	retv_if(layout == NULL, NULL);

	ret = elm_layout_file_set(layout, PAGE_INDICATOR_EDJ, "page_indicator");
	retv_if(ret == EINA_FALSE, NULL);

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);

	_selected_index_set(layout, -1);

#ifdef HOME_TBD
	Evas_Object *focus = page_indicator_focus_add();
	elm_object_part_content_set(layout, "focus,icon", focus);
#endif

	return layout;
}

HAPI void page_indicator_del(Evas_Object *indicator)
{
	ret_if(indicator == NULL);

	evas_object_del(indicator);
}

HAPI void page_indicator_end(Evas_Object *indicator, int input_type, int direction)
{
	ret_if(indicator == NULL);

	int selected_index = _selected_index_get(indicator);

	if (input_type == INDEX_INPUT_ROTARY) {
		//page_indicator_focus_end_tick(indicator, selected_index, direction);
	} else {
		//page_indicator_focus_end_drag(indicator, selected_index, direction);
	}
}
