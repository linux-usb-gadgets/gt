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

#include "gadget.h"
#include "common.h"
#include "parser.h"

struct gt_gadget_create_data {
	const char *name;
	struct gt_setting *attrs;
	int opts;
};

static void gt_gadget_create_destructor(void *data)
{
	struct gt_gadget_create_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_create_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_gadget_create_func(void *data)
{
	struct gt_gadget_create_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_gadget_create_data *)data;
	printf("Gadget create called successfully. Not implemented.\n");
	printf("name = %s, force = %d", dt->name, !!(dt->opts & GT_FORCE));

	ptr = dt->attrs;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int gt_gadget_create_help(void *data)
{
	printf("Gadget create help.\n");
	return -1;
}

static void gt_parse_gadget_create(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_gadget_create_data *dt;
	int ind;
	int c;
	int avaible_opts = GT_FORCE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (ind == argc)
		goto out;

	dt->name = argv[ind++];

	c = gt_parse_setting_list(&dt->attrs, argc - ind, argv + ind);
	if (c < 0)
		goto out;

	executable_command_set(exec, gt_gadget_create_func,
				(void *)dt, gt_gadget_create_destructor);
	return;
out:
	gt_gadget_create_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_rm_data {
	const char *name;
	int opts;
};

static int gt_gadget_rm_help(void *data)
{
	printf("Gadget rm help.\n");
	return -1;
}

static int gt_gadget_rm_func(void *data)
{
	struct gt_gadget_rm_data *dt;

	dt = (struct gt_gadget_rm_data *)data;
	printf("Gadget rm called successfully. Not implemented.\n");
	printf("name = %s, force = %d, recursive = %d\n",
		dt->name, !!(dt->opts & GT_FORCE), !!(dt->opts & GT_RECURSIVE));
	return 0;
}

static void gt_parse_gadget_rm(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_gadget_rm_data *dt;
	int ind;
	int avaible_opts = GT_FORCE | GT_RECURSIVE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (argc - ind != 1)
		goto out;
	dt->name = argv[ind];
	executable_command_set(exec, gt_gadget_rm_func, (void *)dt,
			free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *get_gadget_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_gadget_create, NULL,
			gt_gadget_create_help},
		{"rm", NEXT, gt_parse_gadget_rm, NULL, gt_gadget_rm_help},
//		{"get", AGAIN, gt_parse_gadget_get, NULL, gt_gadget_get_help},
//		{"set", AGAIN, gt_parse_gadget_set, NULL, gt_gadget_set_help},
//		{"enable", AGAIN, gt_parse_gadget_enable, NULL,
//			gt_gadget_enable_help},
//		{"disable", parse_gadget_disable, NULL, gadget_disable_help_func},
//		{"gadget", parse_gadget_gadget, NULL, gadget_gadget_help_func},
//		{"template", parse_gadget_template, NULL, gadget_template_help_func},
//		{"load", parse_gadget_load, NULL, gadget_load_help_func},
//		{"save", parse_gadget_save, NULL, gadget_load_help_func},
		CMD_LIST_END
	};

	return commands;
}

int gt_gadget_help(void *data)
{
	printf("Gadget help function\n");
	return -1;
}
