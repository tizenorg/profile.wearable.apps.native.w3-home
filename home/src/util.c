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

#include <aul.h>
#include <app.h>
#include <appsvc.h>
#include <bundle.h>
#include <Elementary.h>
#include <Evas.h>
#include <dlog.h>
#include <notification_status.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vconf.h>
#include <efl_assist.h>
#include <efl_extension.h>
#include <pkgmgr-info.h>
#include <widget_service.h>

#include <unicode/unum.h>
#include <unicode/ustring.h>

#include "conf.h"
#include "log.h"
#include "util.h"
#include "main.h"
#include "power_mode.h"
#include "dbus.h"
#include "layout.h"
#include "layout_info.h"
#include "clock_service.h"

#include <widget_errno.h>
#include <Ecore.h>

#include "apps/layout.h"
#include "apps/apps_main.h"
//#include "tutorial.h"



HAPI int util_launch_app(const char *appid, const char *key, const char *value)
{
	int pid = 0;

	retv_if(!appid, W_HOME_ERROR_INVALID_PARAMETER);

	if (key && value) {
		bundle *b = bundle_create();
		retv_if(!b, 0);

		bundle_add(b, key, value);
		home_dbus_cpu_booster_signal_send();
		pid = aul_launch_app(appid, b);
		bundle_free(b);
	} else {
		home_dbus_cpu_booster_signal_send();
		pid = aul_launch_app(appid, NULL);
	}

	if (pid < 0) {
		_E("Failed to launch %s(%d)", appid, pid);
	}

	return pid;
}



HAPI int util_feature_enabled_get(int feature)
{
	int is_enabled = 1;

	if (feature & FEATURE_CLOCK_HIDDEN_BUTTON) {
		if (main_get_info()->is_tts) {
			return 0;
		}
		if (cooldown_mode_enabled_get() == 1) {
			return 0;
		}

	} else if (feature & FEATURE_CLOCK_VISUAL_CUE) {
		if (main_get_info()->is_tts) {
			return 0;
		}
		if (main_get_info()->is_alpm_clock_enabled == 1) {
			return 0;
		}
		if (emergency_mode_enabled_get() == 1) {
			return 0;
		}
		if (cooldown_mode_enabled_get() == 1) {
			return 0;
		}
		if (apps_main_show_count_get() > 3) {
			return 0;
		}
	} else if (feature & FEATURE_CLOCK_SELECTOR) {
#ifndef ENABLE_INDICATOR_BRIEFING_VIEW
		if (clock_manager_view_state_get(CLOCK_VIEW_TYPE_DRAWER) == 1) {
			return 0;
		}
#endif
		if (main_get_info()->is_tts) {
			return 0;
		}
		if (emergency_mode_enabled_get() == 1) {
			return 0;
		}
		if (cooldown_mode_enabled_get() == 1) {
			return 0;
		}
	} else if (feature & FEATURE_APPS_BY_BEZEL_UP) {
		if (main_get_info()->is_tts) {
			return 0;
		}
#ifndef ENABLE_INDICATOR_BRIEFING_VIEW
		if (clock_manager_view_state_get(CLOCK_VIEW_TYPE_DRAWER) == 1) {
			return 0;
		}
#endif
		if (emergency_mode_enabled_get() == 1) {
			return 0;
		}
		if (cooldown_mode_enabled_get() == 1) {
			return 0;
		}
	} else if (feature & FEATURE_APPS) {
		if (emergency_mode_enabled_get() == 1) {
			return 0;
		}
		if (cooldown_mode_enabled_get() == 1) {
			return 0;
		}
	} else if (feature & FEATURE_CLOCK_CHANGE) {
		Evas_Object *win = main_get_info()->win;
		retv_if(win == NULL, is_enabled);
		Evas_Object *layout = evas_object_data_get(win, DATA_KEY_LAYOUT);
		retv_if(layout == NULL, is_enabled);

		// disabled in editing
		if (layout_is_edit_mode(layout) == 1) {
			return 0;
		}
	} else if (feature & FEATURE_TUTORIAL) {
		if (emergency_mode_enabled_get() == 1) {
			return 0;
		}
		if (cooldown_mode_enabled_get() == 1) {
			return 0;
		}
	}

	return is_enabled;
}

