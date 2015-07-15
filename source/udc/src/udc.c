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

#include "udc.h"
#include "backend.h"

#define GET_EXECUTABLE(func) \
	(backend_ctx.backend->udc->func ? \
	 backend_ctx.backend->udc->func : \
	 gt_udc_backend_not_implemented.func)

int udc_help_func(void *data)
{
	printf("UDC help func. Not implemented yet.\n");
	return -1;
}

void udc_parse(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void * data)
{
	if(argc == 0)
		// udc should be run without args
		executable_command_set(exec, GET_EXECUTABLE(udc), data, NULL);
	else
		// Wrong syntax for udc command, let's print help
		executable_command_set(exec, cmd->printHelp, data, NULL);
}
