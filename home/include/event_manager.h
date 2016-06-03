/*
 * Samsung API
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#ifndef __W_HOME_EVENT_MANAGER_H__
#define __W_HOME_EVENT_MANAGER_H__

#include "util.h"

typedef enum {
	EVTM_PROPERTY_CLOCK_VISIBILITY = 1,
} evtm_property_e;

typedef enum {
	EVTM_CONTROL_HOME,
	EVTM_CONTROL_APPTRAY,
	EVTM_CONTROL_CLOCK_X_DISPLAY_PROPERTY,
	EVTM_CONTROL_CLOCK_DISPLAY_STATE,
	EVTM_CONTROL_APPCORE_APPCONTROL,
} evtm_control_e;

typedef enum {
	EVTM_INIT_LEVEL_FASTBOOT = 0,
	EVTM_INIT_LEVEL_BOOT,
} evtm_init_level_e;

enum {
	EVTM_INVISIBLE = 0,
	EVTM_VISIBLE,
};

enum {
	EVTM_LAYER_TOP = 0,
	EVTM_LAYER_BOTTOM,
};

enum {
	EVTM_ACTION_OFF = 0,
	EVTM_ACTION_ON,
};

enum {
	EVTM_APP_STATE_BOOT = 0,
	EVTM_APP_STATE_RESUMED ,
	EVTM_APP_STATE_PAUSED,
};

enum {
	EVTM_WIN_STATE_FOREGROUND = 0,
	EVTM_WIN_STATE_BACKGROUND,
};

enum {
	EVTM_POWER_SAVING_MODE_OFF = 0,
	EVTM_POWER_SAVING_MODE_ON,
};

enum {
	EVTM_HOME_STATE_BOOT = 0,
	EVTM_HOME_STATE_RESUMED,
	EVTM_HOME_STATE_PAUSED,
	EVTM_HOME_STATE_MAX,
};

enum {
	EVTM_APPTRAY_STATE_RESUMED = 0,
	EVTM_APPTRAY_STATE_PAUSED,
	EVTM_APPTRAY_STATE_BOOT,
	EVTM_APPTRAY_STATE_MAX,
};

enum {
	EVTM_CLOCKLIST_STATE_HIDE = 0,
	EVTM_CLOCKLIST_STATE_WILL_HIDE,
	EVTM_CLOCKLIST_STATE_WILL_SHOW,
	EVTM_CLOCKLIST_STATE_SHOW,
};

void home_event_manager_init(evtm_init_level_e init_level);
void home_event_manager_fini(void);
int home_event_manager_allowance_get(feature_e feature);
void home_event_manager_appcontrol_calltime_set(int enabled);

#endif