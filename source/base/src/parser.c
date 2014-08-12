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

/**
 * @file parser.c
 * @brief Implementation of functions used for parsing command line options.
 */

#include "parser.h"
#include "command.h"
#include "udc.h"
#include "gadget.h"
#include "configuration.h"
#include "function.h"
#include "settings.h"

#include <stdio.h>

int gt_global_help(void *data)
{
	printf("Global help function\n");
	return -1;
}

static inline const Command *gt_get_command_root_children(const Command *cmd)
{
	static Command commands[] = {
		{ "udc", NEXT, udc_parse, NULL, udc_help_func },
		{ "settings", NEXT, command_parse, gt_settings_get_children, gt_settings_help },
		{ "config", NEXT, command_parse, gt_config_get_children, gt_config_help },
		{ "func", NEXT, command_parse, gt_func_get_children, gt_func_help },
		{ NULL, AGAIN, command_parse, get_gadget_children, gt_global_help },
		{ NULL, AGAIN, NULL, NULL, NULL }
	};

	return commands;
}

static inline const Command *gt_get_command_root(const Command *cmd)
{
	static Command tool_names[] = {
		{ NULL, NEXT, command_parse, gt_get_command_root_children, gt_global_help },
		{ NULL, AGAIN, NULL, NULL, NULL }
	};
	return tool_names;
}

void gt_parse_commands(int argc, char **argv, ExecutableCommand *exec)
{
	static Command command_pre_root = {
		NULL, AGAIN, command_parse, gt_get_command_root, gt_global_help
	};

	command_pre_root.parse(&command_pre_root, argc, argv, exec, NULL);
}
