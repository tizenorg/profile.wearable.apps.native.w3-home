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
#include <package_manager.h>
#include <app_manager.h>
#include <package_info.h>
#include <app_info.h>
#include <Elementary.h>

#include "log.h"
#include "util.h"
#include "apps/apps_data.h"
#include "apps/apps_package_manager.h"

static package_manager_h pkg_mgr = NULL;

static void __apps_package_manager_event_cb(const char *type, const char *package,
		package_manager_event_type_e event_type, package_manager_event_state_e event_state, int progress,
		package_manager_error_e error, void *user_data);
static bool __apps_package_manager_get_item(app_info_h app_handle, void *data);
static bool __apps_package_get_apps_info(app_info_h app_handle, apps_data_s **item);
static void __apps_package_manager_install(const char* package);
static void __apps_package_manager_uninstall(const char* package);

void apps_package_manager_init(void)
{
	int ret;
	if (pkg_mgr != NULL)
		return;

	ret = package_manager_create(&pkg_mgr);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		_APPS_E("package_manager_create : failed[%d]", ret);
	}

	ret = package_manager_set_event_status(pkg_mgr, PACKAGE_MANAGER_STATUS_TYPE_INSTALL|PACKAGE_MANAGER_STATUS_TYPE_UNINSTALL);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		_APPS_E("package_manager_set_event_status : failed[%d]", ret);
	}

	ret = package_manager_set_event_cb(pkg_mgr, __apps_package_manager_event_cb, NULL);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		_APPS_E("package_manager_set_event_cb : failed[%d]", ret);
	}

	_APPS_D("end");
}

void apps_package_manager_fini(void)
{
	if (pkg_mgr) {
		package_manager_destroy(pkg_mgr);
		pkg_mgr = NULL;
	}
}

bool apps_package_manager_get_list(Eina_List **list)
{
	_APPS_D("called");
	int ret;
	app_info_filter_h handle = NULL;

	ret = app_info_filter_create(&handle);
	if (ret != APP_MANAGER_ERROR_NONE) {
		_APPS_D("app_info_filter_create failed: %d", ret);
		return EINA_FALSE;
	}
	app_info_filter_add_bool(handle, PACKAGE_INFO_PROP_APP_NODISPLAY , false);
	app_info_filter_foreach_appinfo(handle, __apps_package_manager_get_item, list);

	_APPS_D("end");
	return EINA_TRUE;
}

void apps_package_manager_update_label(Eina_List *list)
{
	app_info_h app_info = NULL;
	int ret = APP_MANAGER_ERROR_NONE;

	apps_data_s *item = NULL;
	Eina_List *find_list;
	EINA_LIST_FOREACH(list, find_list, item) {
		if (item->label)
			free(item->label);
		app_manager_get_app_info(item->app_id, &app_info);
		ret = app_info_get_label(app_info, &item->label);
		if (APP_MANAGER_ERROR_NONE != ret) {
			_APPS_E("app_info_get_label return [%d] %s", ret, item->label);
		}
	}
}

static void __apps_package_manager_event_cb(const char *type, const char *package,
		package_manager_event_type_e event_type, package_manager_event_state_e event_state, int progress,
		package_manager_error_e error, void *user_data)
{
	if (event_state == PACKAGE_MANAGER_EVENT_STATE_STARTED) {
		_APPS_D("pkg:%s type:%d state:PACKAGE_MANAGER_EVENT_STATE_STARTED", package, event_type);
	} else if (event_state == PACKAGE_MANAGER_EVENT_STATE_PROCESSING) {
		_APPS_D("pkg:%s type:%d PACKAGE_MANAGER_EVENT_STATE_PROCESSING", package, event_type);
	} else if (event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED) {
		_APPS_D("pkg:%s type:%d PACKAGE_MANAGER_EVENT_STATE_COMPLETED", package, event_type);
		if (event_type == PACKAGE_MANAGER_EVENT_TYPE_INSTALL) {
			__apps_package_manager_install(package);
		} else if (event_type == PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL) {
			__apps_package_manager_uninstall(package);
		} else { //PACKAGE_MANAGER_EVENT_TYPE_UPDATE
			_APPS_D("UPDATE - %s", package);
		}
	} else {
		_APPS_E("pkg:%s type:%d state:PACKAGE_MANAGER_EVENT_STATE_COMPLETED: FAILED", package, event_type);
	}
}

