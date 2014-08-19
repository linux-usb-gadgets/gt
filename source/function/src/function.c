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

#include "function.h"
#include "common.h"
#include "parser.h"

static int gt_get_func_name(char **type, char **name, int argc, char **argv)
{
	int tmp;

	if (argc < 0)
		return -1;

	if (strchr(argv[0], '.') != NULL) {
		tmp = gt_parse_function_name(type, name, argv[0]);
		if (tmp < 0)
			return tmp;
		return 1;
	}

	if (argc < 2)
		return -1;

	*type = strdup(argv[0]);
	*name = strdup(argv[1]);
	return 2;
}

struct gt_func_create_data {
	const char *gadget;
	char *type;
	char *name;
	int opts;
	struct gt_setting *attrs;
};

static void gt_func_create_destructor(void *data)
{
	struct gt_func_create_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_func_create_data *)data;
	free(dt->type);
	free(dt->name);
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_func_create_func(void *data)
{
	struct gt_func_create_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_func_create_data *)data;
	printf("Func create called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, name=%s, force=%d",
		dt->gadget, dt->type, dt->name, !!(dt->opts & GT_FORCE));

	for (ptr = dt->attrs; ptr->variable; ptr++)
		printf(", %s=%s", ptr->variable, ptr->value);

	putchar('\n');

	return 0;
}

static int gt_func_create_help(void *data)
{
	printf("Func create help.\n");
	return -1;
}

static void gt_parse_func_create(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_create_data *dt = NULL;
	int ind;
	int tmp;
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

	tmp = gt_get_func_name(&dt->type, &dt->name, argc - ind, argv + ind);
	if (tmp < 0)
		goto out;
	ind += tmp;

	tmp = gt_parse_setting_list(&dt->attrs, argc - ind, argv + ind);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, gt_func_create_func, (void *)dt,
			gt_func_create_destructor);

	return;
out:
	gt_func_create_destructor(dt);
	executable_command_set(exec, gt_func_create_help, data, NULL);
}

struct gt_func_rm_data {
	const char *gadget;
	char *type;
	char *name;
	int opts;
};

static void gt_func_rm_destructor(void *data)
{
	struct gt_func_rm_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_func_rm_data *)data;
	free(dt->type);
	free(dt->name);
	free(dt);
}

static int gt_func_rm_func(void *data)
{
	struct gt_func_rm_data *dt;

	dt = (struct gt_func_rm_data *)data;
	printf("Func rm called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, name=%s, recursive=%d, force=%d\n",
		dt->gadget, dt->type, dt->name, !!(dt->opts & GT_RECURSIVE),
		!!(dt->opts & GT_FORCE));

	return 0;
}

static int gt_func_rm_help(void *data)
{
	printf("Func rm help.\n");
	return -1;
}

static void gt_parse_func_rm(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_rm_data *dt = NULL;
	int ind;
	int tmp;
	int avaible_opts = GT_FORCE | GT_RECURSIVE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (argc - ind < 2)
		goto out;

	dt->gadget = argv[ind++];

	tmp = gt_get_func_name(&dt->type, &dt->name, argc - ind, argv + ind);
	if (tmp < 0)
		goto out;
	ind += tmp;

	if (ind != argc)
		goto out;

	executable_command_set(exec, gt_func_rm_func, (void *)dt,
			gt_func_rm_destructor);

	return;
out:
	gt_func_rm_destructor(dt);
	executable_command_set(exec, gt_func_rm_help, data, NULL);
}


int gt_func_help(void *data)
{
	printf("Function help function\n");
	return -1;
}

const Command *gt_func_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_func_create, NULL,
			gt_func_create_help},
		{"rm", NEXT, gt_parse_func_rm, NULL, gt_func_rm_help},
//		{"get", AGAIN, gt_parse_func_get, NULL, gt_func_get_help},
//		{"set", AGAIN, gt_parse_func_set, NULL, gt_func_set_help},
//		{"load", AGAIN, gt_parse_func_load, NULL, gt_func_load_help},
//		{"save", AGAIN, gt_parse_func_save, NULL, gt_func_save_help}.
//		{"template", AGAIN, parse_command,
//			gt_func_template_get_children, gt_func_template_help},
//		{NULL, PREV, gt_parse_func_func, NULL, gt_func_func_help},
		{NULL, AGAIN, NULL, NULL, NULL}
	};
	return commands;
}
