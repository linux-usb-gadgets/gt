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

struct gt_func_get_data {
	const char *gadget;
	char *type;
	char *name;
	const char **attrs;
};

static void gt_func_get_destructor(void *data)
{
	struct gt_func_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_get_data *)data;
	free(dt->type);
	free(dt->name);
	free(dt->attrs);
	free(dt);
}

static int gt_func_get_func(void *data)
{
	struct gt_func_get_data *dt;
	const char **ptr;

	dt = (struct gt_func_get_data *)data;
	printf("Func get called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, name=%s, attrs=",
		dt->gadget, dt->type, dt->name);

	for (ptr = dt->attrs; *ptr; ptr++)
		printf("%s, ", *ptr);
	putchar('\n');

	return 0;
}

static int gt_func_get_help(void *data)
{
	printf("Func get help.\n");
	return -1;
}

static void gt_parse_func_get(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_get_data *dt = NULL;
	int tmp;
	int i;

	if (argc < 2)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->gadget = argv[0];

	tmp = gt_get_func_name(&dt->type, &dt->name, argc - 1, argv + 1);
	if (tmp < 0)
		goto out;

	dt->attrs = calloc(argc - tmp, sizeof(char *));
	if (dt->attrs == NULL)
		goto out;

	argv += tmp + 1;
	for (i = 0; argv[i]; i++)
		dt->attrs[i] = argv[i];

	executable_command_set(exec, gt_func_get_func, (void *)dt,
			gt_func_get_destructor);

	return;
out:
	gt_func_get_destructor(dt);
	executable_command_set(exec, gt_func_get_help, data, NULL);
}

struct gt_func_set_data {
	const char *gadget;
	char *type;
	char *name;
	struct gt_setting *attrs;
};

static void gt_func_set_destructor(void *data)
{
	struct gt_func_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_set_data *)data;
	free(dt->type);
	free(dt->name);
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_func_set_func(void *data)
{
	struct gt_func_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_func_set_data *)data;
	printf("Func set called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, name=%s", dt->gadget, dt->type, dt->name);

	for (ptr = dt->attrs; ptr->variable; ptr++)
		printf(", %s=%s", ptr->variable, ptr->value);
	putchar('\n');

	return 0;
}

static int gt_func_set_help(void *data)
{
	printf("Func set help.\n");
	return -1;
}

static void gt_parse_func_set(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_set_data *dt = NULL;
	int tmp;

	if (argc < 2)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->gadget = argv[0];
	tmp = gt_get_func_name(&dt->type, &dt->name, --argc, ++argv);
	if (tmp < 0)
		goto out;

	argc -= tmp;
	argv += tmp;
	tmp = gt_parse_setting_list(&dt->attrs, argc, argv);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, gt_func_set_func, (void *)dt,
			gt_func_set_destructor);

	return;
out:
	gt_func_set_destructor(dt);
	executable_command_set(exec, gt_func_get_help, data, NULL);
}

struct gt_func_func_data {
	const char *gadget;
	char *type;
	char *name;
	int opts;
};

static void gt_func_func_destructor(void *data)
{
	struct gt_func_func_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_func_data *)data;
	free(dt->type);
	free(dt->name);
	free(dt);
}

static int gt_func_func_func(void *data)
{
	struct gt_func_func_data *dt;

	dt = (struct gt_func_func_data *)data;
	printf("Func func called successfully. Not implemented.\n");
	printf("gadget=%s", dt->gadget);
	if (dt->type)
		printf(", type=%s", dt->type);
	if (dt->name)
		printf(", name=%s", dt->name);
	printf(", verbose=%d\n", !!(dt->opts & GT_VERBOSE));

	return 0;
}

static int gt_func_func_help(void *data)
{
	printf("Func func help.\n");
	return -1;
}

static void gt_parse_func_func(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_func_data *dt = NULL;
	int ind;
	int tmp;
	int avaible_opts = GT_VERBOSE;

	if (argc < 1)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	dt->gadget = argv[ind++];
	if (argc > ind) {
		tmp = gt_get_func_name(&dt->type, &dt->name, argc - ind, argv + ind);
		if (tmp < 0)
			goto out;
		if (ind + tmp != argc)
			goto out;
	}

	executable_command_set(exec, gt_func_func_func, (void *)dt,
			gt_func_func_destructor);
	return;
out:
	gt_func_func_destructor(dt);
	executable_command_set(exec, gt_func_func_help, data, NULL);
}

struct gt_func_load_data {
	const char *name;
	const char *gadget;
	const char *func;
	const char *file;
	const char *path;
	int opts;
};

static int gt_func_load_func(void *data)
{
	struct gt_func_load_data *dt;

	dt = (struct gt_func_load_data *)data;
	printf("Func load called succesfully. Not implemented.\n");
	if (dt->name)
		printf("name=%s, ", dt->name);
	if (dt->gadget)
		printf("gadget=%s, ", dt->gadget);
	if (dt->func)
		printf("func=%s, ", dt->func);
	if (dt->file)
		printf("file=%s, ", dt->file);
	if (dt->path)
		printf("path=%s, ", dt->path);
	printf("force=%d, stdin=%d\n", !!(dt->opts & GT_FORCE),
			!!(dt->opts & GT_STDIN));

	return 0;
}

