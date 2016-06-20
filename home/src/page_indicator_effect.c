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
#include <Evas.h>
#include <stdbool.h>
#include <math.h>
#include <efl_assist.h>
#include <efl_extension.h>
#include <ui_extension.h>
#include <bundle.h>
#include <dlog.h>
#include <feedback.h>
#include <cairo.h>

#include "util.h"
#include "conf.h"
#include "log.h"
#include "main.h"
#include "db.h"
#include "edit.h"
#include "edit_info.h"
#include "effect.h"
#include "factory.h"
#include "key.h"
#include "layout.h"
#include "layout_info.h"
#include "widget_viewer_evas.h"
#include "page_info.h"
#include "scroller_info.h"
#include "scroller.h"
#include "index.h"
#include "util.h"
#include "xml.h"
#include "page_indicator.h"

#define PI_FOCUS_VI_NONE 0
#define PI_FOCUS_VI_ACTIVATE 1
#define PI_FOCUS_VI_END_TICK 2

#define INDICATOR_RADIUS_OFFSET 9.0
#define INDICATOR_VI_ANGLE_ACTIVATION 1.5
#define INDICATOR_VI_ANGLE_END 13.0
#define INDICATOR_VI_ACTIVATION_DURATION 0.2
#define INDICATOR_VI_END_DURATION 0.3

static double angle_map[31] = {
	0.0,/*0*/ 

	353.0,/*1*/ // temporal 354 -> 353
	348.0,
	342.0,
	336.0,
	330.0,
	324.0,
	318.0,
	312.0,
	306.0,
	300.0,
	294.0,
	288.0,
	282.0,
	276.0,
	270.0,/*15*/

	 7.0,/*16*/ // temporal 6.0 -> 7.0
	12.0,
	18.0,
	24.0,
	30.0,
	36.0,
	42.0,
	48.0,
	54.0,
	60.0,
	66.0,
	72.0,
	78.0,
	84.0,
	90.0,
};

typedef struct {
	double angle_s;
	double angle_e;
	unsigned int rgba;
	double cx;
	double cy;
	double radius;
} arc_info_s;

typedef struct {
	int type;
	int index;
	int w;
	int h;
	int stride;
	double s_time;
	double e_time;
	double r_time;
	double duration;

	int drag;
	int delta;

	Evas_Object *image;
	void *pixels;

	struct _touch {
		int d_x;
		int delta_x;
		int down;
		int drag;
		int direction;
	} touch;

	arc_info_s arc_info;
} indicator_info_s;

static cairo_surface_t *_surface_create(unsigned char *pixels, int w, int h, int stride)
{
	cairo_surface_t *surface = NULL;

	surface = cairo_image_surface_create_for_data(pixels, CAIRO_FORMAT_ARGB32, w, h, stride);

	return surface;
}

#define INDICATOR_VI_ACTIVATION_COLOR_CODE "AO0021S"
static void _draw_arc(cairo_t *cairo, arc_info_s *info)
{
	//_D("cx:%f cy:%f angle s:%f angled e:%f, radius:%f", info->cx, info->cy, info->angle_s, info->angle_e, info->radius);
	cairo_save(cairo);
	cairo_set_operator(cairo, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cairo);
	cairo_set_operator(cairo, CAIRO_OPERATOR_OVER);

	int r = 0, g = 0, b = 0, a = 0;
	ea_theme_color_get(INDICATOR_VI_ACTIVATION_COLOR_CODE, &r, &g, &b, &a,
								NULL, NULL, NULL, NULL,
								NULL, NULL, NULL, NULL);

	//cairo_set_source_rgba(cairo, r, g, b, a);
	cairo_set_source_rgba(cairo, r/255., g/255., b/255., a/255.);
	cairo_set_line_width(cairo, 8.0);
	cairo_set_line_cap(cairo, CAIRO_LINE_CAP_ROUND);
	cairo_save(cairo);

	cairo_new_sub_path(cairo);
	cairo_arc(cairo, info->cx, info->cy, info->radius,
				(info->angle_s - 90) * (M_PI / 180.0), (info->angle_e - 90) * (M_PI / 180.0));
	cairo_stroke(cairo);
	cairo_restore(cairo);
}

