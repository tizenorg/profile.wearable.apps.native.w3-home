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
#include "util.h"
#include "key.h"
#include "main.h"
#include "layout_info.h"
#include "tutorial.h"
#include "apps/apps_conf.h"
#include "apps/layout.h"
#include "apps/apps_main.h"

#define PRIVATE_DATA_KEY_CHECKER_TYPE "ck_tp"
#define PRIVATE_DATA_KEY_DOWN_X "dw_x"
#define PRIVATE_DATA_KEY_DOWN_Y "dw_y"
#define PRIVATE_DATA_KEY_TOP_CHECKER "top_ck"
#define PRIVATE_DATA_KEY_BOTTOM_CHECKER "bt_ck"
#define PRIVATE_DATA_KEY_LAYOUT_IS_EDITED "p_ly_ed"

#define MOVE_UP -2
#define MOVE_DOWN 2



static void _bg_down_cb(void *data, Evas_Object *obj, const char* emission, const char* source)
{
	_APPS_D("BG is pressed");
}

static void _bg_up_cb(void *data, Evas_Object *obj, const char* emission, const char* source)
{
	_APPS_D("BG is released");
}

static key_cb_ret_e _back_key_cb(void *data)
{
	_APPS_D("Back key cb");
}

static apps_error_e _pause_result_cb(void *data)
{
	return APPS_ERROR_NONE;
}

static apps_error_e _resume_result_cb(void *data)
{
	return APPS_ERROR_NONE;
}

static apps_error_e _reset_result_cb(void *data)
{
	return APPS_ERROR_NONE;
}

static Eina_Bool _move_timer_cb(void *data)
{
	return ECORE_CALLBACK_RENEW;
}

#define TIME_MOVE_SCROLLER 1.0f
static void _upper_start_cb(void *data, Evas_Object *obj, void *event_info)
{
}

static void _upper_end_cb(void *data, Evas_Object *obj, void *event_info)
{
}

static Evas_Object *_create_checker(Evas_Object *layout, int type)
{
	return NULL;
}

static void _destroy_checker(Evas_Object *checker)
{
}

static Eina_Bool _push_items_cb(void *layout)
{
	return ECORE_CALLBACK_CANCEL;
}


static char *_access_info_cb(void *data, Evas_Object *obj)
{
	return NULL;
}


HAPI Evas_Object *apps_layout_create(Evas_Object *win, const char *file, const char *group)
{
	Evas_Object *layout = NULL;

	_APPS_D("%s", __func__);

	return layout;
}



HAPI void apps_layout_destroy(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

static Eina_Bool _show_anim_cb(void *data)
{
	_APPS_D("%s", __func__);

	return ECORE_CALLBACK_CANCEL;
}

HAPI apps_error_e apps_layout_show(Evas_Object *win, Eina_Bool show)
{
	_APPS_D("%s", __func__);

	return APPS_ERROR_NONE;
}

HAPI Evas_Object* apps_layout_load_edj(Evas_Object *parent, const char *edjname, const char *grpname)
{
	Evas_Object *layout = NULL;

	_APPS_D("%s", __func__);

	return layout;
}

HAPI void apps_layout_unload_edj(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_rotate(Evas_Object *layout, int is_rotated)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_focus_on(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_focus_off(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_block(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_unblock(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI int apps_layout_is_edited(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_layout_edit(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}

static Eina_Bool _update_list_cb(void *data)
{
	_APPS_D("%s", __func__);

	return ECORE_CALLBACK_CANCEL;
}

HAPI void apps_layout_unedit(Evas_Object *layout)
{
	_APPS_D("%s", __func__);
}
// End of file
