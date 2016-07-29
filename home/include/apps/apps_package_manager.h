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

#ifndef _APPS_PACKAGE_MANAGER_H_
#define _APPS_PACKAGE_MANAGER_H_

#include <app.h>
#include <Elementary.h>

void apps_package_manager_init(void);
void apps_package_manager_fini(void);

bool apps_package_manager_get_list(Eina_List **list);

void apps_package_manager_update_label(Eina_List *list);

#endif // _APPS_PACKAGE_MANAGER_H_
// End of file
