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
#include <efl_assist.h>
#include <efl_extension.h>
#include <bundle.h>
#include <dlog.h>

#include "conf.h"
#include "layout_info.h"
#include "log.h"
#include "util.h"
#include "main.h"
#include "page_info.h"
#include "scroller_info.h"
#include "index_info.h"
#include "index.h"
#include "scroller.h"

#define PRIVATE_DATA_KEY_INDEX_STYLE "pd_p_i_i_s"
#define PRIVATE_DATA_KEY_INDEX_VISIBILITY "pd_p_i_v"
#define PRIVATE_DATA_KEY_INDEX_SHOW_VI_INFO "pd_p_i_s_v_is"
#define PRIVATE_DATA_KEY_INDEX_HIDE_VI_INFO "pd_p_i_h_v_is"

extern index_inf_s g_index_inf_winset;
extern index_inf_s g_index_inf_custom;




inline static index_inf_s *_interface_get(int type)
{
	if (type == INDEX_TYPE_WINSET) {
		return &g_index_inf_winset;
	} else if (type == INDEX_TYPE_CUSTOM) {
		return &g_index_inf_custom;
	} else {
		_E("[Fatal] interface type isn't set:%d", type);
	}

	return NULL;
}

inline static void _interface_type_set(Evas_Object *index, int type)
{
	evas_object_data_set(index, PRIVATE_DATA_KEY_INDEX_STYLE, (void *)type);
}

inline static int _interface_type_get(Evas_Object *index)
{
	return (int)evas_object_data_get(index, PRIVATE_DATA_KEY_INDEX_STYLE);
}

inline static void _visibility_set(Evas_Object *index, int visible)
{
	evas_object_data_set(index, PRIVATE_DATA_KEY_INDEX_VISIBILITY, (void *)visible);
}

inline static int _visibility_get(Evas_Object *index)
{
	return (int)evas_object_data_get(index, PRIVATE_DATA_KEY_INDEX_VISIBILITY);
}


HAPI Evas_Object *index_create(int type, Evas_Object *layout, Evas_Object *scroller, page_direction_e direction)
{
	Evas_Object *index = NULL;
	index_inf_s *inf = _interface_get(type);

	if (inf) {
		if (inf->create) {
			index = inf->create(layout, scroller, direction);
			_interface_type_set(index, type);
			_visibility_set(index, 1);
		}
	} else {
		_E("Failed to get index interface:%d", type);
	}

	return index;
}



HAPI void index_destroy(Evas_Object *index)
{
	int type = _interface_type_get(index);
	index_inf_s *inf = _interface_get(type);

	if (inf) {
		if (inf->destroy) {
			inf->destroy(index);
		}
	} else {
		_E("Failed to get index interface:%d", type);
	}
}



HAPI void index_update(Evas_Object *index, Evas_Object *scroller, index_bring_in_e after)
{
	int type = _interface_type_get(index);
	index_inf_s *inf = _interface_get(type);

	if (inf) {
		if (inf->update) {
			inf->update(index, scroller, after);
		}
	} else {
		_E("Failed to get index interface:%d", type);
	}
}



HAPI void index_time_update(Evas_Object *index, char *time_str)
{
	int type = _interface_type_get(index);
	index_inf_s *inf = _interface_get(type);

	if (inf) {
		if (inf->update_time) {
			inf->update_time(index, time_str);
		} else {
			_E("not implemeneted:update_time:%d", type);
		}
	} else {
		_E("Failed to get index interface:%d", type);
	}
}



HAPI void index_bring_in_page(Evas_Object *index, Evas_Object *page)
{
	int type = _interface_type_get(index);
	index_inf_s *inf = _interface_get(type);

	if (inf) {
		if (inf->bring_in_page) {
			inf->bring_in_page(index, page);
		}
	} else {
		_E("Failed to get index interface");
	}
}

 // End of file


typedef struct {
	double start_time;
	void *object;
	Ecore_Animator *animator;
} index_vi_info_s;

#define INDEX_HIDE_ZOOM_RATE 1.1


