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

#include <Elementary.h>
#include <stdbool.h>
#include <stdio.h>
#include <dlog.h>
#include <efl_assist.h>
#include <efl_extension.h>

#include "util.h"
#include "main.h"
#include "log.h"
#include "util.h"
#include "notify.h"
#include "rotary.h"

#define PRIVATE_DATA_KEY_ROTARY_EXCLUSIVE_FN "p_d_rt_rot_c_fn"
#define PRIVATE_DATA_KEY_ROTARY_HYBRID_FN "p_d_rt_hyb_c_fn"
#define PRIVATE_DATA_KEY_ROTARY_RESUME_FN "p_d_rt_res_c_fn"
#define PRIVATE_DATA_KEY_ROTARY_ACTIVATION_OBJ "p_d_rt_act_c_obj"

static struct _info {
	Eina_List *obj_list;
} s_info = {
	.obj_list = NULL,
};

void _object_push(Evas_Object *obj);
void _object_pop(Evas_Object *obj);
static w_home_error_e _default_resume_cb(void *data);

static void _object_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	if (obj) {
		_object_pop(obj);
		_default_resume_cb(NULL);
	}
}

void _object_push(Evas_Object *obj)
{
	Eina_List *list = s_info.obj_list;
	list = eina_list_prepend(list, obj);
	s_info.obj_list = list;

	/*
	 * Workaround, prevent to access a deleted object
	 */
	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _object_del_cb);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _object_del_cb, NULL);
}

void _object_pop(Evas_Object *obj)
{
	Eina_List *list = s_info.obj_list;
	list = eina_list_remove(list, obj);
	s_info.obj_list = list;
}

Evas_Object *_object_peek(void)
{
	return eina_list_nth(s_info.obj_list, 0);
}

static Eina_Bool _default_rotary_cb(void *data, Evas_Object *obj, Eext_Rotary_Event_Info *info)
{
	Evas_Object *top_obj = _object_peek();
	Eext_Rotary_Object_Event_Cb  cb = NULL;
	if (top_obj != obj) {
		_W("un-popped rotary object:%p", obj);
		return ECORE_CALLBACK_PASS_ON;
	}

	if (ecore_animator_source_get() == ECORE_ANIMATOR_SOURCE_TIMER) {
		ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);
	}

	int rotary_mode = rotary_mode_get();
	if (rotary_mode == ROTARY_MODE_EXCLUSIVE) {
		cb = evas_object_data_get(obj, PRIVATE_DATA_KEY_ROTARY_EXCLUSIVE_FN);
	} else if (rotary_mode == ROTARY_MODE_HYBRID) {
		cb = evas_object_data_get(obj, PRIVATE_DATA_KEY_ROTARY_HYBRID_FN);
	}

	if (cb) {
		return cb(data, obj, info);
	} else {
		return ECORE_CALLBACK_PASS_ON;
	}
}

static w_home_error_e _default_resume_cb(void *data)
{
	Evas_Object *obj = _object_peek();
	w_home_error_e (*cb)(void *) = NULL;

	if (obj) {
		cb = evas_object_data_get(obj, PRIVATE_DATA_KEY_ROTARY_RESUME_FN);
		if (cb) {
			return cb(obj);
		}
	}

	return W_HOME_ERROR_NONE;
}

HAPI void rotary_init(void)
{
	;
}

HAPI void rotary_fini(void)
{
	if (s_info.obj_list) {
		eina_list_free(s_info.obj_list);
		s_info.obj_list = NULL;
	}
}

HAPI void rotary_attach(Evas_Object *scroller, Eext_Rotary_Object_Event_Cb rotary_cb, Eext_Rotary_Object_Event_Cb hybrid_cb, int (*result_cb)(void *))
{
	ret_if(scroller == NULL);
	_W("rotary_attach:%p", scroller);

	evas_object_data_set(scroller, PRIVATE_DATA_KEY_ROTARY_EXCLUSIVE_FN, rotary_cb);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_ROTARY_HYBRID_FN, hybrid_cb);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_ROTARY_RESUME_FN, result_cb);
	if (rotary_cb != NULL || hybrid_cb != NULL) {
		eext_rotary_object_event_callback_priority_add(scroller, EEXT_CALLBACK_PRIORITY_BEFORE, _default_rotary_cb, NULL);
	}

	_object_push(scroller);
	_default_resume_cb(NULL);
}

HAPI void rotary_deattach(Evas_Object *scroller)
{
	Eext_Rotary_Object_Event_Cb  rotary_cb = NULL;
	Eext_Rotary_Object_Event_Cb  hybrid_cb = NULL;
	ret_if(scroller == NULL);
	_W("rotary_deattach:%p", scroller);

	rotary_cb = evas_object_data_del(scroller, PRIVATE_DATA_KEY_ROTARY_EXCLUSIVE_FN);
	hybrid_cb = evas_object_data_del(scroller, PRIVATE_DATA_KEY_ROTARY_HYBRID_FN);
	evas_object_data_del(scroller, PRIVATE_DATA_KEY_ROTARY_RESUME_FN);
	if (rotary_cb != NULL || hybrid_cb != NULL) {
		eext_rotary_object_event_callback_del(scroller, _default_rotary_cb);
	}

	_object_pop(scroller);
	_default_resume_cb(NULL);
}

HAPI void rotary_activation_object_set(Evas_Object *scroller, Evas_Object *act_obj)
{
	ret_if(scroller == NULL);
	ret_if(act_obj == NULL);

	evas_object_data_set(scroller, PRIVATE_DATA_KEY_ROTARY_ACTIVATION_OBJ, act_obj);

	_default_resume_cb(NULL);
}

HAPI Evas_Object *rotary_activation_object_get(Evas_Object *scroller)
{
	retv_if(scroller == NULL, NULL);

	return evas_object_data_get(scroller, PRIVATE_DATA_KEY_ROTARY_ACTIVATION_OBJ);
}

HAPI int rotary_mode_get(void)
{
	return notify_get_info()->rotary_mode;
}
