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
#include <Evas.h>
#include <vconf.h>
#ifdef RUN_ON_DEVICE
#include <bincfg.h>
#include <libfactory.h>
#endif
#include <efl_assist.h>
#include <efl_extension.h>
#include <dlog.h>
#include <string.h>
#include <bundle.h>

#include "bg.h"
#include "log.h"
#include "util.h"
#include "main.h"
#include "conf.h"

#define FACTORY_INFO_MAX_LENGTH 2048
#define FACTORY_EDJ_FILE EDJEDIR"/factory.edj"

static Evas_Object *_main_layout_get(void)
{
	Evas_Object *win = main_get_info()->win;
	Evas_Object *layout = NULL;

	if (win != NULL) {
		layout = evas_object_data_get(win, DATA_KEY_LAYOUT);
	}

	return layout;
}

static Evas_Object *_factory_layout_get(void)
{
	Evas_Object *factory_layout = NULL;
	Evas_Object *main_layout = _main_layout_get();
	if (main_layout != NULL) {
		factory_layout = elm_object_part_content_get(main_layout, "factory_info");
	}

	return factory_layout;
}

static void _factory_layout_add(void)
{
	Evas_Object *factory_layout = NULL;
	Evas_Object *main_layout = _main_layout_get();
	if (main_layout != NULL) {
		factory_layout = elm_layout_add(main_layout);
		ret_if(!factory_layout);

		if (elm_layout_file_set(factory_layout, FACTORY_EDJ_FILE, "factory") == EINA_TRUE) {
			Evas_Object *bg = evas_object_rectangle_add(main_get_info()->e);
			if (bg) {
				evas_object_size_hint_min_set(bg, ELM_SCALE_SIZE(BASE_WIDTH), ELM_SCALE_SIZE(BASE_HEIGHT));
				evas_object_size_hint_max_set(bg, ELM_SCALE_SIZE(BASE_WIDTH), ELM_SCALE_SIZE(BASE_HEIGHT));
				evas_object_color_set(bg, 0, 0, 0, 0);
				evas_object_show(bg);
				elm_object_part_content_set(factory_layout, "bg", bg);
			}
			evas_object_size_hint_weight_set(factory_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_object_part_content_set(main_layout, "factory_info", factory_layout);
			evas_object_show(factory_layout);
		} else {
			_E("Failed to add factory layout");
		}
	}
}

static void _factory_layout_del(void)
{
	Evas_Object *factory_layout = NULL;
	Evas_Object *main_layout = _main_layout_get();
	if (main_layout != NULL) {
		factory_layout = elm_object_part_content_unset(main_layout, "factory_info");
		ret_if(!factory_layout);

		evas_object_del(factory_layout);
	}
}

#define STR_COLOR "COLOR:"
static const char *_change_bg_color(const char *buf)
{
	char *color_adr;
	int len = strlen(STR_COLOR);
	int i = 0;

	color_adr = strstr(buf, STR_COLOR);
	retv_if(!color_adr, NULL);

	for (; i < len; i++) {
		color_adr++;
	}

	return color_adr;
}


static void _set_bg_rgb(Evas_Object *bg, const char *buf)
{
	int rgb;
	int r;
	int g;
	int b;

	ret_if(!bg);
	ret_if(!buf);

	rgb = strtoul(buf, NULL, 16);

	r = (rgb & 0x00FF0000) >> 16;
	g = (rgb & 0x0000FF00) >> 8;
	b = rgb & 0x000000FF;

	evas_object_color_set(bg, r, g, b, 150);
}


static void _factory_layout_text_set(Evas_Object *layout)
{
#ifdef RUN_ON_DEVICE
	char *tmp = NULL;
	const char *color;
	char buf[FACTORY_INFO_MAX_LENGTH + 1] = { 0, };
	int ret = 0;

	ret_if(!layout);

	ret = ftm_get_idle_info_str(buf, FACTORY_INFO_MAX_LENGTH);

	color = _change_bg_color(buf);
	if (color) {
		_D("color:%s", color);

		Evas_Object *bg = elm_object_part_content_get(layout, "bg");
		if (bg) {
			_set_bg_rgb(bg, color);
		}
	}

	if (ret == 0) {
		char *color_addr = strstr(buf, STR_COLOR);
		if (color_addr != NULL) {
			*color_addr = '\0';
		}

		tmp = elm_entry_utf8_to_markup(buf);
		ret_if(tmp == NULL);

		Eina_Strbuf *strbuf = eina_strbuf_manage_new(tmp);
		if (strbuf != NULL) {
			eina_strbuf_replace_all(strbuf, "&lt;", "<");
			eina_strbuf_replace_all(strbuf, "&gt;", ">");
			elm_object_part_text_set(layout, "text", eina_strbuf_string_get(strbuf));
			eina_strbuf_free(strbuf);
		} else {
			elm_object_part_text_set(layout, "text", tmp);
			free(tmp);
		}
	} else {
		_E("Failed to get factory idle info:%d", ret);
	}
#endif
}

static void _factory_layout_update(void)
{
	int ret = -1;
	int idle_info_state = -1;

	ret = vconf_get_int(VCONFKEY_FACTORY_IDLE_INFO_STATE, &idle_info_state);
	if (ret == 0 && idle_info_state == VCONFKEY_FACTORY_IDLE_INFO_READY) {
		Evas_Object *factory_layout = _factory_layout_get();
		if (factory_layout != NULL) {
			_factory_layout_text_set(factory_layout);
		}
	}
}

static void _factory_idle_info_state_vconf_cb(keynode_t *node, void *data)
{
	_D("");
	_factory_layout_update();
}

/*!
 * constructor/deconstructor
 */
HAPI void factory_init(void)
{
#ifdef RUN_ON_DEVICE
	if (bincfg_is_factory_binary()) {
		_factory_layout_add();
		_factory_layout_update();
		if(vconf_notify_key_changed(VCONFKEY_FACTORY_IDLE_INFO_STATE,
			_factory_idle_info_state_vconf_cb, NULL) < 0) {
			_E("Failed to register the factory idle info state callback");
		}
		if(vconf_notify_key_changed(VCONFKEY_SYSMAN_BATTERY_CAPACITY,
			_factory_idle_info_state_vconf_cb, NULL) < 0) {
			_E("Failed to register the factory idle info state callback");
		}
	}
#endif
}

HAPI void factory_fini(void)
{
#ifdef RUN_ON_DEVICE
	if (bincfg_is_factory_binary()) {
		vconf_ignore_key_changed(VCONFKEY_FACTORY_IDLE_INFO_STATE,
			_factory_idle_info_state_vconf_cb);
		vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_CAPACITY,
			_factory_idle_info_state_vconf_cb);
		_factory_layout_del();
	}
#endif
}

HAPI int factory_is_factory_binary(void)
{
#ifdef RUN_ON_DEVICE
	return bincfg_is_factory_binary();
#else
	return 0;
#endif
}
// End of a file
