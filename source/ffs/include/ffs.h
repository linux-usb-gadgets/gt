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

#ifndef __GADGET_TOOL_FFS_FFS_H__
#define __GADGET_TOOL_FFS_FFS_H__

#include <stdbool.h>
#include <stdint.h>
#include "command.h"

struct gt_ffs_backend {
	int (*interface_create)(void *);
	int (*endpoint_create)(void *);
	int (*language_create)(void *);
	int (*string_create)(void *);
	int (*descriptors_load)(void *);
	int (*strings_load)(void *);
};

enum {
	LS = 0,
	FS,
	HS,
	SS
};

struct gt_ffs_interface_create_data {
	int opts;
	unsigned long int speed;
	unsigned long int number;
	unsigned long int eps;
	unsigned long int cls;
	unsigned long int subcls;
	unsigned long int protocol;
	unsigned long int str_idx;
	struct gt_ffs_descs_state *state;
	const char *state_file;
};

struct find_interface_attr {
	bool speed_found;
	bool number_found;
	bool eps_found;
	bool cls_found;
	bool subcls_found;
	bool protocol_found;
	bool str_idx_found;
};

enum {
	ISOC = 1,
	BULK,
	INTR,
};

struct gt_ffs_endpoint_create_data {
	int opts;
	unsigned long int speed;
	unsigned long int number;
	unsigned long int address;
	unsigned long int type;
	unsigned long int max;
	unsigned long int interval;
	struct gt_ffs_descs_state *state;
	const char *state_file;
};

struct gt_ffs_language_create_data {
	int opts;
	unsigned long int code;
	struct gt_ffs_strs_state *state;
	const char *state_file;
};

struct gt_ffs_string_create_data {
	int opts;
	unsigned long int lang;
	char *str;
	struct gt_ffs_strs_state *state;
	const char *state_file;
};

struct gt_ffs_descriptors_load_data {
	int opts;
	struct gt_ffs_descs_state *state;
	const char *state_file;
	const char *file;
	const char *path;
	const char *name;
};

struct gt_ffs_strings_load_data {
	int opts;
	struct gt_ffs_strs_state *state;
	const char *state_file;
	const char *file;
	const char *path;
	const char *name;
};

struct find_endpoint_attr {
	bool speed_found;
	bool number_found;
	bool address_found;
	bool type_found;
	bool max_found;
	bool interval_found;
};

unsigned long int *find_interface_attr(struct gt_ffs_interface_create_data *dt, const char *name, struct find_interface_attr *f);

unsigned long int *find_endpoint_attr(struct gt_ffs_endpoint_create_data *dt, const char *name, struct find_endpoint_attr *f);

/**
 * @brief Gets the next possible commands after config
 * @param[in] cmd actual command (should be config)
 * @return Pointer to table with all children of cmd
 * where the last element is invalid structure filled
 * with NULLs.
 */
const Command *gt_ffs_get_children(const Command *cmd);

/**
 * @brief Help function which should be used if invalid
 * syntax for ffs was entered.
 *
 * @param[in] data additional data
 * @return -1 because invalid syntax has been provided
 */
int gt_ffs_help(void *data);

extern struct gt_ffs_backend gt_ffs_backend_libusbg;
#ifdef WITH_GADGETD
extern struct gt_ffs_backend gt_ffs_backend_gadgetd;
#endif
extern struct gt_ffs_backend gt_ffs_backend_not_implemented;

#endif //__GADGET_TOOL_FFS_FFS_H__
