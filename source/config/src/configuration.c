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
#include "backend.h"

#include <errno.h>
#ifdef WITH_GADGETD
#include <gio/gio.h>
#endif

#define GET_EXECUTABLE(func) \
	(backend_ctx.backend->config->func ? \
	 backend_ctx.backend->config->func : \
	 gt_config_backend_not_implemented.func)

int gt_config_help(void *data)
{
	printf("Configuration help function\n");
	return -1;
}

static void gt_config_create_destructor(void *data)
{
	struct gt_config_create_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_create_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_config_create_help(void *data)
{
	printf("usage: %s config create [options] <gadget_name> <cfg_label> <cfg_id>\n"
	       "Add new config to gadget.\n"
	       "\n"
	       "Options:"
	       "-f, --force\tOverride config if config with given name already exists"
	       "-h, --help\tPrint this help",
	       program_name);
	return -1;
}

static void gt_parse_config_create(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	int c;
	int ind;
	struct gt_config_create_data *dt;
	int avaible_opts = GT_FORCE | GT_HELP;
	char *endptr = NULL;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	/* Since now we support only g1 label 1 version */
	if (argc - ind < 3)
		goto out;

	errno = 0;
	dt->gadget = argv[ind++];
	dt->config_label = argv[ind++];
	dt->config_id = strtoul(argv[ind++], &endptr, 0);
	if (dt->config_id == 0 || errno || (endptr && *endptr != 0))
		goto out;

	c = gt_parse_setting_list(&dt->attrs, argc - ind, argv + ind);
	if (c < 0)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(create),
			       (void *)dt, gt_config_create_destructor);
	return;

out:
	gt_config_create_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_config_rm_help(void *data)
{
	printf("usage: %s config rm [options] <gadget_name> <cfg_label> <cfg_id>\n"
	       "Remove configuration from gadget.\n"
	       "\n"
	       "Options:\n"
	       "  -f, --force\tDisable gadget if it is enabled\n"
	       "  -r, --recursive\tRecurively remove all function bindings from this config\n"
	       "  -h, --help\tPrint this help\n",
	       program_name);
	return -1;
}

static void gt_parse_config_rm(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	int ind;
	struct gt_config_rm_data *dt;
	int avaible_opts = GT_FORCE | GT_RECURSIVE | GT_HELP;
	char *endptr = NULL;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;
	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind != 3)
		goto out;

	dt->gadget = argv[ind++];
	dt->config_label = argv[ind++];
	dt->config_id = strtoul(argv[ind++], &endptr, 0);
	if (dt->config_id == 0 || errno || (endptr && *endptr != 0))
			goto out;
	executable_command_set(exec, GET_EXECUTABLE(rm), (void *)dt, free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_config_get_destructor(void *data)
{
	struct gt_config_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_get_data *)data;
	free(dt->attrs);
	free(dt);
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
	int ind;
	int avaible_opts = GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind < 2)
		goto out;

	dt->gadget = argv[ind++];
	dt->config = argv[ind++];

	dt->attrs = calloc(argc - ind + 1, sizeof(char *));
	if (dt->attrs == NULL)
		goto out;

	i = 0;
	while (argv[ind])
		dt->attrs[i++] = argv[ind++];

	executable_command_set(exec, GET_EXECUTABLE(get), (void *)dt,
			gt_config_get_destructor);
	return;
out:
	gt_config_get_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_config_set_destructor(void *data)
{
	struct gt_config_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_set_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
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
	int ind;
	int avaible_opts = GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind < 3)
		goto out;

	dt->gadget = argv[ind++];
	dt->config = argv[ind++];
	tmp = gt_parse_setting_list(&dt->attrs, argc - ind, argv + ind);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(set), (void *)dt,
			gt_config_set_destructor);
	return;
out:
	gt_config_set_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_config_config_help(void *data)
{
	printf("usage: %s config [command] ...\n"
	       "Manipulate USB configurations\n\n",
		program_name);

	printf("Command:\n"
	       "  show\n"
	       "  create\n"
	       "  rm\n"
	       "  get\n"
	       "  set\n"
	       "  add\n"
	       "  del\n"
	       "  load\n"
	       "  save\n"
	       "  template\n");

	return -1;
}

static int gt_config_show_help(void *data)
{
	printf("usage: %s config show <gadget> [config_name] [config_id]\n"
	       "Show configuration. If no name was specified, show all configurations.\n"
	       "If only config name was specified show only configurations with this name\n",
		program_name);

	printf("Options:\n"
	       "  -v, --verbose\tShow also attributes\n"
	       "  -r, --recursive\tShow details about functions\n"
	       "  --name\tShow only config names (cannot be used with --id\n"
	       "  --id\t\tShow only config ids (cannot be used with  --name)\n"
	       "  -h, --help\tPrint this help\n");

	return -1;
}