/*!
 *
 * usage)
 * app_service(...)
 * {
 * if (power_key pressed) {
 * 	int pid;
 * 	pid = util_find_top_visible_window();
 * 	if (pid == getpid()) {
 *		// if (first page) {
 *		//     LCD_OFF
 *		// } else {
 *		//     SCROLL TO THE FIRST PAGE
 *		// }
 * 	} else {
 *		// elm_win_activate(my_win);
 * 	}
 * }
 *
 */



////////////////////////////////////////////////////////////////////
//  Apps util

HAPI void _evas_object_event_changed_size_hints_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord w, h;

	evas_object_size_hint_min_get(obj, &w, &h);
	_D("%s : min (%d:%d)", data, w, h);

	evas_object_size_hint_max_get(obj, &w, &h);
	_D("%s : max (%d:%d)", data, w, h);
}



HAPI void _evas_object_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	_D("%s(%p) is resized to (%d, %d, %d, %d)", data, obj, x, y, w, h);

	evas_object_size_hint_min_get(obj, &w, &h);
	_D("%s : min (%d:%d)", data, w, h);

	evas_object_size_hint_max_get(obj, &w, &h);
	_D("%s : max (%d:%d)", data, w, h);
}



void _evas_object_event_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_D("%s(%p) IS REMOVED!", (const char *) data, obj);
}



void _evas_object_event_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	_D("%s's GEOMETRY : [%d, %d, %d, %d]", (const char *) data, x, y, w, h);
}



void _evas_object_event_show_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	_D("%s(%p)'s GEOMETRY : [%d, %d, %d, %d]", (const char *) data, obj, x, y, w, h);
}



void _evas_object_event_hide_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	_D("%s(%p)'s GEOMETRY : [%d, %d, %d, %d]", (const char *) data, obj, x, y, w, h);
}



static Eina_Bool _unblock_cb(void *data)
{
	retv_if(NULL == data, EINA_FALSE);
	apps_layout_unblock(data);

	return EINA_FALSE;
}



HAPI void apps_util_post_message_for_launch_fail(const char *name)
{
	ret_if(NULL == name);

	int len = strlen(_("IDS_AT_TPOP_UNABLE_TO_OPEN_PS")) + strlen(name) + SPARE_LEN;

	char *inform = calloc(len, sizeof(char));
	ret_if(NULL == inform);

	snprintf(inform, len, _("IDS_AT_TPOP_UNABLE_TO_OPEN_PS"), name);
	notification_status_message_post(inform);

	free(inform);

	return;
}



static void _svc_cb(bundle *b, int request_code, appsvc_result_val result, void *data)
{
	_D("Request code : %d");

	if (result != APPSVC_RES_OK) {
		char* inform;
		char* inform_with_ret;
		int len;

		const char *name = data;
		ret_if(NULL == name);

		// IDS_AT_TPOP_UNABLE_TO_OPEN_PS : "Unable to open %s"
		len = strlen(_("IDS_AT_TPOP_UNABLE_TO_OPEN_PS")) + strlen(name) + SPARE_LEN;

		inform = calloc(len, sizeof(char));
		if (!inform) {
			_E("cannot calloc for information");
			return;
		}
		snprintf(inform, len, _("IDS_AT_TPOP_UNABLE_TO_OPEN_PS"), name);

		inform_with_ret = calloc(len, sizeof(char));
		if (!inform_with_ret) {
			_E("cannot calloc for information");
			free(inform);
			return;
		}
		snprintf(inform_with_ret, len, "%s(%d)", inform, result);
		notification_status_message_post(inform_with_ret);

		free(inform);
		free(inform_with_ret);
	}
}