#define INDEX_SHOW_VI_DURATION_ZOOM 0.3
static Eina_Bool _index_show_animator_cb(void *data)
{
	double factor[4] = {0.25, 0.46, 0.45, 1.0}; //ease.Out
	double progress_zoom = 0.0;
	double elapsed_time = 0.0;
	double current_time = 0.0;

	Evas_Object *index = data;
	goto_if(!index, ERROR);

	index_vi_info_s *vi_info = evas_object_data_get(index, PRIVATE_DATA_KEY_INDEX_SHOW_VI_INFO);
	goto_if(!vi_info, ERROR);

	Evas_Object *page = vi_info->object;

	current_time = ecore_loop_time_get();
	elapsed_time = current_time - vi_info->start_time;

	progress_zoom = elapsed_time / INDEX_SHOW_VI_DURATION_ZOOM;
	progress_zoom = ecore_animator_pos_map_n(progress_zoom,  ECORE_POS_MAP_CUBIC_BEZIER, 4, factor);

	double center_x, center_y;
	int cur_x, cur_y, cur_w, cur_h;

	evas_object_geometry_get(page, &cur_x, &cur_y, &cur_w, &cur_h);
	center_x = cur_w / 2.0f;
	center_y = cur_h / 2.0f;

	evas_object_show(index);

	Evas_Map *map = evas_map_new(4);
	evas_map_util_points_populate_from_geometry(map, 0, 0, cur_w, cur_h, 0);
	goto_if(!map, ERROR);

	double zoom_rate = INDEX_HIDE_ZOOM_RATE - ((INDEX_HIDE_ZOOM_RATE - 1.0) * progress_zoom);
	evas_map_util_zoom(map, zoom_rate, zoom_rate, center_x, center_y);

	evas_object_map_set(page, map);
	evas_object_map_enable_set(page, EINA_TRUE);
	evas_map_free(map);

	if (progress_zoom < 1.0) {
		return ECORE_CALLBACK_RENEW;
	}

	evas_object_smart_callback_call(index, "index,show", NULL);
ERROR:
	if (evas_object_map_enable_get(index) == EINA_TRUE) {
		evas_object_map_enable_set(index, EINA_FALSE);
	}
	vi_info = evas_object_data_del(index, PRIVATE_DATA_KEY_INDEX_SHOW_VI_INFO);
	if (vi_info) {
		vi_info->animator = NULL;
		free(vi_info);
	}

	return ECORE_CALLBACK_CANCEL;
}

#define INDEX_HIDE_VI_DURATION_ZOOM 0.19
static Eina_Bool _index_hide_animator_cb(void *data)
{
	double factor[4] = {0.45, 0.03, 0.41, 1.0}; //ease.Out
	double progress_zoom = 0.0;
	double elapsed_time = 0.0;
	double current_time = 0.0;

	Evas_Object *index = data;
	goto_if(!index, ERROR);

	index_vi_info_s *vi_info = evas_object_data_get(index, PRIVATE_DATA_KEY_INDEX_HIDE_VI_INFO);
	goto_if(!vi_info, ERROR);

	Evas_Object *page = vi_info->object;

	current_time = ecore_loop_time_get();
	elapsed_time = current_time - vi_info->start_time;

	progress_zoom = elapsed_time / INDEX_HIDE_VI_DURATION_ZOOM;
	progress_zoom = ecore_animator_pos_map_n(progress_zoom,  ECORE_POS_MAP_CUBIC_BEZIER, 4, factor);

	double center_x, center_y;
	int cur_x, cur_y, cur_w, cur_h;

	evas_object_geometry_get(page, &cur_x, &cur_y, &cur_w, &cur_h);
	center_x = cur_w / 2.0f;
	center_y = cur_h / 2.0f;

	Evas_Map *map = evas_map_new(4);
	evas_map_util_points_populate_from_geometry(map, 0, 0, cur_w, cur_h, 0);
	goto_if(!map, ERROR);

	double zoom_rate = 1.0 + ((INDEX_HIDE_ZOOM_RATE - 1.0) * progress_zoom);
	evas_map_util_zoom(map, zoom_rate, zoom_rate, center_x, center_y);

	evas_object_map_set(page, map);
	evas_object_map_enable_set(page, EINA_TRUE);
	evas_map_free(map);

	if (progress_zoom < 1.0) {
		return ECORE_CALLBACK_RENEW;
	}

ERROR:
	vi_info = evas_object_data_del(index, PRIVATE_DATA_KEY_INDEX_HIDE_VI_INFO);
	if (vi_info) {
		vi_info->animator = NULL;
		free(vi_info);
	}

	evas_object_map_enable_set(index, EINA_FALSE);
	evas_object_hide(index);
	evas_object_smart_callback_call(index, "index,hide", NULL);

	return ECORE_CALLBACK_CANCEL;
}

