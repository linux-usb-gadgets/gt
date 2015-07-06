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
#include <strings.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include "gadget.h"
#include "common.h"
#include "parser.h"
#include "backend.h"

#define GET_EXECUTABLE(func) \
	(backend_ctx.backend->gadget->func ? \
	 backend_ctx.backend->gadget->func : \
	 gt_gadget_backend_not_implemented.func)

const struct gt_gadget_str gadget_strs[] = {
	{ "product", usbg_set_gadget_product },
	{ "manufacturer", usbg_set_gadget_manufacturer },
	{ "serialnumber", usbg_set_gadget_serial_number },
};

static void gt_gadget_create_destructor(void *data)
{
	struct gt_gadget_create_data *dt;
	int i;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_create_data *)data;
	for (i = 0; i < GT_GADGET_STRS_COUNT; i++)
		free(dt->str_val[i]);

	free(dt);
}

char *attr_type_get(usbg_gadget_attr a)
{
	switch (a) {
	case B_DEVICE_CLASS:
	case B_DEVICE_SUB_CLASS:
	case B_DEVICE_PROTOCOL:
	case B_MAX_PACKET_SIZE_0:
		return "y";

	case BCD_USB:
	case ID_VENDOR:
	case ID_PRODUCT:
	case BCD_DEVICE:
		return "q";

	default:
		return NULL;
	}
}

static int attr_val_in_range(usbg_gadget_attr a, unsigned long int val)
{
	char *type = attr_type_get(a);

	if (!type)
		return 0;

	if (type[0] == 'y')
		return val <= UINT8_MAX;
	else if (type[0] == 'q')
		return val <= UINT16_MAX;
	else
		return 0;
}

static int gt_gadget_create_help(void *data)
{
	int i;

	printf("usage: %s create <name> [attr=value]...\n"
	       "Create new gadget of specified name, attributes and language strings.\n"
	       "\n"
	       "Attributes:\n",
	       program_name);

	for (i = USBG_GADGET_ATTR_MIN; i < USBG_GADGET_ATTR_MAX; i++)
		printf("  %s\n", usbg_get_gadget_attr_str(i));

	printf("Device strings (en_US locale):\n");

	for (i = 0; i < GT_GADGET_STRS_COUNT; i++)
		printf("  %s\n", gadget_strs[i].name);

	return 0;
}

