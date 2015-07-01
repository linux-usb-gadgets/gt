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
#include <stdlib.h>
#include <string.h>

#include "configuration.h"
#include "common.h"
#include "parser.h"
#include "backend.h"

static int create_func(void *data)
{
	struct gt_config_create_data *dt;

	dt = (struct gt_config_create_data *)data;

	/* TODO implement -f option */
	int usbg_ret;
	usbg_gadget *g;
	usbg_config *c;

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (g == NULL) {
		fprintf(stderr, "Cant get gadget by name\n");
		return -1;
	}

	usbg_ret = usbg_create_config(g, dt->config_id, dt->config_label, NULL, NULL, &c);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr,"Error on config create\n");
		fprintf(stderr,"Error: %s: %s\n", usbg_error_name(usbg_ret),
			usbg_strerror(usbg_ret));
		return -1;
	}

	return 0;
}

static int add_func(void *data)
{
	struct gt_config_add_del_data *dt;

	int usbg_ret = USBG_SUCCESS;
	usbg_function_type f_type;
	usbg_gadget *g;
	usbg_function *f;
	usbg_config *c;
	int n;
	char buff[255];
	_cleanup_g_free_ gchar *func_name = NULL;
	const char *cfg_label = NULL;

	dt = (struct gt_config_add_del_data *)data;
	cfg_label = dt->config_label;

	/* func_name cant be NULL (libusbg requirement) */
	n = snprintf(buff, 255, "%s.%s", dt->type, dt->instance);
	if (n < 0)
		return -1;
	func_name = strdup(buff);

	f_type = usbg_lookup_function_type(dt->type);

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (g == NULL) {
		fprintf(stderr, "Unable to get gadget\n");
		return -1;
	}

	f = usbg_get_function(g, f_type, dt->instance);
	if (f == NULL) {
		fprintf(stderr, "Unable to get function\n");
		return -1;
	}

	if (strcmp(dt->config_label, "") == 0)
		cfg_label = NULL;

	c = usbg_get_config(g, dt->config_id, cfg_label);
	if (c == NULL) {
		fprintf(stderr, "Unable to get config\n");
		return -1;
	}

	usbg_ret = usbg_add_config_function(c, func_name, f);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Unable to attach function\n");
		fprintf(stderr,"Error: %s: %s\n", usbg_error_name(usbg_ret),
			usbg_strerror(usbg_ret));
		return -1;
	}

	return 0;
}

struct gt_config_backend gt_config_backend_libusbg = {
	.create = create_func,
	.add = add_func,
	.rm = NULL,
	.get = NULL,
	.set = NULL,
	.config = NULL,
	.del = NULL,
	.template_default = NULL,
	.template_get = NULL,
	.template_set = NULL,
	.template_rm = NULL,
	.load = NULL,
	.save = NULL,
};
