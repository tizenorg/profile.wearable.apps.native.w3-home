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

#include <app.h>
#include <appcore-efl.h>
#include <aul.h>
#include <bundle.h>
#include <efl_assist.h>
#include <Elementary.h>
#include <sys/types.h>
#include <system_settings.h>
#include <unistd.h>
#include <vconf.h>
#include <Evas.h>

#include "log.h"
#include "util.h"
#include "apps/apps_main.h"
#include "apps/layout.h"
#include "wms.h"
#include "noti_broker.h"

#define LAYOUT_EDJE EDJEDIR"/apps_layout.edj"
#define THEME_EDJE EDJEDIR"/style.edj"
#define LAYOUT_GROUP_NAME "layout"

static apps_main_s apps_main_info = {
	.theme = NULL,
	.font_theme = NULL,
	.longpress_edit_disable = false,
	.state = APPS_APP_STATE_POWER_OFF,
	.mode = APPS_MODE_NORMAL,
	.e = NULL,
	.win = NULL,
	.layout = NULL,
	.root_w = 0,
	.root_h = 0,
};

typedef struct {
	apps_error_e (*result_cb)(void *);
	void *result_data;
} cb_s;



HAPI apps_main_s *apps_main_get_info(void)
{
	return &apps_main_info;
}

HAPI void apps_main_mode_set(int mode)
{
	_APPS_D("Set Apps mode : %d", mode);

	apps_main_info.mode = mode;
}

HAPI int apps_main_mode_get(void)
{
	return apps_main_info.mode;
}

static apps_error_e _rotate_cb(int angle)
{
	_APPS_D("Enter _rotate_cb, angle is %d", angle);
	apps_main_info.angle = angle;

	if (apps_main_info.layout == NULL) {
		_APPS_E("apps layout is NULL");
		return APPS_ERROR_FAIL;
	}

	switch(angle) {
	case 0:
		_APPS_D("Portrait normal");
		apps_main_info.is_rotated = 0;
		break;
	case 90:
		_APPS_D("Landscape reverse");
		apps_main_info.is_rotated = 1;
		break;
	case 180:
		_APPS_D("Portrait reverse");
		apps_main_info.is_rotated = 0;
		break;
	case 270:
		_APPS_D("Landscape normal");
		apps_main_info.is_rotated = 1;
		break;
	default:
		_APPS_E("Cannot reach here, angle is %d", angle);
		break;
	}

	apps_layout_rotate(apps_main_info.layout, apps_main_info.is_rotated);

	return APPS_ERROR_NONE;
}

static void _rotation_changed_cb(void *data, Evas_Object *obj, void *event)
{
	int changed_angle = 0;

	if (apps_main_info.win == NULL) {
		_APPS_E("window is NULL");
		return;
	}

	changed_angle = elm_win_rotation_get(apps_main_info.win);
	_APPS_D("Angle : %d", changed_angle);

	if (changed_angle != apps_main_info.angle) {
		_rotate_cb(changed_angle);
	}
}

static apps_error_e _init_apps_layout(void)
{
	Evas_Object *bg = NULL;
	int angle = 0;

	_APPS_D("%s", __func__);

	if (apps_main_info.layout != NULL) {
		_APPS_E("apps layout is already exist");
		return APPS_ERROR_FAIL;
	}

	apps_main_info.layout = apps_layout_create(apps_main_info.win, LAYOUT_EDJE, LAYOUT_GROUP_NAME);
	if (apps_main_info.layout == NULL) {
		_APPS_E("Failed to create apps layout");
		return APPS_ERROR_FAIL;
	}

#if TBD
	if (APPS_ERROR_NONE != apps_effect_init()) {
		_APPS_E("Failed to init effect sound");
	}

	bg = apps_bg_create(apps_main_info.win, apps_main_info.root_w, apps_main_info.root_h);
	if (bg == NULL) {
		_APPS_E("Failed to create bg");
	}
#endif

	angle = elm_win_rotation_get(apps_main_info.win);
	_APPS_D("Angle : %d", angle);

	if (APPS_ERROR_NONE != _rotate_cb(angle)) {
		_APPS_E("Rotate error");
	}
	evas_object_smart_callback_add(apps_main_info.win, "wm,rotation,changed", _rotation_changed_cb, NULL);

	apps_main_info.state = APPS_APP_STATE_RESET;

	return APPS_ERROR_NONE;
}

