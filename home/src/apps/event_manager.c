
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

#include <Elementary.h>
#include <Evas.h>
#include <Ecore_X.h>
#include <stdbool.h>
#include <vconf.h>
#include <bundle.h>
#include <aul.h>
#include <efl_assist.h>
#include <efl_extension.h>
#include <dlog.h>
#include <appcore-common.h>
#include <widget_viewer_evas.h>

#include "conf.h"
#include "layout.h"
#include "log.h"
#include "util.h"
#include "main.h"
#include "clock_service.h"
#include "power_mode.h"
#include "apps/apps_main.h"
#include "event_manager.h"
#include "move.h"
#include "dbus.h"

static struct _s_info {
	Ecore_Event_Handler *ecore_hdl;
	Ecore_Event_Handler *ecore_visible_hdl;

	struct _app {
		int state;
	} app;

	struct _win {
		int visibility;
		int stack_state;
		int moment_bar_move;
	} win;

	struct _powersaving {
		int mode;
	} powersaving;

	struct _clock {
		int force_pause;
		int visibility;
	} clock;

	struct _home {
		int state;
		int visibility;
		int clocklist_state;
		int editing;
		int addviewer;
		int scrolling;
	} home;

	struct _apptray {
		int state;
		int visibility;
		int edit_visibility;
	} apptray;

	struct _drawer {
		int scrolling;
		int opened;
	} drawer;

	struct _tutorial {
		int state;
	} tutorial;

	struct _appcontrol {
		int calltime;
		int manual_resume;
	} appcontrol;

	struct _lcd {
		int lcdstate;
		int pmstate;
	} lcd;

} s_info = {
	.ecore_hdl = NULL,
	.ecore_visible_hdl = NULL,

	.app.state = EVTM_APP_STATE_BOOT,

	.win.visibility = EVTM_VISIBLE,
	.win.stack_state = EVTM_WIN_STATE_FOREGROUND,
	.win.moment_bar_move = EVTM_ACTION_OFF,

	.clock.force_pause = EVTM_ACTION_OFF,
	.clock.visibility = EVTM_VISIBLE,
	.powersaving.mode = EVTM_POWER_SAVING_MODE_OFF,

	.home.state = EVTM_HOME_STATE_BOOT,
	.home.visibility = EVTM_VISIBLE,

	.home.clocklist_state = EVTM_CLOCKLIST_STATE_HIDE,
	.home.editing = EVTM_ACTION_OFF,
	.home.addviewer = EVTM_ACTION_OFF,
	.home.scrolling = EVTM_ACTION_OFF,

	.apptray.state = EVTM_APPTRAY_STATE_BOOT,
	.apptray.visibility = EVTM_INVISIBLE,
	.apptray.edit_visibility = EVTM_INVISIBLE,

	.drawer.scrolling = EVTM_ACTION_OFF,
	.drawer.opened =  EVTM_ACTION_OFF,

	.tutorial.state = EVTM_ACTION_OFF,

	.appcontrol.calltime = 0,
	.appcontrol.manual_resume = 0,
};

static Evas_Object *_layout_get(void)
{
	Evas_Object *win = main_get_info()->win;
	retv_if(win == NULL, NULL);

	return evas_object_data_get(win, DATA_KEY_LAYOUT);
}



static void _state_init(void)
{
	if (emergency_mode_enabled_get()) {
		s_info.powersaving.mode = EVTM_POWER_SAVING_MODE_ON;
	}
}

static void _set_back_win_gesture(Evas_Object* win, Eina_Bool bshow)
{

	/* back disabled*/
	Ecore_X_Atom atom = 0;
	const char *state_str = NULL;


	atom = ecore_x_atom_get("_E_MOVE_ENABLE_DISABLE_BACK_GESTURE");


	if (bshow) {
		state_str = "1";
		_D("disable back gesture");
	} else {
		state_str = "0";
		_D("enable back gesture");
	}

	ecore_x_window_prop_property_set(elm_win_xwindow_get(win), atom, ECORE_X_ATOM_STRING, 8, (void *)state_str, strlen(state_str));

}

