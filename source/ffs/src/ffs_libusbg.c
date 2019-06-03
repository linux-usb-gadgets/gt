/*
 * Copyright (c) 2019 Collabora Ltd
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
#include <string.h>
#include <linux/usb/ch9.h>
#include <usbg/usbg.h>

#include "common.h"
#include "backend.h"
#include "ffs.h"
#include "ffs_state.h"

static int interface_create_func(void *data)
{
	struct gt_ffs_descs_state *state;
	struct gt_ffs_interface_create_data *dt, *dt_new;
	struct gt_ffs_link *item, **ptr;
	uint32_t *count;

	dt = (struct gt_ffs_interface_create_data *)data;
	state = dt->state;
	if (dt->speed == FS) {
		ptr = &state->fs_descs;
		count = &state->fs_count;
	} else if (dt->speed == HS) {
		ptr = &state->hs_descs;
		count = &state->hs_count;
	} else if (dt->speed == SS) {
		ptr = &state->ss_descs;
		count = &state->ss_count;
	} else
		return -1;

	while (*ptr) {
		if ((*ptr)->type == USB_DT_INTERFACE) {
			struct gt_ffs_interface_create_data *d = (*ptr)->data;
			if (d->number == dt->number) {
				fprintf(stderr, "Duplicate interface detected!\n");
				return -1;
			}
		}
		ptr = &(*ptr)->next;
	}

	item = zalloc(sizeof(*item));
	if (item == NULL)
		return -1;

	dt_new = zalloc(sizeof(*dt_new));
	if (dt_new == NULL)
		goto out_data;

	memcpy(dt_new, dt, sizeof(*dt_new));
	item->data = dt_new;
	item->type = USB_DT_INTERFACE;

	*ptr = item;
	++(*count);
	state->modified = true;

	return 0;

out_data:
	free(item);
	return -1;
}

struct gt_ffs_backend gt_ffs_backend_libusbg = {
	.interface_create = interface_create_func,
};