static void _destroy_apps_layout(void)
{
	if (apps_main_info.win) {
		evas_object_smart_callback_del(apps_main_info.win, "wm,rotation,changed", _rotation_changed_cb);
#if TBD
		apps_bg_destroy(apps_main_info.win);
#endif
	}

#if TBD
	apps_effect_fini();
#endif

	apps_layout_destroy();
}

static void _window_focus_in_cb(void *data, Evas *e, void *event_info)
{
	_APPS_D("%s", __func__);
	apps_main_resume();
}

static void _window_focus_out_cb(void *data, Evas *e, void *event_info)
{
	_APPS_D("%s", __func__);
	apps_main_pause();
}

#define ROTATION_TYPE_NUMBER 1
static apps_error_e _init_apps_win(const char *name, const char *title)
{
	Eina_Bool ret = EINA_FALSE;

	_APPS_D("%s", __func__);

	if (name == NULL) {
		_APPS_E("name is NULL");
		return APPS_ERROR_INVALID_PARAMETER;
	}

	if (title == NULL) {
		_APPS_E("title is NULL");
		return APPS_ERROR_INVALID_PARAMETER;
	}

	/*
	 * Open GL backend
	 */
	elm_config_accel_preference_set("opengl");

	apps_main_info.win = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (apps_main_info.win == NULL) {
		_APPS_E("Failed to add apps window");
		return APPS_ERROR_FAIL;
	}

	elm_win_screen_size_get(apps_main_info.win, NULL, NULL, &apps_main_info.root_w, &apps_main_info.root_h);
	elm_win_alpha_set(apps_main_info.win, EINA_FALSE);
	elm_win_role_set(apps_main_info.win, "no-effect");

	ret = elm_win_wm_rotation_supported_get(apps_main_info.win);
	if (ret == EINA_TRUE) {
		const int rots[ROTATION_TYPE_NUMBER] = { 0 };
		elm_win_wm_rotation_available_rotations_set(apps_main_info.win, rots, ROTATION_TYPE_NUMBER);
	}

	evas_object_color_set(apps_main_info.win, 0, 0, 0, 0);
	evas_object_resize(apps_main_info.win, apps_main_info.root_w, apps_main_info.root_h);
	evas_object_hide(apps_main_info.win);

	elm_win_title_set(apps_main_info.win, title);
	elm_win_borderless_set(apps_main_info.win, EINA_TRUE);

	apps_main_info.e = evas_object_evas_get(apps_main_info.win);
	if (apps_main_info.e == NULL) {
		_APPS_E("Failed to get Evas");
		evas_object_del(apps_main_info.win);
		apps_main_info.win = NULL;
		return APPS_ERROR_FAIL;
	}

	evas_event_callback_add(apps_main_info.e, EVAS_CALLBACK_CANVAS_FOCUS_IN, _window_focus_in_cb, NULL);
	evas_event_callback_add(apps_main_info.e, EVAS_CALLBACK_CANVAS_FOCUS_OUT, _window_focus_out_cb, NULL);

	return APPS_ERROR_NONE;
}

static void _destroy_apps_win(void)
{
	evas_event_callback_del(apps_main_info.e, EVAS_CALLBACK_CANVAS_FOCUS_IN, _window_focus_in_cb);
	evas_event_callback_del(apps_main_info.e, EVAS_CALLBACK_CANVAS_FOCUS_OUT, _window_focus_out_cb);

	evas_object_del(apps_main_info.win);
	apps_main_info.win = NULL;
}

HAPI apps_error_e apps_main_register_cb(
	int state,
	apps_error_e (*result_cb)(void *), void *result_data)
{
	cb_s *cb = NULL;

	if (result_cb == NULL) {
		_APPS_E("result_cb is NULL");
		return APPS_ERROR_INVALID_PARAMETER;
	}

	cb = calloc(1, sizeof(cb_s));
	if (cb == NULL) {
		_APPS_E("Failed to calloc for cb struct");
		return APPS_ERROR_FAIL;
	}

	cb->result_cb = result_cb;
	cb->result_data = result_data;

	apps_main_info.cbs_list[state] = eina_list_append(apps_main_info.cbs_list[state], cb);
	if (apps_main_info.cbs_list[state] == NULL) {
		_APPS_E("Failed to append callback to list");
		return APPS_ERROR_FAIL;
	}

	return APPS_ERROR_NONE;
}

