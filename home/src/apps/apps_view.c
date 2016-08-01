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

#define APPS_VIEW_LONG_PRESS_TIME 0.75
#define APPS_VIEW_MOUSE_MOVE_MIN_DISTANCE 100

enum {
	APPS_MODE_NORMAL = 0,
	APPS_MODE_EDIT,
	APPS_MODE_REORDER,
	APPS_MODE_MAX,
};

static struct _apps_view_s {
	Evas_Object *win;
	Evas_Object *layout;
	Ecore_Idler *apps_item_idler;
	Ecore_Idler *apps_show_idler;
	int root_w;
	int root_h;
	int mode;
} view_info = {
	.win = NULL,
	.layout = NULL,
	.apps_item_idler = NULL,
	.apps_show_idler = NULL,
	.root_w = 0,
	.root_h = 0,
	.mode = APPS_MODE_NORMAL,
};

static struct _mouse_info_s {
	Eina_Bool pressed;
	Eina_Bool long_pressed;
	Evas_Coord down_x;
	Evas_Coord down_y;
	Evas_Coord move_x;
	Evas_Coord move_y;
	Evas_Coord up_x;
	Evas_Coord up_y;
	int offset_x;
	int offset_y;
	Ecore_Timer *long_press_timer;
	Evas_Object *pressed_obj;
} mouse_info = {
	.pressed = EINA_FALSE,
	.long_pressed = EINA_FALSE,
	.down_x = 0,
	.down_y = 0,
	.move_x = 0,
	.move_y = 0,
	.up_x = 0,
	.up_y = 0,
	.offset_x = 0,
	.offset_y = 0,
	.long_press_timer = NULL,
	.pressed_obj = NULL,
};


int apps_view_get_mode(void)
{
	return view_info.mode;
}

void apps_view_set_mode(int mode)
{
	_APPS_D("Set mode : %d -> %d", view_info.mode, mode);
	view_info.mode = mode;
}

static Evas_Object *_create_window(const char *name, const char *title)
{
	Evas_Object *win = NULL;

	/* Open GL backend */
	elm_config_accel_preference_set("opengl");

	win = elm_win_util_standard_add(name, name);
	if (win == NULL) {
		_APPS_E("Failed to add apps window");
		return NULL;
	}

	elm_win_screen_size_get(win, NULL, NULL, &view_info.root_w, &view_info.root_h);

	evas_object_resize(win, view_info.root_w, view_info.root_h);
	evas_object_show(win);

	if (elm_win_wm_rotation_supported_get(win)) {
		const int rots[APPS_VIEW_ROTATION_TYPE_NUM] = { 0 };
		elm_win_wm_rotation_available_rotations_set(win, rots, APPS_VIEW_ROTATION_TYPE_NUM);
	}

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
	evas_object_size_hint_min_set(layout, view_info.root_w, view_info.root_h);
	evas_object_size_hint_max_set(layout, view_info.root_w, view_info.root_h);
	elm_win_resize_object_add(parent, layout);
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

static key_cb_ret_e _home_key_cb(void *data)
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

apps_error_e apps_view_show(void)
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

	ret = key_register_cb(KEY_TYPE_HOME, _home_key_cb, NULL);
	if (W_HOME_ERROR_NONE != ret) {
		_APPS_E("Failed to register home key event");
	}

	view_info.mode = APPS_MODE_NORMAL;

	return APPS_ERROR_NONE;
}

void apps_view_hide(void)
{
	_APPS_D("%s", __func__);

	if (view_info.win == NULL) {
		_APPS_E("apps window is NULL");
		return;
	}

	evas_object_hide(view_info.win);
	evas_object_hide(view_info.layout);
	key_unregister_cb(KEY_TYPE_BACK, _back_key_cb);
	key_unregister_cb(KEY_TYPE_HOME, _home_key_cb);
}

static Eina_Bool _apps_view_long_press_time_cb(void *data)
{
	if (mouse_info.pressed != EINA_TRUE || mouse_info.pressed_obj != data) {
		return ECORE_CALLBACK_CANCEL;
	}

	mouse_info.long_pressed = EINA_TRUE;
	apps_view_set_mode(APPS_MODE_EDIT);

	mouse_info.long_press_timer = NULL;

	return ECORE_CALLBACK_CANCEL;
}

