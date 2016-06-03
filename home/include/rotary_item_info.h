
#ifndef __APPS_ITEM_INFO_H__
#define __APPS_ITEM_INFO_H__

#include <Elementary.h>
#include <Evas.h>
#include <util.h>
#define APP_TYPE_WGT "wgt"
#define APP_TYPE_RECENT_APPS "recent"
#define APP_TYPE_MORE_APPS "more"

typedef struct {
	/* innate features */
	char *pkgid;
	char *appid;
	char *name;
	char *icon;
	char *type;

	int ordering;
	int open_app;
	int tts;
	int removable;
	int uneditable;

	/* acquired features */
	Evas_Object *win;
	Evas_Object *layout;

	Evas_Object *scroller;
	Evas_Object *page;

	Evas_Object *item;
	Evas_Object *item_inner;
	Evas_Object *center;
} item_info_s;

typedef enum {
	ITEM_INFO_LIST_TYPE_INVALID = 0,
	ITEM_INFO_LIST_TYPE_ALL,
	ITEM_INFO_LIST_TYPE_FACTORY_BINARY,
	ITEM_INFO_LIST_TYPE_XML,
	ITEM_INFO_LIST_TYPE_MAX,
} item_info_list_type_e;
#endif // __APPS_ITEM_INFO_H__
