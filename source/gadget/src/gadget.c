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

#include "gadget.h"

const Command *get_gadget_children(const Command *cmd)
{
	static Command commands[] = {
			/*{"create", parse_gadget_create, NULL, gadget_create_help_func},
			{"rm", parse_gadget_rm, NULL, gadget_rm_help_func},
			{"get", parse_gadget_get, NULL, gadget_get_help_func},
			{"set", parse_gadget_set, NULL, gadget_set_help_func},
			{"enable", parse_gadget_enable, NULL, gadget_enable_help_func},
			{"disable", parse_gadget_disable, NULL, gadget_disable_help_func},
			{"gadget", parse_gadget_gadget, NULL, gadget_gadget_help_func},
			{"template", parse_gadget_template, NULL, gadget_template_help_func},
			{"load", parse_gadget_load, NULL, gadget_load_help_func},
			{"save", parse_gadget_save, NULL, gadget_load_help_func}*/
			{NULL, AGAIN, NULL, NULL, NULL}
	};
	return commands;
}

int gt_gadget_help(void *data)
{
	printf("Gadget help function\n");
	return -1;
}
