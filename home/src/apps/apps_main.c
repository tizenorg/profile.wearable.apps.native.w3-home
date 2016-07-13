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
//#include "tutorial.h"
#include "apps/bg.h"
#include "apps/db.h"
#include "apps/effect.h"
#include "apps/item_info.h"
#include "apps/lang.h"
#include "apps/layout.h"
#include "apps/list.h"
#include "apps/apps_main.h"
#include "apps/pkgmgr.h"
#include "apps/scroller.h"
#include "apps/scroller_info.h"
#include "apps/xml.h"
#include "wms.h"
#include "noti_broker.h"


#define LAYOUT_EDJE EDJEDIR"/apps_layout.edj"
#define THEME_EDJE EDJEDIR"/style.edj"
#define LAYOUT_GROUP_NAME "layout"
#define VCONFKEY_APPS_IS_FIRST_BOOT "db/private/org.tizen.w-home/apps_first_boot"

#define PRIVATE_DATA_KEY_EFFECT_ENABLE_ID "k_ef_en"
#define PRIVATE_DATA_KEY_EFFECT_POS_ID "k_ef_ps"

#define PRIVATE_DATA_KEY_FOCUS_IN_EVENT_HANDLER "k_fi_ev_hd"
#define PRIVATE_DATA_KEY_FOCUS_OUT_EVENT_HANDLER "k_fo_ev_hd"


static apps_main_s apps_main_info = {
	.instance_list = NULL,
	.theme = NULL,
	.font_theme = NULL,
	.first_boot = 0,
	.tts = 0,
	.longpress_edit_disable = false,
};



typedef struct {
	apps_error_e (*result_cb)(void *);
	void *result_data;
} cb_s;



static apps_error_e _rotate_cb(instance_info_s *info, int angle)
{
	Evas_Object *layout = NULL;

	_APPS_D("Enter _rotate_cb, angle is %d", angle);
	info->angle = angle;

	switch (angle) {
	case 0:
		_APPS_D("Portrait normal");
		info->is_rotated = 0;
		break;
	case 90:
		_APPS_D("Landscape reverse");
		info->is_rotated = 1;
		break;
	case 180:
		_APPS_D("Portrait reverse");
		info->is_rotated = 0;
		break;
	case 270:
		_APPS_D("Landscape normal");
		info->is_rotated = 1;
		break;
	default:
		_APPS_E("Cannot reach here, angle is %d", angle);
	}

	layout = evas_object_data_get(info->win, DATA_KEY_LAYOUT);
	retv_if(NULL == layout, APPS_ERROR_FAIL);
	apps_layout_rotate(layout, info->is_rotated);

	return APPS_ERROR_NONE;
}



static void _rotation_changed_cb(void *data, Evas_Object *obj, void *event)
{
	instance_info_s *info = data;
	int changed_ang = elm_win_rotation_get(info->win);

	_APPS_D("Angle: %d", changed_ang);

	if (changed_ang != info->angle) {
		_rotate_cb(info, changed_ang);
	}
}



static apps_error_e _init_layout(instance_info_s *info)
{
	Evas_Object *bg = NULL;
	int angle;

	retv_if(!info, APPS_ERROR_INVALID_PARAMETER);

	info->layout = evas_object_data_get(info->win, DATA_KEY_LAYOUT);
	retv_if(info->layout, APPS_ERROR_FAIL);

	info->layout = apps_layout_create(info->win, LAYOUT_EDJE, LAYOUT_GROUP_NAME);
	retv_if(!info->layout, APPS_ERROR_FAIL);

	evas_object_data_set(info->win, DATA_KEY_LAYOUT, info->layout);
	evas_object_resize(info->layout, info->root_w, info->root_h);
	evas_object_show(info->layout);

	if (APPS_ERROR_FAIL == apps_effect_init()) {
		_APPS_E("Failed to init effect sound");
	}

	bg = apps_bg_create(info->win, info->root_w, info->root_h);
	if (!bg) {
		_APPS_E("Cannot create bg");
	}

	angle = elm_win_rotation_get(info->win);
	_APPS_D("Angle: %d\n", angle);

	if (APPS_ERROR_NONE != _rotate_cb(info, angle)) {
		_APPS_E("Rotate error.");
	}
	evas_object_smart_callback_add(info->win, "wm,rotation,changed", _rotation_changed_cb, info);

	info->state = APPS_APP_STATE_RESET;

	return APPS_ERROR_NONE;
}



