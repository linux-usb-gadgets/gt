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
 * @file executable_command.c
 * @brief Declaration of methods related to ExecutableCommand data type.
 */

#include <stdlib.h>

#include "executable_command.h"

void executable_command_set(ExecutableCommand *to_set, ExecutableFunc exec,
			    void *data, CleanupFunc destructor)
{
	if (to_set) {
		to_set->exec = exec;
		to_set->data = data;
		to_set->destructor = destructor;
	}
}

void executable_command_clean(ExecutableCommand *to_clean)
{
	if (to_clean) {
		if (to_clean->destructor) {
			to_clean->destructor(to_clean->data);
		}
		to_clean->exec = NULL;
		to_clean->data = NULL;
		to_clean->destructor = NULL;
	}
}

int executable_command_exec(ExecutableCommand *cmd)
{
	return (cmd && cmd->exec) ? cmd->exec(cmd->data) : -1;
}
