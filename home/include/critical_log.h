/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
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

extern int critical_log(const char *func, int line, const char *fmt, ...);
extern int critical_log_init(const char *tag);
extern void critical_log_fini(void);

#if defined(ENABLE_CRITICAL_LOG)
#define CRITICAL_LOG(args...) critical_log(__func__, __LINE__, args)
#define CRITICAL_LOG_INIT(a) critical_log_init(a)
#define CRITICAL_LOG_FINI() critical_log_fini()
#else
#define CRITICAL_LOG(args...)
#define CRITICAL_LOG_INIT(a) (0)
#define CRITICAL_LOG_FINI()
#endif

/* End of a file */
