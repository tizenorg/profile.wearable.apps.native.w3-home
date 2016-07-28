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

#include "log.h"
#include "util.h"

#include "apps/apps_data.h"
#include "apps/apps_package_manager.h"
#include "apps/apps_db.h"
#include "apps/apps_xml.h"


static struct {
	Eina_List *data_list;
} apps_data_info = {
	.data_list = NULL,
};

static void _apps_data_free(apps_data_s *item);
static void _apps_data_delete_index(int delete_index);
static void _apps_data_insert_index(int insert_index);
static void _apps_data_print();

void apps_data_init(void)
{
	Eina_List *db_list = NULL;
	Eina_List *pkg_list = NULL;
	Eina_List *order_list = NULL;
	_APPS_D("");
	if (apps_data_info.data_list != NULL) {
		_APPS_D("exist");
		_apps_data_print();
		return;
	}

	apps_package_manager_init();
	apps_package_manager_get_list(&pkg_list);

	order_list = apps_xml_read_list();

	if(!apps_db_create()){
		_APPS_D("DB EXIT");
		apps_db_get_list(&db_list);

		Eina_List *find_db_list = NULL;
		apps_data_s *db_item = NULL;
		Eina_List *find_pkg_list = NULL;
		apps_data_s *pkg_item = NULL;
		bool b_find = false;

		EINA_LIST_FOREACH(db_list, find_db_list, db_item) {
			b_find = false;
			find_pkg_list = pkg_item = NULL;

			EINA_LIST_FOREACH(pkg_list, find_pkg_list, pkg_item) {
				if (strcmp(db_item->app_id, pkg_item->app_id) == 0) {
					b_find = true;
					break;
				}
			}
			if (b_find) {
				apps_data_info.data_list = eina_list_append(apps_data_info.data_list, db_item);
				if (pkg_item) {
					pkg_list = eina_list_remove(pkg_list, pkg_item);
					_apps_data_free(pkg_item);
				}
			} else {
				_apps_data_free(db_item);
			}
		}
		db_list = eina_list_free(db_list);

	}

	Eina_List *find_list = NULL;
	apps_data_s *item = NULL;
	EINA_LIST_FOREACH(pkg_list, find_list, item) {
		apps_data_insert(item);
	}
	pkg_list = eina_list_free(pkg_list);

	_apps_data_print();
}

void apps_data_fini(void)
{
	_APPS_D("");
	apps_package_manager_fini();

	Eina_List *find_list = NULL;
	apps_data_s *item = NULL;
	EINA_LIST_FOREACH(apps_data_info.data_list, find_list, item) {
		_apps_data_free(item);
	}
	apps_data_info.data_list = eina_list_free(apps_data_info.data_list);
}

Eina_List *apps_data_get_list(void)
{
	if (apps_data_info.data_list == NULL) {
		apps_package_manager_get_list(&apps_data_info.data_list);
	}
	_apps_data_print();
	return apps_data_info.data_list;
}

void apps_data_delete(apps_data_s *item)
{
	if (apps_data_info.data_list == NULL) {
		_APPS_E("apps_data_info.data_list == NULL");
		return;
	}
	apps_data_info.data_list = eina_list_remove(apps_data_info.data_list, item);
	apps_db_delete(item);
	_apps_data_delete_index(item->position);
	//TODO:Delete from view
	_apps_data_free(item);
	_apps_data_print();
}

void apps_data_delete_by_pkg_id(const char *pkg_id)
{
	_APPS_D("");
	Eina_List *find_list = NULL;
	apps_data_s *item = NULL;
	Eina_List *delete_list = NULL;

	if (apps_data_info.data_list == NULL) {
		_APPS_E("apps_data_info.data_list == NULL");
		return;
	}

	EINA_LIST_FOREACH(apps_data_info.data_list, find_list, item) {
		if (strcmp(pkg_id, item->pkg_id) == 0){
			delete_list = eina_list_append(delete_list,item);
		}
	}

	find_list = NULL;
	item = NULL;
	EINA_LIST_FOREACH(delete_list, find_list, item) {
		apps_data_info.data_list = eina_list_remove(apps_data_info.data_list, item);
		apps_db_delete(item);
		_apps_data_delete_index(item->position);
		//TODO:delete from view
		_apps_data_free(item);
	}
	eina_list_free(delete_list);
	_apps_data_print();
}

void apps_data_delete_by_app_id(const char *app_id)
{
	Eina_List *find_list = NULL;
	apps_data_s *item = NULL;

	if (apps_data_info.data_list == NULL) {
		_APPS_E("apps_data_info.data_list == NULL");
		return;
	}

	EINA_LIST_FOREACH(apps_data_info.data_list, find_list, item) {
		if (strcmp(app_id, item->app_id) == 0) {
			apps_data_info.data_list = eina_list_remove(apps_data_info.data_list, item);
			apps_db_delete(item);
			_apps_data_delete_index(item->position);
			//TODO:delete from view
			_apps_data_free(item);
			break;
		}
	}
	//update DB
	_apps_data_print();
}

void apps_data_insert(apps_data_s *item)
{
	_APPS_D("insert %s", item->label);
	apps_data_info.data_list = eina_list_append(apps_data_info.data_list, item);
	item->position = eina_list_count(apps_data_info.data_list)-1;
	apps_db_insert(item);
	//TODO:insert to View.
	_apps_data_print();
}

static void _apps_data_free(apps_data_s *item)
{
	_APPS_D("%s free ", item->label);
	if(item && item->app_id)
		free(item->app_id);
	if(item && item->pkg_id)
		free(item->pkg_id);
	if(item && item->icon)
		free(item->icon);
	if(item && item->label)
		free(item->label);
	if(item)
		free(item);
}

static void _apps_data_delete_index(int delete_index)
{
	Eina_List *find_list = NULL;
	apps_data_s *item = NULL;

	if (apps_data_info.data_list == NULL) {
		_APPS_E("apps_data_info.data_list == NULL");
		return;
	}

	EINA_LIST_FOREACH(apps_data_info.data_list, find_list, item) {
		if (item->position > delete_index)
			-- item->position;
	}
}
static void _apps_data_insert_index(int insert_index)
{
	Eina_List *find_list = NULL;
	apps_data_s *item = NULL;

	if (apps_data_info.data_list == NULL) {
		_APPS_E("apps_data_info.data_list == NULL");
		return;
	}

	EINA_LIST_FOREACH(apps_data_info.data_list, find_list, item) {
		if (item->position >= insert_index)
			++ item->position;
	}
}

static void _apps_data_print()
{
	Eina_List *find_list = NULL;
	apps_data_s *item = NULL;
	int i = 0 ;

	if (apps_data_info.data_list == NULL) {
		_APPS_E("apps_data_info.data_list == NULL");
		return;
	}

	_APPS_D("================================= ");
	EINA_LIST_FOREACH(apps_data_info.data_list, find_list, item) {
		_APPS_D("[%d] %s , position : %d", i++, item->label, item->position);
	}
	_APPS_D("================================= ");
}
