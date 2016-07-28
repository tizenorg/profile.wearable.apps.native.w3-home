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
#include "apps/apps_view.h"
#include "key.h"

#define APPS_VIEW_WIN_NAME "__APPS__"
#define APPS_VIEW_WIN_TITLE "__APPS__"
#define APPS_VIEW_LAYOUT_EDJE EDJEDIR"/apps_layout.edj"
#define APPS_VIEW_LAYOUT_GROUP_NAME "layout"
#define APPS_VIEW_ROTATION_TYPE_NUM 1

static struct _apps_view_s {
	Evas *e;
	Evas_Object *win;
	Evas_Object *layout;
	Ecore_Idler *apps_item_idler;
	Ecore_Idler *apps_show_idler;
	int root_w;
	int root_h;
} view_info = {
	.e = NULL,
	.win = NULL,
	.layout = NULL,
	.apps_item_idler = NULL,
	.apps_show_idler = NULL,
	.root_w = 0,
	.root_h = 0,
};

HAPI Evas_Object *apps_view_get_window(void)
{
	return view_info.win;
}

HAPI Evas_Object *apps_view_get_layout(void)
{
	return view_info.layout;
}

HAPI int apps_view_get_window_width(void)
{
	return view_info.root_w;
}

HAPI int apps_view_get_window_height(void)
{
	return view_info.root_h;
}

static void _window_focus_in_cb(void *data, Evas *e, void *event_info)
{
	_APPS_D("focus in");
}

static void _window_focus_out_cb(void *data, Evas *e, void *event_info)
{
	_APPS_D("focus out");
}

static apps_error_e _register_window_focus_event(Evas_Object *win)
{
	if (win == NULL) {
		_APPS_E("apps window is NULL");
		return APPS_ERROR_INVALID_PARAMETER;
	}

	view_info.e = evas_object_evas_get(win);
	if (view_info.e == NULL) {
		_APPS_E("Failed to get Evas");
		return APPS_ERROR_FAIL;
	}

	evas_event_callback_add(view_info.e, EVAS_CALLBACK_CANVAS_FOCUS_IN, _window_focus_in_cb, NULL);
	evas_event_callback_add(view_info.e, EVAS_CALLBACK_CANVAS_FOCUS_OUT, _window_focus_out_cb, NULL);

	return APPS_ERROR_NONE;
}

static void _unregister_window_focus_event(void)
{
	if (view_info.e == NULL) {
		_APPS_E("Evas is already NULL");
		return;
	}

	evas_event_callback_del(view_info.e, EVAS_CALLBACK_CANVAS_FOCUS_IN, _window_focus_in_cb);
	evas_event_callback_del(view_info.e, EVAS_CALLBACK_CANVAS_FOCUS_OUT, _window_focus_out_cb);

	view_info.e = NULL;
}

static Evas_Object *_create_window(const char *name, const char *title)
{
	Evas_Object *win = NULL;

	if (name == NULL) {
		_APPS_E("name is NULL");
		return NULL;
	}

	if (title == NULL) {
		_APPS_E("title is NULL");
		return NULL;
	}

	/* Open GL backend */
	elm_config_accel_preference_set("opengl");

	win = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (win == NULL) {
		_APPS_E("Failed to add apps window");
		return NULL;
	}

	elm_win_screen_size_get(win, NULL, NULL, &view_info.root_w, &view_info.root_h);

	elm_win_alpha_set(win, EINA_FALSE); // This order is important
	elm_win_role_set(win, "no-effect");

	if (elm_win_wm_rotation_supported_get(win)) {
		const int rots[APPS_VIEW_ROTATION_TYPE_NUM] = { 0 };
		elm_win_wm_rotation_available_rotations_set(win, rots, APPS_VIEW_ROTATION_TYPE_NUM);
	}

	evas_object_color_set(win, 0, 0, 0, 0);
	evas_object_resize(win, view_info.root_w, view_info.root_h);
	evas_object_hide(win);

	elm_win_title_set(win, title);
	elm_win_borderless_set(win, EINA_TRUE);

	return win;
}

static void _destroy_window(void)
{
	if (view_info.win == NULL) {
		_APPS_E("apps window is already NULL");
		return;
	}

	evas_object_del(view_info.win);
	view_info.win = NULL;
}