static void _destroy_layout(instance_info_s *info)
{
	evas_object_smart_callback_del(info->win, "wm,rotation,changed", _rotation_changed_cb);

	apps_bg_destroy(info->win);

	apps_effect_fini();
	evas_object_data_del(info->win, DATA_KEY_LAYOUT);
	if (info->layout) apps_layout_destroy(info->layout);
}


static void _window_focus_in_cb(void *data, Evas *e, void *event_info)
{
	_APPS_D("focus in");
	noti_broker_event_fire_to_plugin(EVENT_SOURCE_VIEW, EVENT_TYPE_APPS_SHOW, NULL);
	apps_main_resume();
}



static void _window_focus_out_cb(void *data, Evas *e, void *event_info)
{
	_APPS_D("focus out");
	noti_broker_event_fire_to_plugin(EVENT_SOURCE_VIEW, EVENT_TYPE_APPS_HIDE, NULL);
	apps_main_pause();
}



#define ROTATION_TYPE_NUMBER 1
static apps_error_e _init_app_win(instance_info_s *info, const char *name, const char *title)
{
	retv_if(!name, APPS_ERROR_INVALID_PARAMETER);
	retv_if(!title, APPS_ERROR_INVALID_PARAMETER);

	/* Open GL backend */
	elm_config_accel_preference_set("opengl");
	info->win = elm_win_add(NULL, name, ELM_WIN_BASIC);
	retv_if(!info->win, APPS_ERROR_FAIL);
	elm_win_screen_size_get(info->win, NULL, NULL, &info->root_w, &info->root_h);
	apps_main_info.instance_list = eina_list_append(apps_main_info.instance_list, info);

	elm_win_alpha_set(info->win, EINA_FALSE); // This order is important
	elm_win_role_set(info->win, "no-effect");

	if (elm_win_wm_rotation_supported_get(info->win)) {
		const int rots[ROTATION_TYPE_NUMBER] = {0};
		elm_win_wm_rotation_available_rotations_set(info->win, rots, ROTATION_TYPE_NUMBER);
	}

	evas_object_color_set(info->win, 255, 0, 0, 0);
	evas_object_resize(info->win, info->root_w, info->root_h);
	evas_object_hide(info->win);
	evas_object_data_set(info->win, DATA_KEY_INSTANCE_INFO, info);

	elm_win_title_set(info->win, title);
	elm_win_borderless_set(info->win, EINA_TRUE);

	info->e = evas_object_evas_get(info->win);
	if (!info->e) {
		_APPS_E("[%s] Failed to get the e object", __func__);
	}

	info->ee = ecore_evas_ecore_evas_get(info->e);
	if (!info->ee) {
		_APPS_E("[%s] Failed to get ecore_evas object", __func__);
	}

	evas_event_callback_add(info->e, EVAS_CALLBACK_CANVAS_FOCUS_IN, _window_focus_in_cb, info);
	evas_event_callback_add(info->e, EVAS_CALLBACK_CANVAS_FOCUS_OUT, _window_focus_out_cb, info);

	return APPS_ERROR_NONE;
}



static void _destroy_app_win(instance_info_s *info)
{
	ret_if(!info);

	evas_object_data_del(info->win, DATA_KEY_INSTANCE_INFO);

	Ecore_Event_Handler *handler = evas_object_data_del(info->win, PRIVATE_DATA_KEY_FOCUS_IN_EVENT_HANDLER);
	if (handler) ecore_event_handler_del(handler);

	handler = evas_object_data_del(info->win, PRIVATE_DATA_KEY_FOCUS_OUT_EVENT_HANDLER);
	if (handler) ecore_event_handler_del(handler);


	evas_object_del(info->win);
	info->win = NULL;
	apps_main_info.instance_list = eina_list_remove(apps_main_info.instance_list, info);
}



HAPI apps_main_s *apps_main_get_info(void)
{
	return &apps_main_info;
}


HAPI apps_error_e apps_main_register_cb(
	instance_info_s *info,
	int state,
	apps_error_e (*result_cb)(void *), void *result_data)
{
	cb_s *cb = NULL;

	retv_if(NULL == result_cb, APPS_ERROR_INVALID_PARAMETER);

	cb = calloc(1, sizeof(cb_s));
	retv_if(NULL == cb, APPS_ERROR_FAIL);

	cb->result_cb = result_cb;
	cb->result_data = result_data;

	info->cbs_list[state] = eina_list_append(info->cbs_list[state], cb);
	retv_if(NULL == info->cbs_list[state], APPS_ERROR_FAIL);

	return APPS_ERROR_NONE;
}



