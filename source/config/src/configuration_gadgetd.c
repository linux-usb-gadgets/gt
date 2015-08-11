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

#include <stdio.h>

#include "configuration.h"
#include "common.h"
#include "parser.h"
#include "backend.h"

#include <errno.h>
#include <gio/gio.h>

static int create_func(void *data)
{
	struct gt_config_create_data *dt;

	dt = (struct gt_config_create_data *)data;

	/* TODO implement -f option */
	GVariant *gret;
	GError *error = NULL;
	_cleanup_g_free_ gchar *path = NULL;
	_cleanup_g_free_ gchar *out_config_path = NULL;

	gret = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					   "org.usb.gadgetd",
					   "/org/usb/Gadget",
					   "org.usb.device.GadgetManager",
					   "FindGadgetByName",
					   g_variant_new ("(s)",
							  dt->gadget),
					   NULL,
					   G_DBUS_CALL_FLAGS_NONE,
					   -1,
					   NULL,
					   &error);

	if (error) {
		fprintf(stderr, "Failed to get gadget path, %s\n", error->message);
		return -1;
	}

	g_variant_get(gret, "(o)", &path);
	g_variant_unref(gret);

	gret = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					   "org.usb.gadgetd",
					   path,
					   "org.usb.device.Gadget.ConfigManager",
					   "CreateConfig",
					   g_variant_new ("(is)",
							  dt->config_id,
							  dt->config_label),
					   NULL,
					   G_DBUS_CALL_FLAGS_NONE,
					   -1,
					   NULL,
					   &error);
	if (error) {
		fprintf(stderr, "Unknown error, %s\n", error->message);
		return -1;
	}
	g_variant_get(gret, "(o)", &out_config_path);
	g_variant_unref(gret);

	return 0;
}

static int add_func(void *data)
{
	struct gt_config_add_del_data *dt;

	dt = (struct gt_config_add_del_data *)data;

	_cleanup_g_free_ gchar *gpath = NULL;
	_cleanup_g_free_ gchar *fpath = NULL;
	_cleanup_g_free_ gchar *cpath = NULL;
	GVariant *gret;
	GError *error = NULL;
	gboolean function_added;

	gret = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					   "org.usb.gadgetd",
					   "/org/usb/Gadget",
					   "org.usb.device.GadgetManager",
					   "FindGadgetByName",
					   g_variant_new ("(s)",
							  dt->gadget),
					   NULL,
					   G_DBUS_CALL_FLAGS_NONE,
					   -1,
					   NULL,
					   &error);

	if (error) {
		fprintf(stderr, "Failed to find gadget, %s\n", error->message);
		return -1;
	}

	g_variant_get(gret, "(o)", &gpath);
	g_variant_unref(gret);

	gret = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					   "org.usb.gadgetd",
					   gpath,
					   "org.usb.device.Gadget.FunctionManager",
					   "FindFunctionByName",
					   g_variant_new ("(ss)",
							  dt->type,
							  dt->instance),
					   NULL,
					   G_DBUS_CALL_FLAGS_NONE,
					   -1,
					   NULL,
					   &error);

	if (error) {
		fprintf(stderr, "Failed to find function, %s\n", error->message);
		return -1;
	}

	g_variant_get(gret, "(o)", &fpath);
	g_variant_unref(gret);

	gret = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					   "org.usb.gadgetd",
					   gpath,
					   "org.usb.device.Gadget.ConfigManager",
					   "FindConfigByName",
					   g_variant_new ("(is)",
							  dt->config_id,
							  dt->config_label),
					   NULL,
					   G_DBUS_CALL_FLAGS_NONE,
					   -1,
					   NULL,
					   &error);

	if (error) {
		fprintf(stderr, "Failed to find config, %s\n", error->message);
		return -1;
	}

	g_variant_get(gret, "(o)", &cpath);
	g_variant_unref(gret);

	gret = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					   "org.usb.gadgetd",
					   cpath,
					   "org.usb.device.Gadget.Config",
					   "AttachFunction",
					   g_variant_new ("(o)", fpath),
					   NULL,
					   G_DBUS_CALL_FLAGS_NONE,
					   -1,
					   NULL,
					   &error);

	if (error) {
		fprintf(stderr,"Failed to attach function, %s\n", error->message);
		return -1;
	}

	g_variant_get(gret, "(b)", &function_added);
	g_variant_unref(gret);

	return 0;
}

struct gt_config_backend gt_config_backend_gadgetd = {
	.create = create_func,
	.add = add_func,
	.rm = NULL,
	.get = NULL,
	.set = NULL,
	.show = NULL,
	.del = NULL,
	.template_default = NULL,
	.template_get = NULL,
	.template_set = NULL,
	.template_rm = NULL,
	.load = NULL,
	.save = NULL,
};
