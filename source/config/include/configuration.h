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

#ifndef __GADGET_TOOL_CONFIGURATION_CONFIGURATION_H__
#define __GADGET_TOOL_CONFIGURATION_CONFIGURATION_H__

#include <usbg/usbg.h>

#include "command.h"

/**
 * An interface that backends need to implement. Not implemented functions
 * should be filled with NULL pointers. For each function the only argument
 * passed is a pointer to corresponding structure.
 */
struct gt_config_backend {
	/**
	 * Create a configuration
	 */
	int (*create)(void *);
	/**
	 * Remove a configuration
	 */
	int (*rm)(void *);
	/**
	 * Get configuration attributes
	 */
	int (*get)(void *);
	/**
	 * Set configuration attributes
	 */
	int (*set)(void *);
	/**
	 * Show configs
	 */
	int (*show)(void *);
	/**
	 * Add a function to configuration
	 */
	int (*add)(void *);
	/**
	 * Remove function from configuration
	 */
	int (*del)(void *);
	/**
	 * Show templates
	 */
	int (*template_default)(void *);
	/**
	 * Show templates attributes
	 */
	int (*template_get)(void *);
	/**
	 * Set templates attributes
	 */
	int (*template_set)(void *);
	/**
	 * Remove template
	 */
	int (*template_rm)(void *);
	/**
	 * Load config from file
	 */
	int (*load)(void *);
	/**
	 * Save config to file
	 */
	int (*save)(void *);
};

struct gt_config_create_data {
	const char *gadget;
	int config_id;
	const char *config_label;
	int opts;
	struct gt_setting *attrs;
};

struct gt_config_rm_data {
	const char *gadget;
	int config_id;
	const char *config_label;
	int opts;
};

struct gt_config_get_data {
	const char *gadget;
	const char *config;
	const char **attrs;
	int opts;
};

struct gt_config_set_data {
	const char *gadget;
	const char *config;
	struct gt_setting *attrs;
	int opts;
};

struct gt_config_show_data {
	const char *gadget;
	int config_id;
	const char *config_label;
	int opts;
};

struct gt_config_add_del_data {
	const char *gadget;
	int   config_id;
	const char *config_label;
	const char *type;
	const char *instance;
	int opts;
};

struct gt_config_template_data {
	const char *name;
	int opts;
};

struct gt_config_template_get_data {
	const char *name;
	const char **attr;
	int opts;
};

struct gt_config_template_set_data {
	const char *name;
	struct gt_setting *attr;
	int opts;
};

struct gt_config_template_rm_data {
	const char *name;
	int opts;
};

struct gt_config_load_data {
	const char *name;
	const char *gadget;
	const char *config;
	const char *file;
	const char *path;
	int opts;
};

struct gt_config_save_data {
	const char *gadget;
	const char *config;
	const char *name;
	const char *file;
	const char *path;
	int opts;
	struct gt_setting *attrs;
};

/**
 * @brief Gets the next possible commands after config
 * @param[in] cmd actual command (should be config)
 * @return Pointer to table with all children of cmd
 * where the last element is invalid structure filled
 * with NULLs.
 */
const Command *gt_config_get_children(const Command *cmd);

/**
 * @brief Help function which should be used if invalid
 * syntax for config was entered.
 *
 * @param[in] data additional data
 * @return -1 because invalid syntax has been provided
 */
int gt_config_help(void *data);

/**
 * @brief Print given config
 * @param[in] c Config to be printed
 * @param[in] opts Mask from options (recognized flags are GT_VERBOSE,
 * GT_RECURSIVE, GT_NAME, GT_ID).
 * @return 0 on success, -1 otherwise
 */
int gt_print_config_libusbg(usbg_config *c, int opts);

#ifdef WITH_GADGETD
extern struct gt_config_backend gt_config_backend_gadgetd;
#endif
extern struct gt_config_backend gt_config_backend_libusbg;
extern struct gt_config_backend gt_config_backend_not_implemented;

#endif //__GADGET_TOOL_CONFIGURATION_CONFIGURATION_H__
