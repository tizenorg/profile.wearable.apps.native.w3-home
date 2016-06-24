/*
 * w-home
 * Copyright (c) 2013 - 2016 Samsung Electronics Co., Ltd.
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <dlog.h>
#include <system_settings.h>

#include <Elementary.h>
#include <efl_assist.h>
#include <efl_extension.h>
#include <ui_extension.h>

#include "add-viewer.h"
#include "add-viewer_pkgmgr.h"
#include "add-viewer_ucol.h"
#include "add-viewer_package.h"
#include "add-viewer_debug.h"
#include "add-viewer_util.h"
#
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


#define ADD_VIEWER_DATA_KEY_INDEX_INFO "a_d_k_i"
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



HAPI void add_viewer_index_bringin(Evas_Object *index, Evas_Object *page)
{
	Elm_Object_Item *idx_it = NULL;
	const Eina_List *l = NULL;
	index_info_s *index_info = NULL;
	page_index_s *page_index = NULL;
	int idx = 0;
	int found = 0;

	ret_if(!index);
	ret_if(!page);

	index_info = evas_object_data_get(index, ADD_VIEWER_DATA_KEY_INDEX_INFO);
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
		ErrPrint("Cannot find a page(%p)", page);
		return;
	}

	idx_it = elm_index_item_find(index, (void *) idx);
	if (idx_it) {
		elm_index_item_selected_set(idx_it, EINA_TRUE);
	} else {
		ErrPrint("Critical, the index(%p) cannot find the page(%p:%d)", index, page, idx);
	}
}



HAPI Evas_Object *add_viewer_index_create(Evas_Object *layout)
{
	Evas_Object *index = NULL;
	index_info_s *index_info = NULL;

	if (!layout) {
		ErrPrint("Null:layout");
		return NULL;
	}

	index = elm_index_add(layout);
	if (index) {
		elm_object_style_set(index, "circle");

		index_info = calloc(1, sizeof(index_info_s));
		if (!index_info) {
			ErrPrint("Cannot calloc for index_info");
			evas_object_del(index);
			return NULL;
		}
		evas_object_data_set(index, ADD_VIEWER_DATA_KEY_INDEX_INFO, index_info);

		evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(index, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_index_horizontal_set(index, EINA_TRUE);
		elm_index_autohide_disabled_set(index, EINA_TRUE);
		elm_index_level_go(index, 0);
		evas_object_show(index);
	} else {
		ErrPrint("Failed to add index");
	}

	return index;
}



HAPI void add_viewer_index_destroy(Evas_Object *index)
{
	index_info_s *index_info = NULL;
	page_index_s *page_index = NULL;

	ret_if(!index);

	index_info = evas_object_data_del(index, ADD_VIEWER_DATA_KEY_INDEX_INFO);
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



static void _update_page(Evas_Object *index, const Eina_List *list)
{
	index_info_s *index_info = NULL;
	page_index_s *page_index = NULL;

	ret_if(!index);
	ret_if(!list);

	index_info = evas_object_data_get(index, ADD_VIEWER_DATA_KEY_INDEX_INFO);
	ret_if(!index_info);

	/* 0. Remove an old page_index_list */
	if (index_info->page_index_list) {
		EINA_LIST_FREE(index_info->page_index_list, page_index) {
			free(page_index);
		}
		index_info->page_index_list = NULL;
	}

	int total_inserted = eina_list_count(list);
	int style_even = 0;
	int style_base = 0;
	if (!(total_inserted % 2)) {
		style_even = 1;
		style_base = 10 - (total_inserted / 2);
	} else {
		style_base = 9 - (total_inserted / 2);
	}

	DbgPrint("total_inserted:%d style_base:%d", total_inserted, style_base);

	int index_number = 0;
	Evas_Object *page = NULL;
	Elm_Object_Item *idx_it = NULL;
	const Eina_List *l = NULL;

	EINA_LIST_FOREACH(list, l, page) {
		idx_it = elm_index_item_append(index, NULL, NULL, (void *) index_number);
		elm_object_item_style_set(idx_it, _item_style_get(style_base + index_number, style_even));
		DbgPrint("other:%d style:%s", style_base + index_number, _item_style_get(style_base + index_number, style_even));
		//elm_object_item_part_content_set(idx_it, "icon", _item_icon_get(index));

		page_index = calloc(1, sizeof(page_index_s));
		continue_if(!page_index);

		page_index->index = index_number;
		page_index->page = page;
		index_info->page_index_list = eina_list_append(index_info->page_index_list, page_index);

		index_number++;
	}
}



HAPI void add_viewer_index_update(Evas_Object *index, Eina_List *page_list)
{
	index_info_s *index_info = NULL;

	ret_if(!index);
	ret_if(!page_list);

	DbgPrint("Index(%p) is clear", index);
	elm_index_item_clear(index);

	index_info = evas_object_data_get(index, ADD_VIEWER_DATA_KEY_INDEX_INFO);
	ret_if(!index_info);

	elm_object_style_set(index, "circle");

	_update_page(index, page_list);

	elm_index_level_go(index, 0);
}

 // End of file
