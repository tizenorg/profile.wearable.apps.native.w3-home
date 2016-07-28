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

#ifndef __APPS_VIEW_H__
#define __APPS_VIEW_H__

#include "util.h"

HAPI Evas_Object *apps_view_get_window(void);
HAPI Evas_Object *apps_view_get_layout(void);
HAPI int apps_view_get_window_width(void);
HAPI int apps_view_get_window_height(void);

HAPI apps_error_e apps_view_show(void);
HAPI void apps_view_hide(void);
HAPI apps_error_e apps_view_create(void);
HAPI void apps_view_destroy(void);

#endif //__APPS_VIEW_H__

// End of a file
