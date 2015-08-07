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
#include <usbg/usbg.h>

#include "function.h"
#include "common.h"
#include "parser.h"
#include "backend.h"

#define GET_EXECUTABLE(func) \
	(backend_ctx.backend->function->func ? \
	 backend_ctx.backend->function->func : \
	 gt_function_backend_not_implemented.func)

static void gt_func_create_destructor(void *data)
{
	struct gt_func_create_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_func_create_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_func_create_help(void *data)
{
	printf("usage: %s func create <gadget_name> <function_type> <function_name>\n"
	       "Create new function of specified type (refer to `gt func list-types')\n",
		program_name);
	return -1;
}

static void gt_parse_func_create(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_create_data *dt = NULL;
	int ind;
	int tmp;
	int avaible_opts = GT_FORCE | GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind < 3)
		goto out;

	dt->gadget = argv[ind++];
	dt->type = argv[ind++];
	dt->name = argv[ind++];

	tmp = gt_parse_setting_list(&dt->attrs, argc - ind, argv + ind);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(create),
			(void *)dt, gt_func_create_destructor);

	return;
out:
	gt_func_create_destructor(dt);
	executable_command_set(exec, gt_func_create_help, data, NULL);
}

static void gt_func_rm_destructor(void *data)
{
	struct gt_func_rm_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_func_rm_data *)data;
	free(dt);
}


static int gt_func_rm_help(void *data)
{
	printf("usage: %s func rm <gadget> <type> <instance>\n"
	       "Remove function.\n"
	       "\n"
	       "Options:\n"
	       "  -f, --force\tDisable gadget if necessary\n"
	       "  -r, --recursive\tUnbind function from all configs\n"
	       "  -h, --help\tPrint this help\n",
		program_name);

	return -1;
}

static void gt_parse_func_rm(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_rm_data *dt = NULL;
	int ind;
	int avaible_opts = GT_FORCE | GT_RECURSIVE | GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind < 3)
		goto out;

	dt->gadget = argv[ind++];

	dt->type = usbg_lookup_function_type(argv[ind]);
	if (dt->type < 0) {
		fprintf(stderr, "Unknown function type: %s", argv[ind]);
		goto out;
	}
	ind++;

	dt->instance = argv[ind++];

	if (ind != argc)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(rm),
			(void *)dt, gt_func_rm_destructor);

	return;
out:
	gt_func_rm_destructor(dt);
	executable_command_set(exec, gt_func_rm_help, data, NULL);
}

static int gt_func_list_types_help(void *data)
{
	printf("usage: %s func list-types\n"
	       "List available function types.\n"
	       "\n"
	       "Options:\n"
	       "  -q, --quiet\tPrint only list of types\n"
	       "  -h, --help\tPrintf this help\n",
		program_name);
	return -1;
}

static void gt_parse_func_list_types(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_list_types_data *dt = NULL;
	int avaible_opts = GT_QUIET | GT_HELP;
	int ind;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind != 0)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(list_types),
				(void *)dt, free);
	return;

out:
	executable_command_set(exec, gt_func_list_types_help, NULL, NULL);

}

static void gt_func_get_destructor(void *data)
{
	struct gt_func_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_get_data *)data;
	free(dt->attrs);
	free(dt);
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
	int ind = 0;
	int i;
	int avaible_opts = GT_HELP;

	if (argc < 3)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	dt->gadget = argv[ind++];
	dt->type = argv[ind++];
	dt->name = argv[ind++];

	/* One more for NULL */
	dt->attrs = calloc(argc - ind + 1, sizeof(char *));
	if (dt->attrs == NULL)
		goto out;

	argv += ind;
	for (i = 0; argv[i]; i++)
		dt->attrs[i] = argv[i];

	executable_command_set(exec, GET_EXECUTABLE(get), (void *)dt,
			gt_func_get_destructor);

	return;
out:
	gt_func_get_destructor(dt);
	executable_command_set(exec, gt_func_get_help, data, NULL);
}

static void gt_func_set_destructor(void *data)
{
	struct gt_func_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_set_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
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
	int ind = 0;
	int tmp;
	int avaible_opts = GT_HELP;

	if (argc < 3)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	dt->gadget = argv[ind++];
	dt->type = argv[ind++];
	dt->name = argv[ind++];

	argc -= ind;
	argv += ind;
	tmp = gt_parse_setting_list(&dt->attrs, argc, argv);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(set), (void *)dt,
			gt_func_set_destructor);

	return;
out:
	gt_func_set_destructor(dt);
	executable_command_set(exec, gt_func_get_help, data, NULL);
}

static void gt_func_show_destructor(void *data)
{
	struct gt_func_show_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_show_data *)data;
	free(dt);
}

static int gt_func_func_help(void *data)
{
	printf("usage: %s func [command] ...\n"
	       "Manipulate USB function - both in-kernel and functionfs-based\n\n",
		program_name);

	printf("Command:\n"
	       "  show\n"
	       "  create\n"
	       "  rm\n"
	       "  get\n"
	       "  set\n"
	       "  list-types\n"
	       "  load\n"
	       "  save\n"
	       "  template\n");

	return -1;
}

static int gt_func_show_help(void *data)
{
	printf("usage: %s func show <gadget> [type[ [instance]]\n"
	       "Show functions. If no function was specified, show all functions.\n"
	       "If only function type was specified show only functions of given type\n\n",
		program_name);

	printf("Options:\n"
	       "  -v, --verbose\tShow also attributes\n"
	       "  --instance\tShow only function instances (cannot be used with --type)\n"
	       "  --type\t\tShow only function types (cannot be used with  --instance)\n"
	       "  -h, --help\tPrint this help\n");

	return -1;
}

