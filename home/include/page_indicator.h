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

#ifndef _W_HOME_PAGE_INDICATOR_H_
#define _W_HOME_PAGE_INDICATOR_H_

#define PAGE_INDICATOR_ITEM_TYPE_PAGE 1
#define PAGE_INDICATOR_ITEM_TYPE_MORE 2
#define PAGE_INDICATOR_ITEM_TYPE_CLOCK 3

#define PAGE_INDICATOR_ID_PIVOT 0
#define PAGE_INDICATOR_ID_BASE_LEFT 1
#define PAGE_INDICATOR_ID_BASE_RIGHT 16

Evas_Object *page_indicator_item_create(Evas_Object *parent, int layout_type);
Evas_Object *page_indicator_item_find(Evas_Object *indicator, int id);
Evas_Object *page_indicator_item_at(Evas_Object *indicator, int index);
void page_indicator_clock_update(Evas_Object *indicator, char *time_str);
void page_indicator_item_activate(Evas_Object *indicator, Evas_Object *item);
int page_indicator_item_append(Evas_Object *indicator, Evas_Object *item, int type);
void page_indicator_item_clear(Evas_Object *indicator);
void page_indicator_level_go(Evas_Object *indicator, int index);
Evas_Object *page_indicator_add(Evas_Object *parent);
void page_indicator_del(Evas_Object *indicator);
void page_indicator_end(Evas_Object *indicator, int input_type, int direction);

/* effect function */
Evas_Object *page_indicator_focus_add(void);
void page_indicator_focus_activate(Evas_Object *indicator, int index);
void page_indicator_focus_end_tick(Evas_Object *indicator, int index, int direction);
void page_indicator_focus_end_drag(Evas_Object *indicator, int index, int direction);

#endif