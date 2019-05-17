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

const Command *gt_ffs_get_children(const Command *cmd)
{
	static Command commands[] = {
		CMD_LIST_END
	};

	return commands;
}
