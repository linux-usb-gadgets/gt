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

#include <usbg/usbg.h>
#include <stdio.h>

#include "gadget.h"
#include "backend.h"
#include "common.h"

static int create_func(void *data)
{
	struct gt_gadget_create_data *dt;
	int i;
	int r;

	usbg_gadget *g;

	dt = (struct gt_gadget_create_data *)data;

	r = usbg_create_gadget(backend_ctx.libusbg_state,
			       dt->name,
			       NULL,
			       NULL,
			       &g);
	if (r != USBG_SUCCESS) {
		fprintf(stderr, "Unable to create gadget %s: %s\n",
			dt->name, usbg_strerror(r));
		return -1;
	}

	for (i = USBG_GADGET_ATTR_MIN; i < USBG_GADGET_ATTR_MAX; i++) {
		if (dt->attr_val[i] == -1)
			continue;

		r = usbg_set_gadget_attr(g, i, dt->attr_val[i]);
		if (r != USBG_SUCCESS) {
			fprintf(stderr, "Unable to set attribute %s: %s\n",
				usbg_get_gadget_attr_str(i),
				usbg_strerror(r));
			goto err_usbg;
		}
	}

	for (i = 0; i < G_N_ELEMENTS(dt->str_val); i++) {
		if (dt->str_val[i] == NULL)
			continue;

		r = gadget_strs[i].set_fn(g, LANG_US_ENG, dt->str_val[i]);
		if (r != USBG_SUCCESS) {
			fprintf(stderr, "Unable to set string %s: %s\n",
				gadget_strs[i].name,
				usbg_strerror(r));
			goto err_usbg;
		}
	}

	return 0;

err_usbg:
	usbg_rm_gadget(g, USBG_RM_RECURSE);
	return -1;
}

static int enable_func(void *data)
{
	struct gt_gadget_enable_data *dt;

	dt = (struct gt_gadget_enable_data *)data;

	usbg_gadget *g;
	usbg_udc *udc = NULL;
	int usbg_ret;

	if (dt->udc != NULL) {
		udc = usbg_get_udc(backend_ctx.libusbg_state, dt->udc);
		if (udc == NULL) {
			fprintf(stderr, "Failed to get udc\n");
			return -1;
		}
	}

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (g == NULL) {
		fprintf(stderr, "Failed to get gadget\n");
		return -1;
	}

	usbg_ret = usbg_enable_gadget(g, udc);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Failed to enable gadget %s\n", usbg_strerror(usbg_ret));
		return -1;
	}

	return 0;
}

struct gt_gadget_backend gt_gadget_backend_libusbg = {
	.create = create_func,
	.rm = NULL,
	.get = NULL,
	.set = NULL,
	.enable = enable_func,
	.disable = NULL,
	.gadget = NULL,
	.load = NULL,
	.save = NULL,
	.template_default = NULL,
	.template_get = NULL,
	.template_set = NULL,
	.template_rm = NULL,
};
