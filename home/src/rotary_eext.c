#include <Elementary.h>
#include <watch_control.h>
#include <pkgmgr-info.h>
#include <Eina.h>
#include <app.h>
#include "conf.h"
#include "clock_service.h"
#include "layout.h"
#include "log.h"
#include "util.h"
#include "main.h"
#include "page_info.h"
#include "scroller_info.h"
#include "layout_info.h"
#include "scroller.h"
#include "page.h"
#include "apps/apps_main.h"
#include "rotary_item_info.h"
#include "rotary.h"
#include "key.h"
#define ROTARY_SELECTOR_PAGE_COUNT 3
#define ROTARY_SELECTOR_PAGE_ITEM_COUNT 11
void apps_item_info_destroy_rotary(item_info_s *item_info)
{
	ret_if(!item_info);

	if (item_info->pkgid) free(item_info->pkgid);
	if (item_info->appid) free(item_info->appid);
	if (item_info->name) free(item_info->name);
	if (item_info->icon) free(item_info->icon);
	if (item_info->type) free(item_info->type);
	if (item_info) free(item_info);
}

item_info_s *apps_item_info_create_rotary(const char *appid)
{

	item_info_s *item_info = NULL;
	pkgmgrinfo_appinfo_h appinfo_h = NULL;
	pkgmgrinfo_pkginfo_h pkghandle = NULL;
	char *pkgid = NULL;
	char *name = NULL;
	char *icon = NULL;
	char *type = NULL;
	bool nodisplay = false;
	bool enabled = false;
	bool removable = false;

	retv_if(!appid, NULL);

	item_info = calloc(1, sizeof(item_info_s));
	if (NULL == item_info) {
		return NULL;
	}

	goto_if(0 > pkgmgrinfo_appinfo_get_appinfo(appid, &appinfo_h), ERROR);

	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_label(appinfo_h, &name), ERROR);
	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_icon(appinfo_h, &icon), ERROR);
	do {
		break_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_pkgid(appinfo_h, &pkgid));
		break_if(NULL == pkgid);

		break_if(0 > pkgmgrinfo_pkginfo_get_pkginfo(pkgid, &pkghandle));
		break_if(NULL == pkghandle);
	} while (0);

	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_is_nodisplay(appinfo_h, &nodisplay), ERROR);
	if (nodisplay) goto ERROR;

	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_is_enabled(appinfo_h, &enabled), ERROR);
	if (!enabled) goto ERROR;

	goto_if(PMINFO_R_OK != pkgmgrinfo_pkginfo_get_type(pkghandle, &type), ERROR);

	if (pkgid) {
		item_info->pkgid = strdup(pkgid);
		goto_if(NULL == item_info->pkgid, ERROR);
	}

	item_info->appid = strdup(appid);
	goto_if(NULL == item_info->appid, ERROR);

	if (name) {
		if(!strncmp(appid, "com.samsung.app-widget", strlen("com.samsung.app-widget"))) {
			item_info->name = strdup(_("IDS_IDLE_BODY_APPS"));
			goto_if(NULL == item_info->name, ERROR);
		}
		else{
			item_info->name = strdup(name);
			goto_if(NULL == item_info->name, ERROR);
		}
	}

	if (type) {
		item_info->type = strdup(type);
		goto_if(NULL == item_info->type, ERROR);
		if (!strncmp(item_info->type, APP_TYPE_WGT, strlen(APP_TYPE_WGT))) {
			item_info->open_app = 1;
		} else {
			item_info->open_app = 0;
		}
	}
	if (icon) {
				if (strlen(icon) > 0) {
					item_info->icon = strdup(icon);
					goto_if(NULL == item_info->icon, ERROR);
				} else {
					item_info->icon = strdup(DEFAULT_ICON);
					goto_if(NULL == item_info->icon, ERROR);
				}
			} else {
				item_info->icon = strdup(DEFAULT_ICON);
				goto_if(NULL == item_info->icon, ERROR);
			}
	item_info->removable = removable;

	pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
	if (pkghandle) pkgmgrinfo_pkginfo_destroy_pkginfo(pkghandle);

	return item_info;

ERROR:
	apps_item_info_destroy_rotary(item_info);
	if (appinfo_h) pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
	if (pkghandle) pkgmgrinfo_pkginfo_destroy_pkginfo(pkghandle);

	return NULL;
}

