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

#ifndef __GADGET_TOOL_FUNCTION_FUNCTION_H__
#define __GADGET_TOOL_FUNCTION_FUNCTION_H__

#include "command.h"

/**
 * An interface that backends need to implement. Not implemented functions
 * should be filled with NULL pointers. For each function the only argument
 * passed is a pointer to corresponding structure.
 */
struct gt_function_backend {
	/**
	 * Create a function
	 */
	int (*create)(void *);
	/**
	 * Remove a function
	 */
	int (*rm)(void *);
	/**
	 * List all known types of functions
	 */
	int (*list_types)(void *);
	/**
	 * Get attributes of a function
	 */
	int (*get)(void *);
	/**
	 * Set attributes of a function
	 */
	int (*set)(void *);
	/**
	 * Show functions
	 */
	int (*show)(void *);
	/**
	 * Load function from file
	 */
	int (*load)(void *);
	/**
	 * Save function to file
	 */
	int (*save)(void *);
	/**
	 * Function template
	 */
	int (*template_default)(void *);
	/**
	 * Get template attributes
	 */
	int (*template_get)(void *);
	/**
	 * Set template atributes
	 */
	int (*template_set)(void *);
	/**
	 * Remove template
	 */
	int (*template_rm)(void *);
};

struct gt_func_create_data {
	const char *gadget;
	const char *type;
	const char *name;
	int opts;
	struct gt_setting *attrs;
};

struct gt_func_rm_data {
	const char *gadget;
	int type;
	const char *instance;
	int opts;
};

struct gt_func_list_types_data {
	int opts;
};

struct gt_func_get_data {
	const char *gadget;
	const char *type;
	const char *name;
	const char **attrs;
	int opts;
};

struct gt_func_set_data {
	const char *gadget;
	const char *type;
	const char *name;
	struct gt_setting *attrs;
	int opts;
};

struct gt_func_show_data {
	const char *gadget;
	int type;
	const char *instance;
	int opts;
};

struct gt_func_load_data {
	const char *name;
	const char *gadget;
	const char *func;
	const char *file;
	const char *path;
	int opts;
};

struct gt_func_save_data {
	const char *gadget;
	const char *func;
	const char *name;
	const char *file;
	const char *path;
	int opts;
	struct gt_setting *attrs;
};

struct gt_func_template_data {
	const char *name;
	int opts;
};

struct gt_func_template_get_data {
	const char *name;
	const char **attrs;
	int opts;
};

struct gt_func_template_set_data {
	const char *name;
	struct gt_setting *attrs;
	int opts;
};

/**
 * @brief Gets the next possible commands after func
 * @param[in] cmd actual command (should be func)
 * @return Pointer to table with all children of cmd
 * where the last element is invalid structure filled
 * with NULLs.
 */
const Command *gt_func_get_children(const Command *cmd);

/**
 * @brief Help function which should be used if invalid
 * syntax for function was entered.
 *
 * @param[in] data additional data
 * @return -1 because invalid syntax has been provided
 */
int gt_func_help(void *data);

/**
 * @brief Print function
 * @param[in] f Function to be printed
 * @param[in] opts Options of printing
 */
int gt_print_function_libusbg(usbg_function *f, int opts);

extern struct gt_function_backend gt_function_backend_libusbg;
#ifdef WITH_GADGETD
extern struct gt_function_backend gt_function_backend_gadgetd;
#endif
extern struct gt_function_backend gt_function_backend_not_implemented;

#endif //__GADGET_TOOL_FUNCTION_FUNCTION_H__
