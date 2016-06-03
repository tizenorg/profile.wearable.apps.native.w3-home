/*
 * rotary.h
 *
 *  Created on: May 11, 2016
 *      Author: adarsh.sr1
 */

#ifndef ROTARY_H_
#define ROTARY_H_


/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <dlog.h>
#include <efl_extension.h>

//#define ELM_DEMO_EDJ "/opt/usr/apps/org.example.uicomponents1/res/ui_controls.edj"
#define ICON_DIR "/home/owner/apps_rw/org.example.uicomponents1/res/images"


typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *layout;
	Evas_Object *nf;
	Evas_Object *datetime;
	Evas_Object *popup;
	Evas_Object *button;
	Eext_Circle_Surface *circle_surface;
	struct tm saved_time;
} appdata_s;

struct _menu_item {
   char *name;
   void (*func)(void *data, Evas_Object *obj, void *event_info);
};


//void progress_cb(void *data, Evas_Object * obj, void *event_info);



#endif /* ROTARY_H_ */
