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

#ifndef __GADGET_TOOL_GADGET_GADGET_H__
#define __GADGET_TOOL_GADGET_GADGET_H__

#include "command.h"

/**
 * @brief Gets the commands possible for gadget
 * @param[in] cmd actual command
 * @return Pointer to table with all children of cmd
 * where the last element is invalid structure filled
 * with NULLs.
 */
const Command *get_gadget_children(const Command *cmd);

/**
 * @brief Help function which should be used if invalid
 * syntax for gadget was entered.
 *
 * @param[in] data additional data
 * @return -1 because invalid syntax has been provided
 */
int gt_gadget_help(void *data);

#endif //__GADGET_TOOL_GADGET_GADGET_H__
