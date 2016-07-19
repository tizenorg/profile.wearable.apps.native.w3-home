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

#ifndef __APPS_ROTARY_H__
#define __APPS_ROTARY_H__

#include <Evas.h>
#include "apps/apps_main.h"



HAPI void apps_rotary_append_item(Evas_Object *rotary, item_info_s *item_info);
HAPI void apps_rotary_delete_item(Evas_Object *rotary, const char *appid);
HAPI void apps_rotary_destroy(Evas_Object *layout);
HAPI Evas_Object *apps_rotary_create(Evas_Object *layout);



#endif //__APPS_ROTARY_H__

// End of a file