static void _clear(cairo_t *cairo, arc_info_s *info)
{
	//_D("cx:%f cy:%f angle s:%f angled e:%f, radius:%f", info->cx, info->cy, info->angle_s, info->angle_e, info->radius);
	cairo_save(cairo);
	cairo_set_operator(cairo, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cairo);
	cairo_save(cairo);
}

#define PAGE_INDICATOR_INFO_KEY "p_i_e_v_i_k"
static void _on_pixel_get_cb(void *data, Evas_Object *image)
{
	void *pixels = NULL;
	cairo_surface_t *surface = NULL;
	cairo_t *cairo = NULL;
	indicator_info_s *info = evas_object_data_get(image, PAGE_INDICATOR_INFO_KEY);
	goto_if(info == NULL, ERROR);

	pixels = (void *)evas_object_image_data_get(image, EINA_TRUE);
	goto_if(pixels == NULL, ERROR);

	surface = _surface_create(pixels, info->w, info->h, info->stride);
	goto_if(surface == NULL, ERROR);

	cairo = cairo_create(surface);
	goto_if(cairo == NULL, ERROR);

	if (info->index == 0) {
		_clear(cairo, &info->arc_info);
	} else {
		_draw_arc(cairo, &info->arc_info);
	}

ERROR:
	if (pixels) {
		evas_object_image_data_set(image, pixels);
	}
	if (cairo) {
		cairo_destroy(cairo);
	}
	if (surface) {
		cairo_surface_destroy(surface);
	}
}

