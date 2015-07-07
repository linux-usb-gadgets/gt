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
#include "backend.h"

static int create_func(void *data)
{
	struct gt_func_create_data *dt;
	usbg_gadget *g;
	usbg_function_type f_type;
	usbg_function *f;
	int r;

	dt = (struct gt_func_create_data *)data;

	if (dt->attrs->variable) {
		/* TODO add support for attributes */
		printf("Attributes are not supported now\n");
		return -1;
	}


	f_type = usbg_lookup_function_type(dt->type);
	if (f_type < 0) {
		fprintf(stderr, "Unable to find function %s: %s\n",
			dt->type, usbg_strerror(f_type));
		return -1;
	}

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (!g) {
		fprintf(stderr, "Unable to find gadget %s\n",
			dt->gadget);
		return -1;
	}

	r = usbg_create_function(g, f_type, dt->name, NULL, &f);
	if (r < 0) {
		fprintf(stderr, "Unable to create function: %s\n",
			usbg_strerror(r));
		return -1;
	}

	return 0;
}

static int list_types_func(void *data)
{
	int i;
	struct gt_func_list_types_data *dt;

	dt = (struct gt_func_list_types_data *)data;

	if (!(dt->opts & GT_QUIET))
		printf("Functions known by library:\n");

	for (i = USBG_FUNCTION_TYPE_MIN; i < USBG_FUNCTION_TYPE_MAX; i++)
		printf("  %s\n", usbg_get_function_type_str(i));

	return 0;
}

struct gt_function_backend gt_function_backend_libusbg = {
	.create = create_func,
	.rm = NULL,
	.list_types = list_types_func,
	.get = NULL,
	.set = NULL,
	.func = NULL,
	.load = NULL,
	.save = NULL,
	.template_default = NULL,
	.template_get = NULL,
	.template_set = NULL,
	.template_rm = NULL,
};
