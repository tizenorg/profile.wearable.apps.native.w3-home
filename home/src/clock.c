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
#include <watch_control.h>
#include <vconf.h>
#include <pkgmgr-info.h>
#include <pkgmgrinfo_type.h>
#include <Pepper_Efl.h>

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
	Evas_Object *cur_watch;
	Evas_Object *clock_page;
	int pid;
	int launched;
} clock_info_s = {
	.cur_watch = NULL,
	.clock_page = NULL,
	.pid = -1,
	.launched = 0,
};



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
	main_get_info()->clock_focus =  page;
	return page;

ERR:
	if (bg != NULL) evas_object_del(bg);
	if (evb != NULL) evas_object_del(evb);
	if (layout != NULL) evas_object_del(layout);

	return NULL;
}



static void _clock_view_exchange(Evas_Object *item)
{
	page_info_s *page_info = NULL;

	page_info = evas_object_data_get(clock_info_s.clock_page, DATA_KEY_PAGE_INFO);
	ret_if(!page_info);

	if (item != NULL) {
		elm_access_object_unregister(item);
		evas_object_size_hint_min_set(item, 360, 360);
		evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(item);
		elm_object_part_content_set(page_info->item, "item", item);
		evas_object_repeat_events_set(item, EINA_TRUE);
	}
}


static int _try_to_launch(const char *clock_pkgname)
{
	app_control_h watch_control = NULL;

	int clock_app_pid = 0;
	bundle *b;

	if(!clock_pkgname) {
		clock_pkgname = "org.tizen.classic-watch";
	}

	clock_info_s.launched = 0;

	watch_manager_get_app_control(clock_pkgname, &watch_control);
	app_control_to_bundle(watch_control, &b);
	clock_app_pid = appsvc_run_service(b, 0, NULL, NULL); /* Get pid of launched watch-app */
	_D("appsvc_run_service returns [%d]", clock_app_pid);
	clock_info_s.pid = clock_app_pid;
	app_control_destroy(watch_control);

	retv_if(clock_app_pid == -4, -1);

	_D("Succeed to launch : %s", clock_pkgname);

	return 0;
}

static void _wms_clock_vconf_cb(keynode_t *node, void *data)
{
	char *clock_pkgname = NULL;
	int ret = 0;

	clock_pkgname = vconf_get_str(VCONFKEY_WMS_CLOCKS_SET_IDLE);
	if(!clock_pkgname)
		clock_pkgname = "org.tizen.classic-watch";

	_D("clock = (%s), is set", clock_pkgname);
	ret = _try_to_launch(clock_pkgname);

	if (ret < 0) {
		_E("Fail to launch the watch");
		return;
	}
}

void clock_try_to_launch(int pid)
{
	_D("pid[%d] clock_info_s.pid[%d[", pid, clock_info_s.pid);
	if (pid != clock_info_s.pid) {
		_D("Dead process is not faulted clock");
		return;
	}

	clock_info_s.pid = -1;

	if (clock_info_s.launched == 0) {
		_D("clock could not be launched. Use default clock");
		_try_to_launch("org.tizen.classic-watch");
	} else {
		clock_info_s.launched = 0;
		_D("clock is dead. try to relaunch the clock");
		_wms_clock_vconf_cb(NULL, NULL);
	}
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
		watch_manager_send_terminate(clock_info_s.cur_watch);
		_clock_view_exchange(clock);
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
	clock_info_s.pid = pepper_efl_object_pid_get(clock);
	clock_info_s.launched = 1;
}

static void __watch_removed(void *data, Evas_Object *obj, void *event_info)
{
	_D("watch removed");
	Evas_Object *clock = (Evas_Object *)event_info;
	if (clock)
		evas_object_del(clock);
}



void clock_service_init(Evas_Object *win)
{
	Evas_Object *scroller = _scroller_get();
	app_control_h watch_control;
	char *pkg_name = NULL;
	int ret = 0;

	ret_if(!scroller);

	ret = vconf_notify_key_changed(VCONFKEY_WMS_CLOCKS_SET_IDLE, _wms_clock_vconf_cb, NULL);
	if (ret < 0)
		_E("Failed to ignore the vconf callback(WMS_CLOCKS_SET_IDLE) : %d", ret);

	pkg_name = vconf_get_str(VCONFKEY_WMS_CLOCKS_SET_IDLE);

	if(!pkg_name)
	{
		_D("Failed to get vconf string, launching default clock");
		pkg_name = "org.tizen.classic-watch";
	}

	_D("clock = (%s), is set", pkg_name);

	watch_manager_init(win);
	evas_object_smart_callback_add(win, WATCH_SMART_SIGNAL_ADDED, __watch_added, scroller);
	evas_object_smart_callback_add(win, WATCH_SMART_SIGNAL_REMOVED, __watch_removed, scroller);

	ret = _try_to_launch(pkg_name);
	if (ret < 0) {
		_E("Failed to launch:%s", pkg_name);
		goto done;
	}

	return;

done:
	_D("Launching default clock");
	pkg_name = "org.tizen.classic-watch";

	ret = _try_to_launch(pkg_name);
	if (ret < 0) {
		_E("Failed to launch:%s", pkg_name);
		goto done;
	}
}



void clock_service_fini(void)
{
	_D("clock service is finished");
	watch_manager_fini();
}
