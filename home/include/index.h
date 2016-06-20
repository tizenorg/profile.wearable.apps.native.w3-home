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

#ifndef __W_HOME_INDEX_H__
#define __W_HOME_INDEX_H__

#include "page.h"

#define INDEX_TYPE_WINSET 1
#define INDEX_TYPE_CUSTOM 2

#define INDEX_INPUT_ROTARY 1
#define INDEX_INPUT_TOUCH 2

#define INDEX_END_LEFT 1
#define INDEX_END_RIGHT 2

typedef enum {
	INDEX_BRING_IN_NONE = 0,
	INDEX_BRING_IN_AFTER,
} index_bring_in_e;

typedef struct _index_inf_s {
	Evas_Object *(*create) (Evas_Object *, Evas_Object *, page_direction_e);
	void (*destroy) (Evas_Object *);
	void (*update) (Evas_Object *, Evas_Object *, index_bring_in_e );
	void (*update_time) (Evas_Object *, char *);
	void (*bring_in_page) (Evas_Object *, Evas_Object *);
	void (*end) (Evas_Object *, int , int);
} index_inf_s;

extern void index_bring_in_page(Evas_Object *index, Evas_Object *page);

extern Evas_Object *index_create(int type, Evas_Object *layout, Evas_Object *scroller, page_direction_e direction);
extern void index_destroy(Evas_Object *index);
extern void index_time_update(Evas_Object *index, char *time_str);
extern void index_update(Evas_Object *index, Evas_Object *scroller, index_bring_in_e after);
extern void index_show(Evas_Object *index, int vi);
extern void index_hide(Evas_Object *index, int vi);
extern void index_end(Evas_Object *index, int input_type, int direction);

#endif /* __W_HOME_INDEX_H__ */
