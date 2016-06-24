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
#include <app_control.h>
#include <efl_extension.h>
#include <app.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "log.h"
#include "util.h"
#include "apps/apps_conf.h"
#include "apps/effect.h"
#include "apps/item.h"
#include "apps/item_info.h"
#include "apps/layout.h"
#include "apps/apps_main.h"
#include "apps/page.h"
#include "apps/rotary.h"
#include "apps/rotary_info.h"
#include "wms.h"

#ifndef VCONFKEY_SETAPPL_AUTO_OPEN_APPS
#define VCONFKEY_SETAPPL_AUTO_OPEN_APPS				"db/setting/auto_open_apps"
#endif

#define ROTARY_DATA_KEY_TIMER "r_timer"
#define ROTARY_DATA_KEY_RUNNING "r_running"
#define DELAY_TIME 2.0f



static void _item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Eext_Object_Item *item = NULL;
	app_control_h app_control = NULL;
	const char *appid;
	int ret;

	ret_if(!obj);

	item = eext_rotary_selector_selected_item_get(obj);

	appid = eext_rotary_selector_item_part_text_get(item, "selector,sub_text");

	_APPS_D("item(%s) clicked", appid);

	ret = app_control_create(&app_control);
	ret_if(ret != APP_CONTROL_ERROR_NONE);

	ret = app_control_set_app_id(app_control, appid);
	goto_if(ret != APP_CONTROL_ERROR_NONE, OUT);

	ret = app_control_send_launch_request(app_control, NULL, NULL);
	goto_if(ret != APP_CONTROL_ERROR_NONE, OUT);

OUT:
	app_control_destroy(app_control);

	return;

}



static Eina_Bool _selected_timer_cb(void *data)
{
	Evas_Object *obj = data;
	int is_auto_open_apps = 0;


	retv_if(!obj, ECORE_CALLBACK_CANCEL);

	if (vconf_get_bool(VCONFKEY_SETAPPL_AUTO_OPEN_APPS, &is_auto_open_apps) < 0) {
		_APPS_E("Fail to get the vconf key");
		is_auto_open_apps = 0;
	}

	if (is_auto_open_apps) {
		_APPS_D("Item auto Selected");
		_item_clicked_cb(NULL, obj, NULL);
	}

	evas_object_data_del(obj, ROTARY_DATA_KEY_TIMER);
	evas_object_data_del(obj, ROTARY_DATA_KEY_RUNNING);

	return ECORE_CALLBACK_CANCEL;
}



static void _item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ecore_Timer *timer = NULL;

	timer = evas_object_data_get(obj, ROTARY_DATA_KEY_TIMER);

	if (timer) {
		int is_running = evas_object_data_get(obj, ROTARY_DATA_KEY_RUNNING);

		if (!is_running) {
			ecore_timer_del(timer);
			timer = NULL;
		}
	} else {
		timer = ecore_timer_add(DELAY_TIME, _selected_timer_cb, obj);
		evas_object_data_set(obj, ROTARY_DATA_KEY_TIMER, timer);
		evas_object_data_set(obj, ROTARY_DATA_KEY_RUNNING, (void *) 1);
	}
}



static w_home_error_e _resume_rotary_result_cb(void *data)
{
	Evas_Object *rotary_selector = data;
	eext_rotary_object_event_activated_set(rotary_selector, EINA_TRUE);
	return W_HOME_ERROR_NONE;
}



static w_home_error_e _pause_rotary_result_cb(void *data)
{
	Evas_Object *rotary_selector = data;
	eext_rotary_object_event_activated_set(rotary_selector, EINA_FALSE);
	return W_HOME_ERROR_NONE;
}



HAPI void apps_rotary_append_item(Evas_Object *rotary, item_info_s *item_info)
{
	Evas_Object *icon = NULL;
	Eext_Object_Item *item = NULL;
	char *name = NULL;
	char *appid = NULL;

	ret_if(!rotary);
	ret_if(!item_info);

	item = eext_rotary_selector_item_append(rotary);
	ret_if(!item);

	icon = elm_image_add(rotary);
	ret_if(!icon);

	if (item_info->icon) {
		elm_image_file_set(icon, item_info->icon, NULL);
		eext_rotary_selector_item_part_content_set(item, "item,bg_image", EEXT_ROTARY_SELECTOR_ITEM_STATE_NORMAL, icon);
	}

	if (item_info->name) {
		name = strdup(item_info->name);
		eext_rotary_selector_item_part_text_set(item, "selector,main_text", name);
	}

	if (item_info->appid) {
		appid = strdup(item_info->appid);
		eext_rotary_selector_item_part_text_set(item, "selector,sub_text", appid);
	}
}



