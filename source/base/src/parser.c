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
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>


int gt_global_help(void *data)
{
	printf("Usage: %s {OBJECT} [COMMAND]\n"
	       "Object is either implicit (if not specified) or explicit:\n"
	       "  udc\n"
	       "  settings\n"
	       "  config\n"
	       "  func\n"
	       "Implicit gadget commands:\n"
	       "  create\n"
	       "  rm\n"
	       "  get\n"
	       "  set\n"
	       "  enable\n"
	       "  disable\n"
	       "  template\n"
	       "  load\n"
	       "  save\n"
	       "Most commands recognize 'help' argument to display usage information, eg.\n"
	       "  create help\n"
	       "  func get help\n",
	       program_name);

	return 0;
}

static inline const Command *gt_get_command_root_children(const Command *cmd)
{
	static Command commands[] = {
		{ "udc", NEXT, udc_parse, NULL, udc_help_func },
		{ "settings", NEXT, command_parse, gt_settings_get_children, gt_settings_help },
		{ "config", NEXT, command_parse, gt_config_get_children, gt_config_help },
		{ "func", NEXT, command_parse, gt_func_get_children, gt_func_help },
		{ NULL, AGAIN, command_parse, get_gadget_children, gt_global_help },
		CMD_LIST_END
	};

	return commands;
}

static inline const Command *gt_get_command_root(const Command *cmd)
{
	static Command tool_names[] = {
		{ NULL, NEXT, command_parse, gt_get_command_root_children, gt_global_help },
		CMD_LIST_END
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
	gt_setting_list_cleanup(res);
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

int gt_get_options(int *optmask, int allowed_opts, int argc, char **argv)
{
	static struct {
		int flag;
		struct option option;
	} known_opts[] = {
		{GT_FORCE, {"force", no_argument, 0, 'f'}},
		{GT_VERBOSE, {"verbose", no_argument, 0, 'v'}},
		{GT_RECURSIVE, {"recursive", no_argument, 0, 'r'}},
		{GT_OFF, {"off", no_argument, 0, 'o'}},
		{GT_STDIN, {"stdin", no_argument, 0, 1}},
		{GT_STDIN, {"stdout", no_argument, 0, 2}},
		{GT_HELP, {"help", no_argument, 0, 'h'}},
		{GT_QUIET, {"quiet", no_argument, 0, 'q'}},
		{GT_INSTANCE, {"instance", no_argument, 0, 3}},
		{GT_TYPE, {"type", no_argument, 0, 4}},
		{GT_NAME, {"name", no_argument, 0, 5}},
		{GT_ID, {"id", no_argument, 0, 6}},
		{0, {NULL, 0, 0, 0}}
	};

	struct option opts[ARRAY_SIZE(known_opts)];
	char optstring[ARRAY_SIZE(known_opts)];
	int count = 0;
	int short_count = 0;
	int i;
	int c;

	for (i = 0; known_opts[i].flag; i++) {
		if (allowed_opts & known_opts[i].flag) {
			opts[count] = known_opts[i].option;
			if (isalnum(opts[count].val))
				optstring[short_count++] = opts[count].val;
			count++;
		}
	}

	/* Assign last element, which must be zero-filled */
	opts[count] = known_opts[i].option;
	optstring[short_count] = '\0';

	*optmask = 0;
	argc++;
	argv--;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, optstring, opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 'f':
			*optmask |= GT_FORCE;
			break;
		case 'r':
			*optmask |= GT_RECURSIVE;
			break;
		case 'v':
			*optmask |= GT_VERBOSE;
			break;
		case 'o':
			*optmask |= GT_OFF;
			break;
		case 1:
			*optmask |= GT_STDIN;
			break;
		case 2:
			*optmask |= GT_STDOUT;
			break;
		case 'h':
			*optmask |= GT_HELP;
			break;
		case 'q':
			*optmask |= GT_QUIET;
			break;
		case 3:
			*optmask |= GT_INSTANCE;
			break;
		case 4:
			*optmask |= GT_TYPE;
			break;
		case 5:
			*optmask |= GT_NAME;
			break;
		case 6:
			*optmask |= GT_ID;
			break;
		default:
			return -1;
		}
	}

	return optind-1;
}
