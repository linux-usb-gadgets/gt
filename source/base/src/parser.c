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

#include "common.h"
#include "parser.h"
#include "command.h"
#include "udc.h"
#include "gadget.h"
#include "configuration.h"
#include "function.h"
#include "settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int gt_split_by_char(char **first, char **second, const char *str,
		char delimiter)
{
	const char *ptr = str;
	ptr = strchr(str, delimiter);

	if (ptr == NULL){
		printf("Wrong argument: expected %c in %s\n", delimiter, str);
		return -1;
	}

	*first = strndup(str, ptr - str);
	if (*first == NULL)
		return -1;

	*second = strdup(ptr+1);
	if (*second == NULL) {
		free(*first);
		return -1;
	}

	return 0;
}

int gt_parse_setting(struct gt_setting *dst, const char *str)
{
	return gt_split_by_char(&dst->variable, &dst->value, str, '=');
}

int gt_parse_setting_list(struct gt_setting **dst, int argc, char **argv)
{
	int i, tmp;
	struct gt_setting *res;

	res = calloc(argc + 1, sizeof(*res));
	if (res == NULL)
		goto out;

	for (i = 0; i < argc; i++) {
		tmp = gt_parse_setting(&res[i], argv[i]);
		if (tmp < 0)
			goto out;
	}

	*dst = res;
	return argc;
out:
	free(res);
	return -1;
}

void gt_setting_cleanup(void *data)
{
	struct gt_setting *dt;

	if (data == NULL)
		return;

	dt = (struct gt_setting *)data;
	free(dt->variable);
	free(dt->value);
}

void gt_setting_list_cleanup(void *data)
{
	struct gt_setting *dt;

	if (data == NULL)
		return;

	for (dt = (struct gt_setting *)data; dt->variable; dt++)
		gt_setting_cleanup(dt);

	free(data);
}

int gt_parse_function_name(char **type, char **instance, const char *str)
{
	return gt_split_by_char(type, instance, str, '.');
}
