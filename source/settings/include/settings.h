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

#ifndef __GADGET_TOOL_SETTINGS_SETTINGS_H__
#define __GADGET_TOOL_SETTINGS_SETTINGS_H__

#include <libconfig.h>

#include "command.h"

/* user settings file */
#define GT_USER_SETTING_PATH "~/.gt.conf"

struct gt_setting_list {
	const char *default_udc;
	const char *configfs_path;
	const char **lookup_path;
	const char *default_template_path;
	const char *default_gadget;
};

extern struct gt_setting_list gt_settings;

/**
 * @brief Gets the next possible commands after settings
 * @param[in] cmd actual command (should be settings)
 * @return Pointer to table with all children of cmd
 * where the last element is invalid structure filled
 * with NULLs.
 */
const Command *gt_settings_get_children(const Command *cmd);

/**
 * @brief Help function which should be used if invalid
 * syntax for settings was entered.
 *
 * @param[in] data additional data
 * @return -1 because invalid syntax has been provided
 */
int gt_settings_help(void *data);

/**
 * @brief Parse settings
 * @return 0 if success, -1 when error occured
 */
int gt_parse_settings(config_t *config);

#endif //__GADGET_TOOL_SETTINGS_SETTINGS_H__
