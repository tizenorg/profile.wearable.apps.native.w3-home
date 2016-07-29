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
#include <libxml/xmlreader.h>

#include "log.h"
#include "util.h"
#include "apps/apps_data.h"


#define DEFAULT_ORDER_FILE "apps_default_order.xml"
typedef enum {
	_APPS_XML_INVALID,
	_APPS_XML_APP_ID,
	_APPS_XML_POSITION
}APPS_XML_TYPE;

Eina_List *apps_xml_get_list(void)
{
	xmlTextReaderPtr reader = NULL;
	Eina_List *list = NULL;

	int ret = -1;
	const char *name, *value;

	apps_data_s *item = NULL;
	APPS_XML_TYPE input_type = _APPS_XML_INVALID;

	reader = xmlReaderForFile(util_get_res_file_path(DEFAULT_ORDER_FILE), NULL, 0);
	if (reader == NULL) {
		_APPS_E("xmlReaderForFile : %s failed", util_get_res_file_path(DEFAULT_ORDER_FILE));
		return NULL;
	}

	while ((ret = xmlTextReaderRead(reader)) == 1) {
		int dep = 0;
		int node_type = 0;

		dep = xmlTextReaderDepth(reader);
		node_type = xmlTextReaderNodeType(reader);
		continue_if(-1 == dep || -1 == node_type);
		name = (const char*) xmlTextReaderConstName(reader);
		continue_if(!name);

		value = (const char*) xmlTextReaderConstValue(reader);
		//_APPS_D("dep:%d, node_type:%d, name:%s, value:%s", dep, node_type, name, value);
		if (dep == 1 && node_type == 1) {
			item = (apps_data_s *)malloc(sizeof(apps_data_s));
			if (!item) {
				_APPS_E("memory allocation is failed!!!");
				eina_list_free(list);
				xmlFreeTextReader(reader);
				return NULL;
			}
			memset(item, 0, sizeof(apps_data_s));
		} else if(dep == 1 && node_type == 15) {
			if (item != NULL) {
				list = eina_list_append(list, item);
				item = NULL;
			}
		} else if (dep == 2 && node_type == 1 ) {
			if (strcmp(name, "app_id") == 0) {
				input_type = _APPS_XML_APP_ID;
			} else if (strcmp(name, "position") == 0) {
				input_type = _APPS_XML_POSITION;
			} else {
				input_type = _APPS_XML_INVALID;
			}
		} else if (dep == 3 && node_type == 3) {
			if (input_type == _APPS_XML_APP_ID) {
				item->app_id = strdup(value);
			} else if(input_type == _APPS_XML_POSITION) {
				item->position = atoi(value);
			}
		}
	}
	if (item != NULL) {
		if(item->app_id) free(item->app_id);
		free(item);
	}
	xmlFreeTextReader(reader);

	return list;
}
