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
#include <usbg/usbg.h>

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
#ifdef WITH_GADGETD
	_cleanup_g_free_ gchar *func_name = NULL;
#else
	_cleanup_free_ char *func_name = NULL;
#endif
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

static int del_func(void *data)
{
	int usbg_ret = USBG_SUCCESS;
	usbg_binding *b, *found = NULL;
	usbg_config *c;
	usbg_gadget *g;
	usbg_function *f;
	struct gt_config_add_del_data *dt;

	dt = (struct gt_config_add_del_data *)data;

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (g == NULL) {
		fprintf(stderr, "Unable to get gadget\n");
		return -1;
	}

	c = usbg_get_config(g, dt->config_id, dt->config_label);
	if (c == NULL) {
		fprintf(stderr, "Unable to get config\n");
		return -1;
	}

	usbg_for_each_binding(b, c) {
		f = usbg_get_binding_target(b);
		if (f == NULL) {
			fprintf(stderr, "Unable to get binding target\n");
			return -1;
		}

		if (strcmp(usbg_get_function_type_str(usbg_get_function_type(f)), dt->type) == 0 &&
			strcmp(usbg_get_function_instance(f), dt->instance) == 0) {
				found = b;
				break;
		}
	}

	if (found) {
		usbg_ret = usbg_rm_binding(found);
		if (usbg_ret != USBG_SUCCESS) {
			fprintf(stderr, "Error removing binding\n");
			return -1;
		}
	} else {
		fprintf(stderr, "Function not found in given config\n");
	}

	return 0;
}

int gt_print_config_libusbg(usbg_config *c, int opts)
{
	usbg_binding *b;
	usbg_function *f;
	const char *label, *instance, *bname;
	usbg_function_type type;
	struct usbg_config_attrs c_attrs;
	struct usbg_config_strs c_strs;
	int usbg_ret, id;

	label = usbg_get_config_label(c);
	if (!label) {
		fprintf(stderr, "Unable to get config name\n");
		return -1;
	}

	id = usbg_get_config_id(c);

	if (opts & GT_NAME)
		printf("  %s\n", label);
	else if (opts & GT_ID)
		printf("  %d\n", id);
	else
		printf("  Configuration: '%s' ID: %d\n", label, id);

	if (opts & GT_VERBOSE) {
		usbg_ret = usbg_get_config_attrs(c, &c_attrs);
		if (usbg_ret != USBG_SUCCESS) {
			fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
					usbg_strerror(usbg_ret));
			return -1;
		}

		printf("    MaxPower\t\t%d\n", c_attrs.bMaxPower);
		printf("    bmAttributes\t0x%02x\n", c_attrs.bmAttributes);

		usbg_ret = usbg_get_config_strs(c, LANG_US_ENG, &c_strs);
		if (usbg_ret == USBG_ERROR_NOT_FOUND) {
			fprintf(stderr, "Configuration strings not found\n");
		} else if (usbg_ret != USBG_SUCCESS) {
			fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
					usbg_strerror(usbg_ret));
			return -1;
		}

		printf("    configuration\t%s\n", c_strs.configuration);
	}

	if (opts & GT_RECURSIVE) {
		usbg_for_each_binding(b, c) {
			bname = usbg_get_binding_name(b);
			f = usbg_get_binding_target(b);
			instance = usbg_get_function_instance(f);
			type = usbg_get_function_type(f);
			if (!bname || !instance) {
				fprintf(stderr, "Unable to get binding details\n");
				return -1;
			}
			printf("    %s -> %s %s\n", bname,
					usbg_get_function_type_str(type), instance);
		}
	}

	return 0;
}

static int show_func(void *data)
{
	struct gt_config_show_data *dt;
	usbg_gadget *g;
	usbg_config *c;

	dt = (struct gt_config_show_data *)data;

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (g == NULL) {
		fprintf(stderr, "Unable to find gadget %s\n",
			dt->gadget);
		return -1;
	}

	if (dt->config_id > 0) {
		c = usbg_get_config(g, dt->config_id, dt->config_label);
		if (c == NULL) {
			fprintf(stderr, "Unable to find config: %s.%d\n",
					dt->config_label,
					dt->config_id);
			return -1;
		}

		gt_print_config_libusbg(c, dt->opts);
	} else if (dt->config_label) {
		usbg_for_each_config(c, g) {
			if (strcmp(dt->config_label, usbg_get_config_label(c)) == 0)
				gt_print_config_libusbg(c, dt->opts);
		}
	} else {
		usbg_for_each_config(c, g) {
			gt_print_config_libusbg(c, dt->opts);
		}
	}

	return 0;

}

static int rm_func(void *data)
{
	struct gt_config_rm_data *dt;
	usbg_gadget *g;
	usbg_config *c;
	usbg_udc *u;
	int ret, opts = 0;

	dt = (struct gt_config_rm_data *)data;

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (g == NULL) {
		fprintf(stderr, "Unable to find gadget %s\n",
			dt->gadget);
		return -1;
	}

	c = usbg_get_config(g, dt->config_id, dt->config_label);
	if (c == NULL) {
		fprintf(stderr, "Unable to find config: %s.%d\n",
				dt->config_label,
				dt->config_id);
		return -1;
	}

	if (dt->opts & GT_RECURSIVE)
		opts |= USBG_RM_RECURSE;

	if (dt->opts & GT_FORCE) {
		u = usbg_get_gadget_udc(g);
		if (u != NULL) {
			ret = usbg_disable_gadget(g);
			if (ret < 0) {
				fprintf(stderr, "Error disabling gadget: %s\n",
						usbg_strerror(ret));
				return -1;
			}
		}
	}

	ret = usbg_rm_config(c, opts);
	if (ret < 0) {
		fprintf(stderr, "Error removing config: %s\n",
				usbg_strerror(ret));
		return -1;
	}

	return ret;
}

struct gt_config_backend gt_config_backend_libusbg = {
	.create = create_func,
	.add = add_func,
	.rm = rm_func,
	.get = NULL,
	.set = NULL,
	.show = show_func,
	.del = del_func,
	.template_default = NULL,
	.template_get = NULL,
	.template_set = NULL,
	.template_rm = NULL,
	.load = NULL,
	.save = NULL,
};