static Eina_Bool _vi_activate(void *data)
{
	double factor[4] = {0.45, 0.03, 0.41, 1.0}; //ease.Out

	Evas_Object *image = data;
	retv_if(image == NULL, ECORE_CALLBACK_CANCEL);

	indicator_info_s *info = evas_object_data_get(image, PAGE_INDICATOR_INFO_KEY);
	retv_if(info == NULL, ECORE_CALLBACK_CANCEL);

	if (info->index == 0) {
		_on_pixel_get_cb(info, image);
		return ECORE_CALLBACK_CANCEL;
	}

	double elapsed_time = ecore_loop_time_get() - info->s_time;
	double progress = (elapsed_time / INDICATOR_VI_ACTIVATION_DURATION);
	progress = ecore_animator_pos_map_n(progress,  ECORE_POS_MAP_CUBIC_BEZIER, 4, factor);

	info->arc_info.cx = BASE_WIDTH / 2.0;
	info->arc_info.cy = BASE_WIDTH / 2.0;
	info->arc_info.radius = (BASE_WIDTH / 2.0) - INDICATOR_RADIUS_OFFSET;

	info->arc_info.angle_s = angle_map[info->index] - (INDICATOR_VI_ANGLE_ACTIVATION * progress);
	info->arc_info.angle_e = angle_map[info->index] + (INDICATOR_VI_ANGLE_ACTIVATION * progress);

	evas_object_image_pixels_dirty_set(image, EINA_TRUE);

	if (progress < 1.0) {
		return ECORE_CALLBACK_RENEW;
	}

	_D("Done");

	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _vi_end_tick(void *data)
{
	double factor[4] = {0.45, 0.03, 0.41, 1.0}; //ease.Out

	Evas_Object *image = data;
	retv_if(image == NULL, ECORE_CALLBACK_CANCEL);

	indicator_info_s *info = evas_object_data_get(image, PAGE_INDICATOR_INFO_KEY);
	retv_if(info == NULL, ECORE_CALLBACK_CANCEL);

	info->arc_info.cx = BASE_WIDTH / 2.0;
	info->arc_info.cy = BASE_WIDTH / 2.0;
	info->arc_info.radius = (BASE_WIDTH / 2.0) - INDICATOR_RADIUS_OFFSET;

	int finished = 0;
	double elapsed_time = 0.0;
	double progress = 0.0;
	double now = ecore_loop_time_get();
	if (info->e_time <= now) {
		elapsed_time = ecore_loop_time_get() - info->e_time;
		progress = (elapsed_time / INDICATOR_VI_END_DURATION);
		progress = ecore_animator_pos_map_n(progress,  ECORE_POS_MAP_CUBIC_BEZIER, 4, factor);

		if (info->touch.direction == INDEX_END_RIGHT) {
			info->arc_info.angle_s = angle_map[info->index] - INDICATOR_VI_ANGLE_ACTIVATION;
			info->arc_info.angle_e = angle_map[info->index] + INDICATOR_VI_ANGLE_ACTIVATION + ((INDICATOR_VI_ANGLE_END - INDICATOR_VI_ANGLE_ACTIVATION) * (1.0 - progress));
		} else {
			info->arc_info.angle_s = angle_map[info->index] - INDICATOR_VI_ANGLE_ACTIVATION - ((INDICATOR_VI_ANGLE_END - INDICATOR_VI_ANGLE_ACTIVATION) * (1.0 - progress));
			info->arc_info.angle_e = angle_map[info->index] + INDICATOR_VI_ANGLE_ACTIVATION;
		}

		if (progress >= 1.0) {
			finished = 1;
		}
	} else {
		elapsed_time = info->e_time - ecore_loop_time_get();
		progress = (elapsed_time / INDICATOR_VI_END_DURATION);
		progress = ecore_animator_pos_map_n(progress,  ECORE_POS_MAP_CUBIC_BEZIER, 4, factor);

		if (info->touch.direction == INDEX_END_RIGHT) {
			info->arc_info.angle_s = angle_map[info->index] - INDICATOR_VI_ANGLE_ACTIVATION;
			info->arc_info.angle_e = angle_map[info->index] + INDICATOR_VI_ANGLE_ACTIVATION + ((INDICATOR_VI_ANGLE_END - INDICATOR_VI_ANGLE_ACTIVATION) * progress);
		} else {
			info->arc_info.angle_s = angle_map[info->index] - INDICATOR_VI_ANGLE_ACTIVATION - ((INDICATOR_VI_ANGLE_END - INDICATOR_VI_ANGLE_ACTIVATION) * progress);
			info->arc_info.angle_e = angle_map[info->index] + INDICATOR_VI_ANGLE_ACTIVATION;
		}
	}

	evas_object_image_pixels_dirty_set(image, EINA_TRUE);

	if (!finished) {
		return ECORE_CALLBACK_RENEW;
	}

	_D("Done");

	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _vi_end_drag(void *data)
{
	double factor[4] = {0.45, 0.03, 0.41, 1.0}; //ease.Out

	Evas_Object *image = data;
	retv_if(image == NULL, ECORE_CALLBACK_CANCEL);

	indicator_info_s *info = evas_object_data_get(image, PAGE_INDICATOR_INFO_KEY);
	retv_if(info == NULL, ECORE_CALLBACK_CANCEL);

	info->arc_info.cx = BASE_WIDTH / 2.0;
	info->arc_info.cy = BASE_WIDTH / 2.0;
	info->arc_info.radius = (BASE_WIDTH / 2.0) - INDICATOR_RADIUS_OFFSET;

	double progress = 0.0;
	double delta_x = ((double)((info->touch.delta_x < 0) ? -info->touch.delta_x : info->touch.delta_x)) * (1.5);
	progress = delta_x / BASE_WIDTH;
	progress = (progress > 1.0) ? 1.0 : progress;
	progress = ecore_animator_pos_map_n(progress,  ECORE_POS_MAP_CUBIC_BEZIER, 4, factor);

	if (info->touch.direction == INDEX_END_RIGHT) {
		info->arc_info.angle_s = angle_map[info->index] - INDICATOR_VI_ANGLE_ACTIVATION;
		info->arc_info.angle_e = angle_map[info->index] + INDICATOR_VI_ANGLE_ACTIVATION + ((INDICATOR_VI_ANGLE_END - INDICATOR_VI_ANGLE_ACTIVATION) * progress);
	} else {
		info->arc_info.angle_s = angle_map[info->index] - INDICATOR_VI_ANGLE_ACTIVATION - ((INDICATOR_VI_ANGLE_END - INDICATOR_VI_ANGLE_ACTIVATION) * progress);
		info->arc_info.angle_e = angle_map[info->index] + INDICATOR_VI_ANGLE_ACTIVATION;
	}

	evas_object_image_pixels_dirty_set(image, EINA_TRUE);

	if (info->touch.down) {
		return ECORE_CALLBACK_RENEW;
	}

	info->touch.drag = 0;
	info->s_time = ecore_loop_time_get();
	info->r_time = (1.0 - progress) * INDICATOR_VI_END_DURATION;

	_D("Done");

	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _vi_end_drag_release(void *data)
{
	double factor[4] = {0.45, 0.03, 0.41, 1.0}; //ease.Out

	Evas_Object *image = data;
	retv_if(image == NULL, ECORE_CALLBACK_CANCEL);

	indicator_info_s *info = evas_object_data_get(image, PAGE_INDICATOR_INFO_KEY);
	retv_if(info == NULL, ECORE_CALLBACK_CANCEL);

	info->arc_info.cx = BASE_WIDTH / 2.0;
	info->arc_info.cy = BASE_WIDTH / 2.0;
	info->arc_info.radius = (BASE_WIDTH / 2.0) - INDICATOR_RADIUS_OFFSET;

	double elapsed_time = 0.0;
	double progress = 0.0;

	elapsed_time = ecore_loop_time_get() - info->s_time + info->r_time;
	progress = (elapsed_time / INDICATOR_VI_END_DURATION);
	progress = ecore_animator_pos_map_n(progress,  ECORE_POS_MAP_CUBIC_BEZIER, 4, factor);

	if (info->touch.direction == INDEX_END_RIGHT) {
		info->arc_info.angle_s = angle_map[info->index] - INDICATOR_VI_ANGLE_ACTIVATION;
		info->arc_info.angle_e = angle_map[info->index] + INDICATOR_VI_ANGLE_ACTIVATION + ((INDICATOR_VI_ANGLE_END - INDICATOR_VI_ANGLE_ACTIVATION) * (1.0 - progress));
	} else {
		info->arc_info.angle_s = angle_map[info->index] - INDICATOR_VI_ANGLE_ACTIVATION - ((INDICATOR_VI_ANGLE_END - INDICATOR_VI_ANGLE_ACTIVATION) * (1.0 - progress));
		info->arc_info.angle_e = angle_map[info->index] + INDICATOR_VI_ANGLE_ACTIVATION;
	}

	evas_object_image_pixels_dirty_set(image, EINA_TRUE);

	if (progress < 1.0) {
		return ECORE_CALLBACK_RENEW;
	}

	_D("Done");

	return ECORE_CALLBACK_CANCEL;
}

static void _move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *image = data;
	ret_if(image == NULL);

	indicator_info_s *info = evas_object_data_get(image, PAGE_INDICATOR_INFO_KEY);
	ret_if(info == NULL);

	Evas_Event_Mouse_Move *ev = event_info;
	if (ev && info) {
		if (info->touch.d_x < 0) {
			info->touch.d_x = ev->prev.canvas.x;
		}

		info->touch.delta_x = ev->cur.canvas.x - info->touch.d_x;
		_D("info->touch.delta_x:%d", info->touch.delta_x);
	}
}

static void _up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE, _move_cb);
	evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP, _up_cb);
	evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_OUT, _up_cb);

	Evas_Object *image = data;
	
	if (image) {
		indicator_info_s *info = evas_object_data_get(image, PAGE_INDICATOR_INFO_KEY);
		if (info) {
			info->touch.down = 0;
		}

		ecore_animator_add(_vi_end_drag_release, image);
	}
}

