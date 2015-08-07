
/*
 * Copyright (c) 2012-2015 Samsung Electronics Co., Ltd.
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

#include "gadget.h"
#include "backend.h"
#include "common.h"

static int create_func(void *data)
{
	struct gt_gadget_create_data *dt;
	int i;

	dt = (struct gt_gadget_create_data *)data;
	printf("Gadget rm called successfully. Not implemented.\n");
	printf("name = %s, force = %d", dt->name, !!(dt->opts & GT_FORCE));

	for (i = 0; i < ARRAY_SIZE(dt->attr_val); ++i) {
		if (dt->attr_val[i] >= 0) {
			printf(", %s = %d", usbg_get_gadget_attr_str(i),
					dt->attr_val[i]);
		}
	}

	for (i = 0; i < ARRAY_SIZE(dt->str_val); ++i) {
		if (dt->str_val[i] != NULL) {
			printf(", %s = %s", gadget_strs[i].name,
					dt->str_val[i]);
		}
	}

	putchar('\n');

	return 0;
}

static int rm_func(void *data)
{
	struct gt_gadget_rm_data *dt;

	dt = (struct gt_gadget_rm_data *)data;
	printf("Gadget rm called successfully. Not implemented.\n");
	printf("name = %s, force = %d, recursive = %d\n",
		dt->name, !!(dt->opts & GT_FORCE), !!(dt->opts & GT_RECURSIVE));
	return 0;
}

static int get_func(void *data)
{
	struct gt_gadget_get_data *dt;
	int i;

	dt = (struct gt_gadget_get_data *)data;
	printf("Gadget get called successfully. Not implemented yet.\n");
	printf("name = %s, attrs = ", dt->name);

	for (i = 0; i < ARRAY_SIZE(dt->attrs); ++i)
		if (dt->attrs[i] > 0)
			printf("%s, ", usbg_get_gadget_attr_str(i));

	putchar('\n');
	return 0;
}

static int set_func(void *data)
{
	struct gt_gadget_set_data *dt;
	int i;

	dt = (struct gt_gadget_set_data *)data;
	printf("Gadget set called successfully. Not implemented.\n");
	printf("name = %s", dt->name);

	for (i = 0; i < ARRAY_SIZE(dt->attr_val); ++i) {
		if (dt->attr_val[i] >= 0) {
			printf(", %s = %d", usbg_get_gadget_attr_str(i),
					dt->attr_val[i]);
		}
	}

	for (i = 0; i < ARRAY_SIZE(dt->str_val); ++i) {
		if (dt->str_val[i] != NULL) {
			printf(", %s = %s", gadget_strs[i].name,
					dt->str_val[i]);
		}
	}

	putchar('\n');
	return 0;
}

static int enable_func(void *data) {
	struct gt_gadget_enable_data *dt;

	dt = (struct gt_gadget_enable_data *)data;
	printf("Gadget enable called successfully. Not implemented.\n");

	if (dt->gadget)
		printf("gadget = %s, ", dt->gadget);

	if (dt->udc)
		printf("udc = %s", dt->udc);

	putchar('\n');
	return 0;
}

static int disable_func(void *data)
{
	struct gt_gadget_disable_data *dt;

	dt = (struct gt_gadget_disable_data *)data;
	printf("Gadget disable called successfully. Not implemented.\n");
	if (dt->gadget)
		printf("gadget = %s, ", dt->gadget);
	if (dt->udc)
		printf("udc = %s", dt->udc);
	putchar('\n');
	return 0;
}

static int gadget_func(void *data)
{
	struct gt_gadget_gadget_data *dt;

	dt = (struct gt_gadget_gadget_data *)data;
	printf("Gadget gadget called successfully. Not implemented.\n");
	if (dt->name)
		printf("name = %s, ", dt->name);
	printf("recursive = %d, verbose = %d\n",
		!!(dt->opts & GT_RECURSIVE), !!(dt->opts & GT_VERBOSE));
	return 0;
}

static int load_func(void *data)
{
	struct gt_gadget_load_data *dt;

	dt = (struct gt_gadget_load_data *)data;
	printf("Gadget load called successfully. Not implemented.\n");
	if (dt->name)
		printf("name = %s, ", dt->name);
	if (dt->gadget_name)
		printf("gadget = %s, ", dt->gadget_name);
	if (dt->file)
		printf("file %s, ", dt->file);
	if (dt->path)
		printf("path = %s, ", dt->path);

	printf("off = %d, stdin = %d\n",
		!!(dt->opts & GT_OFF), !!(dt->opts & GT_STDIN));

	return 0;
}

static int save_func(void *data)
{
	struct gt_gadget_save_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_gadget_save_data *)data;
	printf("Gadget save called successfully. Not implemented\n");
	if (dt->gadget)
		printf("gadget = %s, ", dt->gadget);
	if (dt->name)
		printf("name = %s, ", dt->name);
	if (dt->file)
		printf("file = %s, ", dt->file);
	if (dt->path)
		printf("path = %s, ", dt->path);
	printf("force = %d, stdout = %d",
		!!(dt->opts & GT_FORCE), !!(dt->opts & GT_STDOUT));

	ptr = dt->attrs;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int template_func(void *data)
{
	struct gt_gadget_template_data *dt;

	dt = (struct gt_gadget_template_data *)data;
	printf("Gadget template called successfully. Not implemented.\n");
	if (dt->name)
		printf("name = %s, ", dt->name);
	printf("verbose = %d, recursive = %d\n",
		!!(dt->opts & GT_VERBOSE), !!(dt->opts & GT_RECURSIVE));
	return 0;
}

static int template_rm_func(void *data)
{
	struct gt_gadget_template_rm_data *dt;

	dt = (struct gt_gadget_template_rm_data *)data;
	printf("Gadget template rm called successfully. Not implemented.\n");
	printf("name = %s\n", dt->name);
	return 0;
}

static int template_set_func(void *data)
{
	struct gt_gadget_template_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_gadget_template_set_data *)data;
	printf("Gadget template set called successfully. Not implemened.\n");
	printf("name = %s", dt->name);
	ptr = dt->attr;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int template_get_func(void *data)
{
	struct gt_gadget_template_get_data *dt;
	const char **ptr;

	dt = (struct gt_gadget_template_get_data *)data;
	printf("Gadget template get called successfully. Not implemented.\n");
	printf("name = %s, attr = ", dt->name);
	ptr = dt->attr;
	while (*ptr) {
		printf("%s, ", *ptr);
		ptr++;
	}

	putchar('\n');
	return 0;
}


struct gt_gadget_backend gt_gadget_backend_not_implemented = {
	.create = create_func,
	.rm = rm_func,
	.get = get_func,
	.set = set_func,
	.enable = enable_func,
	.disable = disable_func,
	.gadget = gadget_func,
	.load = load_func,
	.save = save_func,
	.template_default = template_func,
	.template_rm = template_rm_func,
	.template_set = template_set_func,
	.template_get = template_get_func,
};
