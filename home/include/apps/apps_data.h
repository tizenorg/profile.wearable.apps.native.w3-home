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

#ifndef _APPS_DATA_H_
#define _APPS_DATA_H_

#include <app.h>
#include <Elementary.h>

#include "apps/apps_data_type.h"

void apps_data_init(void);
void apps_data_fini(void);
Eina_List *apps_data_get_list(void);

void apps_data_delete(apps_data_s *item);
void apps_data_delete_by_pkg_id(const char *pkg_id);
void apps_data_delete_by_app_id(const char *app_id);

void apps_data_insert(apps_data_s *item);

#endif // _APPS_DATA_H_
// End of file