#define LAUNCH_WITH_SPECIAL_ROUTINE false
#define PACKAGE_ORANGE_WORLD "widgetMor.orangeMobile"
static bool _launch_special_package(const char *package, const char *name)
{
	retv_if(NULL == package, false);
	retv_if(NULL == name, false);

	if (!strcmp(package, PACKAGE_ORANGE_WORLD)) {
		_D("Special launch for %s", PACKAGE_ORANGE_WORLD);
		bundle *b = bundle_create();
		retv_if(NULL == b, false);

		appsvc_set_operation(b, APPSVC_OPERATION_VIEW);
		appsvc_set_uri(b, "http://m.orange.fr");
		appsvc_run_service(b, 0, _svc_cb, (void *) name);

		bundle_free(b);

		return true;
	}

	return false;
}



#define LAYOUT_BLOCK_INTERVAL		1.0
HAPI void apps_util_launch(Evas_Object *win, const char *package, const char *name)
{
	ret_if(NULL == package);

	if (!name) name = package;
	if (LAUNCH_WITH_SPECIAL_ROUTINE && _launch_special_package(package, name)) {
		return;
	}

	home_dbus_cpu_booster_signal_send();

	int ret_aul = aul_open_app(package);
	if (ret_aul < AUL_R_OK) {

		if(ret_aul == AUL_R_EREJECTED && cooldown_mode_warning_get()) {
			util_create_toast_popup(win, _("IDS_SM_BODY_THE_DEVICE_TEMPERATURE_IS_TOO_HIGH"));
		}
		else {
			char* inform;
			char* inform_with_ret;
			int len;

			// IDS_AT_TPOP_UNABLE_TO_OPEN_PS : "Unable to open %s"
			len = strlen(_("IDS_AT_TPOP_UNABLE_TO_OPEN_PS")) + strlen(name) + SPARE_LEN;

			inform = calloc(len, sizeof(char));
			if (!inform) {
				_E("cannot calloc for information");
				return;
			}
			snprintf(inform, len, _("IDS_AT_TPOP_UNABLE_TO_OPEN_PS"), name);

			inform_with_ret = calloc(len, sizeof(char));
			if (!inform_with_ret) {
				_E("cannot calloc for information");
				free(inform);
				return;
			}
			snprintf(inform_with_ret, len, "%s(%d)", inform, ret_aul);
			notification_status_message_post(inform_with_ret);

			free(inform);
			free(inform_with_ret);
		}
	} else {
		apps_util_notify_to_home(ret_aul);
		_D("Launch app's ret : [%d]", ret_aul);
		_T(package);

		Evas_Object *layout = evas_object_data_get(win, DATA_KEY_LAYOUT);
		ret_if(NULL == layout);
		apps_layout_block(layout);
		ecore_timer_add(LAYOUT_BLOCK_INTERVAL, _unblock_cb, layout);
	}
}



HAPI void apps_util_launch_main_operation(Evas_Object *win, const char *app_id, const char *name)
{
	ret_if(NULL == app_id);

	if (!name) name = app_id;
	app_control_h service = NULL;
	ret_if(APP_CONTROL_ERROR_NONE != app_control_create(&service));
	ret_if(NULL == service);

	app_control_set_operation(service, APP_CONTROL_OPERATION_MAIN);
	app_control_set_app_id(service, app_id);

	home_dbus_cpu_booster_signal_send();

	int ret = app_control_send_launch_request(service, NULL, NULL);
	if (APP_CONTROL_ERROR_NONE != ret) {

		if(ret == APP_CONTROL_ERROR_LAUNCH_REJECTED && cooldown_mode_warning_get()) {
			util_create_toast_popup(win, _("IDS_SM_BODY_THE_DEVICE_TEMPERATURE_IS_TOO_HIGH"));
		}
		else {
			char* inform;
			char* inform_with_ret;
			int len;

			// IDS_AT_TPOP_UNABLE_TO_OPEN_PS : "Unable to open %s"
			len = strlen(_("IDS_AT_TPOP_UNABLE_TO_OPEN_PS")) + strlen(name) + SPARE_LEN;

			inform = calloc(len, sizeof(char));
			if (!inform) {
				_E("cannot calloc for information");
				goto ERROR;
			}
			snprintf(inform, len, _("IDS_AT_TPOP_UNABLE_TO_OPEN_PS"), name);

			inform_with_ret = calloc(len, sizeof(char));
			if (!inform_with_ret) {
				_E("cannot calloc for information");
				free(inform);
				goto ERROR;
			}
			snprintf(inform_with_ret, len, "%s(%d)", inform, ret);
			notification_status_message_post(inform_with_ret);

			free(inform);
			free(inform_with_ret);
		}
	} else {
		_SD("Launch an app(%s:%s) ret : [%d]", app_id, name, ret);
		_T(app_id);

		Evas_Object *layout = evas_object_data_get(win, DATA_KEY_LAYOUT);
		ret_if(NULL == layout);
		apps_layout_block(layout);
		ecore_timer_add(LAYOUT_BLOCK_INTERVAL, _unblock_cb, layout);
	}

ERROR:
	app_control_destroy(service);
}



