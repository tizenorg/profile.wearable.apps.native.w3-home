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

#ifndef __APPS_MANAGER_H__
#define __APPS_MANAGER_H__

#include "util.h"

HAPI void apps_manager_set_mode(int mode);
HAPI int apps_manager_get_mode(void);
HAPI Eina_Bool apps_manager_get_visibility(void);

HAPI apps_error_e apps_manager_init(void);
HAPI void apps_manager_fini(void);
HAPI void apps_manager_pause(void);
HAPI void apps_manager_resume(void);

HAPI apps_error_e apps_manager_show(void);
HAPI void apps_manager_hide(void);

#endif //__APPS_MANAGER_H__

// End of a file