static int _apps_sort_cb(const void *d1, const void *d2)
{
	item_info_s *tmp1 = (item_info_s *) d1;
	item_info_s *tmp2 = (item_info_s *) d2;

	if (NULL == tmp1 || NULL == tmp1->name) return 1;
	else if (NULL == tmp2 || NULL == tmp2->name) return -1;

	return strcmp(tmp1->name, tmp2->name);
}
#ifdef TELEPHONY_DISABLE
#define MESSAGE_PKG "com.samsung.message"
#define VCONFKEY_WMS_HOST_STATUS_VENDOR "db/wms/host_status/vendor"
#endif
int _package_info_cb(pkgmgrinfo_appinfo_h handle, void *user_data)
{
	Eina_List **list = user_data;
	char *appid = NULL;
	item_info_s *item_info = NULL;

	retv_if(NULL == handle, 0);
	retv_if(NULL == user_data, 0);

	pkgmgrinfo_appinfo_get_appid(handle, &appid);
	retv_if(NULL == appid, 0);



	item_info = apps_item_info_create_rotary(appid);
	if (NULL == item_info) {
		_E("%s does not have the item_info", appid);
		return 0;
	}

	*list = eina_list_append(*list, item_info);

	return 1;
}

static Eina_List *_read_all_apps_rotary(Eina_List **list)
{
	pkgmgrinfo_appinfo_filter_h handle = NULL;

	retv_if(PMINFO_R_OK != pkgmgrinfo_appinfo_filter_create(&handle), NULL);
	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_filter_add_bool(handle, PMINFO_APPINFO_PROP_APP_NODISPLAY, 0), ERROR);
	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_filter_foreach_appinfo(handle, _package_info_cb, list), ERROR);
	*list = eina_list_sort(*list, eina_list_count(*list), _apps_sort_cb);

ERROR:
	if (handle) pkgmgrinfo_appinfo_filter_destroy(handle);

	return *list;
}

static key_cb_ret_e _back_key_show_clock(void *data)
{
	Evas_Object *win = (Evas_Object *)data;
	evas_object_show(win);
		key_unregister_cb(KEY_TYPE_BACK, _back_key_show_clock);
		return KEY_CB_RET_CONTINUE;

}
static key_cb_ret_e _back_key_hide_new_win(void *data)
{
	Evas_Object *win = (Evas_Object *)data;

	evas_object_hide(win);
		key_unregister_cb(KEY_TYPE_BACK, _back_key_hide_new_win);
		return KEY_CB_RET_CONTINUE;
	
}
static key_cb_ret_e _back_key_hide_rotary(void *data)
{
	Evas_Object *win = (Evas_Object *)data;
	evas_object_hide(win);
		key_unregister_cb(KEY_TYPE_BACK, _back_key_hide_rotary);
		return KEY_CB_RET_CONTINUE;

}
static void
_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	_D("selected");
	Eext_Object_Item *item;
	const char *main_text;
	const char *sub_text;
	/* Get current seleted item object */
	item = eext_rotary_selector_selected_item_get(obj);

	/* Get set text for the item */
	main_text = eext_rotary_selector_item_part_text_get(item, "selector,main_text");
	sub_text = eext_rotary_selector_item_part_text_get(item, "selector,sub_text");

	printf("Item Selected! Currently Selected %s, %s\n", main_text, sub_text);
}

static void
_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	_D("clicked");
	Eext_Object_Item *item;
	const char *main_text;
	const char *sub_text;

	/* Get current seleted item object */
	item = eext_rotary_selector_selected_item_get(obj);
	/* Get set text for the item */
	main_text = eext_rotary_selector_item_part_text_get(item, "selector,main_text");
	sub_text = eext_rotary_selector_item_part_text_get(item, "selector,sub_text");
	app_control_h app_control;
	if (app_control_create(&app_control)== APP_CONTROL_ERROR_NONE)
	{
	//Setting an app ID.
	    if (app_control_set_app_id(app_control, sub_text) == APP_CONTROL_ERROR_NONE)
	    {
	        if(app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_NONE)
	        {
	            LOGI("App launch request sent!");
	        }
	    }
	    if (app_control_destroy(app_control) == APP_CONTROL_ERROR_NONE)
	    {
	        LOGI("App control destroyed.");
	    }
	}
	printf("Item Clicked!, Currently Selected %s, %s\n", main_text, sub_text);
}