static void _state_control(evtm_control_e control)
{
	int app_state = s_info.app.state;
	int win_state = s_info.win.stack_state;
	int win_visibility = s_info.win.visibility;
	int home_visible = s_info.home.visibility;
	int home_editing = s_info.home.editing;
	int home_clocklist = s_info.home.clocklist_state;
	int home_addviewer = s_info.home.addviewer;
	int apptray_visible = s_info.apptray.visibility;
	int clock_visible = s_info.clock.visibility;
	int tutorial_state = s_info.tutorial.state;
	int lcd_on = s_info.lcd.lcdstate;

	int home_resumed = (s_info.home.state == EVTM_HOME_STATE_RESUMED) ? 1 : 0;
	int apptray_resumed = (s_info.apptray.state == EVTM_APPTRAY_STATE_RESUMED) ? 1 : 0;

	_W("control:%d, app_state:%d win_state:%d(%d) pm_state:%d home_visible:%d clock_visible:%d tutorial_state:%d editing : %d, home_clocklist:%d, addviewer:%d scrolling : %d, powersaving : %d, apptray state : %d, apptray visibility : %d, apptray edit visibility : %d",
		control, app_state, win_state, win_visibility, lcd_on, home_visible, clock_visible, tutorial_state, s_info.home.editing, home_clocklist, s_info.home.addviewer, s_info.home.scrolling, s_info.powersaving.mode, s_info.apptray.state, s_info.apptray.visibility, s_info.apptray.edit_visibility); 

	switch(control) {
	case EVTM_CONTROL_HOME:
		if (app_state == EVTM_APP_STATE_RESUMED) {
			if (home_visible == EVTM_VISIBLE &&
				home_clocklist != EVTM_CLOCKLIST_STATE_SHOW &&
				home_addviewer == EVTM_ACTION_OFF) {
				if (!home_resumed) {
					home_resume();
					s_info.home.state = EVTM_HOME_STATE_RESUMED;

					/*
					 * Workaround
					 * To fix problem which clock is stopped when home is resumed
					 */
					if (s_info.clock.force_pause == EVTM_ACTION_ON) {
						clock_inf_widget_force_resume();
						s_info.clock.force_pause = EVTM_ACTION_OFF;
					}
				}
			 } else {
			 	if (home_resumed) {
			 		home_pause();
			 		s_info.home.state = EVTM_HOME_STATE_PAUSED;
			 	}
			 }
		} else if (app_state == EVTM_APP_STATE_PAUSED) {
			if (home_resumed) {
				home_pause();
				s_info.home.state = EVTM_HOME_STATE_PAUSED;
			}
		}
	break;

	case EVTM_CONTROL_APPTRAY:
		if (app_state == EVTM_APP_STATE_RESUMED) {
			if (apptray_visible == EVTM_VISIBLE) {
				if (!apptray_resumed) {
					apps_main_resume();
					s_info.apptray.state = EVTM_APPTRAY_STATE_RESUMED;
					if(main_get_info()->is_tts)
						_set_back_win_gesture(main_get_info()->win, EINA_FALSE);
					else
						_set_back_win_gesture(main_get_info()->win, EINA_TRUE);
				}
				if(win_state == EVTM_WIN_STATE_BACKGROUND){
					_set_back_win_gesture(main_get_info()->win, EINA_FALSE);
				}
				else{
					if(s_info.apptray.edit_visibility == EVTM_VISIBLE){
						move_hide_move_clock(0);
						_set_back_win_gesture(main_get_info()->win, EINA_FALSE);
					}
					else{
						move_show_move_clock();
						if(main_get_info()->is_tts)
							_set_back_win_gesture(main_get_info()->win, EINA_FALSE);
						else
							_set_back_win_gesture(main_get_info()->win, EINA_TRUE);
					}
				}

				apps_main_state_atom_set(1);
			 } else {
			 	if (apptray_resumed) {
			 		apps_main_pause();
			 		s_info.apptray.state = EVTM_APPTRAY_STATE_PAUSED;
			 		_set_back_win_gesture(main_get_info()->win, EINA_FALSE);
			 	}
			 	move_hide_move_clock(1);
				apps_main_state_atom_set(0);
			 }
		} else if (app_state == EVTM_APP_STATE_PAUSED) {
		 	if (apptray_resumed) {
		 		apps_main_pause();
		 		s_info.apptray.state = EVTM_APPTRAY_STATE_PAUSED;
		 		_set_back_win_gesture(main_get_info()->win, EINA_FALSE);
		 	}
		}
	break;

	case EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY:
		if (app_state == EVTM_APP_STATE_RESUMED) {
			if (clock_visible == EVTM_VISIBLE &&
				win_state == EVTM_WIN_STATE_FOREGROUND &&
				home_visible == EVTM_VISIBLE &&
				home_addviewer == EVTM_ACTION_OFF &&
				home_editing == EVTM_ACTION_OFF &&
				home_clocklist == EVTM_CLOCKLIST_STATE_HIDE &&
				tutorial_state == EVTM_ACTION_OFF &&
				s_info.drawer.scrolling == EVTM_ACTION_OFF) {
				clock_util_clock_state_atom_set(1);
				clock_util_clock_state_vconf_set(1);
			} else {
				clock_util_clock_state_atom_set(0);
				clock_util_clock_state_vconf_set(0);
			}
		} else if (app_state == EVTM_APP_STATE_PAUSED) {
			clock_util_clock_state_atom_set(0);
			clock_util_clock_state_vconf_set(0);
		}
	break;

	case EVTM_CONTROL_CLOCK_DISPLAY_STATE:
	{
		if (s_info.win.moment_bar_move == EVTM_ACTION_ON ||
			(s_info.drawer.scrolling == EVTM_ACTION_ON && app_state == EVTM_APP_STATE_RESUMED)) {
			if (!home_resumed && clock_visible == EVTM_VISIBLE) {
				_W("clock/widget (force) resumed");
				widget_viewer_evas_notify_resumed_status_of_viewer();
				clock_inf_widget_force_resume();
				clock_service_resume();

				s_info.clock.force_pause = EVTM_ACTION_OFF;
			}
		} else {
			if (!home_resumed && s_info.drawer.opened == EVTM_ACTION_ON) {
				_W("clock/widget (force) paused");
				widget_viewer_evas_notify_paused_status_of_viewer();
				clock_inf_widget_force_pause();
				clock_service_pause();

				s_info.clock.force_pause = EVTM_ACTION_ON;
			}
		}
	}
	break;

	/*
	 * Because appcontrol changes app state to "running", resume callback will be not called on upcomming resumming events(lcd, window)
	 * To workaround this, we have to call the appcore resume callback manually
	 */
	case EVTM_CONTROL_APPCORE_APPCONTROL:
	{
		if (s_info.appcontrol.manual_resume) {
			if (app_state == EVTM_APP_STATE_RESUMED &&
				(win_visibility == EVTM_INVISIBLE || lcd_on ==  EVTM_ACTION_OFF)) {
				s_info.appcontrol.manual_resume = 0;
				_W("appcore paused manually");
				home_appcore_pause();
			}
		} else {
			if (app_state == EVTM_APP_STATE_PAUSED &&
				(win_state == EVTM_WIN_STATE_FOREGROUND || win_visibility == EVTM_VISIBLE) &&
				lcd_on ==  EVTM_ACTION_ON &&
				s_info.appcontrol.calltime > 0) {
				s_info.appcontrol.manual_resume = 1;
				_W("appcore resumed manually");
				home_appcore_resume();
			}
		}
	}
	break;
	}
}

