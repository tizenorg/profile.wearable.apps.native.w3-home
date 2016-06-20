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
#include <efl_assist.h>
#include <efl_extension.h>
#include <bundle.h>
#include <dlog.h>

#include "conf.h"
#include "layout_info.h"
#include "log.h"
#include "util.h"
#include "main.h"
#include "page_info.h"
#include "scroller_info.h"
#include "index_info.h"
#include "index.h"
#include "scroller.h"



typedef struct {
	Evas_Object *page;
	int index;
} page_index_s;


#define INDEX_EVEN_ITEM_NUM 20
#define INDEX_ODD_ITEM_NUM 19
static const char *_item_style_get(int index, int even)
{
	static const char g_it_style_odd[INDEX_ODD_ITEM_NUM][20] = {
		"item/odd_1",
		"item/odd_2",
		"item/odd_3",
		"item/odd_4",
		"item/odd_5",
		"item/odd_6",
		"item/odd_7",
		"item/odd_8",
		"item/odd_9",
		"item/odd_10",
		"item/odd_11",
		"item/odd_12",
		"item/odd_13",
		"item/odd_14",
		"item/odd_15",
		"item/odd_16",
		"item/odd_17",
		"item/odd_18",
		"item/odd_19",
	};
	static const char g_it_style_even[INDEX_EVEN_ITEM_NUM][20] = {
		"item/even_1",
		"item/even_2",
		"item/even_3",
		"item/even_4",
		"item/even_5",
		"item/even_6",
		"item/even_7",
		"item/even_8",
		"item/even_9",
		"item/even_10",
		"item/even_11",
		"item/even_12",
		"item/even_13",
		"item/even_14",
		"item/even_15",
		"item/even_16",
		"item/even_17",
		"item/even_18",
		"item/even_19",
		"item/even_20",
	};

	index = (index < 0) ? 0 : index;
	if (even) {
		index = (index >= INDEX_EVEN_ITEM_NUM) ? INDEX_EVEN_ITEM_NUM - 1 : index;

		return g_it_style_even[index];
	} else {
		index = (index >= INDEX_ODD_ITEM_NUM) ? INDEX_ODD_ITEM_NUM - 1 : index;

		return g_it_style_odd[index];
	}
}

static void _bring_in_page(Evas_Object *index, Evas_Object *page)
{
	Elm_Object_Item *idx_it = NULL;
	const Eina_List *l = NULL;
	index_info_s *index_info = NULL;
	page_index_s *page_index = NULL;
	int idx = 0;
	int found = 0;

	ret_if(!index);
	ret_if(!page);

	index_info = evas_object_data_get(index, DATA_KEY_INDEX_INFO);
	ret_if(!index_info);
	ret_if(!index_info->page_index_list);

	EINA_LIST_FOREACH(index_info->page_index_list, l, page_index) {
		if (page_index->page == page) {
			idx = page_index->index;
			found = 1;
			break;
		}
	}

	if (!found) {
		_E("Cannot find a page(%p)", page);
		return;
	}

	idx_it = elm_index_item_find(index, (void *) idx);
	if (idx_it) {
		elm_index_item_selected_set(idx_it, EINA_TRUE);
	} else {
		_E("Critical, the index(%p) cannot find the page(%p:%d)", index, page, idx);
	}
}



