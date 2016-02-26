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

#ifndef __CLOCK_SERVICE_H
#define __CLOCK_SERVICE_H

#define PAGE_CLOCK_EDJE_FILE EDJEDIR"/page_clock.edj"

#define CLOCK_RET_OK 0
#define CLOCK_RET_FAIL 1
#define CLOCK_RET_ASYNC 2
#define CLOCK_RET_NEED_DESTROY_PREVIOUS 4

#define CLOCK_SERVICE_MODE_NORMAL 1
#define CLOCK_SERVICE_MODE_NORMAL_RECOVERY 2
#define CLOCK_SERVICE_MODE_RECOVERY 3
#define CLOCK_SERVICE_MODE_EMERGENCY 4
#define CLOCK_SERVICE_MODE_COOLDOWN 5

#define CLOCK_APP_TYPE_NATIVE 1
#define CLOCK_APP_TYPE_WEBAPP 2
#define CLOCK_APP_FACTORY_DEFAULT "org.tizen.w-idle-clock-weather2"
#define CLOCK_APP_RECOVERY "org.tizen.idle-clock-digital"
#define CLOCK_APP_EMERGENCY "org.tizen.idle-clock-ups"
#define CLOCK_APP_COOLDOWN "org.tizen.idle-clock-emergency"

#define CLOCK_STATE_IDLE 0
#define CLOCK_STATE_WAITING 1
#define CLOCK_STATE_RUNNING 2

/*
 * Events
 */
#define CLOCK_EVENT_MAP(SOURCE, TYPE) ((SOURCE << 16) | TYPE)

/*
 * instant events
 */
#define CLOCK_EVENT_VIEW 0x0000
#define CLOCK_EVENT_VIEW_READY				CLOCK_EVENT_MAP(CLOCK_EVENT_VIEW, 0x0)
#define CLOCK_EVENT_VIEW_RESIZED			CLOCK_EVENT_MAP(CLOCK_EVENT_VIEW, 0x1)
#define CLOCK_EVENT_VIEW_HIDDEN_SHOW		CLOCK_EVENT_MAP(CLOCK_EVENT_VIEW, 0x2)
#define CLOCK_EVENT_VIEW_HIDDEN_HIDE		CLOCK_EVENT_MAP(CLOCK_EVENT_VIEW, 0x4)
#define CLOCK_EVENT_APP 0x0001
#define CLOCK_EVENT_APP_PROVIDER_ERROR		CLOCK_EVENT_MAP(CLOCK_EVENT_APP, 0x0)
#define CLOCK_EVENT_APP_PROVIDER_ERROR_FATAL	CLOCK_EVENT_MAP(CLOCK_EVENT_APP, 0x1)
#define CLOCK_EVENT_APP_PAUSE				CLOCK_EVENT_MAP(CLOCK_EVENT_APP, 0x2)
#define CLOCK_EVENT_APP_RESUME				CLOCK_EVENT_MAP(CLOCK_EVENT_APP, 0x4)
#define CLOCK_EVENT_APP_LANGUAGE_CHANGED	CLOCK_EVENT_MAP(CLOCK_EVENT_APP, 0x8)
#define CLOCK_EVENT_DEVICE 0x0002
#define CLOCK_EVENT_DEVICE_BACK_KEY			CLOCK_EVENT_MAP(CLOCK_EVENT_DEVICE, 0x0)
#define CLOCK_EVENT_DEVICE_LCD 0x0004
#define CLOCK_EVENT_DEVICE_LCD_ON			CLOCK_EVENT_MAP(CLOCK_EVENT_DEVICE_LCD, 0x0)
#define CLOCK_EVENT_DEVICE_LCD_OFF			CLOCK_EVENT_MAP(CLOCK_EVENT_DEVICE_LCD, 0x1)

/*
 * ongoing events
 */
#define CLOCK_EVENT_SCREEN_READER 0x0008
#define CLOCK_EVENT_SCREEN_READER_ON		CLOCK_EVENT_MAP(CLOCK_EVENT_SCREEN_READER, 0x0)
#define CLOCK_EVENT_SCREEN_READER_OFF		CLOCK_EVENT_MAP(CLOCK_EVENT_SCREEN_READER, 0x1)
#define CLOCK_EVENT_SAP 0x0020
#define CLOCK_EVENT_SAP_ON		CLOCK_EVENT_MAP(CLOCK_EVENT_SAP, 0x0)
#define CLOCK_EVENT_SAP_OFF		CLOCK_EVENT_MAP(CLOCK_EVENT_SAP, 0x1)
#define CLOCK_EVENT_MODEM 0x0040
#define CLOCK_EVENT_MODEM_ON		CLOCK_EVENT_MAP(CLOCK_EVENT_MODEM, 0x0)
#define CLOCK_EVENT_MODEM_OFF		CLOCK_EVENT_MAP(CLOCK_EVENT_MODEM, 0x1)
#define CLOCK_EVENT_DND 0x0080
#define CLOCK_EVENT_DND_ON		CLOCK_EVENT_MAP(CLOCK_EVENT_DND, 0x0)
#define CLOCK_EVENT_DND_OFF		CLOCK_EVENT_MAP(CLOCK_EVENT_DND, 0x1)
#define CLOCK_EVENT_SCROLLER 0x0100
#define CLOCK_EVENT_SCROLLER_FREEZE_ON	CLOCK_EVENT_MAP(CLOCK_EVENT_SCROLLER, 0x0)
#define CLOCK_EVENT_SCROLLER_FREEZE_OFF	CLOCK_EVENT_MAP(CLOCK_EVENT_SCROLLER, 0x1)
#define CLOCK_EVENT_POWER 0x0200
#define CLOCK_EVENT_POWER_ENHANCED_MODE_ON	CLOCK_EVENT_MAP(CLOCK_EVENT_POWER, 0x0)
#define CLOCK_EVENT_POWER_ENHANCED_MODE_OFF	CLOCK_EVENT_MAP(CLOCK_EVENT_POWER, 0x1)
#define CLOCK_EVENT_SIM 0x0400
#define CLOCK_EVENT_SIM_INSERTED	CLOCK_EVENT_MAP(CLOCK_EVENT_SIM, 0x0)
#define CLOCK_EVENT_SIM_NOT_INSERTED	CLOCK_EVENT_MAP(CLOCK_EVENT_SIM, 0x1)
#define CLOCK_EVENT_CFWD 0x0800

#define CLOCK_EVENT_CATEGORY(X) CLOCK_EVENT_MAP(X, 0x0)

#define CLOCK_SMART_SIGNAL_PAUSE "clock,paused"
#define CLOCK_SMART_SIGNAL_RESUME "clock,resume"
#define CLOCK_SMART_SIGNAL_VIEW_REQUEST "clock,view,request"

#define CLOCK_VIEW_REQUEST_DRAWER_HIDE 1
#define CLOCK_VIEW_REQUEST_DRAWER_SHOW 2

#define CLOCK_ATTACHED 1
#define CLOCK_CANDIDATE 2

#define CLOCK_INF_MINICONTROL 1
#define CLOCK_INF_WIDGET 2

#define CLOCK_CONF_NONE 0
#define CLOCK_CONF_WIN_ACTIVATION 1
#define CLOCK_CONF_CLOCK_CONFIGURATION 2

#define CLOCK_VIEW_TYPE_DRAWER 1

#define VCONFKEY_MUSIC_STATUS "memory/homescreen/music_status"

extern void clock_service_init(void);
extern void clock_service_fini(void);

#endif