HAPI void apps_util_launch_with_arg(Evas_Object *win, const char *app_id, const char *arg, const char *name)
{
	ret_if(NULL == app_id);
	ret_if(NULL == arg);

	if (!name) name = app_id;
	if (LAUNCH_WITH_SPECIAL_ROUTINE && _launch_special_package(app_id, name)) {
		return;
	}

	_SD("Argument:(%s)", arg);
	int len = strlen(arg);

	bundle *b = NULL;
	b = bundle_decode((bundle_raw *) arg, len);

	/* AUL requests : Reset the caller appid as App-tray */
	const char *value = (char *)bundle_get_val(b, AUL_K_CALLER_APPID);
	if (value) bundle_del(b, AUL_K_CALLER_APPID);

	home_dbus_cpu_booster_signal_send();

	int ret = aul_launch_app(app_id, b);
	bundle_free(b);

	if (0 > ret) {
		char* inform;
		char* inform_with_ret;
		int len;

		// IDS_AT_TPOP_UNABLE_TO_OPEN_PS : "Unable to open %s"
		len = strlen(_("IDS_AT_TPOP_UNABLE_TO_OPEN_PS")) + strlen(name) + SPARE_LEN;

		inform = calloc(len, sizeof(char));
		if (!inform) {
			_E("cannot calloc for information");
			return;
		}
		snprintf(inform, len, _("IDS_AT_TPOP_UNABLE_TO_OPEN_PS"), name);

		inform_with_ret = calloc(len, sizeof(char));
		if (!inform_with_ret) {
			_E("cannot calloc for information");
			free(inform);
			return;
		}
		snprintf(inform_with_ret, len, "%s(%d)", inform, ret);
		notification_status_message_post(inform_with_ret);

		free(inform);
		free(inform_with_ret);
	} else {
		_SD("Launch an app(%s:%s) ret : [%d]", app_id, name, ret);
		_T(app_id);

		Evas_Object *layout = evas_object_data_get(win, DATA_KEY_LAYOUT);
		ret_if(NULL == layout);
		apps_layout_block(layout);
		ecore_timer_add(LAYOUT_BLOCK_INTERVAL, _unblock_cb, layout);
	}
}