static Evas_Object *_create_layout(Evas_Object *parent)
{
	Evas_Object *layout = NULL;
	Eina_Bool ret = EINA_FALSE;

	if (parent == NULL) {
		_APPS_E("parent is NULL");
		return NULL;
	}

	layout = elm_layout_add(parent);
	if (layout == NULL) {
		_APPS_E("Failed to add apps layout");
		return NULL;
	}

	ret = elm_layout_file_set(layout, APPS_VIEW_LAYOUT_EDJE, APPS_VIEW_LAYOUT_GROUP_NAME);
	if (ret != EINA_TRUE) {
		_APPS_E("Failed to set layout file");
		evas_object_del(layout);
		return NULL;
	}

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_resize(layout, view_info.root_w, view_info.root_h);
	evas_object_show(layout);

	return layout;
}

static void _destroy_layout(void)
{
	if (view_info.layout == NULL) {
		_APPS_E("apps layout is already NULL");
		return;
	}

	evas_object_del(view_info.layout);
	view_info.layout = NULL;
}

static Eina_Bool _push_items(void *layout)
{
	if (layout == NULL) {
		_APPS_E("layout is NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	return ECORE_CALLBACK_CANCEL;
}

static key_cb_ret_e _back_key_cb(void *data)
{
	_APPS_D("%s", __func__);

	return KEY_CB_RET_STOP;
}

static Eina_Bool _show_idler_cb(void *data)
{
	_APPS_D("%s", __func__);

	if (view_info.layout == NULL) {
		_APPS_E("apps layout is NULL");
		return APPS_ERROR_FAIL;
	}

	if (view_info.win == NULL) {
		_APPS_E("apps window is NULL");
		return APPS_ERROR_FAIL;
	}

	evas_object_show(view_info.layout);
	evas_object_show(view_info.win);
	elm_win_activate(view_info.win);

	view_info.apps_show_idler = NULL;

	return ECORE_CALLBACK_CANCEL;
}

HAPI apps_error_e apps_view_show(void)
{
	int ret = 0;

	_APPS_D("%s", __func__);

	if (view_info.apps_show_idler) {
		ecore_idler_del(view_info.apps_show_idler);
		view_info.apps_show_idler = NULL;
	}

	view_info.apps_show_idler = ecore_idler_add(_show_idler_cb, NULL);
	if (view_info.apps_show_idler == NULL) {
		_APPS_E("Failed to add apps show idler");
		return APPS_ERROR_FAIL;
	}

	ret = key_register_cb(KEY_TYPE_BACK, _back_key_cb, NULL);
	if (W_HOME_ERROR_NONE != ret) {
		_APPS_E("Failed to register back key event");
	}

	return APPS_ERROR_NONE;
}

HAPI void apps_view_hide(void)
{
	_APPS_D("%s", __func__);

	if (view_info.win == NULL) {
		_APPS_E("apps window is NULL");
		return;
	}

	evas_object_hide(view_info.win);
	key_unregister_cb(KEY_TYPE_BACK, _back_key_cb);
}

HAPI apps_error_e apps_view_create(void)
{
	int ret = 0;

	_APPS_D("%s", __func__);

	view_info.win =  _create_window(APPS_VIEW_WIN_NAME, APPS_VIEW_WIN_TITLE);
	if (view_info.win == NULL) {
		_APPS_E("Failed to create apps window");
		return APPS_ERROR_FAIL;
	}

	ret = _register_window_focus_event(view_info.win);
	if (APPS_ERROR_NONE != ret) {
		_APPS_E("Failed to register window focus event");
		_destroy_window();
		return APPS_ERROR_FAIL;
	}

	view_info.layout = _create_layout(view_info.win);
	if (view_info.layout == NULL) {
		_APPS_E("Failed to create apps layout");
		_unregister_window_focus_event();
		_destroy_window();
		return APPS_ERROR_FAIL;
	}

	view_info.apps_item_idler = ecore_idler_add(_push_items, view_info.layout);
	if (view_info.apps_item_idler == NULL) {
		_APPS_E("Failed to push app items");
	}

	return APPS_ERROR_NONE;
}

HAPI void apps_view_destroy(void)
{
	_APPS_D("%s", __func__);

	if (view_info.apps_item_idler) {
		ecore_idler_del(view_info.apps_item_idler);
		view_info.apps_item_idler = NULL;
	}

	_destroy_layout();
	_unregister_window_focus_event();
	_destroy_window();
}
