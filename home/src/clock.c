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

	obj_sub = elm_object_part_content_get(layout, "clock_bg");
	if (obj_sub != NULL) {
		evas_object_del(obj_sub);
	}
	obj_sub = elm_object_part_content_get(layout, "event_blocker");
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
	evas_object_repeat_events_set(evb, 1);
	evas_object_color_set(evb, 0, 0, 0, 0);
	evas_object_show(evb);
	elm_object_part_content_set(layout, "event_blocker", evb);

	if (item != NULL) {
		elm_access_object_unregister(item);
	}

	Evas_Object *focus_apps = elm_button_add(layout);
	if (focus_apps != NULL) {
		elm_object_theme_set(focus_apps, main_get_info()->theme);
		elm_object_style_set(focus_apps, "transparent");
		evas_object_smart_callback_add(focus_apps, "clicked", _apps_clicked_cb, NULL);
		elm_object_part_content_set(layout, "focus.bottom.cue", focus_apps);
	}

	if (item != NULL) {
		elm_object_part_content_set(layout, "item", item);
	} else {
	/* it is just for test */
		evb = evas_object_rectangle_add(main_get_info()->e);
		goto_if(evb == NULL, ERR);
		evas_object_size_hint_min_set(evb, 100, 100);
		evas_object_size_hint_weight_set(evb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_color_set(evb, 0, 255, 0, 255);
		evas_object_show(evb);
		elm_object_part_content_set(layout, "item", evb);
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



static Evas_Object *_clock_view_empty_add(void)
{
	Evas_Object *scroller = _scroller_get();
	retv_if(scroller == NULL, NULL);

	Evas_Object *page = _clock_view_add(scroller, NULL);

	return page;
}



void clock_service_init(void)
{
	Evas_Object *clock = NULL;
	Evas_Object *page = NULL;
	Evas_Object *scroller = _scroller_get();

	char *pkg_name = NULL;

	retv_if(scroller == NULL, NULL);
	//pkg_name = "org.tizen.idle-clock-digital";

	if (!pkg_name) {
		Evas_Object *empty_page = NULL;

		_D("Create the empty page");
		empty_page = _clock_view_empty_add();
		ret_if(!empty_page);

		return;
	}

	//clock = widget_create(scroller, pkg_name);
	page = _clock_view_add(scroller, clock);
	if (!page) {
		_E("Fail to create the page");
		evas_object_del(clock);
	}

}



void clock_service_fini(void)
{
	_D("clock service is finished");
}
