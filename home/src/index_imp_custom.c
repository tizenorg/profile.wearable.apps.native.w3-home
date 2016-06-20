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
#include "page_indicator.h"



typedef struct {
	Evas_Object *page;
	int index;
} page_index_s;

static void _bring_in_page(Evas_Object *index, Evas_Object *page)
{
	Evas_Object *idx_it = NULL;
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

	idx_it = page_indicator_item_find(index, idx);
	if (idx_it) {
		page_indicator_item_activate(index, idx_it);
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

	index = page_indicator_add(layout);
	retv_if(!index, NULL);
	elm_object_style_set(index, "circle_empty");

	index_info = calloc(1, sizeof(index_info_s));
	if (!index_info) {
		_E("Cannot calloc for index_info");
		evas_object_del(index);
		return NULL;
	}
	evas_object_data_set(index, DATA_KEY_INDEX_INFO, index_info);

	evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(index, EVAS_HINT_FILL, EVAS_HINT_FILL);
	page_indicator_level_go(index, PAGE_INDICATOR_ID_PIVOT);
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
	page_indicator_item_clear(index);
	page_indicator_del(index);
}



#ifdef HOME_TBD
#define MAX_INDEX_NUMBER 17
static void _update_left(Evas_Object *scroller, Evas_Object *index, const Eina_List *list)
{
#if 0
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
		style_base = 8 - (total_inserted / 2);
	} else {
		style_base = 8 - (total_inserted / 2) - 1;
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
				elm_object_item_part_content_set(idx_it, "icon", _item_icon_get(index, 1));
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
				elm_object_item_part_content_set(idx_it, "icon", _item_icon_get(index, 0));
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
				elm_object_item_part_content_set(idx_it, "icon", _item_icon_get(index, 2));
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
#endif
}
#endif



static void _add_index_info_to_list(Eina_List **list, void *pageid, int index)
{
	page_index_s *page_index = NULL;
	if (index != -1) {
		page_index = calloc(1, sizeof(page_index_s));
		if (page_index) {
			page_index->page = pageid;
			page_index->index = index;

			*list = eina_list_append(*list, page_index);
		}
	}
}

static void _update_index(Evas_Object *scroller, Evas_Object *index, const Eina_List *list)
{
	index_info_s *index_info = NULL;
	page_info_s *page_info = NULL;
	page_index_s *page_index = NULL;

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

	int new_index = -1;

	Evas_Object *page = NULL;
	const Eina_List *l = NULL;
	EINA_LIST_FOREACH(list, l, page) {
		page_info = evas_object_data_get(page, DATA_KEY_PAGE_INFO);
		continue_if(!page_info);

		new_index = -1;
		if (page_info->direction == PAGE_DIRECTION_CENTER) {
			Evas_Object *item = page_indicator_item_create(index, PAGE_INDICATOR_ITEM_TYPE_CLOCK);
			if (item) {
				new_index = page_indicator_item_append(index, item, PAGE_INDICATOR_ID_PIVOT);
				if (new_index < 0) {
					evas_object_del(item);
					item = NULL;
				}
			}

			_add_index_info_to_list(&(index_info->page_index_list), page, new_index);
		} else if (page_info->direction == PAGE_DIRECTION_RIGHT) {
			Evas_Object *item = page_indicator_item_create(index, PAGE_INDICATOR_ITEM_TYPE_PAGE);
			if (item) {
				new_index = page_indicator_item_append(index, item, PAGE_INDICATOR_ID_BASE_RIGHT);
				if (new_index < 0) {
					evas_object_del(item);
					item = NULL;
				}
			}

			_add_index_info_to_list(&(index_info->page_index_list), page, new_index);
		} else if (page_info->direction == PAGE_DIRECTION_LEFT) {
			Evas_Object *item = NULL;
			if (page_info->sub_page.num <= 0) {
				item = page_indicator_item_create(index, PAGE_INDICATOR_ITEM_TYPE_PAGE);
				if (item) {
					new_index = page_indicator_item_append(index, item, PAGE_INDICATOR_ID_BASE_LEFT);
					if (new_index < 0) {
						evas_object_del(item);
						item = NULL;
					}
				}

				_add_index_info_to_list(&(index_info->page_index_list), page, new_index);
			} else {
				int k;
				for (k = page_info->sub_page.start_index; k <= page_info->sub_page.num; k++) {
					item = page_indicator_item_create(index, PAGE_INDICATOR_ITEM_TYPE_PAGE);
					if (item) {
						new_index = page_indicator_item_append(index, item, PAGE_INDICATOR_ID_BASE_LEFT);
						if (new_index >= 0) {
							_add_index_info_to_list(&(index_info->page_index_list), (void *)k, new_index);
						} else {
							evas_object_del(item);
							item = NULL;
						}
					}
				}
			}
		}
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
	page_indicator_item_clear(index);

	scroller_info = evas_object_data_get(scroller, DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	index_info = evas_object_data_get(index, DATA_KEY_INDEX_INFO);
	ret_if(!index_info);

	list = elm_box_children_get(scroller_info->box);
	ret_if(!list);

	elm_object_theme_set(index, main_get_info()->theme);
	elm_object_style_set(index, "circle_empty");

	_update_index(scroller, index, list);

	eina_list_free(list);

	if (INDEX_BRING_IN_AFTER == after) {
		page_current = scroller_get_focused_page(scroller);
		ret_if(!page_current);
		_bring_in_page(index, page_current);
	}
}



static void _update_time(Evas_Object *index, char *time_str)
{
	page_indicator_clock_update(index, time_str);
}



static void _end(Evas_Object *index, int input_type, int direction)
{
#ifdef HOME_TBD
	page_indicator_end(index, input_type, direction);
#endif
}

index_inf_s g_index_inf_custom = {
	.create = _create,
	.destroy = _destroy,
	.update = _update,
	.update_time = _update_time,
	.bring_in_page = _bring_in_page,
	.end = _end,
};
 // End of file
