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

#include <sqlite3.h>

#include "log.h"
#include "util.h"
#include "apps/apps_data.h"
#include "apps/apps_db.h"

#define APPS_DB_NAME ".apps-data.db"
#define QUERY_MAXLEN	4096
static sqlite3 *apps_db = NULL;

enum {
	COL_ID = 0,
	COL_APPLICATION_ID,
	COL_PACKAGE_ID,
	COL_LABEL,
	COL_ICON,
	COL_POSITION,
};

#define CREATE_APPS_DB_TABLE "create table if not exists apps(\
		id INTEGER PRIMARY KEY AUTOINCREMENT,\
		appId		TEXT,\
		pkgId		TEXT,\
		label		TEXT,\
		iconPath	TEXT,\
		position	TEXT);"

#define UPDATE_APPS_DB_TABLE "UPDATE apps set \
		appId='%s',\
		pkgId='%s',\
		label='%s',\
		iconPath='%s,\
		position=%d' WHERE id = %d"

#define INSERT_APPS_DB_TABLE "INSERT into apps \
		(appId, pkgId, label, iconPath, position)\
		VALUES('%s','%s','%s','%s',%d)"

#define SELECT_ITEM "SELECT * FROM apps;"

static bool _apps_db_open(void);

bool apps_db_create(void)
{
	char *errMsg;
	int ret;
	const char *db_path = util_get_data_file_path(APPS_DB_NAME);
	FILE *fp = fopen(db_path, "r");
	if (fp) {
		fclose(fp);
		LOGE("Apps DB[%s] exist", db_path);
		return false;
	}

	ret = sqlite3_open(db_path, &apps_db);
	if (ret != SQLITE_OK) {
		LOGE("sqlite error : [%d] : path [%s]", ret, db_path);
		return false;
	}
	ret = sqlite3_exec(apps_db, "PRAGMA journal_mode = PERSIST",
		NULL, NULL, &errMsg);
	if (ret != SQLITE_OK) {
		LOGE("SQL error(%d) : %s", ret, errMsg);
		sqlite3_free(errMsg);
		return false;
	}

	ret = sqlite3_exec(apps_db, CREATE_APPS_DB_TABLE, NULL, NULL, &errMsg);
	if (ret != SQLITE_OK) {
		LOGE("SQL error(%d) : %s", ret, errMsg);
		sqlite3_free(errMsg);
		return false;
	}

	return true;
}

bool apps_db_close(void)
{
	if (apps_db) {
		sqlite3_exec(apps_db, "COMMIT TRANSACTION", NULL, NULL, NULL);
		sqlite3_close(apps_db);
		apps_db = NULL;
	}
	return true;
}

bool apps_db_update(apps_data_s *item)
{
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt;
	if (!_apps_db_open())
		return false;

	snprintf(query, QUERY_MAXLEN, UPDATE_APPS_DB_TABLE,
			item->app_id,
			item->pkg_id,
			item->label,
			item->icon,
			item->position,
			item->db_id);
	int ret = sqlite3_prepare(apps_db, query, QUERY_MAXLEN , &stmt, NULL);
	if (ret != SQLITE_OK) {
		LOGE("sqlite error : [%s,%s]", query, sqlite3_errmsg(apps_db));
		return false;
	}
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	apps_db_close();
	return true;

}

bool apps_db_insert(apps_data_s *item)
{
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt;
	if (!_apps_db_open())
		return false;

	snprintf(query, QUERY_MAXLEN, INSERT_APPS_DB_TABLE,
			item->app_id,
			item->pkg_id,
			item->label,
			item->icon,
			item->position);

	int ret = sqlite3_prepare(apps_db, query, QUERY_MAXLEN , &stmt, NULL);
	if (ret != SQLITE_OK) {
		LOGE("sqlite error : [%s,%s]", query, sqlite3_errmsg(apps_db));
		return false;
	}
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	item->db_id = (int)sqlite3_last_insert_rowid(apps_db);

	apps_db_close();
	return true;
}

bool apps_db_delete(apps_data_s *item)
{
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt;
	if (!_apps_db_open())
		return false;

	snprintf(query, QUERY_MAXLEN, "DELETE FROM apps WHERE id=%d", item->db_id);
	int ret = sqlite3_prepare(apps_db, query, QUERY_MAXLEN , &stmt, NULL);
	if (ret != SQLITE_OK) {
		LOGE("sqlite error : [%s,%s]", query, sqlite3_errmsg(apps_db));
		return false;
	}
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	apps_db_close();
	return true;
}

bool apps_db_get_list(Eina_List **apps_db_list)
{
	sqlite3_stmt *stmt;
	const char *str = NULL;

	if (!_apps_db_open())
		return false;

	int ret = sqlite3_prepare_v2(apps_db, SELECT_ITEM, strlen(SELECT_ITEM), &stmt, NULL);
	if (ret != SQLITE_OK) {
		LOGE("sqlite error : [%s,%s]", SELECT_ITEM, sqlite3_errmsg(apps_db));
		return false;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		apps_data_s *item = (apps_data_s *)malloc(sizeof(apps_data_s));
		memset(item, 0, sizeof(apps_data_s));

		item->db_id = sqlite3_column_int(stmt, COL_ID);

		str = (const char *) sqlite3_column_text(stmt, COL_APPLICATION_ID);
		item->app_id = (!str || !strlen(str)) ? NULL : strdup(str);

		str = (const char *) sqlite3_column_text(stmt, COL_PACKAGE_ID);
		item->pkg_id = (!str || !strlen(str)) ? NULL : strdup(str);

		str = (const char *) sqlite3_column_text(stmt, COL_LABEL);
		item->label = (!str) ? NULL : strdup(str);

		str = (const char *)sqlite3_column_text(stmt, COL_ICON);
		item->icon = (!str || !strlen(str)) ? NULL : strdup(str);

		item->position = sqlite3_column_int(stmt, COL_POSITION);
		*apps_db_list = eina_list_append(*apps_db_list, item);
	}
	sqlite3_finalize(stmt);
	apps_db_close();

	return true;
}

static bool _apps_db_open(void)
{
	if (!apps_db) {
		int ret;
		ret = sqlite3_open(util_get_data_file_path(APPS_DB_NAME), &apps_db);
		if (ret != SQLITE_OK) {
			LOGE("sqlite error : [%d] : path [%s]", ret, util_get_data_file_path(APPS_DB_NAME));
			return false;
		}
	}
	return true;
}
