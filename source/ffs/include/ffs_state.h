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

#ifndef __GADGET_TOOL_FFS_FFS_STATE_H__
#define __GADGET_TOOL_FFS_FFS_STATE_H__

#include <stdbool.h>

#include "ffs.h"

struct gt_ffs_link {
	struct gt_ffs_link *next;
	uint8_t type; /* ch9.h: USB_DT_ENDPOINT, USB_DT_INTERFACE */
	void *data;
};

struct gt_ffs_lang {
	struct gt_ffs_link *strs;
	uint16_t code;
	uint16_t count;
};

struct gt_ffs_descs_state {
	struct gt_ffs_link *fs_descs, *hs_descs, *ss_descs;
	uint32_t flags;
	uint32_t fs_count, hs_count, ss_count;
	bool modified;
};

struct gt_ffs_strs_state {
	struct gt_ffs_link *langs;
	uint32_t str_count;
	uint32_t lang_count;
	bool modified;
};

struct gt_ffs_descs_state *gt_ffs_build_descs_state(const char *descs);
struct gt_ffs_strs_state *gt_ffs_build_strs_state(const char *strs);
void gt_ffs_cleanup_descs_state(struct gt_ffs_descs_state *state, const char *descs);
void gt_ffs_cleanup_strs_state(struct gt_ffs_strs_state *state, const char *strs);

#endif // __GADGET_TOOL_FFS_FFS_STATE_H__