HAPI void index_show(Evas_Object *index, int vi)
{
	ret_if(index == NULL);
	int is_paused = (main_get_info()->state == APP_STATE_RESUME) ? 0 : 1;

	_W("is_paused:%d show VI:%d visibility:%d vi:%p", is_paused, vi, _visibility_get(index), evas_object_data_get(index, PRIVATE_DATA_KEY_INDEX_SHOW_VI_INFO));

	if (is_paused == 1) return ;
	if (_visibility_get(index) == 1) return ;
	if (evas_object_data_get(index, PRIVATE_DATA_KEY_INDEX_SHOW_VI_INFO)) {
		return ;
	}

	index_vi_info_s *vi_info_hide = evas_object_data_del(index, PRIVATE_DATA_KEY_INDEX_HIDE_VI_INFO);
	if (vi_info_hide) {
		ecore_animator_del(vi_info_hide->animator);
		vi_info_hide->animator = NULL;
		free(vi_info_hide);
	}

	if (vi) {
		index_vi_info_s *vi_info = (index_vi_info_s*)calloc(1, sizeof(index_vi_info_s));
		if (vi_info) {
			vi_info->start_time = ecore_loop_time_get();
			vi_info->object = index;
			evas_object_data_set(index, PRIVATE_DATA_KEY_INDEX_SHOW_VI_INFO, vi_info);
			vi_info->animator = ecore_animator_add(_index_show_animator_cb, index);
		}
	} else {
		evas_object_smart_callback_call(index, "index,show", NULL);
		if (evas_object_map_enable_get(index) == EINA_TRUE) {
			evas_object_map_enable_set(index, EINA_FALSE);
		}
		evas_object_show(index);
	}

	_visibility_set(index, 1);
}

HAPI void index_hide(Evas_Object *index, int vi)
{
	ret_if(index == NULL);

	_W("hide VI:%d visibility:%d vi:%p", vi, _visibility_get(index), evas_object_data_get(index, PRIVATE_DATA_KEY_INDEX_SHOW_VI_INFO));

	if (_visibility_get(index) == 0) return ;
	if (evas_object_data_get(index, PRIVATE_DATA_KEY_INDEX_HIDE_VI_INFO)) {
		return ;
	}

	index_vi_info_s *vi_info_show = evas_object_data_del(index, PRIVATE_DATA_KEY_INDEX_SHOW_VI_INFO);
	if (vi_info_show) {
		ecore_animator_del(vi_info_show->animator);
		vi_info_show->animator = NULL;
		free(vi_info_show);
	}

	if (vi) {
		index_vi_info_s *vi_info = (index_vi_info_s*)calloc(1, sizeof(index_vi_info_s));
		if (vi_info) {
			vi_info->start_time = ecore_loop_time_get();
			vi_info->object = index;

			evas_object_data_set(index, PRIVATE_DATA_KEY_INDEX_HIDE_VI_INFO, vi_info);
			vi_info->animator = ecore_animator_add(_index_hide_animator_cb, index);
		}
	} else {
		evas_object_smart_callback_call(index, "index,hide", NULL);
		evas_object_map_enable_set(index, EINA_FALSE);
		evas_object_hide(index);
	}

	_visibility_set(index, 0);
}

HAPI void index_end(Evas_Object *index, int input_type, int direction)
{
	ret_if(index == NULL);

	int type = _interface_type_get(index);
	index_inf_s *inf = _interface_get(type);

	if (inf) {
		if (inf->end) {
			inf->end(index, input_type, direction);
		}
	} else {
		_E("Failed to get index interface");
	}
}