HAPI void apps_util_launch_with_bundle(Evas_Object *win, const char *app_id, bundle *b, const char *name)
{
	ret_if(NULL == app_id);
	ret_if(NULL == b);

	if (!name) name = app_id;
	if (LAUNCH_WITH_SPECIAL_ROUTINE && _launch_special_package(app_id, name)) {
		return;
	}

	home_dbus_cpu_booster_signal_send();

	int ret = aul_launch_app(app_id, b);
	if (0 > ret) {
		char* inform;
		char* inform_with_ret;
		int len;

		// IDS_AT_TPOP_UNABLE_TO_OPEN_PS : "Unable to open %s"
		len = strlen(_("IDS_AT_TPOP_UNABLE_TO_OPEN_PS")) + strlen(name) + SPARE_LEN;

		inform = calloc(len, sizeof(char));
		if (!inform) {
			_E("cannot calloc for information");
			return;
		}
		snprintf(inform, len, _("IDS_AT_TPOP_UNABLE_TO_OPEN_PS"), name);

		inform_with_ret = calloc(len, sizeof(char));
		if (!inform_with_ret) {
			_E("cannot calloc for information");
			free(inform);
			return;
		}
		snprintf(inform_with_ret, len, "%s(%d)", inform, ret);
		notification_status_message_post(inform_with_ret);

		free(inform);
		free(inform_with_ret);
	} else {
		_SD("Launch an app(%s:%s) ret : [%d]", app_id, name, ret);
		_T(app_id);

		Evas_Object *layout = evas_object_data_get(win, DATA_KEY_LAYOUT);
		ret_if(NULL == layout);
		apps_layout_block(layout);
		ecore_timer_add(LAYOUT_BLOCK_INTERVAL, _unblock_cb, layout);
	}
}



HAPI void apps_util_notify_to_home(int pid)
{
	char *pkgname;

	if (pid <= 0)
		return;

	pkgname = vconf_get_str("db/setting/menuscreen/package_name");
	if (!pkgname)
		return;

	if (strcmp(pkgname, "org.tizen.cluster-home")) {
		free(pkgname);
		return;
	}

	_D("Service: (%s)\n", pkgname);
	free(pkgname);
}



static void _toast_popup_destroy_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = obj;
	ret_if(!popup);

	evas_object_del(popup);

	Evas_Object *parent = data;
	ret_if(!parent);

	evas_object_data_del(parent, DATA_KEY_POPUP);
}


/* This API is not allowed to use on the others */
extern Evas_Object *elm_object_part_access_object_get(const Evas_Object *obj, const char *part);
#define UTIL_TOAST_POPUP_TIMER 3.0
int util_create_toast_popup(Evas_Object *parent, const char* text)
{
	retv_if(!parent, EINA_FALSE);

	Evas_Object *popup = elm_popup_add(parent);
	retv_if(!popup, EINA_FALSE);

	elm_object_style_set(popup, POPUP_STYLE_TOAST);
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, ELM_NOTIFY_ALIGN_FILL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_part_text_set(popup, "elm.text", text);
	elm_object_domain_translatable_part_text_set(popup, "elm.text", PROJECT, text);
	if(main_get_info()->is_tts == EINA_FALSE)
		elm_popup_timeout_set(popup, UTIL_TOAST_POPUP_TIMER);

	evas_object_smart_callback_add(popup, "block,clicked", _toast_popup_destroy_cb, parent);
	evas_object_smart_callback_add(popup, "timeout", _toast_popup_destroy_cb, parent);

	evas_object_data_set(parent, DATA_KEY_POPUP, popup);

	evas_object_show(popup);

	return EINA_TRUE;
}



