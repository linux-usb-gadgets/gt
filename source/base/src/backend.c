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

struct gt_backend_ctx backend_ctx = {
	.backend = GT_BACKEND_AUTO,
};

int gt_backend_init(const char *program_name, enum gt_option_flags flags)
{
	enum gt_backend_type backend;
	GError *err = NULL;

	if (strcmp(program_name, "gt") == 0)
		backend = GT_BACKEND_LIBUSBG;
	else if (strcmp(program_name, "gadgetctl") == 0)
		backend = GT_BACKEND_GADGETD;
	else
		backend = GT_BACKEND_AUTO;

	if (backend == GT_BACKEND_GADGETD || backend == GT_BACKEND_AUTO) {
		GDBusConnection *conn;

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
			fprintf(stderr, "Unable to initialize gadgetd backend\n");
			goto out_gadgetd;
		}

		backend_ctx.backend = GT_BACKEND_GADGETD;
		backend_ctx.gadgetd_conn = conn;
		return 0;

	}

out_gadgetd:
	if (err && backend == GT_BACKEND_GADGETD)
		return -1;

	if (backend == GT_BACKEND_LIBUSBG || backend == GT_BACKEND_AUTO) {
		usbg_state *s = NULL;
		int r;

		r = usbg_init("/sys/kernel/config", &s);
		if (r != USBG_SUCCESS) {
			fprintf(stderr, "Unable to initialize libusbg backend: %s\n", usbg_strerror(r));
			goto out_libusbg;
		}

		backend_ctx.backend = GT_BACKEND_LIBUSBG;
		backend_ctx.libusbg_state = s;
		return 0;
	}

out_libusbg:
	return -1;
}
