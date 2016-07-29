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

#ifndef _APPS_DB_H_
#define _APPS_DB_H_

#include <app.h>
#include "apps/apps_data_type.h"

bool apps_db_create(void);
bool apps_db_close(void);

bool apps_db_update(apps_data_s *item);
bool apps_db_insert(apps_data_s *item);
bool apps_db_delete(apps_data_s *item);

bool apps_db_get_list(Eina_List **apps_db_list);

#endif // _APPS_DB_H_
// End of file