#define POPUP_EDJ EDJEDIR"/popup.edj"
int util_create_check_popup(Evas_Object *parent, const char* text, void _clicked_cb(void *, Evas_Object *, void *))
{
	Evas_Object *popup = NULL;
	Evas_Object *popup_scroller = NULL;
	Evas_Object *popup_layout_inner = NULL;
	Evas_Object *label = NULL;
	Evas_Object *check = NULL;
	Evas_Object *button = NULL;

	retv_if(!parent, EINA_FALSE);

	if (evas_object_data_get(parent, DATA_KEY_CHECK_POPUP)) return EINA_TRUE;

	popup = elm_popup_add(parent);
	retv_if(!popup, EINA_FALSE);

	elm_object_style_set(popup, POPUP_STYLE_DEFAULT);
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, ELM_NOTIFY_ALIGN_FILL);

	popup_scroller = elm_scroller_add(popup);
	goto_if(!popup_scroller, ERROR);
	elm_object_style_set(popup_scroller, "effect");
	evas_object_size_hint_weight_set(popup_scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_scroller_content_min_limit(popup_scroller, EINA_FALSE, EINA_TRUE);
	evas_object_size_hint_max_set(popup_scroller, POPUP_TEXT_MAX_WIDTH, POPUP_TEXT_MAX_HEIGHT);
	elm_object_content_set(popup, popup_scroller);
	evas_object_show(popup_scroller);

	popup_layout_inner = elm_layout_add(popup_scroller);
	goto_if(!popup_layout_inner, ERROR);
	elm_layout_file_set(popup_layout_inner, POPUP_EDJ, "popup_checkview_internal");
	evas_object_size_hint_weight_set(popup_layout_inner, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(popup_scroller, popup_layout_inner);

	label = elm_label_add(popup_layout_inner);
	goto_if(!label, ERROR);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, _("IDS_HS_POP_TO_INSTALL_OR_UNINSTALL_APPLICATIONS_USE_THE_SAMSUNG_GEAR_APPLICATION_ON_YOUR_MOBILE_DEVICE"));
	elm_object_domain_translatable_text_set(label, PROJECT, "IDS_HS_POP_TO_INSTALL_OR_UNINSTALL_APPLICATIONS_USE_THE_SAMSUNG_GEAR_APPLICATION_ON_YOUR_MOBILE_DEVICE");
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(popup_layout_inner, "label", label);

	check = elm_check_add(popup);
	goto_if(!check, ERROR);
	elm_object_style_set(check, "popup");
	elm_object_text_set(check, _("IDS_CLOCK_BODY_DONT_REPEAT_ABB"));
	elm_object_domain_translatable_text_set(check, PROJECT, "IDS_CLOCK_BODY_DONT_REPEAT_ABB");
	elm_object_part_content_set(popup_layout_inner, "elm.swallow.end", check);
	evas_object_show(check);
	evas_object_data_set(popup, DATA_KEY_CHECK, check);

	button = elm_button_add(popup);
	goto_if(!button, ERROR);
	elm_object_style_set(button, BUTTON_STYLE_POPUP);
	elm_object_text_set(button, _("IDS_ST_BUTTON_OK"));
	elm_object_domain_translatable_text_set(button, PROJECT, "IDS_ST_BUTTON_OK");
	elm_object_part_content_set(popup, "button1", button);
	evas_object_smart_callback_add(button, "clicked", _clicked_cb, parent);

	evas_object_data_set(parent, DATA_KEY_CHECK_POPUP, popup);
	evas_object_show(popup);

	return EINA_TRUE;

ERROR:
	if (popup) evas_object_del(popup);
	return EINA_FALSE;
}



static char *_get_app_id(const char *pkgname)
{
	int ret = 0;
	char *appid = 0;
	pkgmgrinfo_pkginfo_h handle = NULL;
	retv_if(pkgname == NULL, NULL);

	ret = pkgmgrinfo_pkginfo_get_pkginfo(pkgname, &handle);
	if (ret != PMINFO_R_OK) {
		_E("Failed to get pkg information from pkgmgr");
		goto ERR;
	}
	ret = pkgmgrinfo_pkginfo_get_mainappid(handle, &appid);
	if (ret != PMINFO_R_OK) {
		_E("Failed to get mainapp ID information from pkgmgr");
		goto ERR;
	}

	if (appid != NULL) {
		appid = strdup(appid);
	}
ERR:
	if (handle != NULL) {
		pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
	}

	return appid;
}



static char *_get_app_pkgname(const char *appid)
{
	int ret = 0;
	char *pkgname = NULL;
	pkgmgrinfo_appinfo_h handle = NULL;
	retv_if(appid == NULL, NULL);

	ret = pkgmgrinfo_appinfo_get_appinfo(appid, &handle);
	if (ret != PMINFO_R_OK) {
		_E("Failed to get app information from pkgmgr");
		goto ERR;
	}

	ret = pkgmgrinfo_appinfo_get_pkgname(handle, &pkgname);
	if (ret != PMINFO_R_OK) {
		_E("Failed to get pkgname from app info handler");
		goto ERR;
	}
	if (pkgname != NULL) {
		pkgname = strdup(pkgname);
	}

ERR:
	if (handle != NULL) {
		pkgmgrinfo_appinfo_destroy_appinfo(handle);
	}

	return pkgname;
}