HAPI void apps_main_unregister_cb(
	instance_info_s *info,
	int state,
	apps_error_e (*result_cb)(void *))
{
	const Eina_List *l;
	const Eina_List *n;
	cb_s *cb;
	EINA_LIST_FOREACH_SAFE(info->cbs_list[state], l, n, cb) {
		continue_if(NULL == cb);
		if (result_cb != cb->result_cb) continue;
		info->cbs_list[state] = eina_list_remove(info->cbs_list[state], cb);
		free(cb);
		return;
	}
}



static void _execute_cbs(instance_info_s *info, int state)
{
	const Eina_List *l;
	const Eina_List *n;
	cb_s *cb;
	EINA_LIST_FOREACH_SAFE(info->cbs_list[state], l, n, cb) {
		continue_if(NULL == cb);
		continue_if(NULL == cb->result_cb);

		if (APPS_ERROR_NONE != cb->result_cb(cb->result_data)) _APPS_E("There are some problems");
	}
}


HAPI void apps_main_init()
{
	_APPS_D("APPS INIT");

	/* Data set */
	instance_info_s *info = NULL;
	if(eina_list_count(apps_main_info.instance_list) == 0) {
		info = calloc(1, sizeof(instance_info_s));
		ret_if(NULL == info);
	} else {
		_APPS_D("apps already created");
		return;
	}

	apps_main_info.theme = elm_theme_new();
	elm_theme_ref_set(apps_main_info.theme, NULL);
	elm_theme_extension_add(apps_main_info.theme, THEME_EDJE);

	info->state = APPS_APP_STATE_CREATE;

	appcore_measure_start();

	apps_pkgmgr_init();

	if (vconf_get_int(VCONFKEY_APPS_IS_FIRST_BOOT, &apps_main_info.first_boot) < 0) {
		_APPS_E("Cannot get the vconf for %s", VCONFKEY_APPS_IS_FIRST_BOOT);
	}
	vconf_set_int(VCONFKEY_APPS_IS_FIRST_BOOT, 0);

	apps_main_info.scale = elm_config_scale_get();

	if (vconf_get_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, &apps_main_info.tts) < 0) {
		_APPS_E("Cannot get the vconf for %s", VCONFKEY_SETAPPL_ACCESSIBILITY_TTS);
	}

	apps_main_info.longpress_edit_disable = true;

	_execute_cbs(info, APPS_APP_STATE_CREATE);
	appcore_set_i18n(PROJECT, LOCALEDIR);

	ret_if(APPS_ERROR_FAIL == _init_app_win(info, "__APPS__", "__APPS__"));
	ret_if(APPS_ERROR_FAIL == _init_layout(info));

	return;
}

HAPI void apps_main_fini()
{
	instance_info_s *info = eina_list_nth(apps_main_get_info()->instance_list, 0);
	ret_if(NULL == info);

	elm_theme_extension_del(apps_main_info.theme, THEME_EDJE);
	elm_theme_free(apps_main_info.theme);

	info->state = APPS_APP_STATE_TERMINATE;
	_execute_cbs(info, APPS_APP_STATE_TERMINATE);

	_destroy_layout(info);
	_destroy_app_win(info);
}


HAPI void apps_main_launch(int launch_type)
{
	_APPS_D("APPS LAUNCH");

	if(eina_list_count(apps_main_info.instance_list) == 0) {
		apps_main_init();
	}

	instance_info_s *info = eina_list_nth(apps_main_get_info()->instance_list, 0);
	ret_if(!info);

	if (launch_type == APPS_LAUNCH_EDIT) {
		_APPS_D("Edit the Apps");

		apps_layout_edit(info->layout);

		if (APPS_ERROR_FAIL == apps_layout_show(info->win, EINA_TRUE)) {
			_APPS_E("Cannot show tray");
		}
	} else if (launch_type == APPS_LAUNCH_HIDE) {
		_APPS_D("Hide the Apps");
		if (info->win) {
			_APPS_D("There is a window already");
			apps_layout_show(info->win, EINA_FALSE);
			return;
		}

		/* Standalone */
		if (APPS_ERROR_FAIL == apps_layout_show(info->win, EINA_FALSE)) {
			_APPS_E("Cannot show tray");
		}
	} else if (launch_type == APPS_LAUNCH_SHOW) {
		int initial_popup;
		_APPS_D("Show the Apps");
		if (vconf_get_int(VCONFKEY_APPS_IS_INITIAL_POPUP, &initial_popup) < 0) {
			_APPS_E("Cannot get the vconf for %s", VCONFKEY_APPS_IS_INITIAL_POPUP);
			initial_popup = 1;
		}

		if (info->win) {
			_APPS_D("There is a window already");
			apps_layout_show(info->win, EINA_TRUE);
			return;
		}

		/* Standalone */
		if (APPS_ERROR_FAIL == apps_layout_show(info->win, EINA_TRUE)) {
			_APPS_E("Cannot show tray");
		}

	}

	_execute_cbs(info, APPS_APP_STATE_RESET);
}


