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
#if 0
#ifndef __W_HOME_ROTARY_H__
#define __W_HOME_ROTARY_H__

#include <efl_assist.h>
#include <efl_extension.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ROTARY_MODE_HYBRID 0
#define ROTARY_MODE_EXCLUSIVE 1

extern void rotary_init(void);
extern void rotary_fini(void);
extern void rotary_attach(Evas_Object *scroller, Eext_Rotary_Object_Event_Cb rotary_cb, Eext_Rotary_Object_Event_Cb hybrid_cb, int (*result_cb)(void *));
extern void rotary_deattach(Evas_Object *scroller);
extern int rotary_mode_get(void);
Evas_Object *rotary_activation_object_get(Evas_Object *scroller);
void rotary_activation_object_set(Evas_Object *scroller, Evas_Object *act_obj);

#ifdef __cplusplus
}
#endif

#endif
#endif