static Evas_Object *_create(Evas_Object *layout, Evas_Object *scroller, page_direction_e direction)
{
	Evas_Object *index = NULL;
	scroller_info_s *scroller_info = NULL;
	index_info_s *index_info = NULL;

	retv_if(!layout, NULL);
	retv_if(!scroller, NULL);

	scroller_info = evas_object_data_get(scroller, DATA_KEY_SCROLLER_INFO);
	retv_if(!scroller_info, NULL);

	index = elm_index_add(layout);
	retv_if(!index, NULL);
	//elm_object_theme_set(index, main_get_info()->theme);
	elm_object_style_set(index, "circle");

	index_info = calloc(1, sizeof(index_info_s));
	if (!index_info) {
		_E("Cannot calloc for index_info");
		evas_object_del(index);
		return NULL;
	}
	evas_object_data_set(index, DATA_KEY_INDEX_INFO, index_info);

	evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(index, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_index_horizontal_set(index, EINA_TRUE);
	elm_index_autohide_disabled_set(index, EINA_TRUE);
	elm_index_level_go(index, 0);
	evas_object_show(index);

	index_info->direction = direction;
	index_info->layout = layout;
	index_info->scroller = scroller;

	return index;
}



static void _destroy(Evas_Object *index)
{
	index_info_s *index_info = NULL;
	page_index_s *page_index = NULL;

	ret_if(!index);

	index_info = evas_object_data_del(index, DATA_KEY_INDEX_INFO);
	ret_if(!index_info);

	if (index_info->page_index_list) {
		EINA_LIST_FREE(index_info->page_index_list, page_index) {
			free(page_index);
		}
		index_info->page_index_list = NULL;
	}

	free(index_info);
	elm_index_item_clear(index);
	evas_object_del(index);
}



#define MAX_INDEX_NUMBER 17
static void _update_left(Evas_Object *scroller, Evas_Object *index, const Eina_List *list)
{
	Evas_Object *page = NULL;
	Elm_Object_Item *idx_it = NULL;
	Eina_List *reverse_list = NULL;
	const Eina_List *l = NULL;

	index_info_s *index_info = NULL;
	page_info_s *page_info = NULL;
	page_index_s *page_index = NULL;
	page_direction_e before_direction = PAGE_DIRECTION_LEFT;

	int extra_idx = 0, page_index_inserting = 0;
	int center_inserted = 0, other_inserted = 0, cur_inserted = 0;
	int center_count = 0, other_count = 0, cur_count = 0;
	int cur_mid_idx = 0;
	int total_count = 0, total_inserted = 0;

	ret_if(!index);
	ret_if(!list);

	index_info = evas_object_data_get(index, DATA_KEY_INDEX_INFO);
	ret_if(!index_info);

	/* 0. Remove an old page_index_list */
	if (index_info->page_index_list) {
		EINA_LIST_FREE(index_info->page_index_list, page_index) {
			free(page_index);
		}
		index_info->page_index_list = NULL;
	}

	/* 1. Make indexes (cur/center/other) */
	total_count = scroller_count(scroller);
	total_inserted = scroller_count_direction(scroller, PAGE_DIRECTION_LEFT);
	if (scroller_count_direction(scroller, PAGE_DIRECTION_CENTER)) total_inserted++;
	if (scroller_count_direction(scroller, PAGE_DIRECTION_RIGHT)) total_inserted++;

	if (total_inserted > MAX_INDEX_NUMBER) {
		total_inserted = MAX_INDEX_NUMBER;
	}

	int style_even = 0;
	int style_base = 0;
	if (!(total_inserted % 2)) {
		style_even = 1;
		style_base = 10 - (total_inserted / 2);
	} else {
		style_base = 9 - (total_inserted / 2);
	}

	reverse_list = eina_list_reverse_clone(list);
	ret_if(!reverse_list);

	int index_total = total_inserted;
	total_inserted--;
	int indicator_index = 0;
	EINA_LIST_FREE(reverse_list, page) {
		_D("total_count:%d style_base:%d indicator_index:%d", index_total, style_base, indicator_index);
		page_info = evas_object_data_get(page, DATA_KEY_PAGE_INFO);
		continue_if(!page_info);

		int style_index = style_base + (index_total - indicator_index) - 1;
		if (page_info->direction == PAGE_DIRECTION_CENTER) {
			if (!center_inserted && total_inserted >= 0) {
				center_inserted++;
				idx_it = elm_index_item_prepend(index, NULL, NULL, (void *) total_inserted);
				total_inserted--;
#ifdef RUN_ON_DEVICE
				elm_object_item_style_set(idx_it, _item_style_get(style_index, style_even));
				_D("clock:%d style:%s", style_index, _item_style_get(style_index, style_even));
#endif
				indicator_index++;
			}
			center_count++;
		} else if (page_info->direction == index_info->direction) {
			if (total_inserted >= 0) {
				cur_inserted++;
				idx_it = elm_index_item_prepend(index, NULL, NULL, (void *) total_inserted);
				total_inserted--;
#ifdef RUN_ON_DEVICE
				elm_object_item_style_set(idx_it, _item_style_get(style_index, style_even));
				_D("other:%d style:%s", style_index, _item_style_get(style_index, style_even));
#endif
				indicator_index++;
			}
			cur_count++;
		} else {
			if (!other_inserted && total_inserted >= 0) {
				other_inserted++;
				idx_it = elm_index_item_prepend(index, NULL, NULL, (void *) total_inserted);
				total_inserted--;
#ifdef RUN_ON_DEVICE
				elm_object_item_style_set(idx_it, _item_style_get(style_index, style_even));
				_D("page:%d style:%s", style_index, _item_style_get(style_index, style_even));
#endif
				indicator_index++;
			}
			other_count++;
		}
	}

	cur_mid_idx = (cur_inserted - 1) / 2;
	extra_idx = total_count - other_count - center_count - cur_inserted;

	/* 2. Make a new page_index_list */
	page_index_inserting = -1;
	EINA_LIST_FOREACH(list, l, page) {
		page_info = evas_object_data_get(page, DATA_KEY_PAGE_INFO);
		continue_if(!page_info);

		page_index = calloc(1, sizeof(page_index_s));
		continue_if(!page_index);

		if (before_direction == page_info->direction) {
			if (index_info->direction == page_info->direction) {
				if (page_index_inserting == cur_mid_idx && extra_idx > 0) {
					extra_idx--;
				} else {
					page_index_inserting++;
				}
			}
		} else {
			before_direction = page_info->direction;
			page_index_inserting++;
		}

		page_index->page = page;
		page_index->index = page_index_inserting;

		_D("Index(%p-%d) is updating, page(%p-%d:%d)(start:%d, mid:%d, extra:%d)"
				, index, index_info->direction
				, page, page_info->direction, page_index_inserting
				, 0, cur_mid_idx, extra_idx);

		index_info->page_index_list = eina_list_append(index_info->page_index_list, page_index);
	}
}



static void _update_right(Evas_Object *scroller, Evas_Object *index, const Eina_List *list)
{
	Evas_Object *page = NULL;
	Elm_Object_Item *idx_it = NULL;
	const Eina_List *l = NULL;

	index_info_s *index_info = NULL;
	page_info_s *page_info = NULL;
	page_index_s *page_index = NULL;
	page_direction_e before_direction = PAGE_DIRECTION_MAX;

	int extra_idx = 0, page_index_inserting = 0;
	int center_inserted = 0, other_inserted = 0, cur_inserted = 0;
	int center_count = 0, other_count = 0, cur_count = 0, index_number = 0;
	int cur_start_idx = 0, cur_mid_idx = 0;
	int total_count = 0, total_inserted = 0;

	ret_if(!index);
	ret_if(!list);

	index_info = evas_object_data_get(index, DATA_KEY_INDEX_INFO);
	ret_if(!index_info);

	/* 0. Remove an old page_index_list */
	if (index_info->page_index_list) {
		EINA_LIST_FREE(index_info->page_index_list, page_index) {
			free(page_index);
		}
		index_info->page_index_list = NULL;
	}

	/* 1. Make indexes (cur/center/other) */
	total_count = scroller_count(scroller);
	total_inserted = scroller_count_direction(scroller, PAGE_DIRECTION_RIGHT);
	if (scroller_count_direction(scroller, PAGE_DIRECTION_CENTER)) total_inserted++;
	if (scroller_count_direction(scroller, PAGE_DIRECTION_LEFT)) total_inserted++;

	if (total_inserted > MAX_INDEX_NUMBER) {
		total_inserted = MAX_INDEX_NUMBER;
	}

	int style_even = 0;
	int style_base = 0;
	if (!(total_inserted % 2)) {
		style_even = 1;
		style_base = 10 - (total_inserted / 2);
	} else {
		style_base = 9 - (total_inserted / 2);
	}

	_D("total_inserted:%d style_base:%d", total_inserted, style_base);

	EINA_LIST_FOREACH(list, l, page) {
		page_info = evas_object_data_get(page, DATA_KEY_PAGE_INFO);
		continue_if(!page_info);

		if (page_info->direction == PAGE_DIRECTION_CENTER) {
			if (!center_inserted) {
				center_inserted++;
				idx_it = elm_index_item_append(index, NULL, NULL, (void *) index_number);
#ifdef RUN_ON_DEVICE
				elm_object_item_style_set(idx_it, _item_style_get(style_base + index_number, style_even));
				_D("center:%d style:%s", style_base + index_number, _item_style_get(style_base + index_number, style_even));
#endif
				index_number++;
			}
			center_count++;
		} else if (page_info->direction == index_info->direction) {
			if (index_number < total_inserted) {
				if (!cur_inserted) {
					cur_start_idx = index_number;
				}

				cur_inserted++;
				idx_it = elm_index_item_append(index, NULL, NULL, (void *) index_number);
#ifdef RUN_ON_DEVICE
				elm_object_item_style_set(idx_it, _item_style_get(style_base + index_number, style_even));
				_D("page:%d style:%s", style_base + index_number, _item_style_get(style_base + index_number, style_even));
#endif
				index_number++;
			}
			cur_count++;
		} else {
			if (!other_inserted) {
				other_inserted++;
				idx_it = elm_index_item_append(index, NULL, NULL, (void *) index_number);
#ifdef RUN_ON_DEVICE
				elm_object_item_style_set(idx_it, _item_style_get(style_base + index_number, style_even));
				_D("other:%d style:%s", style_base + index_number, _item_style_get(style_base + index_number, style_even));
#endif
				index_number++;
			}
			other_count++;
		}
	}

	cur_mid_idx = (cur_start_idx * 2 + cur_inserted - 1) / 2;
	extra_idx = total_count - other_count - center_count - cur_inserted;

	/* 2. Make a new page_index_list */
	page_index_inserting = -1;
	EINA_LIST_FOREACH(list, l, page) {
		page_info = evas_object_data_get(page, DATA_KEY_PAGE_INFO);
		continue_if(!page_info);

		page_index = calloc(1, sizeof(page_index_s));
		continue_if(!page_index);

		if (before_direction == page_info->direction) {
			if (index_info->direction == page_info->direction) {
				if (page_index_inserting == cur_mid_idx && extra_idx > 0) {
					extra_idx--;
				} else {
					page_index_inserting++;
				}
			}
		} else {
			before_direction = page_info->direction;
			page_index_inserting++;
		}

		page_index->page = page;
		page_index->index = page_index_inserting;

		_D("Index(%p-%d) is updating, page(%p-%d:%d)(start:%d, mid:%d, extra:%d)"
				, index, index_info->direction
				, page, page_info->direction, page_index_inserting
				, 0, cur_mid_idx, extra_idx);

		index_info->page_index_list = eina_list_append(index_info->page_index_list, page_index);
	}
}



static void _update(Evas_Object *index, Evas_Object *scroller, index_bring_in_e after)
{
	Evas_Object *page_current = NULL;
	Eina_List *list = NULL;

	scroller_info_s *scroller_info = NULL;
	index_info_s *index_info = NULL;

	ret_if(!index);
	ret_if(!scroller);

	_D("Index(%p) is clear", index);
	elm_index_item_clear(index);

	scroller_info = evas_object_data_get(scroller, DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	index_info = evas_object_data_get(index, DATA_KEY_INDEX_INFO);
	ret_if(!index_info);

	list = elm_box_children_get(scroller_info->box);
	ret_if(!list);

	//elm_object_theme_set(index, main_get_info()->theme);
	elm_object_style_set(index, "circle");

	if (PAGE_DIRECTION_LEFT == index_info->direction) {
		_update_left(scroller, index, list);
	} else {
		_update_right(scroller, index, list);
	}
	eina_list_free(list);

	elm_index_level_go(index, 0);

	if (INDEX_BRING_IN_AFTER == after) {
		page_current = scroller_get_focused_page(scroller);
		ret_if(!page_current);
		_bring_in_page(index, page_current);
	}
}


index_inf_s g_index_inf_winset = {
	.create = _create,
	.destroy = _destroy,
	.update = _update,
	.bring_in_page = _bring_in_page,
};

 // End of file
