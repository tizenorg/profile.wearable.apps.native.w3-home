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
#include <Elementary.h>
#include <Evas.h>
#include <vconf.h>

#include "log.h"
#include "util.h"

HAPI void apps_main_init(void)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_main_fini(void)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_main_show(void)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_main_pause(void)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_main_resume(void)
{
	_APPS_D("%s", __func__);
}

HAPI void apps_main_hide(void)
{
	_APPS_D("%s", __func__);
}

HAPI Eina_Bool apps_main_is_visible(void)
{
	_APPS_D("%s", __func__);

	return EINA_TRUE;
}

HAPI void apps_main_language_chnage(void)
{
	_APPS_D("%s", __func__);
}
