/*
 * Copyright (c) 2019 Collabora Ltd
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

#include "ffs.h"
#include "backend.h"

int gt_ffs_help(void *data)
{
	printf("FFS help func. Not implemented yet.\n");
	return -1;
}

static int gt_ffs_interface_help(void *data)
{
	printf("FFS interface help func. Not implemented yet.\n");

	return -1;
}

static int gt_ffs_endpoint_help(void *data)
{
	printf("FFS endpoint help func. Not implemented yet.\n");

	return -1;
}

static int gt_ffs_language_help(void *data)
{
	printf("FFS language help func. Not implemented yet.\n");

	return -1;
}

static int gt_ffs_string_help(void *data)
{
	printf("FFS string help func. Not implemented yet.\n");

	return -1;
}

static int gt_ffs_interface_create_help(void *data)
{
	printf("FFS interface create help func. Not implemented yet.\n");

	return -1;
}

const Command *gt_ffs_interface_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, NULL, NULL, gt_ffs_interface_create_help},
		CMD_LIST_END
	};

	return commands;
}

static int gt_ffs_endpoint_create_help(void *data)
{
	printf("FFS endpoint create help func. Not implemented yet.\n");

	return -1;
}

const Command *gt_ffs_endpoint_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, NULL, NULL, gt_ffs_endpoint_create_help},
		CMD_LIST_END
	};

	return commands;
}

static int gt_ffs_language_create_help(void *data)
{
	printf("FFS language create help func. Not implemented yet.\n");

	return -1;
}

const Command *gt_ffs_language_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, NULL, NULL, gt_ffs_language_create_help},
		CMD_LIST_END
	};

	return commands;
}

static int gt_ffs_string_create_help(void *data)
{
	printf("FFS string create help func. Not implemented yet.\n");

	return -1;
}

const Command *gt_ffs_string_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, NULL, NULL, gt_ffs_string_create_help},
		CMD_LIST_END
	};

	return commands;
}

const Command *gt_ffs_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"interface", NEXT, command_parse, gt_ffs_interface_get_children, gt_ffs_interface_help},
		{"endpoint", NEXT, command_parse, gt_ffs_endpoint_get_children, gt_ffs_endpoint_help},
		{"language", NEXT, command_parse, gt_ffs_language_get_children, gt_ffs_language_help},
		{"string", NEXT, command_parse, gt_ffs_string_get_children, gt_ffs_string_help},
		CMD_LIST_END
	};

	return commands;
}