HAPI void apps_main_pause()
{
	if(eina_list_count(apps_main_info.instance_list) == 0) {
		_APPS_E("Pause starts");
		return;
	}

	instance_info_s *info = eina_list_nth(apps_main_get_info()->instance_list, 0);
	ret_if(!info);

	_APPS_D("Pause starts");
	if(info->state == APP_STATE_PAUSE) {
		_APPS_E("paused already");
		return;
	}

	if (apps_layout_is_edited(info->layout)) {
		apps_layout_unedit(info->layout);
	}

	info->state = APPS_APP_STATE_PAUSE;
	_execute_cbs(info, APPS_APP_STATE_PAUSE);


	int isOrderChange = (int)evas_object_data_get(info->win, DATA_KEY_IS_ORDER_CHANGE);
	if(isOrderChange) {
		wms_change_apps_order(W_HOME_WMS_BACKUP);
		evas_object_data_set(info->win, DATA_KEY_IS_ORDER_CHANGE, (void*)0);
	}

	_APPS_D("Pause done");

	return;
}



HAPI void apps_main_resume()
{
	if(eina_list_count(apps_main_info.instance_list) == 0) {
		_APPS_E("Resume starts");
		return;
	}

	instance_info_s *info = eina_list_nth(apps_main_get_info()->instance_list, 0);
	ret_if(!info);

	_APPS_D("Resume starts");
	if(info->state == APP_STATE_RESUME) {
		_APPS_E("resumed already");
		return;
	}

	info->state = APPS_APP_STATE_RESUME;
	_execute_cbs(info, APPS_APP_STATE_RESUME);

	return;
}


HAPI void apps_main_language_chnage()
{
	const Eina_List *l, *ln;
	char *lang = NULL;
	instance_info_s *info;

	_APPS_D("Language is changed");

	EINA_LIST_FOREACH_SAFE(apps_main_info.instance_list, l, ln, info) {
		Evas_Object *scroller = NULL;
		scroller_info_s *scroller_info = NULL;

		continue_if(!info);
		continue_if(!info->layout);

		scroller = evas_object_data_get(info->layout, DATA_KEY_SCROLLER);
		continue_if(!scroller);

		scroller_info = evas_object_data_get(scroller, DATA_KEY_SCROLLER_INFO);
		continue_if(!scroller_info);

		apps_item_info_list_change_language(scroller_info->list);
		apps_scroller_change_language(scroller);
	}

	lang = vconf_get_str(VCONFKEY_LANGSET);
	if (!lang) {
		return;
	}
	elm_language_set(lang);
	free(lang);

	return;
}



HAPI Eina_Bool apps_main_is_visible()
{
	instance_info_s *info = eina_list_nth(apps_main_get_info()->instance_list, 0);
	retv_if(NULL == info, EINA_FALSE);

	Eina_Bool bShow = evas_object_visible_get(info->win);
	_APPS_D("bShow:[%d]", bShow);

	return bShow;
}