static void _apps_view_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Down *ev = event_info;

	if (view_info.mode != APPS_MODE_NORMAL) {
		_APPS_E("Apps mode is not normal(%d)", view_info.mode);
		return;
	}

	_APPS_D("Mouse Down(%d, %d)", ev->output.x, ev->output.y);

	mouse_info.pressed = EINA_TRUE;
	mouse_info.pressed_obj = obj;

	mouse_info.down_x = mouse_info.move_x = ev->output.x;
	mouse_info.down_y = mouse_info.move_y = ev->output.y;

	if (mouse_info.long_press_timer) {
		ecore_timer_del(mouse_info.long_press_timer);
		mouse_info.long_press_timer = NULL;
	}

	mouse_info.long_press_timer = ecore_timer_add(APPS_VIEW_LONG_PRESS_TIME, _apps_view_long_press_time_cb, obj);
	if (mouse_info.long_press_timer == NULL) {
		_APPS_E("Failed to add long press timer");
	}
}

static void _apps_view_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *ev = event_info;

	if (mouse_info.pressed != EINA_TRUE || mouse_info.pressed_obj != obj) {
		return;
	}

	mouse_info.move_x = ev->cur.output.x;
	mouse_info.move_y = ev->cur.output.y;

	if (mouse_info.long_pressed != EINA_TRUE) {
		int distance = (mouse_info.move_x - mouse_info.down_x) * (mouse_info.move_x -mouse_info.down_x);
		distance += (mouse_info.move_y - mouse_info.down_y) * (mouse_info.move_y - mouse_info.down_y);

		if (distance > APPS_VIEW_MOUSE_MOVE_MIN_DISTANCE) {
			if (mouse_info.long_press_timer) {
				ecore_timer_del(mouse_info.long_press_timer);
				mouse_info.long_press_timer = NULL;
			}
			return;
		}
	}
}

static void _apps_view_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;

	if (mouse_info.pressed != EINA_TRUE || mouse_info.pressed_obj != obj) {
		return;
	}

	_APPS_D("Mouse Up(%d, %d)", ev->output.x, ev->output.y);

	mouse_info.pressed = EINA_FALSE;

	if (mouse_info.long_press_timer) {
		ecore_timer_del(mouse_info.long_press_timer);
		mouse_info.long_press_timer = NULL;
	}

	mouse_info.up_x = ev->output.x;
	mouse_info.up_y = ev->output.y;

	mouse_info.long_pressed = EINA_FALSE;
}

apps_error_e apps_view_create(void)
{
	_APPS_D("%s", __func__);

	view_info.win = _create_window(APPS_VIEW_WIN_NAME, APPS_VIEW_WIN_TITLE);
	if (view_info.win == NULL) {
		_APPS_E("Failed to create apps window");
		return APPS_ERROR_FAIL;
	}

	view_info.layout = _create_layout(view_info.win);
	if (view_info.layout == NULL) {
		_APPS_E("Failed to create apps layout");
		_destroy_window();
		return APPS_ERROR_FAIL;
	}

	view_info.apps_item_idler = ecore_idler_add(_push_items, view_info.layout);
	if (view_info.apps_item_idler == NULL) {
		_APPS_E("Failed to push app items");
	}

	evas_object_event_callback_add(view_info.layout, EVAS_CALLBACK_MOUSE_DOWN, _apps_view_down_cb, NULL);
	evas_object_event_callback_add(view_info.layout, EVAS_CALLBACK_MOUSE_MOVE, _apps_view_move_cb, NULL);
	evas_object_event_callback_add(view_info.layout, EVAS_CALLBACK_MOUSE_UP, _apps_view_up_cb, NULL);

	return APPS_ERROR_NONE;
}

void apps_view_destroy(void)
{
	_APPS_D("%s", __func__);

	evas_object_event_callback_del(view_info.layout, EVAS_CALLBACK_MOUSE_DOWN, _apps_view_down_cb);
	evas_object_event_callback_del(view_info.layout, EVAS_CALLBACK_MOUSE_MOVE, _apps_view_move_cb);
	evas_object_event_callback_del(view_info.layout, EVAS_CALLBACK_MOUSE_UP, _apps_view_up_cb);

	if (view_info.apps_item_idler) {
		ecore_idler_del(view_info.apps_item_idler);
		view_info.apps_item_idler = NULL;
	}

	_destroy_layout();
	_destroy_window();
}
