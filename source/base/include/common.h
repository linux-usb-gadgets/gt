/*
 * Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __GADGET_TOOL_COMMON_H__
#define __GADGET_TOOL_COMMON_H__

#include <stdlib.h>
#ifdef WITH_GADGETD
#include <gio/gio.h>
#endif

/**
 * @brief Short program name
 * @description Basename of program invocation name
 */
extern char *program_name;

static inline void *zalloc(size_t size)
{
	return calloc(1, size);
}

#define streq(a, b) (strcmp(a, b) == 0)
#define strcaseeq(a, b) (strcasecmp(a, b) == 0)

#define ARRAY_SIZE(array) sizeof(array)/sizeof(*array)

#ifdef WITH_GADGETD
static inline void _cleanup_fn_g_free_(void *p) {
	g_free(*(gpointer *)p);
}
#endif

static inline void _cleanup_fn_free_(void *p) {
	free(*(void **)p);
}

#define _cleanup_(fn)   __attribute__((cleanup(fn)))
#ifdef WITH_GADGETD
#define _cleanup_g_free_ _cleanup_(_cleanup_fn_g_free_)
#endif
#define _cleanup_free_  _cleanup_(_cleanup_fn_free_)

#endif //__GADGET_TOOL_COMMON_H__
