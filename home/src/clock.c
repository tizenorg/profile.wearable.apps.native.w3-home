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
#include <watch_control.h>
#include <vconf.h>
#include <pkgmgr-info.h>
#include <pkgmgrinfo_type.h>

#include "conf.h"
#include "clock_service.h"
#include "layout.h"
#include "log.h"
#include "util.h"
#include "main.h"
#include "page_info.h"
#include "scroller_info.h"
#include "scroller.h"
#include "page.h"
#include "apps/apps_main.h"

static struct {
	app_control_h cur_watch_control;
	app_control_h priv_watch_control;
	Evas_Object *cur_watch;
	Evas_Object *clock_page;
} clock_info_s = {
	.cur_watch_control = NULL,
	.priv_watch_control = NULL,
	.cur_watch = NULL,
	.clock_page = NULL,
};



static char *_get_app_id(const char *pkgname)
{
	int ret = 0;
	char *appid = 0;
	pkgmgrinfo_pkginfo_h handle = NULL;
	retv_if(pkgname == NULL, NULL);

	ret = pkgmgrinfo_pkginfo_get_usr_pkginfo(pkgname, getuid(), &handle);
	if (ret != PMINFO_R_OK) {
		_E("Failed to get pkg information from pkgmgr");
		goto ERR;
	}
	ret = pkgmgrinfo_pkginfo_get_mainappid(handle, &appid);
	if (ret != PMINFO_R_OK) {
		_E("Failed to get mainapp ID information from pkgmgr");
		goto ERR;
	}

	if (appid != NULL) {
		appid = strdup(appid);
	}
ERR:
	if (handle != NULL) {
		pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
	}

	return appid;
}



static Evas_Object *_scroller_get(void)
{
	Evas_Object *win = main_get_info()->win;
	Evas_Object *layout = NULL;
	Evas_Object *scroller = NULL;

	if (win != NULL) {
		layout = evas_object_data_get(win, DATA_KEY_LAYOUT);
		if (layout != NULL) {
			scroller = elm_object_part_content_get(layout, "scroller");
		}
	}

	return scroller;
}



static void _apps_clicked_cb(void *cbdata, Evas_Object *obj, void *event_info)
{
	if (util_feature_enabled_get(FEATURE_APPS) == 1) {
		apps_main_launch(APPS_LAUNCH_SHOW);
	}
}



static void _del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *page = obj;
	Evas_Object *layout = data;
	Evas_Object *obj_sub = NULL;
	ret_if(page == NULL);
	ret_if(layout == NULL);

	obj_sub = elm_object_part_content_unset(layout, "clock_bg");
	if (obj_sub != NULL) {
		evas_object_del(obj_sub);
	}
	obj_sub = elm_object_part_content_unset(layout, "event_blocker");
	if (obj_sub != NULL) {
		evas_object_del(obj_sub);
	}
}



static Evas_Object *_clock_view_add(Evas_Object *parent, Evas_Object *item)
{
	Eina_Bool ret = EINA_TRUE;
	Evas_Object *page = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *bg = NULL;
	Evas_Object *evb = NULL;
	retv_if(parent == NULL, NULL);

	layout = elm_layout_add(parent);
	goto_if(layout == NULL, ERR);
	ret = elm_layout_file_set(layout, PAGE_CLOCK_EDJE_FILE, "clock_page");
	goto_if(ret == EINA_FALSE, ERR);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);

	bg = evas_object_rectangle_add(main_get_info()->e);
	goto_if(bg == NULL, ERR);
	evas_object_size_hint_min_set(bg, main_get_info()->root_w, main_get_info()->root_h);
	evas_object_size_hint_max_set(bg, main_get_info()->root_w, main_get_info()->root_h);
	evas_object_color_set(bg, 0, 0, 0, 0);
	evas_object_show(bg);
	elm_object_part_content_set(layout, "clock_bg", bg);

	evb = evas_object_rectangle_add(main_get_info()->e);
	goto_if(evb == NULL, ERR);
	evas_object_size_hint_min_set(evb, main_get_info()->root_w, main_get_info()->root_h);
	evas_object_size_hint_weight_set(evb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_color_set(evb, 0, 0, 0, 0);
	evas_object_show(evb);
	elm_object_part_content_set(layout, "event_blocker", evb);

	if (item != NULL) {
		elm_access_object_unregister(item);
		evas_object_size_hint_min_set(item, 360, 360);
		evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(item);
		elm_object_part_content_set(layout, "item", item);
		evas_object_repeat_events_set(item, EINA_TRUE);
	}

	page = page_create(parent
					, layout
					, NULL, NULL
					, main_get_info()->root_w, main_get_info()->root_h
					, PAGE_CHANGEABLE_BG_OFF, PAGE_REMOVABLE_OFF);
	goto_if(page == NULL, ERR);

	page_set_effect(page, page_effect_none, page_effect_none);
	evas_object_event_callback_add(page, EVAS_CALLBACK_DEL, _del_cb, layout);

	return page;

ERR:
	if (bg != NULL) evas_object_del(bg);
	if (evb != NULL) evas_object_del(evb);
	if (layout != NULL) evas_object_del(layout);

	return NULL;
}



