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

#include "command.h"

struct gt_ffs_backend {
};

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