static int _get_app_type(char *pkgname)
{
	int ret = 0;
	char *apptype = NULL;
	pkgmgrinfo_pkginfo_h handle = NULL;
	retv_if(pkgname == NULL, APP_TYPE_NATIVE);

	ret = pkgmgrinfo_pkginfo_get_pkginfo(pkgname, &handle);
	if (ret != PMINFO_R_OK) {
		_E("Failed to get pkg information from pkgmgr");
		return APP_TYPE_NATIVE;
	}

	ret = pkgmgrinfo_pkginfo_get_type(handle, &apptype);
	if (ret != PMINFO_R_OK) {
		_E("Failed to get app type from pkginfo handler");
		goto ERR;
	}

	if (apptype != NULL) {
		if (strcmp(apptype, "wgt") == 0) {
			pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
			return APP_TYPE_WEB;
		} else {
			char *widget_id;
			widget_id = widget_service_get_widget_id(pkgname);
			if (widget_id) {
				_D("Widget: %s\n", widget_id);
				free(widget_id);
				pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
				return APP_TYPE_WIDGET;
			}
			_D("MC: %s\n", pkgname);
		}
	}

ERR:
	pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
	return APP_TYPE_NATIVE;
}



HAPI int util_get_app_type(const char *appid)
{
	int pkgtype = APP_TYPE_NATIVE;
	char *pkgname = NULL;
	retv_if(appid == NULL, APP_TYPE_NATIVE);

	pkgname = _get_app_pkgname(appid);
	if(pkgname == NULL) {
		_E("Failed to get pkgname");
		goto DONE;
	}
	pkgtype = _get_app_type(pkgname);

DONE:
	free(pkgname);

	return pkgtype;
}



/*
 * This APIs should be used only for the packages which have only one app
 * returned pointer have to be free by you
 */
HAPI char *util_get_pkgname_by_appid(const char *appid)
{
	return _get_app_pkgname(appid);
}



HAPI char *util_get_appid_by_pkgname(const char *pkgname)
{
	return _get_app_id(pkgname);
}



HAPI const char *util_basename(const char *name)
{
	int length;
	length = name ? strlen(name) : 0;
	if (!length) {
		return ".";
	}

	while (--length > 0 && name[length] != '/');

	return length <= 0 ? name : (name + length + (name[length] == '/'));
}



HAPI double util_timestamp(void)
{
#if defined(_USE_ECORE_TIME_GET)
	return ecore_time_get();
#else
	struct timeval tv;
	if (gettimeofday(&tv, NULL) < 0) {
		static unsigned long internal_count = 0;
		_E("failed to get time of day(%d)", errno);
		tv.tv_sec = internal_count++;
		tv.tv_usec = 0;
	}

	return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0f;
#endif
}



HAPI void util_activate_home_window(void)
{
	if (apps_main_is_visible() == EINA_TRUE) {
		apps_main_launch(APPS_LAUNCH_HIDE);
	}
}

HAPI void util_raise_home_window(int force_notify)
{
	Evas_Object *win = main_get_info()->win;
	if (win) {
		elm_win_activate(win);
	}
}

/* You have to free the returned value */
HAPI char *util_get_count_str_from_icu(int count)
{
	char *p = NULL;
	char *locale_tmp = NULL;
	char *ret_str = NULL;
	char locale[LOCALE_LEN] = { 0, };
	char res[LOCALE_LEN] = { 0, };
	int slen;

	UErrorCode status = U_ZERO_ERROR;
	UNumberFormat *num_fmt;
	UChar result[20] = { 0, };

	uint32_t number = count;
	int32_t len = (int32_t) (sizeof(result) / sizeof((result)[0]));

	locale_tmp = vconf_get_str(VCONFKEY_REGIONFORMAT);
	retv_if(!locale_tmp, NULL);

	slen = strlen(locale_tmp);

	if( slen > 0 && slen <= LOCALE_LEN)
		strncpy(locale, locale_tmp, slen);

	free(locale_tmp);

	if(locale[0] != '\0') {
		p = strstr(locale, ".UTF-8");
		if (p) *p = 0;
	}

	num_fmt = unum_open(UNUM_DEFAULT, NULL, -1, locale, NULL, &status);
	unum_format(num_fmt, number, result, len, NULL, &status);
	u_austrcpy(res, result);
	unum_close(num_fmt);

	ret_str = strdup(res);
	return ret_str;
}