static void _clock_view_exchange(Evas_Object *item)
{
	Eina_Bool ret = EINA_TRUE;
	Evas_Object *priv_clock = NULL;
	page_info_s *page_info = NULL;

	page_info = evas_object_data_get(clock_info_s.clock_page, DATA_KEY_PAGE_INFO);
	ret_if(!page_info);

	priv_clock = elm_object_part_content_unset(page_info->item, "item");
	if (priv_clock)
		evas_object_del(priv_clock);

	if (item != NULL) {
		elm_access_object_unregister(item);
		evas_object_size_hint_min_set(item, 360, 360);
		evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(item);
		elm_object_part_content_set(page_info->item, "item", item);
		evas_object_repeat_events_set(item, EINA_TRUE);
	}
}



static Evas_Object *_clock_view_empty_add(void)
{
	Evas_Object *scroller = _scroller_get();
	retv_if(scroller == NULL, NULL);

	Evas_Object *page = _clock_view_add(scroller, NULL);

	return page;
}



static void __watch_added(void *data, Evas_Object *obj, void *event_info)
{
	_D("watch added");
	Evas_Object *clock = (Evas_Object *)event_info;
	Evas_Object *page = NULL;
	Evas_Object *scroller = (Evas_Object *)data;

	if (!clock) {
		_E("Fail to create the clock");
		return;
	}

	if (clock_info_s.cur_watch) {
		_clock_view_exchange(clock);
		if (clock_info_s.cur_watch_control) {
			app_control_send_terminate_request(clock_info_s.cur_watch_control);
			app_control_destroy(clock_info_s.cur_watch_control);
		}
	} else {
		page = _clock_view_add(scroller, clock);
		if (!page) {
			_E("Fail to create the page");
			evas_object_del(clock);
			return;
		}
		if (scroller_push_page(scroller, page, SCROLLER_PUSH_TYPE_CENTER) != W_HOME_ERROR_NONE) {
			_E("Fail to push the page into scroller");
		}
		clock_info_s.clock_page = page;
	}

	clock_info_s.cur_watch = clock;
}

static void __watch_removed(void *data, Evas_Object *obj, void *event_info)
{
	_D("watch removed");
}

static void _wms_clock_vconf_cb(keynode_t *node, void *data)
{
	char *clock_pkgname = NULL;
	char *clock_appid = NULL;
	app_control_h watch_control;

	clock_pkgname = vconf_get_str(VCONFKEY_WMS_CLOCKS_SET_IDLE);
	if(!clock_pkgname)
		clock_pkgname = "org.tizen.idle-clock-digital";

	clock_appid = _get_app_id(clock_pkgname);
	_D("clock = %s(%s), is set", clock_appid, clock_pkgname);

	if (clock_info_s.cur_watch_control) {
		clock_info_s.priv_watch_control = clock_info_s.cur_watch_control;
	}

	watch_manager_get_app_control(clock_appid, &watch_control);
	clock_info_s.cur_watch_control = watch_control;
	app_control_send_launch_request(watch_control, NULL, NULL);
	free(clock_appid);
}

void clock_service_init(Evas_Object *win)
{
	Evas_Object *clock = NULL;
	Evas_Object *page = NULL;
	Evas_Object *scroller = _scroller_get();
	app_control_h watch_control;
	char *pkg_name = NULL;
	char *clock_appid = NULL;
	int ret = 0;

	ret_if(!scroller);

	ret = vconf_notify_key_changed(VCONFKEY_WMS_CLOCKS_SET_IDLE, _wms_clock_vconf_cb, NULL);
	if (ret < 0)
		_E("Failed to ignore the vconf callback(WMS_CLOCKS_SET_IDLE) : %d", ret);

	pkg_name = vconf_get_str(VCONFKEY_WMS_CLOCKS_SET_IDLE);

	if(!pkg_name)
	{
		_D("Failed to get vconf string, launching default clock");
		pkg_name = "org.tizen.idle-clock-digital";
	}

#if 0
	if (!pkg_name) {
		Evas_Object *empty_page = NULL;

		_D("Create the empty page");
		empty_page = _clock_view_empty_add();
		ret_if(!empty_page);

		if (scroller_push_page(scroller, empty_page, SCROLLER_PUSH_TYPE_CENTER) != W_HOME_ERROR_NONE) {
			_E("Fail to push the page into scroller");
		}

		return;
	}
#endif

	clock_appid = _get_app_id(pkg_name);
	_D("clock = %s(%s), is set", clock_appid, pkg_name);

	watch_manager_init(win);
	evas_object_smart_callback_add(win, WATCH_SMART_SIGNAL_ADDED, __watch_added, scroller);
	evas_object_smart_callback_add(win, WATCH_SMART_SIGNAL_REMOVED, __watch_removed, scroller);
	watch_manager_get_app_control(clock_appid, &watch_control);

	ret = app_control_send_launch_request(watch_control, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		_E("Failed to launch:%d", ret);
		goto done;
	}

	clock_info_s.cur_watch_control = watch_control;
	free(clock_appid);

	return;

done:
	_D("Launching default clock");
	pkg_name = "org.tizen.idle-clock-digital";
	watch_manager_get_app_control(pkg_name, &watch_control);
	app_control_send_launch_request(watch_control, NULL, NULL);
	clock_info_s.cur_watch_control = watch_control;
}



void clock_service_fini(void)
{
	_D("clock service is finished");
	watch_manager_fini();
}