static void _del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	indicator_info_s *info = evas_object_data_del(obj, PAGE_INDICATOR_INFO_KEY);
	if (info) {
		free(info);
	}
}

Evas_Object *page_indicator_focus_add(void)
{
	Evas_Object *image = NULL;

	indicator_info_s *info = (indicator_info_s *)calloc(1, sizeof(indicator_info_s));
	if (!info) {
		_E("Failed to allocate indicator info");
		return NULL;
	}

	info->type = PI_FOCUS_VI_NONE;

	image = evas_object_image_add(main_get_info()->e);
	if (image) {
		evas_object_data_set(image, PAGE_INDICATOR_INFO_KEY, info);
		evas_object_repeat_events_set(image, EINA_TRUE);
		evas_object_image_alpha_set(image, EINA_TRUE);
		evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
		evas_object_image_fill_set(image, 0, 0, BASE_WIDTH, BASE_HEIGHT);
		evas_object_image_size_set(image, BASE_WIDTH, BASE_HEIGHT);
		evas_object_resize(image, BASE_WIDTH, BASE_HEIGHT);
		evas_object_image_content_hint_set(image, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);
		evas_object_image_pixels_get_callback_set(image, _on_pixel_get_cb, NULL);
		evas_object_event_callback_add(image, EVAS_CALLBACK_DEL, _del_cb, NULL);
		evas_object_show(image);
	} else {
		_E("Failed to add effect image");
	}

	if (image == NULL) {
		free(info);
	}

	return image;
}

