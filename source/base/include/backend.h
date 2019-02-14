/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

/**
 * @file backend.h
 * @brief Declaration of methods related to different backends available
 */

#ifndef __GADGET_TOOL_BACKEND_H__
#define __GADGET_TOOL_BACKEND_H__

#include <usbg/usbg.h>
#ifdef WITH_GADGETD
#include <gio/gio.h>
#endif
#include "parser.h"

/**
 * @brief Initialize global backend type according to supplied program name and opts.
 * @param[in] program_name program invocation name
 * @param[in] flags program flags
 */
int gt_backend_init(const char *program_name, enum gt_option_flags flags);

/**
 * Backend type
 */
enum gt_backend_type {
	GT_BACKEND_AUTO = 0,
#ifdef WITH_GADGETD
	GT_BACKEND_GADGETD,
#endif
	GT_BACKEND_LIBUSBG,
	GT_BACKEND_NOT_IMPLEMENTED,
};

struct gt_backend {
	struct gt_function_backend *function;
	struct gt_gadget_backend *gadget;
	struct gt_config_backend *config;
	struct gt_udc_backend *udc;
};

struct gt_backend_ctx {
	enum gt_backend_type backend_type;

	struct gt_backend *backend;

	union {
		usbg_state *libusbg_state;
#ifdef WITH_GADGETD
		GDBusConnection *gadgetd_conn;
#endif
	};
};

extern struct gt_backend gt_backend_libusbg;
#ifdef WITH_GADGETD
extern struct gt_backend gt_backend_gadgetd;
#endif
extern struct gt_backend gt_backend_not_implemented;

extern struct gt_backend_ctx backend_ctx;

#endif /* __GADGET_TOOL_BACKEND_H__ */