static bool __apps_package_manager_get_item(app_info_h app_handle, void *data)
{
	Eina_List **list = (Eina_List **)data;
	apps_data_s *item = NULL;
	if (__apps_package_get_apps_info(app_handle, &item)) {
		*list = eina_list_append(*list, item);
	}
	return true;
}

static bool __apps_package_get_apps_info(app_info_h app_handle, apps_data_s **item)
{
	bool nodisplay = false;
	int ret;
	package_info_h p_handle = NULL;

	apps_data_s *new_item = (apps_data_s *)malloc(sizeof(apps_data_s));
	if (!new_item) {
		_APPS_E("memory allocation is failed!!!");
		return false;
	}

	memset(new_item, 0, sizeof(apps_data_s));
	*item = new_item;

	app_info_is_nodisplay(app_handle, &nodisplay);
	if (nodisplay)
		goto ERROR;

	new_item->db_id = -1;
	new_item->position = -1;

	ret = app_info_get_app_id(app_handle, &new_item->app_id);
	if (APP_MANAGER_ERROR_NONE != ret) {
		_APPS_E("app_info_get_app_id return [%d] %s", ret, new_item->app_id);
		goto ERROR;
	}

	ret = app_info_get_package(app_handle, &new_item->pkg_id);
	if (APP_MANAGER_ERROR_NONE != ret) {
		_APPS_E("app_info_get_package return [%d] %s", ret, new_item->pkg_id);
		goto ERROR;
	}

	ret = app_info_get_label(app_handle, &new_item->label);
	if (APP_MANAGER_ERROR_NONE != ret) {
		_APPS_E("app_info_get_label return [%d] %s", ret, new_item->label);
		goto ERROR;
	}

	ret = app_info_get_icon(app_handle, &new_item->icon);
	if (APP_MANAGER_ERROR_NONE != ret) {
		_APPS_E("app_info_get_icon return [%d]", ret);
		goto ERROR;
	}

	ret = package_manager_get_package_info(new_item->pkg_id, &p_handle);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		_APPS_E("Failed to inialize package handle for item : %s", new_item->pkg_id);
		goto ERROR;
	}

	ret = package_info_is_removable_package(p_handle, &new_item->is_removable);
	if (PACKAGE_MANAGER_ERROR_NONE != ret) {
		_APPS_E("package_info_is_removable_package  return [%d]", ret);
		goto ERROR;
	}

	if (!new_item->icon || !ecore_file_can_read(new_item->icon)) {
		if (new_item->icon) free(new_item->icon);
		new_item->icon = malloc(MAX_FILE_PATH_LEN);
		snprintf(new_item->icon, MAX_FILE_PATH_LEN,
				"%s", util_get_res_file_path(IMAGE_DIR"/unknown.png"));
	}
	return true;

ERROR:
	if (new_item && new_item->app_id)
		free(new_item->app_id);
	if (new_item && new_item->icon)
		free(new_item->icon);
	if (new_item && new_item->label)
		free(new_item->label);
	if (new_item && new_item->pkg_id)
		free(new_item->pkg_id);
	if (new_item) free(new_item);
	return false;
}

static void __apps_package_manager_install(const char* package)
{
	app_info_h app_info = NULL;
	apps_data_s *item = NULL;

	app_manager_get_app_info(package, &app_info);
	if (__apps_package_get_apps_info(app_info, &item)) {
		apps_data_insert(item);
	}
	app_info_destroy(app_info);
}

static void __apps_package_manager_uninstall(const char* package)
{
	_APPS_D("");
	apps_data_delete_by_pkg_id(package);
}
