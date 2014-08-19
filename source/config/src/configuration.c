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

struct gt_config_get_data {
	const char *gadget;
	const char *config;
	const char **attrs;
};

static void gt_config_get_destructor(void *data)
{
	struct gt_config_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_get_data *)data;
	free(dt->attrs);
	free(dt);
}

static int gt_config_get_func(void *data)
{
	struct gt_config_get_data *dt;
	const char **ptr;

	dt = (struct gt_config_get_data *)data;
	printf("Config get called successfully. Not implemented.\n");
	printf("gadget = %s, config = %s, attrs = ",
		dt->gadget, dt->config);
	ptr = dt->attrs;
	while (*ptr) {
		printf("%s, ", *ptr);
		ptr++;
	}
	putchar('\n');
	return 0;
}

static int gt_config_get_help(void *data)
{
	printf("Config get help.\n");
	return -1;
}

static void gt_parse_config_get(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_config_get_data *dt = NULL;
	int i;

	if (argc < 2)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->gadget = argv[0];
	dt->config = argv[1];

	dt->attrs = calloc(argc - 1, sizeof(char *));
	if (dt->attrs == NULL)
		goto out;

	argv += 2;
	for (i = 0; argv[i]; i++)
		dt->attrs[i] = argv[i];

	executable_command_set(exec, gt_config_get_func, (void *)dt,
			gt_config_get_destructor);
	return;
out:
	gt_config_get_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_config_set_data {
	const char *gadget;
	const char *config;
	struct gt_setting *attrs;
};

static void gt_config_set_destructor(void *data)
{
	struct gt_config_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_set_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_config_set_func(void *data)
{
	struct gt_config_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_config_set_data *)data;
	printf("Config set called successfully. Not implemented.\n");
	printf("gadget = %s, config = %s", dt->gadget, dt->config);
	ptr = dt->attrs;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int gt_config_set_help(void *data)
{
	printf("Config set help.\n");
	return -1;
}

static void gt_parse_config_set(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_config_set_data *dt = NULL;
	int tmp;

	if (argc < 3)
		goto out;
	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->gadget = argv[0];
	dt->config = argv[1];
	tmp = gt_parse_setting_list(&dt->attrs, argc - 2, argv + 2);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, gt_config_set_func, (void *)dt,
			gt_config_set_destructor);
	return;
out:
	gt_config_set_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_config_config_data {
	const char *gadget;
	const char *config;
	int opts;
};

static int gt_config_config_func(void *data)
{
	struct gt_config_config_data *dt;

	dt = (struct gt_config_config_data *)data;
	printf("Config config called successfully. Not implemented.\n");
	printf("gadget = %s", dt->gadget);
	if (dt->config)
		printf(", config = %s", dt->config);
	printf(", verbose = %d, recursive = %d\n",
		!!(dt->opts & GT_VERBOSE), !!(dt->opts & GT_RECURSIVE));

	return 0;
}

static int gt_config_config_help(void *data)
{
	printf("Config config help.\n");
	return -1;
}

static void gt_parse_config_config(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	int ind;
	struct gt_config_config_data *dt = NULL;
	int avaible_opts = GT_VERBOSE | GT_RECURSIVE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (ind == argc || argc - ind > 2)
		goto out;

	dt->gadget = argv[ind++];
	if (ind < argc)
		dt->config = argv[ind++];

	executable_command_set(exec, gt_config_config_func, (void *)dt, free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_config_add_data {
	const char *gadget;
	const char *config;
	char *type;
	char *instance;
};

static void gt_config_add_destructor(void *data)
{
	struct gt_config_add_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_add_data *)data;
	free(dt->type);
	free(dt->instance);
	free(dt);
}

static int gt_config_add_func(void *data)
{
	struct gt_config_add_data *dt;

	dt = (struct gt_config_add_data *)data;
	printf("Config add called successfully. Not implemented.\n");
	printf("gadget = %s, conf = %s, type = %s, instance = %s\n",
			dt->gadget, dt->config, dt->type, dt->instance);
	return 0;
}

static int gt_config_add_help(void *data)
{
	printf("Config add help.\n");
	return -1;
}

static void gt_parse_config_add(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_config_add_data *dt = NULL;
	int tmp;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	switch (argc) {
	case 3:
		tmp = gt_parse_function_name(&dt->type, &dt->instance,
				argv[2]);
		if (tmp < 0)
			goto out;
		break;
	case 4:
		dt->type = strdup(argv[2]);
		dt->instance = strdup(argv[3]);
		break;
	default:
		goto out;
	}

	dt->gadget = argv[0];
	dt->config = argv[1];

	executable_command_set(exec, gt_config_add_func, (void *)dt,
			gt_config_add_destructor);

	return;
out:
	gt_config_add_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_config_del_data {
	const char *gadget;
	const char *config;
	char *type;
	char *instance;
};

static void gt_config_del_destructor(void *data)
{
	struct gt_config_del_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_del_data *)data;
	free(dt->type);
	free(dt->instance);
	free(dt);
}

static int gt_config_del_func(void *data)
{
	struct gt_config_del_data *dt;

	dt = (struct gt_config_del_data *)data;
	printf("Config del called successfully. Not implemented.\n");
	printf("gadget = %s, conf = %s, type = %s, instance = %s\n",
			dt->gadget, dt->config, dt->type, dt->instance);
	return 0;
}

static int gt_config_del_help(void *data)
{
	printf("Config del help.\n");
	return -1;
}

static void gt_parse_config_del(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_config_add_data *dt = NULL;
	int tmp;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	switch (argc) {
	case 3:
		tmp = gt_parse_function_name(&dt->type, &dt->instance,
				argv[2]);
		if (tmp < 0)
			goto out;
		break;
	case 4:
		dt->type = strdup(argv[2]);
		if (dt->type == NULL)
			goto out;
		dt->instance = strdup(argv[3]);
		if (dt->instance == NULL)
			goto out;
		break;
	default:
		goto out;
	}

	dt->gadget = argv[0];
	dt->config = argv[1];

	executable_command_set(exec, gt_config_del_func, (void *)dt,
			gt_config_del_destructor);

	return;
out:
	gt_config_del_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_config_template_data {
	const char *name;
	int opts;
};

static int gt_config_template_func(void *data)
{
	struct gt_config_template_data *dt;

	dt = (struct gt_config_template_data *)data;
	printf("Config template called successfully. Not implemented.\n");
	if (dt->name)
		printf("name = %s, ", dt->name);
	printf("verbose = %d, recursive = %d\n",
		!!(dt->opts & GT_VERBOSE), !!(dt->opts & GT_RECURSIVE));
	return 0;
}

static int gt_config_template_help(void *data)
{
	printf("Config template help.\n");
	return -1;
}

static void gt_parse_config_template(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	int ind;
	struct gt_config_template_data *dt = NULL;
	int avaible_opts = GT_VERBOSE | GT_RECURSIVE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (argc - ind > 1)
		goto out;

	dt->name = argv[ind];

	executable_command_set(exec, gt_config_template_func, (void *)dt, free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_config_template_get_data {
	const char *name;
	const char **attr;
};

static void gt_config_template_get_destructor(void *data)
{
	struct gt_config_template_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_template_get_data *)data;
	free(dt->attr);
	free(dt);
}

static int gt_config_template_get_func(void *data)
{
	struct gt_config_template_get_data *dt;
	const char **ptr;

	dt = (struct gt_config_template_get_data *)data;
	printf("Config template get called successfully. Not implemented.\n");
	printf("name = %s, attr = ", dt->name);
	ptr = dt->attr;
	while (*ptr) {
		printf("%s, ", *ptr);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int gt_config_template_get_help(void *data)
{
	printf("Config template get help.\n");
	return -1;
}

static void gt_parse_config_template_get(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int i;
	struct gt_config_template_get_data *dt = NULL;

	if (argc < 1)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->name = argv[0];
	dt->attr = calloc(argc, sizeof(char *));
	if(dt->attr == NULL)
		goto out;

	argv++;
	for (i = 0; argv[i]; i++)
		dt->attr[i] = argv[i];

	executable_command_set(exec, gt_config_template_get_func, (void *)dt,
		gt_config_template_get_destructor);

	return;
out:
	gt_config_template_get_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_config_template_set_data {
	const char *name;
	struct gt_setting *attr;
};

static void gt_config_template_set_destructor(void *data)
{
	struct gt_config_template_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_template_set_data *)data;
	gt_setting_list_cleanup(dt->attr);
	free(dt);
}

static int gt_config_template_set_func(void *data)
{
	struct gt_config_template_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_config_template_set_data *)data;
	printf("Config template set called successfully. Not implemened.\n");
	printf("name = %s", dt->name);
	ptr = dt->attr;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int gt_config_template_set_help(void *data)
{
	printf("Config template set help.\n");
	return -1;
}

static void gt_parse_config_template_set(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int tmp;
	struct gt_config_template_set_data *dt = NULL;

	if (argc < 2)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->name = argv[0];
	tmp = gt_parse_setting_list(&dt->attr, argc - 1, argv + 1);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, gt_config_template_set_func, (void *)dt,
		gt_config_template_set_destructor);

	return;
out:
	gt_config_template_set_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_config_template_rm_func(void *data)
{
	const char *dt;

	dt = (const char *)data;
	printf("Config template rm called successfully. Not implemented.\n");
	printf("name = %s\n", dt);
	return 0;
}

static int gt_config_template_rm_help(void *data)
{
	printf("Config template rm help.\n");
	return -1;
}

static void gt_parse_config_template_rm(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	const char *dt = NULL;

	if (argc != 1)
		goto out;

	dt = argv[0];
	executable_command_set(exec, gt_config_template_rm_func, (void *)dt,
			NULL);

	return;
out:
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static const Command *gt_config_template_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"rm", NEXT, gt_parse_config_template_rm, NULL,
			gt_config_template_rm_help},
		{"set", NEXT, gt_parse_config_template_set, NULL,
			gt_config_template_set_help},
		{"get", NEXT, gt_parse_config_template_get, NULL,
			gt_config_template_get_help},
		{NULL, AGAIN, gt_parse_config_template, NULL,
			gt_config_template_help},
		CMD_LIST_END
	};

	return commands;
}

struct gt_config_load_data {
	const char *name;
	const char *gadget;
	const char *config;
	const char *file;
	const char *path;
	int opts;
};

static int gt_config_load_func(void *data)
{
	struct gt_config_load_data *dt;

	dt = (struct gt_config_load_data *)data;
	printf("Config load called successfully. Not implemented.\n");
	if (dt->name)
		printf("name = %s, ", dt->name);
	if (dt->gadget)
		printf("gadget = %s, ", dt->gadget);
	if (dt->config)
		printf("config = %s, ", dt->config);
	if (dt->file)
		printf("file = %s, ", dt->file);
	if (dt->path)
		printf("path = %s, ", dt->path);
	printf("recursive = %d, force = %d, stdin = %d\n",
		!!(dt->opts & GT_RECURSIVE), !!(dt->opts & GT_FORCE),
		!!(dt->opts & GT_STDIN));

	return 0;
}

static int gt_config_load_help(void *data)
{
	printf("Config load help.\n");
	return -1;
}

static void gt_parse_config_load(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int c;
	struct gt_config_load_data *dt = NULL;
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
		c = getopt_long(argc, argv, "fr", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 'f':
			dt->opts |= GT_FORCE;
			break;
		case 'r':
			dt->opts |= GT_RECURSIVE;
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

	if (argc - optind > 3 || argc - optind < 2)
		goto out;

	dt->name = argv[optind++];
	dt->gadget = argv[optind++];
	if (dt->opts & GT_STDIN || dt->file) {
		dt->config = dt->name;
		dt->name = NULL;
		if (optind < argc)
			goto out;
	}

	if (optind < argc)
		dt->config = argv[optind++];

	executable_command_set(exec, gt_config_load_func, (void *)dt, free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_config_save_data {
	const char *gadget;
	const char *config;
	const char *name;
	const char *file;
	const char *path;
	int opts;
	struct gt_setting *attrs;
};

static void gt_config_save_destructor(void *data)
{
	struct gt_config_save_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_save_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_config_save_func(void *data)
{
	struct gt_config_save_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_config_save_data *)data;
	printf("Config save called successfully. Not implemented.\n");
	if (dt->gadget)
		printf("gadget=%s, ", dt->gadget);
	if (dt->config)
		printf("config=%s, ", dt->config);
	if (dt->name)
		printf("name=%s, ", dt->name);
	if (dt->file)
		printf("file=%s, ", dt->file);
	if (dt->path)
		printf("path=%s, ", dt->path);
	printf("force=%d, stdout=%d",
		!!(dt->opts & GT_FORCE), !!(dt->opts & GT_STDOUT));

	ptr = dt->attrs;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');


	return 0;
}

static int gt_config_save_help(void *data)
{
	printf("Config save help.\n");
	return -1;
}

static void gt_parse_config_save(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int c;
	struct gt_config_save_data *dt = NULL;
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
	dt->config = argv[optind++];
	if (optind < argc
	   && !dt->file
	   && !(dt->opts & GT_STDOUT)
	   && strchr(argv[optind], '=') == NULL)
		dt->name = argv[optind++];

	c = gt_parse_setting_list(&dt->attrs, argc - optind,
			argv + optind);
	if (c < 0)
		goto out;

	executable_command_set(exec, gt_config_save_func, (void *)dt,
			gt_config_save_destructor);

	return;
out:
	gt_config_save_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *gt_config_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_config_create, NULL,
		 gt_config_create_help},
		{"rm", NEXT, gt_parse_config_rm, NULL, gt_config_rm_help},
		{"get", NEXT, gt_parse_config_get, NULL, gt_config_get_help},
		{"set", NEXT, gt_parse_config_set, NULL, gt_config_set_help},
		{"add", NEXT, gt_parse_config_add, NULL, gt_config_add_help},
		{"del", NEXT, gt_parse_config_del, NULL, gt_config_del_help},
		{"template", NEXT, command_parse,
			gt_config_template_get_children,
			gt_config_template_help},
		{"load", NEXT, gt_parse_config_load, NULL,
			gt_config_load_help},
		{"save", NEXT, gt_parse_config_save, NULL,
			gt_config_save_help},
		{NULL, AGAIN, gt_parse_config_config, NULL,
			gt_config_config_help},
		CMD_LIST_END
	};

	return commands;
}
