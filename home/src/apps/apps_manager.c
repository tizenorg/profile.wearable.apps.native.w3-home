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
#include "apps/apps.h"
#include "apps/apps_view.h"

static struct _apps_s {
	int state;
} apps_info = {
	.state = APPS_STATE_NONE,
};

int apps_get_state(void)
{
	return apps_info.state;
}

apps_error_e apps_init(void)
{
	int ret = 0;

	_APPS_D("%s", __func__);

	apps_info.state = APPS_STATE_NONE;

	ret = apps_view_create();
	if (APPS_ERROR_NONE != ret) {
		_APPS_E("Failed to create apps view");
		return APPS_ERROR_FAIL;
	}

	return APPS_ERROR_NONE;
}

void apps_fini(void)
{
	_APPS_D("%s", __func__);

	apps_info.state = APPS_STATE_NONE;

	apps_view_destroy();
}

apps_error_e apps_show(void)
{
	int ret = 0;

	_APPS_D("Show APPS");

	if (apps_info.state == APPS_STATE_NONE) {
		ret = apps_init();
		if (APPS_ERROR_NONE != ret) {
			_APPS_E("Failed to initialize apps");
			return APPS_ERROR_FAIL;
		}
	}

	apps_info.state = APPS_STATE_SHOW;

	ret = apps_view_show();
	if (APPS_ERROR_NONE != ret) {
		_APPS_E("Failed to show apps view");
		return APPS_ERROR_FAIL;
	}

	return APPS_ERROR_NONE;
}

void apps_hide(void)
{
	_APPS_D("Hide APPS");

	apps_view_hide();

	apps_info.state = APPS_STATE_HIDE;
}