HAPI void apps_rotary_destroy(Evas_Object *layout)
{
	Evas_Object *rotary = NULL;
	rotary_info_s *rotary_info = NULL;

	ret_if(!layout);

	rotary = elm_object_part_content_unset(layout, "scroller");
	ret_if(!rotary);

	rotary_info = evas_object_data_get(rotary, DATA_KEY_ROTARY_INFO);
	ret_if(!rotary_info);

	apps_main_unregister_cb(rotary_info->instance_info, APPS_APP_STATE_RESUME, _resume_rotary_result_cb);
	apps_main_unregister_cb(rotary_info->instance_info, APPS_APP_STATE_PAUSE, _pause_rotary_result_cb);
	evas_object_smart_callback_del(rotary, "item,selected", _item_selected_cb);
	evas_object_smart_callback_del(rotary, "item,clicked", _item_clicked_cb);

	free(rotary_info);
	evas_object_del(rotary);
}



static Eina_Bool _init_timer_cb(void *data)
{
	Evas_Object *rotary = data;
	Eina_List *list = NULL;
	rotary_info_s *rotary_info = NULL;
	item_info_s *item_info = NULL;
	int index, count;

	retv_if(!rotary, ECORE_CALLBACK_CANCEL);

	rotary_info = evas_object_data_get(rotary, DATA_KEY_ROTARY_INFO);
	retv_if(!rotary_info, ECORE_CALLBACK_CANCEL);

	list = evas_object_data_get(rotary_info->layout, DATA_KEY_LIST);
	retv_if(!list, ECORE_CALLBACK_CANCEL);

	index = (int) evas_object_data_get(rotary, DATA_KEY_LIST_INDEX);
	count = eina_list_count(list);
	if (index == count) goto OUT;

	item_info = eina_list_nth(list, index);
	goto_if(!item_info, OUT);

	item_info->ordering = index;
	apps_rotary_append_item(rotary, item_info);

	index ++;
	if (index == count) goto OUT;
	evas_object_data_set(rotary, DATA_KEY_LIST_INDEX, (void *) index);

	return ECORE_CALLBACK_RENEW;

OUT:
	_APPS_D("Loading apps is done");

	/* FIXME : Bring into the top */

	evas_object_data_del(rotary, DATA_KEY_LIST_INDEX);
	evas_object_data_del(rotary, DATA_KEY_TIMER);

	index = 0;

	return ECORE_CALLBACK_CANCEL;
}



HAPI Evas_Object *apps_rotary_create(Evas_Object *layout)
{
	Evas_Object *win = NULL;
	Evas_Object *rotary_selector = NULL;
	Ecore_Timer *timer = NULL;
	instance_info_s *instance_info = NULL;
	rotary_info_s *rotary_info = NULL;

	win = evas_object_data_get(layout, DATA_KEY_WIN);
	retv_if(!win, NULL);

	instance_info = evas_object_data_get(win, DATA_KEY_INSTANCE_INFO);
	retv_if(!instance_info, NULL);

	rotary_selector = eext_rotary_selector_add(win);
	retv_if(!rotary_selector, NULL);

	eext_rotary_object_event_activated_set(rotary_selector, EINA_TRUE);

	evas_object_smart_callback_add(rotary_selector, "item,selected", _item_selected_cb, NULL);
	evas_object_smart_callback_add(rotary_selector, "item,clicked", _item_clicked_cb, NULL);

	evas_object_show(rotary_selector);

	timer = ecore_timer_add(0.01f, _init_timer_cb, rotary_selector);
	if (!timer) {
		_APPS_E("Cannot add a timer");
		apps_rotary_destroy(layout);
		return NULL;
	}

	rotary_info = calloc(1, sizeof(rotary_info_s));
	if (!rotary_info) {
		_APPS_E("Cannot calloc for the rotary_info");
		apps_rotary_destroy(layout);
		return NULL;
	}

	rotary_info->win = win;
	rotary_info->instance_info = instance_info;
	rotary_info->layout = layout;

	evas_object_data_set(rotary_selector, DATA_KEY_ROTARY_INFO, rotary_info);

	if (W_HOME_ERROR_NONE != apps_main_register_cb(instance_info, APPS_APP_STATE_RESUME, _resume_rotary_result_cb, rotary_selector)) {
		_APPS_E("Cannot register the pause callback");
	}

	if (W_HOME_ERROR_NONE != apps_main_register_cb(instance_info, APPS_APP_STATE_PAUSE, _pause_rotary_result_cb, rotary_selector)) {
		_APPS_E("Cannot register the pause callback");
	}
	return rotary_selector;
}



