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
#ifndef ROTARY_H_
#define ROTARY_H_

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <dlog.h>
#include <efl_extension.h>
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

#endif