HAPI int util_is_arbic()
{
	int ret = 0;
	char *locale_tmp = vconf_get_str(VCONFKEY_LANGSET);
	retv_if(!locale_tmp, 0);

	if(!strncmp(locale_tmp, "ar", 2))
	{
		ret = 1;
	}

	return ret;
}


#define PRIVATE_DATE_KEY_CIRCLE_SCROLLER_OBJ "p_d_k_c_s_o_cso"
HAPI void util_uxt_scroller_set_rotary_event_enabled(Evas_Object *scroller, Eina_Bool enabled)
{
	ret_if(scroller == NULL);

	Evas_Object *circle_scroller = evas_object_data_get(scroller, PRIVATE_DATE_KEY_CIRCLE_SCROLLER_OBJ);

	eext_rotary_object_event_activated_set(circle_scroller, enabled);
	
/*	if (circle_scroller == NULL) {
		Eext_Circle_Surface *surf = main_get_info()->surface;
		if (surf) {
			circle_scroller = eext_circle_object_scroller_add(scroller, surf);
			if (circle_scroller) {
				eext_circle_object_scroller_policy_set(circle_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
				eext_rotary_object_event_activated_set(circle_scroller, enabled);
				evas_object_data_set(scroller, PRIVATE_DATE_KEY_CIRCLE_SCROLLER_OBJ, circle_scroller);
			} else {
				_E("Failed to create circle scroller of %p", scroller);
			}
		} else {
			_E("Failed to get surface object");
		}
	} else {
		eext_rotary_object_event_activated_set(circle_scroller, enabled);
	}*/
}

HAPI int util_host_vender_id_get(void)
{
	return 0;
	//int ret = W_HOME_VENDOR_ID_UNKNOWN;

/*	char *vender = vconf_get_str(VCONFKEY_WMS_HOST_STATUS_VENDOR);
	if (vender) {
		if (strcmp(vender, HOME_HOST_VENDOR_LO) == 0) {
			ret = W_HOME_VENDOR_ID_LO;
		}
		else if (strcmp(vender, HOME_HOST_V
		ENDER_SAMSUNG) == 0) {
			ret = W_HOME_VENDOR_ID_SAMSUNG;
		}

		free(vender);
	}*/

	//return ret;
}

#ifdef __MDM_ENABLED__
HAPI void util_mdm_get_service()
{
	if (mdm_get_service() == MDM_RESULT_SUCCESS)
	{
		_W("mdm_get_service is success");
		/* MDM Á¤Ã¥ °¡Á®¿À±â */
		apps_main_set_mdm_policy(true);
	}
	else
	{
		_W("mdm_get_service is fail");
		/* ±âÁ¸ APP ±â´É »ç¿ë¿¡ ¹®Á¦°¡ ¾ø¾î¾ßÇÔ. */
		apps_main_set_mdm_policy(false);
	}
}


HAPI void util_mdm_release_service()
{
//	if (apps_main_get_mdm_policy() == true)
	{
		_W("mdm release service");
		mdm_release_service();
	}
}

HAPI int util_mdm_is_restricted(const char *appid)
{
	retv_if(!appid, 0);

	if (apps_main_get_mdm_policy() == false)
	{
		_W("mdm restrict is false");
		return 0;
	}

	mdm_status_t status;
	status = mdm_get_application_uninstallation_disabled(appid);
	if (status == MDM_DISABLED) {
		_SD("#MDM app[%s] is diabled", appid);
		return 1;

	}
	return 0;
}
#endif

// End of a file