static int gt_func_load_help(void *data)
{
	printf("Func load help.\n");
	return -1;
}

static void gt_parse_func_load(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_load_data *dt = NULL;
	int c;
	struct option opts[] = {
			{"force", no_argument, 0, 'f'},
			{"recursive", no_argument, 0, 'r'},
			{"file", required_argument, 0, 1},
			{"stdin", no_argument, 0, 2},
			{"path", required_argument, 0, 3},
			{0, 0, 0, 0}
		};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	argv--;
	argc++;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "f", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			dt->opts |= GT_FORCE;
			break;
		case 1:
			if (dt->path || dt->opts & GT_STDIN)
				goto out;
			dt->file = optarg;
			break;
		case 2:
			if (dt->path || dt->file)
				goto out;
			dt->opts |= GT_STDIN;
			break;
		case 3:
			if (dt->file || dt->opts & GT_STDIN)
				goto out;
			dt->path = optarg;
			break;
		default:
			goto out;
		}
	}

	if (argc - optind < 2 || argc - optind > 3)
		goto out;

	dt->name = argv[optind++];
	dt->gadget = argv[optind++];
	if (dt->opts & GT_STDIN || dt->file) {
		dt->func = dt->name;
		dt->name = NULL;
		if (optind < argc)
			goto out;
	}

	if (optind < argc)
		dt->func = argv[optind];

	executable_command_set(exec, gt_func_load_func, (void *)dt, free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_func_save_data {
	const char *gadget;
	const char *func;
	const char *name;
	const char *file;
	const char *path;
	int opts;
	struct gt_setting *attrs;
};

static void gt_func_save_destructor(void *data)
{
	struct gt_func_save_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_func_save_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_func_save_func(void *data)
{
	struct gt_func_save_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_func_save_data *)data;

	printf("Func save called successfully. Not implemented.\n");
	if (dt->gadget)
		printf("gadget=%s, ", dt->gadget);
	if (dt->func)
		printf("func=%s, ", dt->func);
	if (dt->name)
		printf("name=%s, ", dt->name);
	if (dt->file)
		printf("file=%s, ", dt->file);
	if (dt->path)
		printf("path=%s, ", dt->path);
	printf("force=%d, stdout=%d", !!(dt->opts & GT_FORCE),
			!!(dt->opts & GT_STDOUT));

	for (ptr = dt->attrs; ptr->variable; ptr++)
		printf(", %s=%s", ptr->variable, ptr->value);
	putchar('\n');

	return 0;
}

static int gt_func_save_help(void *data)
{
	printf("Func save help.\n");
	return -1;
}

static void gt_parse_func_save(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_save_data *dt = NULL;
	int c;
	struct option opts[] = {
			{"force", no_argument, 0, 'f'},
			{"file", required_argument, 0, 1},
			{"stdout", no_argument, 0, 2},
			{"path", required_argument, 0, 3},
			{0, 0, 0, 0}
		};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	argv--;
	argc++;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "f", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			dt->opts |= GT_FORCE;
			break;
		case 1:
			if (dt->path || dt->opts & GT_STDOUT)
				goto out;
			dt->file = optarg;
			break;
		case 2:
			if (dt->path || dt->file)
				goto out;
			dt->opts |= GT_STDOUT;
			break;
		case 3:
			if (dt->file || dt->opts & GT_STDOUT)
				goto out;
			dt->path = optarg;
			break;
		default:
			goto out;
		}
	}

	if (argc - optind < 2)
		goto out;

	dt->gadget = argv[optind++];
	dt->func = argv[optind++];
	if (optind < argc
	   && !dt->file
	   && !(dt->opts & GT_STDOUT)
	   && strchr(argv[optind], '=') == NULL)
		dt->name = argv[optind++];

	c = gt_parse_setting_list(&dt->attrs, argc - optind, argv + optind);
	if (c < 0)
		goto out;

	executable_command_set(exec, gt_func_save_func, (void *)dt,
			gt_func_save_destructor);

	return;
out:
	gt_func_save_destructor(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
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
		{"get", NEXT, gt_parse_func_get, NULL, gt_func_get_help},
		{"set", NEXT, gt_parse_func_set, NULL, gt_func_set_help},
		{"load", NEXT, gt_parse_func_load, NULL, gt_func_load_help},
		{"save", NEXT, gt_parse_func_save, NULL, gt_func_save_help},
//		{"template", AGAIN, parse_command,
//			gt_func_template_get_children, gt_func_template_help},
		{NULL, AGAIN, gt_parse_func_func, NULL, gt_func_func_help},
		{NULL, AGAIN, NULL, NULL, NULL}
	};
	return commands;
}
