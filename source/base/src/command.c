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
 * @file command.c
 * @brief Declaration of methods related to Command data type.
 */

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "command.h"



static inline bool name_matches(const char *candidate, const char *pattern)
{
	// If pattern is NULL it suits only to NULL
	if (!pattern)
		return !candidate;
	// If candidate is NULL it suits to all strings
	return (candidate && (strcmp(candidate, pattern) == 0))
			|| (!candidate);
}

void command_parse(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	const Command *next = cmd->getChildren(cmd);
	bool found = false;

	while (next->parse) {
		if (name_matches(next->name, *argv)) {
			found = true;
			break;
		}
		++next;
	}

	if (found)
		// We've found a suitable child so let's go deeper
		next->parse(next, argc - next->distance, argv + next->distance,
			    exec, data);
	else
		// We haven't found any suitable child so let's print help
		executable_command_set(exec, cmd->printHelp, data, NULL);
}
