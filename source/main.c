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
 * @file main.c
 * @brief Main file of gadget tool.
 * @details Gadget tool is a command line tool for gadget creation and
 * maintenance. Firstly gt needs to parse command line arguments to determine
 * what needs to be done. Then gt just executes a function related to action
 * specified by user.
 */

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <libconfig.h>

#include "common.h"
#include "parser.h"
#include "backend.h"
#include "executable_command.h"
#include "settings.h"

char *program_name;

static char *program_name_get(char *name, char **copy)
{
	char *p;

	/* POSIX version of basename(3) modifies its argument, thus the copy */
	p = strdup(name);
	if (!p)
		return name;
	*copy = p;

	return basename(p);
}

int main(int argc, char **argv)
{
	int ret;
	ExecutableCommand cmd;
	char *buf = NULL;
	config_t cfg;

	program_name = program_name_get(argv[0], &buf);
	ret = gt_backend_init(program_name, 0);
	if (ret < 0)
		goto out;

	config_init(&cfg);
	ret = gt_parse_settings(&cfg);
	if (ret < 0)
		goto out1;

	gt_parse_commands(argc, argv, &cmd);

	ret = executable_command_exec(&cmd);
	executable_command_clean(&cmd);

out1:
	config_destroy(&cfg);
out:
	free(buf);
	return ret;
}
