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

#ifndef __GADGET_TOOL_GADGET_GADGET_H__
#define __GADGET_TOOL_GADGET_GADGET_H__

#include <usbg/usbg.h>

#include "command.h"

/**
 * An interface that backends need to implement. Not implemented functions
 * should be filled with NULL pointers. For each function the only argument
 * passed is a pointer to corresponding structure.
 */
struct gt_gadget_backend {
	/**
	 * Create a gadget
	 */
	int (*create)(void *);
	/**
	 * Remove a gadget
	 */
	int (*rm)(void *);
	/**
	 * Get gadget attributes
	 */
	int (*get)(void *);
	/**
	 * Set gadget attributes
	 */
	int (*set)(void *);
	/**
	 * Enable a gadget
	 */
	int (*enable)(void *);
	/**
	 * Disable a gadget
	 */
	int (*disable)(void *);
	/**
	 * Show gadgets
	 */
	int (*gadget)(void *);
	/**
	 * Load gadget from file
	 */
	int (*load)(void *);
	/**
	 * Save gadget to file
	 */
	int (*save)(void *);
	/**
	 * Show gadget templates
	 */
	int (*template_default)(void *);
	/**
	 * Get template attributes
	 */
	int (*template_get)(void *);
	/**
	 * Set template attributes
	 */
	int (*template_set)(void *);
	/**
	 * Remove template
	 */
	int (*template_rm)(void *);
};

#define GT_GADGET_STRS_COUNT 3

struct gt_gadget_str {
	char *name;
	int (*set_fn)(usbg_gadget *, int, const char *);
};

extern const struct gt_gadget_str gadget_strs[];

struct gt_gadget_create_data {
	const char *name;
	int attr_val[USBG_GADGET_ATTR_MAX];
	char *str_val[GT_GADGET_STRS_COUNT];
	int opts;
};

struct gt_gadget_rm_data {
	const char *name;
	int opts;
};

struct gt_gadget_get_data {
	const char *name;
	int attrs[USBG_GADGET_ATTR_MAX];
	int opts;
};

struct gt_gadget_set_data {
	const char *name;
	int attr_val[USBG_GADGET_ATTR_MAX];
	char *str_val[GT_GADGET_STRS_COUNT];
	int opts;
};

struct gt_gadget_enable_data {
	const char *gadget;
	const char *udc;
	int opts;
};

struct gt_gadget_disable_data {
	const char *gadget;
	const char *udc;
	int opts;
};

struct gt_gadget_gadget_data {
	const char *name;
	int opts;
};

struct gt_gadget_load_data {
	const char *name;
	const char *gadget_name;
	const char *file;
	const char *path;
	int opts;
};

struct gt_gadget_save_data {
	const char *gadget;
	const char *name;
	const char *file;
	const char *path;
	struct gt_setting *attrs;
	int opts;
};

struct gt_gadget_template_data {
	const char *name;
	int opts;
};

struct gt_gadget_template_rm_data {
	const char *name;
	int opts;
};

struct gt_gadget_template_set_data {
	const char *name;
	struct gt_setting *attr;
	int opts;
};

struct gt_gadget_template_get_data {
	const char *name;
	const char **attr;
	int opts;
};

/**
 * @brief Gets the commands possible for gadget
 * @param[in] cmd actual command
 * @return Pointer to table with all children of cmd
 * where the last element is invalid structure filled
 * with NULLs.
 */
const Command *get_gadget_children(const Command *cmd);

/**
 * @brief Help function which should be used if invalid
 * syntax for gadget was entered.
 *
 * @param[in] data additional data
 * @return -1 because invalid syntax has been provided
 */
int gt_gadget_help(void *data);

#ifdef WITH_GADGETD
extern struct gt_gadget_backend gt_gadget_backend_gadgetd;
#endif
extern struct gt_gadget_backend gt_gadget_backend_libusbg;
extern struct gt_gadget_backend gt_gadget_backend_not_implemented;

#endif //__GADGET_TOOL_GADGET_GADGET_H__