static w_home_error_e _app_resume_cb(void *data)
{
	int prev_state = s_info.app.state;
	int cur_state = EVTM_APP_STATE_RESUMED;
	_W("state: %d -> %d", prev_state, cur_state);

	s_info.app.state = cur_state;

	if (prev_state != cur_state) {
		_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
		_state_control(EVTM_CONTROL_HOME);
		_state_control(EVTM_CONTROL_APPTRAY);
	}

	return W_HOME_ERROR_NONE;
}

static w_home_error_e _app_pause_cb(void *data)
{
	int prev_state = s_info.app.state;
	int cur_state = EVTM_APP_STATE_PAUSED;
	_W("state: %d -> %d", prev_state, cur_state);

	s_info.app.state = cur_state;
	s_info.appcontrol.manual_resume = 0;

	if (prev_state != cur_state) {
		_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
		_state_control(EVTM_CONTROL_HOME);
		_state_control(EVTM_CONTROL_APPTRAY);
	}

	return W_HOME_ERROR_NONE;
}

static Eina_Bool _ecore_x_message_cb(void *data,int type ,void *event)
{
	Ecore_X_Event_Client_Message *ev = event;
	retv_if(ev == NULL, ECORE_CALLBACK_PASS_ON);

	if (ev->message_type == ecore_x_atom_get("_E_MOVE_W_HOME_FOREGROUND_BACKGROUND_STATE"))
	{
		int prev_state = s_info.win.stack_state;
		if (ev->data.l[0] == 1) // foreground
		{
			s_info.win.stack_state = EVTM_WIN_STATE_FOREGROUND;
		}
		else if(ev->data.l[0] == 0) // background
		{
			s_info.win.stack_state = EVTM_WIN_STATE_BACKGROUND;
		}

		_W("state: %d -> %d", prev_state, s_info.win.stack_state);

		if (prev_state != s_info.win.stack_state) {
			_state_control(EVTM_CONTROL_APPCORE_APPCONTROL);
			_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
			_state_control(EVTM_CONTROL_APPTRAY);
		}
	}
	else if (ev->message_type == ecore_x_atom_get("_E_MOVE_W_HOME_CLOCK_ANIMATION_STATE"))
	{
		int prev_state = s_info.win.moment_bar_move;

		_W("moment bar move:%d", ev->data.l[0]);
		if (ev->data.l[0] == 1) // move
		{
			s_info.win.moment_bar_move = EVTM_ACTION_ON;
		}
		else if(ev->data.l[0] == 0) // stop
		{
			s_info.win.moment_bar_move = EVTM_ACTION_OFF;
		}

		if (prev_state != s_info.win.moment_bar_move) {
			_state_control(EVTM_CONTROL_CLOCK_DISPLAY_STATE);
		}
	}

	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool _window_visibility_cb(void *data, int type, void *event)
{
	Ecore_X_Event_Window_Visibility_Change *ev = event;
	retv_if(ev == NULL, ECORE_CALLBACK_PASS_ON);

	if (!main_get_info()->win) {
		return ECORE_CALLBACK_PASS_ON;
	}

	Ecore_X_Window xWin = elm_win_xwindow_get(main_get_info()->win);

	if (xWin != ev->win) {
		return ECORE_CALLBACK_PASS_ON;
	}

	_W("Window [0x%X] is now visible(%d)", xWin, ev->fully_obscured);

	int prev_state = s_info.win.visibility;

	if (ev->fully_obscured == 1) {
		s_info.win.visibility = EVTM_INVISIBLE;
	} else {
		s_info.win.visibility = EVTM_VISIBLE;
	}

	_W("state: %d -> %d", prev_state, s_info.win.visibility);

	if (s_info.win.visibility != prev_state) {
		_state_control(EVTM_CONTROL_APPCORE_APPCONTROL);
	}

	return ECORE_CALLBACK_PASS_ON;
}

static void _enhanced_power_mode_on_cb(void *user_data, void *event_info)
{
	int prev_state = s_info.powersaving.mode;
	int cur_state = EVTM_POWER_SAVING_MODE_ON;
	_W("state: %d -> %d", prev_state, cur_state);

	s_info.powersaving.mode = cur_state;

	if (cur_state != prev_state) {
		_state_control(EVTM_CONTROL_HOME);
		_state_control(EVTM_CONTROL_APPTRAY);
		_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
	}
}

static void _enhanced_power_mode_off_cb(void *user_data, void *event_info)
{
	int prev_state = s_info.powersaving.mode;
	int cur_state = EVTM_POWER_SAVING_MODE_OFF;
	_W("state: %d -> %d", prev_state, cur_state);

	s_info.powersaving.mode = cur_state;

	if (cur_state != prev_state) {
		_state_control(EVTM_CONTROL_HOME);
		_state_control(EVTM_CONTROL_APPTRAY);
		_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
	}
}

static void _home_layout_edit_cb(void *data, Evas_Object *obj, const char *em, const char *src)
{
	int prev_state = s_info.home.editing;

	 if (em) {
	 	_W("%s", em);
	 	if (strcmp(em, "edit,show") == 0) {
	 		s_info.home.editing = EVTM_ACTION_ON;
	 	} else if (strcmp(em, "edit,hide") == 0) {
	 		s_info.home.editing = EVTM_ACTION_OFF;
	 	}

	 	if (prev_state != s_info.home.editing) {
		 	_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
		 	_state_control(EVTM_CONTROL_HOME);
	 	}
	 }
}

static void _home_layout_clocklist_cb(void *data, Evas_Object *obj, const char *em, const char *src)
{
	int prev_state = s_info.home.clocklist_state;

	 if (em) {
	 	_W("%s", em);
	 	if (strcmp(em, "clocklist,will,show") == 0) {
	 		s_info.home.clocklist_state = EVTM_CLOCKLIST_STATE_WILL_SHOW;
	 	}
	 	else if (strcmp(em, "clocklist,show") == 0) {
	 		s_info.home.clocklist_state = EVTM_CLOCKLIST_STATE_SHOW;
	 	}
	 	else if (strcmp(em, "clocklist,will,hide") == 0) {
	 		s_info.home.clocklist_state = EVTM_CLOCKLIST_STATE_WILL_HIDE;
	 	}
	 	else if (strcmp(em, "clocklist,hide") == 0) {
	 		s_info.home.clocklist_state = EVTM_CLOCKLIST_STATE_HIDE;
	 	}

	 	if (prev_state != s_info.home.clocklist_state) {
		 	_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
		 	_state_control(EVTM_CONTROL_HOME);
	 	}
	 }
}

static void _home_add_viewer_show_cb(void *data, Evas_Object *obj, const char *em, const char *src)
{
	int prev_state = s_info.home.addviewer;

	 if (em) {
	 	_W("%s", em);
	 	if (strcmp(em, "add_viewer,show") == 0) {
	 		s_info.home.addviewer = EVTM_ACTION_ON;
	 	} else if (strcmp(em, "add_viewer,hide") == 0) {
	 		s_info.home.addviewer = EVTM_ACTION_OFF;
	 	}

	 	if (prev_state != s_info.home.addviewer) {
		 	_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
		 	_state_control(EVTM_CONTROL_HOME);
	 	}
	 }
}

static void _home_scroll_cb(void *data, Evas_Object *obj, const char *em, const char *src)
{
	 if (em) {
	 	_W("%s", em);
	 	if (strcmp(em, "scroll,start") == 0) {
	 		s_info.home.scrolling = EVTM_ACTION_ON;
	 	} else if (strcmp(em, "scroll,done") == 0) {
	 		s_info.home.scrolling = EVTM_ACTION_OFF;
	 	}
	 }
}

static void _apptray_visibility_cb(void *data, Evas_Object *obj, const char *em, const char *src)
{
	int prev_state = s_info.apptray.state;

	 if (em) {
	 	_W("%s", em);
	 	if (strcmp(em, "apps,show") == 0) {
	 		s_info.home.visibility = EVTM_INVISIBLE;
	 		s_info.apptray.visibility = EVTM_VISIBLE;
	 		apps_main_show_count_add();
	 	} else if (strcmp(em, "apps,hide") == 0) {
	 		s_info.home.visibility = EVTM_VISIBLE;
	 		s_info.apptray.visibility = EVTM_INVISIBLE;
	 	} else if (strcmp(em, "apps,done") == 0) {
	 		s_info.home.visibility = EVTM_VISIBLE;
	 		s_info.apptray.visibility = EVTM_INVISIBLE;
	 		s_info.apptray.state = EVTM_APPTRAY_STATE_PAUSED;
	 	} else if (strcmp(em, "apps,edit,show") == 0) {
			s_info.apptray.edit_visibility = EVTM_VISIBLE;
	 	} else if (strcmp(em, "apps,edit,hide") == 0) {
	 		s_info.apptray.edit_visibility = EVTM_INVISIBLE;
	 	}

	 	_W("state: %d -> %d", prev_state, s_info.apptray.visibility);

		_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
		_state_control(EVTM_CONTROL_HOME);
		_state_control(EVTM_CONTROL_APPTRAY);
	 }
}

static void _clock_view_visible_cb(void *user_data, void *event_info)
{
	int prev_state = s_info.clock.visibility;
	int cur_state = EVTM_VISIBLE;
	_W("state: %d -> %d", prev_state, cur_state);

	s_info.clock.visibility = cur_state;

	if (prev_state != cur_state) {
		_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
	}
}

static void _clock_view_obscured_cb(void *user_data, void *event_info)
{
	int prev_state = s_info.clock.visibility;
	int cur_state = EVTM_INVISIBLE;
	_W("state: %d -> %d", prev_state, cur_state);

	s_info.clock.visibility = cur_state;

	if (prev_state != cur_state) {
		_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
	}
}


static void _move_module_anim_start_cb(void *data, Evas_Object *obj, void *event_info)
{
	int prev_state = s_info.drawer.scrolling;
	int cur_state = EVTM_ACTION_ON;
	_W("state: %d -> %d", prev_state, cur_state);

	s_info.drawer.scrolling = cur_state;

	if (prev_state != cur_state) {
		_state_control(EVTM_CONTROL_CLOCK_DISPLAY_STATE);
		_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
	}
}

static void _move_module_anim_end_cb(void *data, Evas_Object *obj, void *event_info)
{
	int prev_state = s_info.drawer.scrolling;
	int cur_state = EVTM_ACTION_OFF;
	_W("state: %d -> %d", prev_state, cur_state);

	char *apps_state = data;
	if (apps_state) {
		_D("apps_state:%s", apps_state);
		s_info.drawer.opened = (strcmp(apps_state, "apps,show") == 0) ? EVTM_ACTION_ON : EVTM_ACTION_OFF;
	}
	s_info.drawer.scrolling = cur_state;

	if(event_info) {
		if(strcmp((char*)event_info, "apps,hide") == 0) {
			s_info.apptray.visibility = EVTM_INVISIBLE;
		}
	}

	if (prev_state != cur_state) {
		_state_control(EVTM_CONTROL_CLOCK_DISPLAY_STATE);
		if(s_info.apptray.visibility == EVTM_VISIBLE)
			_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
	}
}

static void _tutorial_state_vconf_cb(keynode_t *node, void *data)
{
	ret_if(!node);

	int prev_state = s_info.tutorial.state;
	int cur_state = vconf_keynode_get_int(node);
	cur_state = (cur_state) ? EVTM_ACTION_ON : EVTM_ACTION_OFF;

	_W("state: %d -> %d", prev_state, cur_state);

	s_info.tutorial.state = cur_state;

	if (prev_state != cur_state) {
		_state_control(EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY);
	}
}

static void _lcd_on_cb(void *user_data, void *event_info)
{
	s_info.lcd.lcdstate = EVTM_ACTION_ON;
	_W("lcd state: %d", s_info.lcd.lcdstate);

	_state_control(EVTM_CONTROL_APPCORE_APPCONTROL);
}

static void _lcd_off_cb(void *user_data, void *event_info)
{
	s_info.lcd.lcdstate = EVTM_ACTION_OFF;
	_W("lcd state: %d", s_info.lcd.lcdstate);

	_state_control(EVTM_CONTROL_APPCORE_APPCONTROL);
}

HAPI void home_event_manager_init(evtm_init_level_e init_level)
{
	_state_init();

	switch (init_level) {
	case EVTM_INIT_LEVEL_FASTBOOT:
	{
		if (main_register_cb(APP_STATE_RESUME, _app_resume_cb, NULL) != W_HOME_ERROR_NONE) {
			_E("Cannot register the resume callback");
		}
		if (main_register_cb(APP_STATE_PAUSE, _app_pause_cb, NULL) != W_HOME_ERROR_NONE) {
			_E("Cannot register the pause callback");
		}
		s_info.ecore_hdl = ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE, _ecore_x_message_cb, NULL);
		if (!s_info.ecore_hdl) {
			_E("Couldn't register the ecore event message handler");
		}
		s_info.ecore_visible_hdl = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE, _window_visibility_cb, NULL);
		if (!s_info.ecore_visible_hdl) {
			_E("Failed to add an ecore event handler (VISIBILITY_CHANGE)");
		}

		int pmstate = VCONFKEY_PM_STATE_NORMAL;
		if (vconf_get_int(VCONFKEY_PM_STATE, &pmstate) != 0) {
			_E("Failed to get PM STATE");
			pmstate = VCONFKEY_PM_STATE_NORMAL;
		}
		s_info.lcd.pmstate = (pmstate != VCONFKEY_PM_STATE_LCDOFF) ? EVTM_ACTION_ON : EVTM_ACTION_OFF;
		s_info.lcd.lcdstate = s_info.lcd.pmstate;

		if (home_dbus_register_cb(DBUS_EVENT_LCD_ON, _lcd_on_cb, NULL) != W_HOME_ERROR_NONE) {
			_E("Failed to register lcd status changed cb");
		}
		if (home_dbus_register_cb(DBUS_EVENT_LCD_OFF, _lcd_off_cb, NULL) != W_HOME_ERROR_NONE) {
			_E("Failed to register lcd status changed cb");
		}
		power_mode_register_cb(POWER_MODE_ENHANCED_ON, _enhanced_power_mode_on_cb, NULL);
		power_mode_register_cb(POWER_MODE_ENHANCED_OFF, _enhanced_power_mode_off_cb, NULL);
		if (vconf_notify_key_changed(VCONF_KEY_HOME_IS_TUTORIAL, _tutorial_state_vconf_cb, NULL) < 0) {
			_E("Failed to register callback");
		}
	}
	break;
	case EVTM_INIT_LEVEL_BOOT:
	{
		Evas_Object *layout = _layout_get();

		if (layout != NULL) {
			elm_layout_signal_callback_add(layout, "clocklist,*", "clocklist", _home_layout_clocklist_cb, NULL);
			elm_layout_signal_callback_add(layout, "edit,*", "edit", _home_layout_edit_cb, NULL);
			elm_layout_signal_callback_add(layout, "add_viewer,*", "add_viewer", _home_add_viewer_show_cb, NULL);
			elm_layout_signal_callback_add(layout, "apps,*", "apps", _apptray_visibility_cb, NULL);
			elm_layout_signal_callback_add(layout, "scroll,*", "home", _home_scroll_cb, NULL);
			evas_object_smart_callback_add(layout, "move,anim,start", _move_module_anim_start_cb, NULL);
			evas_object_smart_callback_add(layout, "move,anim,end", _move_module_anim_end_cb, NULL);
		}
		if (clock_view_event_register_cb(CLOCK_VIEW_EVENT_VISIBLE, _clock_view_visible_cb, NULL) != CLOCK_RET_OK) {
			_E("Cannot register the clock view attached callback");
		}
		if (clock_view_event_register_cb(CLOCK_VIEW_EVENT_OBSCURED, _clock_view_obscured_cb, NULL) != CLOCK_RET_OK) {
			_E("Cannot register the clock view attached callback");
		}
	}
	break;
	}
}

