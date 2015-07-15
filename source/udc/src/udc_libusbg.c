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

#include "backend.h"
#include "udc.h"

static int udc_func(void *data)
{
	usbg_udc *u;
	const char *name;

	usbg_for_each_udc(u, backend_ctx.libusbg_state) {
		name = usbg_get_udc_name(u);
		if (name == NULL) {
			fprintf(stderr, "Error getting udc name\n");
			return -1;
		}

		puts(name);
	}

	return 0;
}

struct gt_udc_backend gt_udc_backend_libusbg = {
	.udc = udc_func,
};
