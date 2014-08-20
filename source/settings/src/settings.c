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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "settings.h"
#include "common.h"
#include "parser.h"

static int gt_check_settings_var(const char *name)
{
	static const char *vars[] = {
		"default-udc",
		"config-fs-path",
		"lookup-path",
		"default-template-path",
		"default-gadget",
		NULL
	};
	int i = 0;

	for (i = 0; vars[i]; i++) {
		if (strcmp(name, vars[i]) == 0)
			return 0;
	}

	return -1;
}

int gt_settings_help(void *data)
{
	printf("Settings help function\n");
	return -1;
}

static int gt_settings_get_func(void *data)
{
	const char **dt;

	dt = (const char **)data;

	printf("Settings get called successfully. Not implemented yet.\n");
	while (*dt) {
		printf("%s, ", *dt);
		dt++;
	}

	putchar('\n');
	return 0;
}

static int gt_settings_get_help(void *data)
{
	printf("Settings get help function\n");
	return -1;
}

void gt_settings_parse_get(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	const char **dt = NULL;
	int i;

	dt = calloc(argc + 1, sizeof(*dt));
	if (dt == NULL)
		goto out;

	for (i = 0; i < argc; i++){
		if (gt_check_settings_var(argv[i]) < 0) {
			printf("Unrecognized variable name\n");
			goto out;
		}
		dt[i] = argv[i];
	}

	executable_command_set(exec, gt_settings_get_func, (void *)dt, free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_settings_set_func(void *data)
{
	struct gt_setting *ptr;

	ptr = (struct gt_setting *)data;
	printf("Settings set called successfully. Not implemented yet.\n");
	while (ptr->variable) {
		printf("%s = %s, ", ptr->variable, ptr->value);
		ptr++;
	}
	putchar('\n');
	return 0;
}

static int gt_settings_set_help(void *data)
{
	printf("Settings set help.\n");
	return -1;
}

void gt_settings_parse_set(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_setting *dt = NULL;
	struct gt_setting *ptr = NULL;
	int tmp;

	if (argc == 0)
		goto out;

	tmp = gt_parse_setting_list(&dt, argc, argv);
	if (tmp < 0)
		goto out;

	ptr = dt;
	while (ptr->variable) {
		if (gt_check_settings_var(ptr->variable) < 0) {
			printf("Unrecognized variable name: %s\n",
				ptr->variable);
			goto out;
		}
		ptr++;

	executable_command_set(exec, gt_settings_set_func,
			(void *)dt, gt_setting_list_cleanup);
	}
	return;
out:
	gt_setting_list_cleanup((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_settings_append_func(void *data)
{
	struct gt_setting *dt;

	dt = (struct gt_setting *)data;
	printf("Settings append called successfully. Not implemented,\n");
	printf("var = %s, val = %s\n", dt->variable, dt->value);
	return 0;
}

static int gt_settings_append_help(void *data)
{
	printf("Settings append help.\n");
	return -1;
}

void gt_settings_parse_append(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_setting *dt;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	switch (argc) {
	case 0:
		goto out;
	case 1:
		printf("Expected value after '%s'\n", argv[0]);
		goto out;
	case 2:
		dt->variable = argv[0];
		if (gt_check_settings_var(dt->variable) < 0) {
			printf("Unrecognized variable name\n");
			goto out;
		}
		dt->value = argv[1];
		executable_command_set(exec, gt_settings_append_func,
			(void *)dt, free);
		break;
	default:
		printf("Too many arguments\n");
		goto out;
	}
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_settings_detach_func(void *data)
{
	struct gt_setting *dt;

	dt = (struct gt_setting *)data;
	printf("Settings detach called successfully. Not implemented.\n");
	printf("var = %s, val = %s\n", dt->variable, dt->value);
	return 0;
}

static int gt_settings_detach_help(void *data)
{
	printf("Settings detach help.\n");
	return -1;
}

void gt_settings_parse_detach(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_setting *dt;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	switch (argc) {
	case 0:
		goto out;
	case 1:
		printf("Expected value after '%s'\n", argv[0]);
		goto out;
	case 2:
		dt->variable = argv[0];
		if (gt_check_settings_var(dt->variable) < 0) {
			printf("Unrecognized variable name\n");
			goto out;
		}
		dt->value = argv[1];
		executable_command_set(exec, gt_settings_detach_func,
			(void *)dt, free);
		break;
	default:
		printf("Too many arguments\n");
		goto out;
	}
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *gt_settings_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"get", NEXT, gt_settings_parse_get, NULL,
			gt_settings_get_help},
		{"set", NEXT, gt_settings_parse_set, NULL,
			gt_settings_set_help},
		{"append", NEXT, gt_settings_parse_append, NULL,
			gt_settings_append_help},
		{"detach", NEXT, gt_settings_parse_detach, NULL,
			gt_settings_detach_help},
		CMD_LIST_END
	};

	return commands;
}