HAPI void home_event_manager_fini(void)
{
	Evas_Object *layout = _layout_get();
	if (layout != NULL) {
		elm_layout_signal_callback_del(layout, "clocklist,*", "clocklist", _home_layout_clocklist_cb);
		elm_layout_signal_callback_del(layout, "edit,*", "edit", _home_layout_edit_cb);
		elm_layout_signal_callback_del(layout, "add_viewer,*", "add_viewer", _home_add_viewer_show_cb);
		elm_layout_signal_callback_del(layout, "apps,*", "apps", _apptray_visibility_cb);
		elm_layout_signal_callback_del(layout, "scroll,*", "home", _home_scroll_cb);
		evas_object_smart_callback_del(layout, "move,anim,start", _move_module_anim_start_cb);
		evas_object_smart_callback_del(layout, "move,anim,end", _move_module_anim_end_cb);
	}

	clock_view_event_unregister_cb(CLOCK_VIEW_EVENT_VISIBLE, _clock_view_visible_cb);
	clock_view_event_unregister_cb(CLOCK_VIEW_EVENT_OBSCURED, _clock_view_obscured_cb);

	main_unregister_cb(APP_STATE_RESUME, _app_resume_cb);
	main_unregister_cb(APP_STATE_PAUSE, _app_pause_cb);

	if (s_info.ecore_hdl) {
		ecore_event_handler_del(s_info.ecore_hdl);
		s_info.ecore_hdl = NULL;
	}
	power_mode_unregister_cb(POWER_MODE_ENHANCED_ON, _enhanced_power_mode_on_cb);
	power_mode_unregister_cb(POWER_MODE_ENHANCED_OFF, _enhanced_power_mode_off_cb);

	if (vconf_ignore_key_changed(VCONF_KEY_HOME_IS_TUTORIAL, _tutorial_state_vconf_cb) < 0) {
		_E("Failed to ignore callback");
	}
	home_dbus_unregister_cb(DBUS_EVENT_LCD_ON, _lcd_on_cb);
	home_dbus_unregister_cb(DBUS_EVENT_LCD_OFF, _lcd_off_cb);
	if (s_info.ecore_visible_hdl) {
		ecore_event_handler_del(s_info.ecore_visible_hdl);
		s_info.ecore_visible_hdl = NULL;
	}
}