void page_indicator_focus_activate(Evas_Object *indicator, int index) {
	ret_if(indicator == NULL);

	Evas_Object *image = elm_object_part_content_get(indicator, "focus,icon");
	if (image) {
		indicator_info_s *info = (indicator_info_s *)evas_object_data_get(image, PAGE_INDICATOR_INFO_KEY);
		if (info) {
			info->type = PI_FOCUS_VI_ACTIVATE;
			info->index = index;
			info->w = BASE_WIDTH;
			info->h = BASE_HEIGHT;
			info->stride = evas_object_image_stride_get(image);
			info->s_time = ecore_loop_time_get();

			ecore_animator_add(_vi_activate, image);
		} else {
			_E("Failed to alloc a memory for info");
		}
	} else {
		_E("Failed to focus image object");
	}
}

void page_indicator_focus_end_tick(Evas_Object *indicator, int index, int direction) {
	ret_if(indicator == NULL);

	static double calltime = 0.0;

	if (ecore_loop_time_get() - calltime < (INDICATOR_VI_END_DURATION)) {
		_W("end effect canceled");
		return;
	}

	effect_play_vibration_by_name(FEEDBACK_PATTERN_END_EFFECT);

	Evas_Object *image = elm_object_part_content_get(indicator, "focus,icon");
	if (image) {
		indicator_info_s *info = (indicator_info_s *)evas_object_data_get(image, PAGE_INDICATOR_INFO_KEY);
		if (info) {
			info->type = PI_FOCUS_VI_END_TICK;
			info->index = index;
			info->w = BASE_WIDTH;
			info->h = BASE_HEIGHT;
			info->stride = evas_object_image_stride_get(image);
			info->s_time = ecore_loop_time_get();
			info->e_time = info->s_time + 0.010;
			info->touch.direction = direction;

			ecore_animator_add(_vi_end_tick, image);

			calltime = info->s_time;
		} else {
			_E("Failed to alloc a memory for info");
		}
	} else {
		_E("Failed to focus image object");
	}
}

void page_indicator_focus_end_drag(Evas_Object *indicator, int index, int direction) {
	ret_if(indicator == NULL);

	effect_play_vibration_by_name(FEEDBACK_PATTERN_END_EFFECT);

	Evas_Object *image = elm_object_part_content_get(indicator, "focus,icon");
	if (image) {
		indicator_info_s *info = evas_object_data_get(image, PAGE_INDICATOR_INFO_KEY);
		if (info) {
			info->type = PI_FOCUS_VI_END_TICK;
			info->index = index;
			info->w = BASE_WIDTH;
			info->h = BASE_HEIGHT;
			info->stride = evas_object_image_stride_get(image);
			info->s_time = ecore_loop_time_get();
			info->e_time = info->s_time + INDICATOR_VI_END_DURATION;
			info->touch.d_x = -360;
			info->touch.delta_x = 0;
			info->touch.down = 1;
			info->touch.drag = 1;
			info->touch.direction = direction;

			evas_object_event_callback_add(indicator, EVAS_CALLBACK_MOUSE_MOVE, _move_cb, image);
			evas_object_event_callback_add(indicator, EVAS_CALLBACK_MOUSE_UP, _up_cb, image);
			evas_object_event_callback_add(indicator, EVAS_CALLBACK_MOUSE_OUT, _up_cb, image);

			ecore_animator_add(_vi_end_drag, image);
		} else {
			_E("Failed to alloc a memory for info");
		}
	} else {
		_E("Failed to focus image object");
	}
}