HAPI void apps_main_unregister_cb(
	int state,
	apps_error_e (*result_cb)(void *))
{
	const Eina_List *l;
	const Eina_List *n;
	cb_s *cb;

	EINA_LIST_FOREACH_SAFE(apps_main_info.cbs_list[state], l, n, cb) {
		if (cb == NULL) {
			continue;
		}

		if (result_cb != cb->result_cb) {
			continue;
		}

		apps_main_info.cbs_list[state] = eina_list_remove(apps_main_info.cbs_list[state], cb);
		free(cb);
		return;
	}
}

HAPI apps_error_e apps_main_init(void)
{
	int ret = 0;

	_APPS_D("%s", __func__);

	apps_main_info.state = APPS_APP_STATE_CREATE;
	apps_main_mode_set(APPS_MODE_NORMAL);

	apps_main_info.theme = elm_theme_new();
	elm_theme_ref_set(apps_main_info.theme, NULL);
	elm_theme_extension_add(apps_main_info.theme, THEME_EDJE);

	apps_main_info.longpress_edit_disable = true;
	appcore_set_i18n(PROJECT, LOCALEDIR);

	ret = _init_apps_win("__APPS__", "__APPS__");
	if (APPS_ERROR_NONE != ret) {
		_APPS_E("Failed to initiailze apps window");
		return APPS_ERROR_FAIL;
	}

	ret = _init_apps_layout();
	if (APPS_ERROR_NONE != ret) {
		_APPS_E("Failed to initialize apps layout");
		_destroy_apps_win();
		return APPS_ERROR_FAIL;
	}

	return APPS_ERROR_NONE;
}

HAPI void apps_main_fini(void)
{
	_APPS_D("%s", __func__);

	elm_theme_extension_del(apps_main_info.theme, THEME_EDJE);
	elm_theme_free(apps_main_info.theme);
	apps_main_info.theme = NULL;

	_destroy_apps_layout();
	_destroy_apps_win();

	apps_main_info.state = APPS_APP_STATE_TERMINATE;
	apps_main_mode_set(APPS_MODE_NORMAL);
}

HAPI void apps_main_launch(int launch_type)
{
	if (apps_main_info.win == NULL) {
		int ret = 0;

		ret = apps_main_init();
		if (APPS_ERROR_NONE != ret) {
			_APPS_E("Failed to initialize apps");
			return;
		}
	}

	_APPS_D("launch type : %d", launch_type);

	if (launch_type == APPS_LAUNCH_SHOW) {
		_APPS_D("Show Apps");
		apps_layout_show();
	} else if (launch_type == APPS_LAUNCH_HIDE) {
		_APPS_D("Hide Apps");
		apps_layout_hide();
	}
}


HAPI void apps_main_pause(void)
{
	_APPS_D("%s", __func__);

	apps_main_info.state = APPS_APP_STATE_PAUSE;
	apps_main_mode_set(APPS_MODE_NORMAL);
}

HAPI void apps_main_resume(void)
{
	_APPS_D("%s", __func__);

	apps_main_info.state = APPS_APP_STATE_RESUME;
	apps_main_mode_set(APPS_MODE_NORMAL);
}

HAPI void apps_main_language_chnage(void)
{
	_APPS_D("%s", __func__);
}

HAPI Eina_Bool apps_main_is_visible()
{
	Eina_Bool is_visible = EINA_FALSE;

	is_visible = evas_object_visible_get(apps_main_info.win);
	_APPS_D("apps window visibility : %d", is_visible);

	return is_visible;
}

#define APPS_ORDER_XML_PATH	DATADIR"/apps_order.xml"
HAPI void apps_main_list_backup(void)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_main_list_restore(void)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_main_list_reset(void)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_main_list_tts(int is_tts)
{
	_APPS_D("%s", __func__);
}

// End of a file