void
_item_create(Evas_Object *rotary_selector)
{
	Evas_Object *image;
	Eext_Object_Item * item;
	char buf[255] = {0};
	int i, j;
	Eina_List *pkgmgr_list = NULL;
	_read_all_apps_rotary(&pkgmgr_list);
	item_info_s *temp =  NULL;
	int  value = eina_list_count(pkgmgr_list);
	int count_apps = 0;
		for (i = 0; i < ROTARY_SELECTOR_PAGE_COUNT; i++)
			{
				for (j = 0; j < ROTARY_SELECTOR_PAGE_ITEM_COUNT; j++)
				{
					if(count_apps == value)
						break;
					item = eext_rotary_selector_item_append(rotary_selector);
					temp = (item_info_s *)eina_list_nth(pkgmgr_list,(i*ROTARY_SELECTOR_PAGE_ITEM_COUNT+j));
					image = elm_image_add(rotary_selector);
					elm_image_file_set(image, temp->icon, NULL);

					/* Set the icon of the selector item. */
					eext_rotary_selector_item_part_content_set(item,
																			 "item,bg_image",
																			 EEXT_ROTARY_SELECTOR_ITEM_STATE_NORMAL,
																			 image);

					/* Set the main/sub text visible when the item is selected. */
					snprintf(buf, sizeof(buf), "%s",temp->name);
					eext_rotary_selector_item_part_text_set(item, "selector,main_text", buf);

					snprintf(buf, sizeof(buf), "%s",temp->appid);
					eext_rotary_selector_item_part_text_set(item, "selector,sub_text", buf);
					count_apps++;
				}
			}

}

static w_home_error_e _resume_rotary_result_cb(void *data)
{
	Evas_Object *nf =(Evas_Object *)data ;
	eext_rotary_object_event_activated_set(nf, EINA_TRUE);
	main_unregister_cb(APP_STATE_RESUME, _resume_rotary_result_cb);
	return W_HOME_ERROR_NONE;
	}
key_cb_ret_e  _eext_rotary_selector_cb(void *data)
{
	page_info_s *page_info = NULL;
	Evas_Object *nf =(Evas_Object *)data ;
	Evas_Object *page =(Evas_Object *)data ;
	page_info = evas_object_data_get(page, DATA_KEY_PAGE_INFO);
	//ret_if(!page_info);
		Evas_Object *temp = scroller_get_focused_page(page_info->scroller);
		if(temp == main_get_info()->clock_focus)
		{
			Evas_Object *rotary_selector;
						evas_object_hide(nf);
						Evas_Object *new_win =  elm_win_add(nf,"rotary_selector",ELM_WIN_BASIC);
						rotary_selector = eext_rotary_selector_add(new_win);
						if(rotary_selector != NULL)
							_D("rotary selector made ");
						eext_rotary_object_event_activated_set(rotary_selector, EINA_TRUE);
						_item_create(rotary_selector);
						/* Add smart callback */
						evas_object_smart_callback_add(rotary_selector, "item,selected", _item_selected_cb, NULL);
						evas_object_smart_callback_add(rotary_selector, "item,clicked", _item_clicked_cb, NULL);
						evas_object_show(new_win);
						evas_object_show(rotary_selector);
				
						if (W_HOME_ERROR_NONE != key_register_cb(KEY_TYPE_BACK, _back_key_hide_new_win,new_win)) {
										_APPS_E("Cannot register the key callback");
									}
						if (W_HOME_ERROR_NONE != key_register_cb(KEY_TYPE_BACK,_back_key_hide_rotary,rotary_selector)) {
									_APPS_E("Cannot register the key callback");
								}
						if (W_HOME_ERROR_NONE != key_register_cb(KEY_TYPE_BACK, _back_key_show_clock,nf)) {
							_APPS_E("Cannot register the key callback");
						}
						if (W_HOME_ERROR_NONE != main_register_cb(APP_STATE_RESUME, _resume_rotary_result_cb, rotary_selector)) {
								_E("Cannot register the pause callback");
							}


		}
		else
		{
			_D("different");
		}
		return KEY_CB_RET_STOP;

}
