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

#ifndef __GADGET_TOOL_UDC_UDC_H__
#define __GADGET_TOOL_UDC_UDC_H__

#include "command.h"

struct gt_udc_backend {
	int (*udc)(void *);
};

/**
 * @brief Help function which should be used if invalid
 * syntax for udc was entered.
 *
 * @param[in] data additional data
 * @return -1 because invalid syntax has been provided
 */
int udc_help_func(void *data);

/**
 * @brief Gets the next possible commands after udc
 * @param[in] cmd actual command (should be udc)
 * @return Pointer to table with all children of cmd
 * where the last element is invalid structure filled
 * with NULLs.
 */
void udc_parse(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void * data);

extern struct gt_udc_backend gt_udc_backend_libusbg;
#ifdef WITH_GADGETD
extern struct gt_udc_backend gt_udc_backend_gadgetd;
#endif
extern struct gt_udc_backend gt_udc_backend_not_implemented;

#endif //__GADGET_TOOL_UDC_UDC_PARSE_H__
