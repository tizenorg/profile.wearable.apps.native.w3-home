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
#include "wms.h"
#include "noti_broker.h"

#define LAYOUT_EDJE EDJEDIR"/apps_layout.edj"
#define THEME_EDJE EDJEDIR"/style.edj"
#define LAYOUT_GROUP_NAME "layout"

static apps_main_s apps_main_info = {
	.theme = NULL,
	.font_theme = NULL,
	.first_boot = 0,
	.tts = 0,
	.longpress_edit_disable = false,
	.state = APPS_APP_STATE_POWER_OFF,
	.mode = APPS_MODE_NORMAL,
	.win = NULL,
	.layout = NULL,
};



HAPI apps_main_s *apps_main_get_info(void)
{
	return &apps_main_info;
}

HAPI apps_error_e apps_main_register_cb(
	apps_main_s *apps_main_info,
	int state,
	apps_error_e (*result_cb)(void *), void *result_data)
{
	return APPS_ERROR_NONE;
}

HAPI void apps_main_unregister_cb(
	apps_main_s *apps_main_info,
	int state,
	apps_error_e (*result_cb)(void *))
{
	return;
}

HAPI void apps_main_init(void)
{
	_APPS_D("%s", __func__);

	apps_main_info.state = APPS_APP_STATE_CREATE;
	apps_main_info.mode = APPS_MODE_NORMAL;
}

HAPI void apps_main_fini(void)
{
	_APPS_D("%s", __func__);

	apps_main_info.state = APPS_APP_STATE_TERMINATE;
	apps_main_info.mode = APPS_MODE_NORMAL;
}

HAPI void apps_main_launch(int launch_type)
{
	if (apps_main_info.win == NULL) {
		apps_main_init();
	}

	_APPS_D("launch type : %d", launch_type);

	if (launch_type == APPS_LAUNCH_SHOW) {
		_APPS_D("Show Apps");
	} else if (launch_type == APPS_LAUNCH_HIDE) {
		_APPS_D("Hide Apps");
	}
}


HAPI void apps_main_pause(void)
{
	_APPS_D("%s", __func__);

	apps_main_info.state = APPS_APP_STATE_PAUSE;
	apps_main_info.mode = APPS_MODE_NORMAL;
}

HAPI void apps_main_resume(void)
{
	_APPS_D("%s", __func__);

	apps_main_info.state = APPS_APP_STATE_RESUME;
	apps_main_info.mode = APPS_MODE_NORMAL;
}

HAPI void apps_main_language_chnage(void)
{
	_APPS_D("%s", __func__);
}

HAPI Eina_Bool apps_main_is_visible()
{
	Eina_Bool is_visible = EINA_FALSE;

	_APPS_D("%s", __func__);

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