static void gt_parse_gadget_create(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_gadget_create_data *dt;
	int ind;
	int c;
	int avaible_opts = GT_FORCE | GT_HELP;
	struct gt_setting *setting, *attrs = NULL;
	_Bool iter;
	int i;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (ind == argc)
		goto out;

	dt->name = argv[ind++];

	c = gt_parse_setting_list(&attrs, argc - ind, argv + ind);
	if (c < 0)
		goto out;

	for (i = 0; i < USBG_GADGET_ATTR_MAX; i++)
		dt->attr_val[i] = -1;
	memset(dt->str_val, 0, sizeof(dt->str_val));

	for (setting = attrs; setting->variable; setting++) {
		int attr_id;

		iter = TRUE;

		attr_id = usbg_lookup_gadget_attr(setting->variable);
		if (attr_id >= 0) {
			unsigned long int val;
			char *endptr = NULL;

			errno = 0;
			val = strtoul(setting->value, &endptr, 0);
			if (errno
			    || *setting->value == 0
			    || (endptr && *endptr != 0)
			    || attr_val_in_range(attr_id, val) == 0) {

				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				goto out;
			}

			dt->attr_val[attr_id] = val;
			iter = FALSE;
		}

		for (i = 0; iter && i < GT_GADGET_STRS_COUNT; i++) {
			if (streq(setting->variable, gadget_strs[i].name)) {
				dt->str_val[i] = setting->value;
				iter = FALSE;
				break;
			}
		}

		if (iter) {
			fprintf(stderr, "Unknown attribute passed: %s\n", setting->variable);
			goto out;
		}
	}

	executable_command_set(exec, GET_EXECUTABLE(create),
				(void *)dt, gt_gadget_create_destructor);

	return;
out:
	gt_setting_list_cleanup(attrs);
	gt_gadget_create_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_gadget_rm_help(void *data)
{
	printf("usage: %s rm [options] <name> \n"
	       "Remove gadget of specified name\n"
	       "\n"
	       "Options:\n"
	       "  -f, --force\tDisable gadget if it was enabled\n"
	       "  -r, --recursive\tRemove configs and functions recursively\n"
	       "  -h, --help\tPrint this help\n",
	       program_name);
	return -1;
}

static void gt_parse_gadget_rm(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_gadget_rm_data *dt;
	int ind;
	int avaible_opts = GT_FORCE | GT_RECURSIVE | GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind != 1)
		goto out;
	dt->name = argv[ind];
	executable_command_set(exec, GET_EXECUTABLE(rm), (void *)dt,
			free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_gadget_get_destructor(void *data)
{
	struct gt_gadget_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_get_data *)data;

	free(dt->attrs);
	free(dt);
}

static int gt_gadget_get_help(void *data)
{
	printf("Gadget get help.\n");
	return -1;
}

static void gt_parse_gadget_get(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void * data)
{
	struct gt_gadget_get_data *dt = NULL;
	int i;
	int ind;
	int avaible_opts = GT_HELP;

	if (argc == 0)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	dt->name = argv[ind++];
	dt->attrs = calloc(argc - ind + 1, sizeof(char *));
	if (dt->attrs == NULL)
		goto out;

	i = 0;
	while (argv[ind])
		dt->attrs[i++] = argv[ind++];

	executable_command_set(exec, GET_EXECUTABLE(get), (void *)dt,
			gt_gadget_get_destructor);

	return;
out:
	gt_gadget_get_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_gadget_set_destructor(void *data)
{
	struct gt_gadget_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_set_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_gadget_set_help(void *data)
{
	printf("Gadget set help.\n");
	return -1;
}

static void gt_parse_gadget_set(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void * data)
{
	struct gt_gadget_set_data *dt = NULL;
	int tmp;
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
	tmp = gt_parse_setting_list(&dt->attrs, argc - ind, argv + ind);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(set), (void *)dt,
			gt_gadget_set_destructor);
	return;
out:
	gt_gadget_set_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_gadget_enable_help(void *data)
{
	printf("usage: %s disable [options] [gadget] \n"
	       "Remove gadget of specified name\n"
	       "\n"
	       "Options:\n"
	       "  -u=<udc>, --udc=<udc>\tDisable gadget which is active at given udc\n"
	       "  -h, --help\tPrint this help\n",
	       program_name);

	return -1;
}

static void gt_parse_gadget_enable(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_gadget_enable_data *dt;
	int ind;
	int avaible_opts = GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	switch (argc - ind) {
	case 1:
		dt->gadget = argv[ind++];
		break;
	case 2:
		dt->gadget = argv[ind++];
		dt->udc = argv[ind++];
		break;
	default:
		goto out;
	}

	executable_command_set(exec, GET_EXECUTABLE(enable),
				(void *)dt, free);
	return;
out:
	free((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_gadget_disable_help(void *data)
{
	printf("usage: %s disable [options] [gadget] \n"
	       "Disable gadget. If gadget has been specified it is disabled, otherwise: if "
	       "only one gadget exist it is used, if more than one gadget exist including "
	       "default gadget, the default is disabled else error due to ambiguous gadget name "
	       "unless -u or --udc option was used."
	       "\n"
	       "Options:\n"
	       "  -u=<udc>, --udc=<udc>\tDisable gadget which is active at given udc\n"
	       "  -h, --help\tPrint this help\n",
	       program_name);

	return -1;
}

static void gt_parse_gadget_disable(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_gadget_disable_data *dt;
	int c;
	struct option opts[] = {
		{"udc", required_argument, 0, 'u'},
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
		c = getopt_long(argc, argv, "u:h", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 'u':
			dt->udc = optarg;
			break;
		case 'h':
			goto out;
			break;
		default:
			goto out;
		}
	}

	if (optind < argc - 1 || (dt->udc && optind < argc)) {
		printf("Too many arguments\n");
		goto out;
	}

	dt->gadget = argv[optind];
	executable_command_set(exec, GET_EXECUTABLE(disable), (void *)dt, free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_gadget_gadget_help(void *data)
{
	printf("Gadget gadget help.\n");
	return -1;
}

static void gt_parse_gadget_gadget(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_gadget_gadget_data *dt;
	int ind;
	int avaible_opts = GT_RECURSIVE | GT_VERBOSE | GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	switch (argc - ind) {
	case 0:
		break;
	case 1:
		dt->name = argv[ind];
		break;
	default:
		goto out;
	}

	executable_command_set(exec, GET_EXECUTABLE(gadget), (void *)dt,
		free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_gadget_load_help(void *data)
{
	printf("Gadget load help\n");
	return -1;
}

static void gt_parse_gadget_load(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int c;
	struct gt_gadget_load_data *dt;
	struct option opts[] = {
		{"off", no_argument, 0, 'o'},
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
		c = getopt_long(argc, argv, "oh", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 'o':
			dt->opts |= GT_OFF;
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

	if (optind == argc || optind < argc - 2)
		goto out;

	dt->name = argv[optind++];
	if (dt->opts & GT_STDIN || dt->file) {
		dt->gadget_name = dt->name;
		dt->name = NULL;
		if (optind < argc)
			goto out;
	}

	if (optind < argc)
		dt->gadget_name = argv[optind++];

	executable_command_set(exec, GET_EXECUTABLE(load), (void *)dt, free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_gadget_save_destructor(void *data)
{
	struct gt_gadget_save_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_save_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_gadget_save_help(void *data)
{
	printf("Gadget save help.\n");
	return -1;
}

static void gt_parse_gadget_save(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int c;
	struct gt_gadget_save_data *dt;
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
			if (dt->file || dt->path)
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

	if (optind == argc)
		goto out;

	dt->gadget = argv[optind++];

	if(optind < argc
	  && !dt->file
	  && !(dt->opts & GT_STDOUT)
	  && strchr(argv[optind], '=') == NULL)
		dt->name = argv[optind++];

	c = gt_parse_setting_list(&dt->attrs, argc - optind,
			argv + optind);
	if (c < 0)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(save), (void *)dt,
		gt_gadget_save_destructor);

	return;
out:
	gt_gadget_save_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_gadget_template_help(void *data)
{
	printf("Gadget template help.\n");
	return -1;
}

static void gt_parse_gadget_template(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int ind;
	struct gt_gadget_template_data *dt;
	int avaible_opts = GT_VERBOSE | GT_RECURSIVE | GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind > 1)
		goto out;

	if (argv[ind])
		dt->name = argv[ind];

	executable_command_set(exec, GET_EXECUTABLE(template_default), (void *)dt,
		free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_gadget_template_rm_help(void *data)
{
	printf("Gadget template rm help.\n");
	return -1;
}

static void gt_parse_gadget_template_rm(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_gadget_template_rm_data *dt = NULL;
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
		NULL);

	return;
out:
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_gadget_template_set_destructor(void *data)
{
	struct gt_gadget_template_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_template_set_data *)data;
	gt_setting_list_cleanup(dt->attr);
	free(dt);
}

static int gt_gadget_template_set_help(void *data)
{
	printf("Gadget template set help.\n");
	return -1;
}

static void gt_parse_gadget_template_set(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int tmp;
	struct gt_gadget_template_set_data *dt;
	int ind;
	int avaible_opts = GT_HELP;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0 || dt->opts & GT_HELP)
		goto out;

	if (argc - ind != 2)
		goto out;

	dt->name = argv[ind++];
	tmp = gt_parse_setting_list(&dt->attr, argc - ind, argv + ind);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(set), (void *)dt,
		gt_gadget_template_set_destructor);

	return;
out:
	gt_gadget_template_set_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static void gt_gadget_template_get_destructor(void *data)
{
	struct gt_gadget_template_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_template_get_data *)data;
	free(dt->attr);
	free(dt);
}

static int gt_gadget_template_get_help(void *data)
{
	printf("Gadget template get help.\n");
	return -1;
}

static void gt_parse_gadget_template_get(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int i;
	struct gt_gadget_template_get_data *dt;
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
	dt->attr = calloc(argc, sizeof(char *));
	if (dt->attr == NULL)
		goto out;

	i = 0;
	while (argv[ind])
		dt->attr[i++] = argv[ind++];

	executable_command_set(exec, GET_EXECUTABLE(get), (void *)dt,
		gt_gadget_template_get_destructor);

	return;
out:
	gt_gadget_template_get_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *get_gadget_template_children(const Command *cmd)
{
	static Command commands[] = {
		{"get", NEXT, gt_parse_gadget_template_get, NULL,
			gt_gadget_template_get_help},
		{"set", NEXT, gt_parse_gadget_template_set, NULL,
			gt_gadget_template_set_help},
		{"rm", NEXT, gt_parse_gadget_template_rm, NULL,
			gt_gadget_template_rm_help},
		{NULL, AGAIN, gt_parse_gadget_template, NULL,
			gt_gadget_template_help},
		CMD_LIST_END
	};

	return commands;
}

const Command *get_gadget_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_gadget_create, NULL,
			gt_gadget_create_help},
		{"rm", NEXT, gt_parse_gadget_rm, NULL, gt_gadget_rm_help},
		{"get", NEXT, gt_parse_gadget_get, NULL, gt_gadget_get_help},
		{"set", NEXT, gt_parse_gadget_set, NULL, gt_gadget_set_help},
		{"enable", NEXT, gt_parse_gadget_enable, NULL,
			gt_gadget_enable_help},
		{"disable", NEXT, gt_parse_gadget_disable, NULL,
			gt_gadget_disable_help},
		{"gadget", NEXT, gt_parse_gadget_gadget, NULL,
			gt_gadget_gadget_help},
		{"template", NEXT, command_parse, get_gadget_template_children,
			gt_gadget_template_help},
		{"load", NEXT, gt_parse_gadget_load, NULL,
			gt_gadget_load_help},
		{"save", NEXT, gt_parse_gadget_save, NULL,
			gt_gadget_save_help},
		CMD_LIST_END
	};

	return commands;
}

int gt_gadget_help(void *data)
{
	printf("Gadget help function\n");
	return -1;
}
