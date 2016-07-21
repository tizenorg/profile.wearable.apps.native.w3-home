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

#include <Elementary.h>
#include <app_control.h>
#include <efl_assist.h>

#include "log.h"
#include "util.h"
#include "key.h"
#include "main.h"
#include "layout_info.h"
#include "apps/apps_conf.h"
#include "apps/layout.h"
#include "apps/apps_main.h"

#define MOVE_UP -2
#define MOVE_DOWN 2



static void _bg_down_cb(void *data, Evas_Object *obj, const char* emission, const char* source)
{
	_APPS_D("BG is pressed");
}

static void _bg_up_cb(void *data, Evas_Object *obj, const char* emission, const char* source)
{
	_APPS_D("BG is released");
}

static key_cb_ret_e _back_key_cb(void *data)
{
	_APPS_D("Back key cb");
}

static apps_error_e _pause_result_cb(void *data)
{
	_APPS_D("%s", __func__);

	return APPS_ERROR_NONE;
}

static apps_error_e _resume_result_cb(void *data)
{
	_APPS_D("%s", __func__);

	return APPS_ERROR_NONE;
}

static apps_error_e _reset_result_cb(void *data)
{
	_APPS_D("%s", __func__);

	return APPS_ERROR_NONE;
}

static Eina_Bool _push_items_cb(void *layout)
{
	return ECORE_CALLBACK_CANCEL;
}

HAPI Evas_Object *apps_layout_create(Evas_Object *win, const char *file, const char *group)
{
	apps_main_s *apps_main_info = NULL;
	Evas_Object *layout = NULL;
	Ecore_Idler *idle_timer = NULL;
	Eina_Bool ret = EINA_FALSE;

	_APPS_D("%s", __func__);

	if (win == NULL) {
		_APPS_E("windown is NULL");
		return NULL;
	}

	if (file == NULL) {
		_APPS_E("file is NULL");
		return NULL;
	}

	if (group == NULL) {
		_APPS_E("group is NULL");
		return NULL;
	}

	apps_main_info = apps_main_get_info();
	if (apps_main_info == NULL) {
		_APPS_E("Failed to get apps main info");
		return NULL;
	}

	layout = elm_layout_add(win);
	if (layout == NULL) {
		_APPS_E("Failed to add layout");
		return NULL;
	}

	ret = elm_layout_file_set(layout, file, group);
	if (ret == EINA_FALSE) {
		_APPS_E("Failed to set file to layout(%s/%s)", file, group);
		evas_object_del(layout);
		return NULL;
	}

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_resize(layout, apps_main_info->root_w, apps_main_info->root_h);
	evas_object_show(layout);

	elm_object_signal_callback_add(layout, "bg,down", "bg", _bg_down_cb, NULL);
	elm_object_signal_callback_add(layout, "bg,up", "bg", _bg_up_cb, NULL);

	if (APPS_ERROR_NONE != apps_main_register_cb(APPS_APP_STATE_PAUSE, _pause_result_cb, NULL)) {
		_APPS_E("Failed to register the pause callback");
	}

	if (APPS_ERROR_NONE != apps_main_register_cb(APPS_APP_STATE_RESUME, _resume_result_cb, NULL)) {
		_APPS_E("Failed to register the resume callback");
	}

	if (APPS_ERROR_NONE != apps_main_register_cb(APPS_APP_STATE_RESET, _reset_result_cb, NULL)) {
		_APPS_E("Failed to register the reset callback");
	}

#if TBD
	item_badge_register_changed_cb(layout);
#endif

	idle_timer = ecore_idler_add(_push_items_cb, layout);
	if (idle_timer == NULL) {
		_APPS_E("Failed to add idle timer for items");
	}

	return layout;
}

HAPI void apps_layout_destroy(void)
{
	apps_main_s *apps_main_info = NULL;
	Ecore_Idler *idle_timer = NULL;

	_APPS_D("%s", __func__);

	apps_main_info = apps_main_get_info();
	if (apps_main_info == NULL) {
		_APPS_E("Failed to get apps main info");
		return;
	}

	if (apps_main_info->layout == NULL) {
		_APPS_E("layout is NULL");
		return;
	}

#if TBD
	idle_timer = _get_idle_time();
	if (idle_timer) {
		ecore_idler_dle(idle_timer);
	}

	item_badge_unregister_changed_cb();
#endif

	apps_main_unregister_cb(APPS_APP_STATE_PAUSE, _pause_result_cb);
	apps_main_unregister_cb(APPS_APP_STATE_RESUME, _resume_result_cb);
	apps_main_unregister_cb(APPS_APP_STATE_RESET, _reset_result_cb);

	edje_object_signal_callback_del(_EDJ(apps_main_info->layout), "bg,down", "bg", _bg_down_cb);
	edje_object_signal_callback_del(_EDJ(apps_main_info->layout), "bg,up", "bg", _bg_up_cb);

	if (apps_main_info->layout) {
		evas_object_del(apps_main_info->layout);
		apps_main_info->layout = NULL;
	}
}

static Eina_Bool _show_anim_cb(void *data)
{
	Evas_Object *win = NULL;
	apps_main_s *apps_main_info = NULL;

	_APPS_D("%s", __func__);

	apps_main_info = apps_main_get_info();
	if (apps_main_info == NULL) {
		_APPS_E("Failed to get apps main info");
		return APPS_ERROR_FAIL;
	}

	if (apps_main_info->win == NULL) {
		_APPS_E("window is NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	if (apps_main_info->layout == NULL) {
		_APPS_E("layout is NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	evas_object_show(apps_main_info->layout);
	evas_object_show(apps_main_info->win);
	elm_win_activate(apps_main_info->win);

	return ECORE_CALLBACK_CANCEL;
}

HAPI apps_error_e apps_layout_show(void)
{
	apps_main_s *apps_main_info = NULL;
	Ecore_Idler *animator = NULL;

	_APPS_D("%s", __func__);

	apps_main_info = apps_main_get_info();
	if (apps_main_info == NULL) {
		_APPS_E("Failed to get apps main info");
		return APPS_ERROR_FAIL;
	}

	animator = ecore_idler_add(_show_anim_cb, NULL);
	if (animator == NULL) {
		_APPS_E("Failed to add show animator");
		return APPS_ERROR_FAIL;
	}

	if (W_HOME_ERROR_NONE != key_register_cb(KEY_TYPE_BACK, _back_key_cb, apps_main_info->win)) {
		_APPS_E("Failed to register back key event");
	}

	return APPS_ERROR_NONE;
}

HAPI apps_error_e apps_layout_hide(void)
{
	apps_main_s *apps_main_info = NULL;

	_APPS_D("%s", __func__);

	apps_main_info = apps_main_get_info();
	if (apps_main_info == NULL) {
		_APPS_E("Failed to get apps main info");
		return APPS_ERROR_FAIL;
	}

	if (apps_main_info->longpress_edit_disable && apps_layout_is_edited(apps_main_info->layout)) {
		apps_layout_unedit(apps_main_info->layout);
	}

	evas_object_hide(apps_main_info->win);
	key_unregister_cb(KEY_TYPE_BACK, _back_key_cb);

	return APPS_ERROR_NONE;
}

HAPI void apps_layout_rotate(Evas_Object *layout, int is_rotated)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_focus_on(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_focus_off(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_block(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_unblock(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI int apps_layout_is_edited(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_edit(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

static Eina_Bool _update_list_cb(void *data)
{
	_APPS_D("%s", __func__);

	return ECORE_CALLBACK_CANCEL;
}

HAPI void apps_layout_unedit(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}
// End of file