#define APPS_ORDER_XML_PATH	DATADIR"/apps_order.xml"
HAPI void apps_main_list_backup()
{
	instance_info_s *info = eina_list_nth(apps_main_get_info()->instance_list, 0);
	ret_if(!info);

	Evas_Object *layout = evas_object_data_get(info->win, DATA_KEY_LAYOUT);
	ret_if(!layout);

	Evas_Object *scroller = evas_object_data_get(layout, DATA_KEY_SCROLLER);
	ret_if(!scroller);

	apps_scroller_write_list(scroller);

	scroller_info_s *scroller_info = evas_object_data_get(scroller, DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	if(APPS_ERROR_NONE != apps_db_read_list(scroller_info->list)) {
		_APPS_E("");
	}

	if(APPS_ERROR_NONE != apps_xml_read_list(APPS_ORDER_XML_PATH, scroller_info->list)) {
		_APPS_E("");
	}
}


static int _sort_by_order_cb(const void *d1, const void *d2)
{
	const item_info_s *v1 = d1;
	const item_info_s *v2 = d2;

	if (v1->ordering < 0) return 1;
	else if (v2->ordering < 0) return -1;

	if(v1->ordering > v2->ordering)
		return 1;
	else if(v1->ordering < v2->ordering)
		return -1;
	else
		return 0;
}


HAPI void apps_main_list_restore()
{
	instance_info_s *info = eina_list_nth(apps_main_get_info()->instance_list, 0);
	ret_if(!info);

	Evas_Object *layout = evas_object_data_get(info->win, DATA_KEY_LAYOUT);
	ret_if(!layout);

	if(apps_layout_is_edited(layout)) {
		_APPS_E("Apps is editing");
		return;
	}

	Evas_Object *scroller = evas_object_data_get(layout, DATA_KEY_SCROLLER);
	ret_if(!scroller);

	Eina_List *restore_list = apps_xml_write_list(APPS_ORDER_XML_PATH);
	ret_if(!restore_list);

	restore_list = eina_list_sort(restore_list, eina_list_count(restore_list), _sort_by_order_cb);
	ret_if(!restore_list);

	if(APPS_ERROR_NONE != apps_scroller_read_list(scroller, restore_list)) {
		_APPS_E("");
	}

	apps_scroller_write_list(scroller);

	scroller_info_s *scroller_info = evas_object_data_get(scroller, DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	Eina_List *layout_list = evas_object_data_get(layout, DATA_KEY_LIST);
	eina_list_free(layout_list);
	layout_list = eina_list_clone(scroller_info->list);
	evas_object_data_set(layout, DATA_KEY_LIST, layout_list);

	if(APPS_ERROR_NONE != apps_db_read_list(scroller_info->list)) {
		_APPS_E("");
	}

	if(APPS_ERROR_NONE != apps_xml_read_list(APPS_ORDER_XML_PATH, scroller_info->list)) {
		_APPS_E("");
	}
}


HAPI void apps_main_list_reset()
{
	instance_info_s *info = eina_list_nth(apps_main_get_info()->instance_list, 0);
	ret_if(!info);

	Evas_Object *layout = evas_object_data_get(info->win, DATA_KEY_LAYOUT);
	ret_if(!layout);

	if(apps_layout_is_edited(layout)) {
		_APPS_E("Apps is editing");
		return;
	}

	Evas_Object *scroller = evas_object_data_get(layout, DATA_KEY_SCROLLER);
	ret_if(!scroller);

	Eina_List *reset_list = apps_item_info_list_create(ITEM_INFO_LIST_TYPE_XML);
	ret_if(!reset_list);

	if(APPS_ERROR_NONE != apps_scroller_read_list(scroller, reset_list)) {
		_APPS_E("");
	}

	apps_scroller_write_list(scroller);

	scroller_info_s *scroller_info = evas_object_data_get(scroller, DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	Eina_List *layout_list = evas_object_data_get(layout, DATA_KEY_LIST);
	eina_list_free(layout_list);
	layout_list = eina_list_clone(scroller_info->list);
	evas_object_data_set(layout, DATA_KEY_LIST, layout_list);

	if(APPS_ERROR_NONE != apps_db_read_list(scroller_info->list)) {
		_APPS_E("");
	}

	if(APPS_ERROR_NONE != apps_xml_read_list(APPS_ORDER_XML_PATH, scroller_info->list)) {
		_APPS_E("");
	}
}


HAPI void apps_main_list_tts(int is_tts)
{
	_D("tts is [%d]", is_tts);

	apps_main_get_info()->tts = is_tts;
}


#define VCONKEY_HOME_APPS_SHOW_COUNT "db/private/org.tizen.w-home/apps_flickup_count"
HAPI void apps_main_show_count_add(void)
{
	int val = 0;

	if (vconf_get_int(VCONKEY_HOME_APPS_SHOW_COUNT, &val) == 0) {
		val++;
		if (vconf_set_int(VCONKEY_HOME_APPS_SHOW_COUNT, val) != 0) {
			_E("Failed to set apps show count vconf");
		}
	} else {
		_E("Failed to get apps show count vconf");
	}
}


HAPI int apps_main_show_count_get(void)
{
	int val = 0, ret = 99;

	if (vconf_get_int(VCONKEY_HOME_APPS_SHOW_COUNT, &val) == 0) {
		ret = val;
	} else {
		_E("Failed to get apps show count vconf");
	}

	return ret;
}
// End of a file
