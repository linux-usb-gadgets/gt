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

#include <stdio.h>
#include <string.h>
#include <usbg/usbg.h>
#include <glib.h>
#include <gio/gio.h>

#include "backend.h"
#include "function.h"
#include "gadget.h"
#include "configuration.h"
#include "udc.h"

struct gt_backend_ctx backend_ctx = {
	.backend_type = GT_BACKEND_AUTO,
};

struct gt_backend gt_backend_libusbg = {
	.function = &gt_function_backend_libusbg,
	.gadget = &gt_gadget_backend_libusbg,
	.config = &gt_config_backend_libusbg,
	.udc = &gt_udc_backend_libusbg,
};

struct gt_backend gt_backend_gadgetd = {
	.function = &gt_function_backend_gadgetd,
	.gadget = &gt_gadget_backend_gadgetd,
	.config = &gt_config_backend_gadgetd,
	.udc = &gt_udc_backend_gadgetd,
};

int gt_backend_init(const char *program_name, enum gt_option_flags flags)
{
	enum gt_backend_type backend_type;
	GError *err = NULL;

	if (strcmp(program_name, "gt") == 0)
		backend_type = GT_BACKEND_LIBUSBG;
	else if (strcmp(program_name, "gadgetctl") == 0)
		backend_type = GT_BACKEND_GADGETD;
	else
		backend_type = GT_BACKEND_AUTO;

	if (backend_type == GT_BACKEND_GADGETD || backend_type == GT_BACKEND_AUTO) {
		GDBusConnection *conn;

		backend_ctx.backend = &gt_backend_gadgetd;

#if ! GLIB_CHECK_VERSION(2, 36, 0)
		g_type_init();
#endif

		conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
		if (err) {
			fprintf(stderr, "Unable to connect to d-bus: %s\n", err->message);
			goto out_gadgetd;
		}

		g_dbus_connection_call_sync(conn,
					    "org.usb.gadgetd",
					    "/",
					    "org.freedesktop.DBus.Peer",
					    "Ping",
					    NULL,
					    NULL,
					    G_DBUS_CALL_FLAGS_NONE,
					    -1,
					    NULL,
					    &err);
		if (err) {
			/* We omit showing glib-provided error message here
			 * as it's not really that useful for end-users.
			 *
			 * This message could be probably shown in verbose
			 * mode (which we don't have yet).
			 */
			fprintf(stderr, "Unable to initialize gadgetd backend_type\n");
			goto out_gadgetd;
		}

		backend_ctx.backend_type = GT_BACKEND_GADGETD;
		backend_ctx.gadgetd_conn = conn;
		return 0;

	}

out_gadgetd:
	if (err && backend_type == GT_BACKEND_GADGETD)
		return -1;

	if (backend_type == GT_BACKEND_LIBUSBG || backend_type == GT_BACKEND_AUTO) {
		usbg_state *s = NULL;
		int r;

		backend_ctx.backend = &gt_backend_libusbg;

		r = usbg_init("/sys/kernel/config", &s);
		if (r != USBG_SUCCESS) {
			fprintf(stderr, "Unable to initialize libusbg backend_type: %s\n", usbg_strerror(r));
			goto out_libusbg;
		}

		backend_ctx.backend_type = GT_BACKEND_LIBUSBG;
		backend_ctx.libusbg_state = s;
		return 0;
	}

out_libusbg:
	return -1;
}
