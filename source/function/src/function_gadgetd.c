/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <glib.h>
#include <gio/gio.h>

#include "parser.h"
#include "backend.h"
#include "function.h"

static int create_func(void *data)
{
	struct gt_func_create_data *dt;
	gchar *gadget_objpath = NULL;
	GError *err = NULL;
	GVariant *v;

	dt = (struct gt_func_create_data *)data;

	if (dt->attrs->variable) {
		/* TODO add support for attributes */
		printf("Attributes are not supported now\n");
		return -1;
	}

	v = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					"org.usb.gadgetd",
					"/org/usb/Gadget",
					"org.usb.device.GadgetManager",
					"FindGadgetByName",
					g_variant_new("(s)", dt->gadget),
					NULL,
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&err);
	if (err) {
		fprintf(stderr, "Unable to find gadget %s: %s\n", dt->gadget, err->message);
		return -1;
	}

	g_variant_get(v, "(o)", &gadget_objpath);
	g_variant_unref(v);

	v = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					"org.usb.gadgetd",
					gadget_objpath,
					"org.usb.device.Gadget.FunctionManager",
					"CreateFunction",
					g_variant_new("(ss)", dt->name, dt->type),
					NULL,
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&err);
	if (err) {
		fprintf(stderr, "Unable to create function: %s\n", err->message);
		g_free(gadget_objpath);
		return -1;
	}

	g_free(gadget_objpath);
	g_variant_unref(v);

	return 0;
}

static int list_types_func(void *data)
{
	GError *err = NULL;
	GVariantIter *iter;
	GVariant *v;
	gchar *s;
	struct gt_func_list_types_data *dt;

	dt = (struct gt_func_list_types_data *)data;

	v = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					"org.usb.gadgetd",
					"/org/usb/Gadget",
					"org.usb.device.GadgetManager",
					"ListAvailableFunctions",
					NULL,
					NULL,
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&err);

	if (err) {
		fprintf(stderr, "Unable to get function list: %s\n", err->message);
		return -1;
	}

	if (!(dt->opts & GT_QUIET))
		printf("Discovered functions:\n");

	g_variant_get(v, "(as)", &iter);
	while (g_variant_iter_loop(iter, "s", &s))
	       printf("  %s\n", s);

	g_variant_iter_free(iter);
	g_variant_unref(v);

	return 0;
}

struct gt_function_backend gt_function_backend_gadgetd = {
	.create = create_func,
	.rm = NULL,
	.list_types = list_types_func,
	.get = NULL,
	.set = NULL,
	.show = NULL,
	.load = NULL,
	.save = NULL,
	.template_default = NULL,
	.template_get = NULL,
	.template_set = NULL,
	.template_rm = NULL,
};