static void gt_parse_config_show(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	int ind;
	struct gt_config_show_data *dt = NULL;
	int avaible_opts = GT_VERBOSE | GT_RECURSIVE | GT_HELP | GT_NAME | GT_ID;
	char *endptr = NULL;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (ind == argc || argc - ind > 3)
		goto out;

	dt->gadget = argv[ind++];
	if (ind < argc)
		dt->config_label = argv[ind++];

	if (ind < argc) {
		dt->config_id = strtoul(argv[ind++], &endptr, 0);
		if (dt->config_id == 0 || errno || (endptr && *endptr != 0))
			goto out;
	}

	executable_command_set(exec, GET_EXECUTABLE(show), (void *)dt, free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_config_add_help(void *data)
{
	printf("usage: %s config add <gadget_name> <cfg_label> <cfg_id> <func_type> <func_instance>\n"
	       "Add function to specific configuration.\n"
	       "\n",
	       program_name);
	return -1;
}

static void gt_parse_config_add(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	/* TODO validate ID */
	struct gt_config_add_del_data *dt = NULL;
	char *endptr = NULL;
	int ind;
	int avaible_opts = GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	dt->gadget = argv[ind++];

	switch (argc - ind) {
	case 3:
	/* we are parsing e.g G1 1 acm usb0 or G1 1 ffs.one i_name */
		errno = 0;
		dt->config_id = strtoul(argv[ind++], &endptr, 0);
		if (dt->config_id == 0 || errno || (endptr && *endptr != 0))
			goto out;
		dt->config_label = "";
		dt->type = argv[ind++];
		dt->instance = argv[ind++];
		break;
	case 4:
	/* we are parsing e.g G1 label 1 acm usb0 or G1 label 1 ffs.one usb0*/
		errno = 0;
		dt->config_label = argv[ind++];
		dt->config_id = strtoul(argv[ind++], &endptr, 0);
		if (dt->config_id == 0 || errno || (endptr && *endptr != 0))
			goto out;
		dt->type = argv[ind++];
		dt->instance = argv[ind++];
		break;
	default:
		goto out;
	}

	executable_command_set(exec, GET_EXECUTABLE(add), (void *)dt, free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_config_del_help(void *data)
{
	printf("Config del help.\n");
	return -1;
}

static void gt_parse_config_del(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_config_add_del_data *dt = NULL;
	char *endptr = NULL;
	int ind;
	int avaible_opts = GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	dt->gadget = argv[ind++];

	switch (argc - ind) {
	case 3:
	/* we are parsing e.g G1 1 acm usb0 or G1 1 ffs.one i_name */
		errno = 0;
		dt->config_id = strtoul(argv[ind++], &endptr, 0);
		if (dt->config_id == 0 || errno || (endptr && *endptr != 0))
			goto out;
		dt->config_label = "";
		dt->type = argv[ind++];
		dt->instance = argv[ind++];
		break;
	case 4:
	/* we are parsing e.g G1 label 1 acm usb0 or G1 label 1 ffs.one usb0*/
		errno = 0;
		dt->config_label = argv[ind++];
		dt->config_id = strtoul(argv[ind++], &endptr, 0);
		if (dt->config_id == 0 || errno || (endptr && *endptr != 0))
			goto out;
		dt->type = argv[ind++];
		dt->instance = argv[ind++];
		break;
	default:
		goto out;
	}


	executable_command_set(exec, GET_EXECUTABLE(del), (void *)dt, free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
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
	int avaible_opts = GT_VERBOSE | GT_RECURSIVE | GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind > 1)
		goto out;

	dt->name = argv[ind];

	executable_command_set(exec, GET_EXECUTABLE(template_default), (void *)dt, free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_config_template_get_destructor(void *data)
{
	struct gt_config_template_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_template_get_data *)data;
	free(dt->attr);
	free(dt);
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
	int ind;
	int avaible_opts = GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind < 1)
		goto out;

	dt->name = argv[ind++];
	dt->attr = calloc(argc - ind + 1, sizeof(char *));
	if (dt->attr == NULL)
		goto out;

	i = 0;
	while (argv[ind])
		dt->attr[i++] = argv[ind++];

	executable_command_set(exec, GET_EXECUTABLE(template_get), (void *)dt,
		gt_config_template_get_destructor);

	return;
out:
	gt_config_template_get_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_config_template_set_destructor(void *data)
{
	struct gt_config_template_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_template_set_data *)data;
	gt_setting_list_cleanup(dt->attr);
	free(dt);
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
	int ind;
	int avaible_opts = GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind < 2)
		goto out;

	dt->name = argv[ind++];
	tmp = gt_parse_setting_list(&dt->attr, argc - ind, argv + ind);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(template_set), (void *)dt,
		gt_config_template_set_destructor);

	return;
out:
	gt_config_template_set_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_config_template_rm_help(void *data)
{
	printf("Config template rm help.\n");
	return -1;
}

static void gt_parse_config_template_rm(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_config_template_rm_data *dt = NULL;
	int ind;
	int avaible_opts = GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind != 1)
		goto out;

	dt->name = argv[ind++];
	executable_command_set(exec, GET_EXECUTABLE(template_rm), (void *)dt,
			free);

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
		case 'h':
			goto out;
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

	executable_command_set(exec, GET_EXECUTABLE(load), (void *)dt, free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_config_save_destructor(void *data)
{
	struct gt_config_save_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_config_save_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
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

	executable_command_set(exec, GET_EXECUTABLE(save), (void *)dt,
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
		{"show", NEXT, gt_parse_config_show, NULL,
			gt_config_show_help},
		{NULL, AGAIN, gt_parse_config_show, NULL,
			gt_config_config_help},
		CMD_LIST_END
	};

	return commands;
}
