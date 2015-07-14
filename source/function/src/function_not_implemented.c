/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <usbg/usbg.h>

#include "function.h"
#include "common.h"
#include "parser.h"

static int create_func(void *data)
{
	struct gt_func_create_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_func_create_data *)data;
	printf("Func create called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, name=%s, force=%d",
			dt->gadget, dt->type, dt->name, !!(dt->opts & GT_FORCE));

	for (ptr = dt->attrs; ptr->variable; ptr++)
		printf(", %s=%s", ptr->variable, ptr->value);

	putchar('\n');

	return 0;
}

static int list_types_func(void *data)
{
	printf("Func list-typed called successfuly. Not implemented yet.\n");
	return 0;
}

static int rm_func(void *data)
{
	struct gt_func_rm_data *dt;

	dt = (struct gt_func_rm_data *)data;
	printf("Func rm called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, instance=%s, recursive=%d, force=%d\n",
		dt->gadget, usbg_get_function_type_str(dt->type), dt->instance,
		!!(dt->opts & GT_RECURSIVE),
		!!(dt->opts & GT_FORCE));

	return 0;
}

static int get_func(void *data)
{
	struct gt_func_get_data *dt;
	const char **ptr;

	dt = (struct gt_func_get_data *)data;
	printf("Func get called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, name=%s, attrs=",
		dt->gadget, dt->type, dt->name);

	for (ptr = dt->attrs; *ptr; ptr++)
		printf("%s, ", *ptr);
	putchar('\n');

	return 0;
}

static int set_func(void *data)
{
	struct gt_func_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_func_set_data *)data;
	printf("Func set called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, name=%s", dt->gadget, dt->type, dt->name);

	for (ptr = dt->attrs; ptr->variable; ptr++)
		printf(", %s=%s", ptr->variable, ptr->value);
	putchar('\n');

	return 0;
}

static int show_func(void *data)
{
	struct gt_func_show_data *dt;

	dt = (struct gt_func_show_data *)data;
	printf("Func func called successfully. Not implemented.\n");
	printf("gadget=%s", dt->gadget);

	if (dt->instance) {
		printf(", type=%s", usbg_get_function_type_str(dt->type));
		printf(", instance=%s", dt->instance);
	}

	printf(", verbose=%d\n", !!(dt->opts & GT_VERBOSE));

	return 0;
}

static int load_func(void *data)
{
	struct gt_func_load_data *dt;

	dt = (struct gt_func_load_data *)data;
	printf("Func load called succesfully. Not implemented.\n");
	if (dt->name)
		printf("name=%s, ", dt->name);
	if (dt->gadget)
		printf("gadget=%s, ", dt->gadget);
	if (dt->func)
		printf("func=%s, ", dt->func);
	if (dt->file)
		printf("file=%s, ", dt->file);
	if (dt->path)
		printf("path=%s, ", dt->path);
	printf("force=%d, stdin=%d\n", !!(dt->opts & GT_FORCE),
			!!(dt->opts & GT_STDIN));

	return 0;
}

static int save_func(void *data)
{
	struct gt_func_save_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_func_save_data *)data;

	printf("Func save called successfully. Not implemented.\n");
	if (dt->gadget)
		printf("gadget=%s, ", dt->gadget);
	if (dt->func)
		printf("func=%s, ", dt->func);
	if (dt->name)
		printf("name=%s, ", dt->name);
	if (dt->file)
		printf("file=%s, ", dt->file);
	if (dt->path)
		printf("path=%s, ", dt->path);
	printf("force=%d, stdout=%d", !!(dt->opts & GT_FORCE),
			!!(dt->opts & GT_STDOUT));

	for (ptr = dt->attrs; ptr->variable; ptr++)
		printf(", %s=%s", ptr->variable, ptr->value);
	putchar('\n');

	return 0;
}

static int template_func(void *data)
{
	struct gt_func_template_data *dt;

	dt = (struct gt_func_template_data *)data;
	printf("Func template called successfully. Not implemented.\n");
	if (dt->name)
		printf("name=%s, ", dt->name);
	printf("verbose=%d\n", !!(dt->opts & GT_VERBOSE));

	return 0;
}

static int template_get_func(void *data)
{
	struct gt_func_template_get_data *dt;
	const char **ptr;

	dt = (struct gt_func_template_get_data *)data;
	printf("Func template get called successfully. Not implemented.\n");
	printf("name=%s, attrs=", dt->name);
	for (ptr = dt->attrs; *ptr; ptr++)
		printf("%s, ", *ptr);
	putchar('\n');

	return 0;
}

static int template_set_func(void *data)
{
	struct gt_func_template_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_func_template_set_data *)data;
	printf("Func template set called successfully. Not implemented.\n");
	printf("name=%s", dt->name);

	for (ptr = dt->attrs; ptr->variable; ptr++)
		printf(", %s=%s", ptr->variable, ptr->value);
	putchar('\n');

	return 0;
}

static int template_rm_func(void *data)
{
	const char *dt;

	dt = (const char *)data;
	printf("Func template rm called successfully. Not implemented.\n");
	printf("name=%s\n", dt);

	return 0;
}

struct gt_function_backend gt_function_backend_not_implemented = {
	.create = create_func,
	.rm = rm_func,
	.list_types = list_types_func,
	.get = get_func,
	.set = set_func,
	.show = show_func,
	.load = load_func,
	.save = save_func,
	.template_default = template_func,
	.template_get = template_get_func,
	.template_set = template_set_func,
	.template_rm = template_rm_func,
	.create = create_func,
};
