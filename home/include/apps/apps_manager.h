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

int apps_get_state(void);

void apps_fini(void);
void apps_pause(void);
void apps_resume(void);

apps_error_e apps_show(void);
void apps_hide(void);

#endif //__APPS_MANAGER_H__

// End of a file
