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

#include "configuration.h"
#include "common.h"
#include "parser.h"
#include "backend.h"

static int create_func(void *data)
{
	struct gt_config_create_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_config_create_data *)data;

	printf("Config create called successfully. Not implemented.\n");
	printf("gadget = %s, cfg_label = %s, cfg_id = %d, force = %d",
		dt->gadget, dt->config_label, dt->config_id, !!(dt->opts & GT_FORCE));

	ptr = dt->attrs;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');

	return 0;
}

static int rm_func(void *data)
{
	struct gt_config_rm_data *dt;

	dt = (struct gt_config_rm_data *)data;
	printf("Config rm called successfully. Not implemented.\n");
	printf("gadget = %s, config_name = %s, config_id = %d, force = %d, recursive = %d\n",
		dt->gadget, dt->config_label, dt->config_id, !!(dt->opts & GT_FORCE),
		!!(dt->opts & GT_RECURSIVE));

	return 0;
}

static int get_func(void *data)
{
	struct gt_config_get_data *dt;
	const char **ptr;

	dt = (struct gt_config_get_data *)data;
	printf("Config get called successfully. Not implemented.\n");
	printf("gadget = %s, config = %s, attrs = ",
		dt->gadget, dt->config);
	ptr = dt->attrs;
	while (*ptr) {
		printf("%s, ", *ptr);
		ptr++;
	}
	putchar('\n');
	return 0;
}

static int set_func(void *data)
{
	struct gt_config_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_config_set_data *)data;
	printf("Config set called successfully. Not implemented.\n");
	printf("gadget = %s, config = %s", dt->gadget, dt->config);
	ptr = dt->attrs;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int show_func(void *data)
{
	struct gt_config_show_data *dt;

	dt = (struct gt_config_show_data *)data;
	printf("Config config called successfully. Not implemented.\n");
	printf("gadget = %s", dt->gadget);
	if (dt->config_label)
		printf(", config_label = %s", dt->config_label);
	if (dt->config_id)
		printf(", config_id = %d", dt->config_id);
	printf(", verbose = %d, recursive = %d\n",
		!!(dt->opts & GT_VERBOSE), !!(dt->opts & GT_RECURSIVE));

	return 0;
}

static int del_func(void *data)
{
	struct gt_config_add_del_data *dt;

	dt = (struct gt_config_add_del_data *)data;
	printf("Config del called successfully. Not implemented.\n");
	printf("gadget = %s, cfg_label = %s, cfg_id = %d, type = %s, instance = %s\n",
			dt->gadget, dt->config_label, dt->config_id, dt->type, dt->instance);

	return 0;
}

static int add_func(void *data)
{
	struct gt_config_add_del_data *dt;

	dt = (struct gt_config_add_del_data *)data;
	printf("Config add called successfully. Not implemented.\n");
	printf("gadget = %s, cfg_label = %s, cfg_id = %d, type = %s, instance = %s\n",
			dt->gadget, dt->config_label, dt->config_id, dt->type, dt->instance);

	return 0;
}

static int template_func(void *data)
{
	struct gt_config_template_data *dt;

	dt = (struct gt_config_template_data *)data;
	printf("Config template called successfully. Not implemented.\n");
	if (dt->name)
		printf("name = %s, ", dt->name);
	printf("verbose = %d, recursive = %d\n",
		!!(dt->opts & GT_VERBOSE), !!(dt->opts & GT_RECURSIVE));
	return 0;
}

static int template_get_func(void *data)
{
	struct gt_config_template_get_data *dt;
	const char **ptr;

	dt = (struct gt_config_template_get_data *)data;
	printf("Config template get called successfully. Not implemented.\n");
	printf("name = %s, attr = ", dt->name);
	ptr = dt->attr;
	while (*ptr) {
		printf("%s, ", *ptr);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int template_set_func(void *data)
{
	struct gt_config_template_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_config_template_set_data *)data;
	printf("Config template set called successfully. Not implemened.\n");
	printf("name = %s", dt->name);
	ptr = dt->attr;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int template_rm_func(void *data)
{
	struct gt_config_template_rm_data *dt;

	dt = (struct gt_config_template_rm_data *)data;

	printf("Config template rm called successfully. Not implemented.\n");
	printf("name = %s\n", dt->name);
	return 0;
}

static int load_func(void *data)
{
	struct gt_config_load_data *dt;

	dt = (struct gt_config_load_data *)data;
	printf("Config load called successfully. Not implemented.\n");
	if (dt->name)
		printf("name = %s, ", dt->name);
	if (dt->gadget)
		printf("gadget = %s, ", dt->gadget);
	if (dt->config)
		printf("config = %s, ", dt->config);
	if (dt->file)
		printf("file = %s, ", dt->file);
	if (dt->path)
		printf("path = %s, ", dt->path);
	printf("recursive = %d, force = %d, stdin = %d\n",
		!!(dt->opts & GT_RECURSIVE), !!(dt->opts & GT_FORCE),
		!!(dt->opts & GT_STDIN));

	return 0;
}

static int save_func(void *data)
{
	struct gt_config_save_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_config_save_data *)data;
	printf("Config save called successfully. Not implemented.\n");
	if (dt->gadget)
		printf("gadget=%s, ", dt->gadget);
	if (dt->config)
		printf("config=%s, ", dt->config);
	if (dt->name)
		printf("name=%s, ", dt->name);
	if (dt->file)
		printf("file=%s, ", dt->file);
	if (dt->path)
		printf("path=%s, ", dt->path);
	printf("force=%d, stdout=%d",
		!!(dt->opts & GT_FORCE), !!(dt->opts & GT_STDOUT));

	ptr = dt->attrs;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');


	return 0;
}

struct gt_config_backend gt_config_backend_not_implemented = {
	.create = create_func,
	.rm = rm_func,
	.get = get_func,
	.set = set_func,
	.show= show_func,
	.del = del_func,
	.add = add_func,
	.template_default = template_func,
	.template_get = template_get_func,
	.template_set = template_set_func,
	.template_rm = template_rm_func,
	.load = load_func,
	.save = save_func,
};