static void gt_parse_func_show(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_show_data *dt = NULL;
	int ind;
	int avaible_opts = GT_VERBOSE | GT_HELP | GT_INSTANCE | GT_TYPE;

	if (argc < 1)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (dt->opts & GT_INSTANCE && dt->opts & GT_TYPE)
		goto out;

	dt->gadget = argv[ind++];
	dt->type = -1;
	dt->instance = NULL;
	if (argc > ind) {
		dt->type = usbg_lookup_function_type(argv[ind]);
		if (dt->type < 0) {
			fprintf(stderr, "Unknown function type: %s", argv[ind]);
			goto out;
		}

		ind++;
		if (argc > ind)
			dt->instance = argv[ind++];
	}

	executable_command_set(exec, GET_EXECUTABLE(show), (void *)dt,
			gt_func_show_destructor);
	return;
out:
	gt_func_show_destructor(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
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
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}
		};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	argv--;
	argc++;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "frh", opts, &opt_index);
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
		case 'h':
			goto out;
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

	executable_command_set(exec, GET_EXECUTABLE(load),
			(void *)dt, free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_func_save_destructor(void *data)
{
	struct gt_func_save_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_func_save_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
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
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}
		};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	argv--;
	argc++;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "fh", opts, &opt_index);
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
		case 'h':
			goto out;
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

	executable_command_set(exec, GET_EXECUTABLE(save), (void *)dt,
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

static int gt_func_template_help(void *data)
{
	printf("Func template help.\n");
	return -1;
}

static void gt_parse_func_template(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_template_data *dt;
	int avaible_opts = GT_VERBOSE | GT_HELP;
	int ind;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind > 1)
		goto out;

	dt->name = argv[ind++];

	executable_command_set(exec, GET_EXECUTABLE(template_default),
			(void *)dt, free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_func_template_get_destructor(void *data)
{
	struct gt_func_template_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_template_get_data *)data;
	free(dt->attrs);
	free(dt);
}

static int gt_func_template_get_help(void *data)
{
	printf("Func template get help called successfully.\n");
	return -1;
}

static void gt_parse_func_template_get(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_template_get_data *dt = NULL;
	int i;
	int ind = 0;
	int avaible_opts = GT_HELP;

	if (argc < 1)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	dt->name = argv[ind++];

	dt->attrs = calloc(argc, sizeof(char *));
	if (dt->attrs == NULL)
		goto out;

	argv += 1;
	for (i = 0; argv[i]; i++)
		dt->attrs[i] = argv[i];

	executable_command_set(exec, GET_EXECUTABLE(template_get),
			(void *)dt, gt_func_template_get_destructor);
	return;

out:
	gt_func_template_get_destructor(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_func_template_set_destructor(void *data)
{
	struct gt_func_template_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_template_set_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_func_template_set_help(void *data)
{
	printf("Func template set help.\n");
	return -1;
}

static void gt_parse_func_template_set(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_template_set_data *dt = NULL;
	int tmp;
	int ind = 0;
	int avaible_opts = GT_HELP;

	if (argc < 2)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	dt->name = argv[ind++];
	tmp = gt_parse_setting_list(&dt->attrs, argc - ind, argv + ind);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(template_set), (void *)dt,
			gt_func_template_set_destructor);
	return;

out:
	gt_func_template_set_destructor(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_func_template_rm_help(void *data)
{
	printf("Func template rm help.\n");
	return -1;
}

static void gt_parse_func_template_rm(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	const char *name;
	int ind = 0;
	int avaible_opts = GT_HELP;
	int opts = 0;

	if (argc != 1)
		goto out;

	ind = gt_get_options(&opts, avaible_opts, argc, argv);
	if (ind < 0 || opts & GT_HELP)
		goto out;

	name = argv[ind];

	executable_command_set(exec, GET_EXECUTABLE(template_rm), (void *)name, NULL);

	return;
out:
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *gt_func_template_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"get", NEXT, gt_parse_func_template_get, NULL,
			gt_func_template_get_help},
		{"set", NEXT, gt_parse_func_template_set, NULL,
			gt_func_template_set_help},
		{"rm", NEXT, gt_parse_func_template_rm, NULL,
			gt_func_template_rm_help},
		{NULL, AGAIN, gt_parse_func_template, NULL,
			gt_func_template_help},
		{NULL, AGAIN, NULL, NULL, NULL}
	};
	return commands;
}

const Command *gt_func_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_func_create, NULL,
			gt_func_create_help},
		{"rm", NEXT, gt_parse_func_rm, NULL, gt_func_rm_help},
		{"get", NEXT, gt_parse_func_get, NULL, gt_func_get_help},
		{"set", NEXT, gt_parse_func_set, NULL, gt_func_set_help},
		{"list-types", NEXT, gt_parse_func_list_types, NULL, gt_func_list_types_help},
		{"load", NEXT, gt_parse_func_load, NULL, gt_func_load_help},
		{"save", NEXT, gt_parse_func_save, NULL, gt_func_save_help},
		{"template", NEXT, command_parse,
			gt_func_template_get_children, gt_func_template_help},
		{"show", NEXT, gt_parse_func_show, NULL, gt_func_show_help},
		{NULL, AGAIN, gt_parse_func_show, NULL, gt_func_func_help},
		{NULL, AGAIN, NULL, NULL, NULL}
	};
	return commands;
}