HAPI int home_event_manager_allowance_get(feature_e feature)
{
	if (feature == FEATURE_MOVE_MODULE_DRAGGING) {
		if (s_info.home.editing == EVTM_ACTION_ON ||
			s_info.home.clocklist_state != EVTM_CLOCKLIST_STATE_HIDE ||
			s_info.home.addviewer == EVTM_ACTION_ON ||
			s_info.home.scrolling == EVTM_ACTION_ON ||
			s_info.powersaving.mode == EVTM_POWER_SAVING_MODE_ON ||
			s_info.apptray.state == EVTM_APPTRAY_STATE_BOOT ||
			((s_info.apptray.visibility == EVTM_VISIBLE) && (s_info.apptray.edit_visibility == EVTM_VISIBLE))) {
			return 0;
		}
	} else if (feature == FEATURE_HOME_EDITING) {
		if (s_info.home.editing == EVTM_ACTION_ON ||
			s_info.home.clocklist_state != EVTM_CLOCKLIST_STATE_HIDE ||
			s_info.home.addviewer == EVTM_ACTION_ON ||
			s_info.home.scrolling == EVTM_ACTION_ON ||
			s_info.powersaving.mode == EVTM_POWER_SAVING_MODE_ON ||
			s_info.apptray.visibility == EVTM_VISIBLE) {
			return 0;
		}
	} else if (feature == FEATURE_ADDVIEWER) {
		if (s_info.home.scrolling == EVTM_ACTION_ON) {
			return 0;
		}
	}

	return 1;
}

HAPI int home_event_manager_bool_property_get(evtm_property_e property)
{
	if (property == EVTM_PROPERTY_CLOCK_VISIBILITY) {
		if (s_info.clock.visibility && s_info.win.visibility == EVTM_VISIBLE) {
			return 1;
		}
	}

	return 0;
}

HAPI void home_event_manager_appcontrol_calltime_set(int enabled)
{
	s_info.appcontrol.calltime = enabled;
}
