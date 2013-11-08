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

#include "settings.h"

int gt_settings_help(void *data)
{
	printf("Settings help function\n");
	return -1;
}

const Command *gt_settings_get_children(const Command *cmd)
{
	static Command commands[] = {
		//	{"get", NEXT, settings_parse_get, get_gt_root_children, global_help_func},
		//	{"set", NEXT, settings_parse_set, get_gt_root_children, global_help_func},
		//	{"append", NEXT, settings_parse_append, get_gt_root_children, global_help_func},
		//	{"detach", NEXT, settings_parse_detach, get_gt_root_children, global_help_func},
			{NULL, AGAIN, NULL, NULL, NULL}
	};
	return commands;
}
