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
#include <unistd.h>
#include <getopt.h>

#include "configuration.h"
#include "common.h"
#include "parser.h"

int gt_config_help(void *data)
{
	printf("Configuration help function\n");
	return -1;
}

struct gt_config_create_data {
	const char *gadget;
	const char *config;
	struct gt_setting *attrs;
	int opts;
};

static void gt_config_create_destructor(void *data)
{
	struct gt_config_create_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_create_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int  gt_config_create_func(void *data)
{
	struct gt_config_create_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_config_create_data *)data;
	printf("Config create called successfully. Not implemented.\n");
	printf("gadget = %s, config = %s, force = %d",
		dt->gadget, dt->config, !!(dt->opts & GT_FORCE));

	ptr = dt->attrs;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int gt_config_create_help(void *data)
{
	printf("Config create help.\n");
	return -1;
}

static void gt_parse_config_create(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	int c;
	int ind;
	struct gt_config_create_data *dt;
	int avaible_opts = GT_FORCE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;
	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (argc - ind < 2)
		goto out;

	dt->gadget = argv[ind++];
	dt->config = argv[ind++];
	c = gt_parse_setting_list(&dt->attrs, argc - ind, argv + ind);
	if (c < 0)
		goto out;

	executable_command_set(exec, gt_config_create_func,
				(void *)dt, gt_config_create_destructor);
	return;

out:
	gt_config_create_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_config_rm_data {
	const char *gadget;
	const char *config;
	int opts;
};

static int gt_config_rm_func(void *data)
{
	struct gt_config_rm_data *dt;

	dt = (struct gt_config_rm_data *)data;
	printf("Config rm called successfully. Not implemented.\n");
	printf("gadget = %s, config = %s, force = %d, recursive = %d\n",
		dt->gadget, dt->config, !!(dt->opts & GT_FORCE),
		!!(dt->opts & GT_RECURSIVE));

	return 0;
}

static int gt_config_rm_help(void *data)
{
	printf("Config rm help.\n");
	return -1;
}

static void gt_parse_config_rm(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	int ind;
	struct gt_config_rm_data *dt;
	int avaible_opts = GT_FORCE | GT_RECURSIVE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;
	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (argc - ind != 2)
		goto out;

	dt->gadget = argv[ind++];
	dt->config = argv[ind++];
	executable_command_set(exec, gt_config_rm_func, (void *)dt, free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}
const Command *gt_config_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_config_create, NULL,
		 gt_config_create_help},
		{"rm", NEXT, gt_parse_config_rm, NULL, gt_config_rm_help},
//		{"get", AGAIN, gt_parse_config_get, NULL, gt_config_get_help},
//		{"set", AGAIN, gt_parse_config_set, NULL, gt_config_set_help},
//		{"add", AGAIN, gt_parse_config_add, NULL, gt_config_add_help},
//		{"del", AGAIN, gt_parse_config_del, NULL, gt_config_del_help},
//		{"template", NEXT, command_parse,
//			gt_config_template_get_children,
//			gt_config_template_help},
//		{NULL, PREV, gt_parse_config_config, NULL,
//			gt_config_config_help},
		CMD_LIST_END
	};

	return commands;
}
