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
#include <Elementary.h>
#include <efl_assist.h>

#include "log.h"
#include "apps/apps_manager.h"
#include "apps/apps_view.h"

static struct _apps_manager_s {
	int state;
	int mode;
} manager_info = {
	.state = APPS_APP_STATE_POWER_OFF,
	.mode = APPS_MODE_NORMAL,
};

static void _set_mode(int mode)
{
	manager_info.mode = mode;
}

static int _get_mode(void)
{
	return manager_info.mode;
}

int apps_manager_get_state(void)
{
	return manager_info.state;
}

apps_error_e apps_manager_init(void)
{
	int ret = 0;

	_APPS_D("%s", __func__);

	manager_info.state = APPS_APP_STATE_CREATE;

	ret = apps_view_create();
	if (APPS_ERROR_NONE != ret) {
		_APPS_E("Failed to create apps view");
		return APPS_ERROR_FAIL;
	}

	return APPS_ERROR_NONE;
}

void apps_manager_fini(void)
{
	_APPS_D("%s", __func__);

	manager_info.state = APPS_APP_STATE_TERMINATE;

	apps_view_destroy();
}

void apps_manager_pause(void)
{
	_APPS_D("%s", __func__);

	manager_info.state = APPS_APP_STATE_PAUSE;
}

void apps_manager_resume(void)
{
	_APPS_D("%s", __func__);

	manager_info.state = APPS_APP_STATE_RESUME;
}

apps_error_e apps_manager_show(void)
{
	Evas_Object *win = NULL;
	int ret = 0;

	_APPS_D("Show APPS");

	if (manager_info.state == APPS_APP_STATE_POWER_OFF) {
		ret = apps_manager_init();
		if (APPS_ERROR_NONE != ret) {
			_APPS_E("Failed to initialize apps");
			return APPS_ERROR_FAIL;
		}
	}

	apps_manager_resume();

	ret = apps_view_show();
	if (APPS_ERROR_NONE != ret) {
		_APPS_E("Failed to show apps view");
		return APPS_ERROR_FAIL;
	}

	return APPS_ERROR_NONE;
}

void apps_manager_hide(void)
{
	_APPS_D("Hide APPS");

	apps_view_hide();

	apps_manager_pause();
}

