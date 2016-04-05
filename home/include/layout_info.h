/*
 * Samsung API
 * Copyright (c) 2009-2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __W_HOME_LAYOUT_INFO_H__
#define __W_HOME_LAYOUT_INFO_H__



typedef struct {
	/* innate features */
	Evas_Object *win;
	Evas_Object *scroller;
	Evas_Object *index;
	Evas_Object *gestuer_layer;
	Evas_Object *tutorial;

	/* acquired features */
	Evas_Object *edit;
	Evas_Object *pressed_page;
	Evas_Object *pressed_item;
} layout_info_s;

#endif // __W_HOME_LAYOUT_INFO_H__
