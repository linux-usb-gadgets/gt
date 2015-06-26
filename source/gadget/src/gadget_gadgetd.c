/*
 * Copyright (c) 2012-2015 Samsung Electronics Co., Ltd.
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
#include <usbg/usbg.h>

#include "gadget.h"
#include "backend.h"
#include "common.h"

char *attr_type_get(usbg_gadget_attr a);

static int create_func(void *data)
{
	struct gt_gadget_create_data *dt;
	int i;

	GVariantBuilder *b;
	GVariant *gattrs;
	GVariant *gstrs;
	GVariant *v;
	GError *err = NULL;

	dt = (struct gt_gadget_create_data *)data;

	b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
	for (i = USBG_GADGET_ATTR_MIN; i < USBG_GADGET_ATTR_MAX; i++) {
		if (dt->attr_val[i] == -1)
			continue;

		g_variant_builder_add(b, "{sv}",
				      usbg_get_gadget_attr_str(i),
				      g_variant_new(attr_type_get(i), dt->attr_val[i]));
	}
	gattrs = g_variant_builder_end(b);

	b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
	for (i = 0; i < GT_GADGET_STRS_COUNT; i++) {
		if (dt->str_val[i] == NULL)
			continue;

		g_variant_builder_add(b, "{sv}",
				      gadget_strs[i].name,
				      g_variant_new("s", dt->str_val[i]));
	}
	gstrs = g_variant_builder_end(b);

	v = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					"org.usb.gadgetd",
					"/org/usb/Gadget",
					"org.usb.device.GadgetManager",
					"CreateGadget",
					g_variant_new("(s@a{sv}@a{sv})",
						      dt->name,
						      gattrs,
						      gstrs),
					NULL,
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&err);

	if (err) {
		fprintf(stderr, "Unable to create gadget: %s\n", err->message);
		return -1;
	}

	g_variant_unref(v);
	return 0;
}

static int enable_func(void *data)
{
	struct gt_gadget_enable_data *dt;

	dt = (struct gt_gadget_enable_data *)data;

	/* TODO add support for enabling well known UDC */
	GVariant *gret;
	GError *error = NULL;
	GDBusObjectManager *manager;
	GList *objects;
	GList *l;
	const gchar *obj_path = NULL;
	_cleanup_g_free_ gchar *g_path = NULL;
	const gchar *msg = NULL;
	gboolean out_gadget_enabled = 0;

	gret = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
					   "org.usb.gadgetd",
					   "/org/usb/Gadget",
					   "org.usb.device.GadgetManager",
					   "FindGadgetByName",
					   g_variant_new("(s)",
							 dt->gadget),
					   NULL,
					   G_DBUS_CALL_FLAGS_NONE,
					   -1,
					   NULL,
					   &error);

	if (error) {
		fprintf(stderr, "Failed to get gadget, %s\n", error->message);
		return -1;
	}

	g_variant_get(gret, "(o)", &g_path);
	g_variant_unref(gret);

	manager = g_dbus_object_manager_client_new_sync(backend_ctx.gadgetd_conn,
					       G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
					       "org.usb.gadgetd",
					       "/org/usb/Gadget",
					       NULL,
					       NULL,
					       NULL,
					       NULL,
					       &error);

	if (error) {
		fprintf(stderr, "Failed to get dbus object manager, %s\n", error->message);
		return -1;
	}

	/* get first "free" udc and enable gadget */
	objects = g_dbus_object_manager_get_objects(manager);
	for (l = objects; l != NULL; l = l->next)
	{
		GDBusObject *object = G_DBUS_OBJECT(l->data);
		obj_path = g_dbus_object_get_object_path(G_DBUS_OBJECT(object));

		if (g_str_has_prefix(obj_path, "/org/usb/Gadget/UDC")) {
			obj_path = g_dbus_object_get_object_path(G_DBUS_OBJECT(object));

			gret = g_dbus_connection_call_sync(backend_ctx.gadgetd_conn,
							   "org.usb.gadgetd",
							   obj_path,
							   "org.usb.device.UDC",
							   "EnableGadget",
							   g_variant_new("(o)",
									 g_path),
							   NULL,
							   G_DBUS_CALL_FLAGS_NONE,
							   -1,
							   NULL,
							   &error);
			if (error) {
				msg = error->message;
				goto out;
			}
			g_variant_get(gret, "(b)", &out_gadget_enabled);
			if (out_gadget_enabled) {
				g_variant_unref(gret);
				goto out;
			}
		}
	}

	if (l == NULL) {
		fprintf(stderr, "Failed to enable gadget, no UDC found\n");
		return -1;
	}

out:
	g_list_foreach(objects, (GFunc)g_object_unref, NULL);
	g_list_free(objects);
	g_object_unref(manager);

	if (msg != NULL) {
		fprintf(stderr, "Failed to enable gadget, %s\n", msg);
		return -1;
	}

	return 0;
}

struct gt_gadget_backend gt_gadget_backend_gadgetd = {
	.create = create_func,
	.rm = NULL,
	.get = NULL,
	.set = NULL,
	.enable = enable_func,
	.disable = NULL,
	.gadget = NULL,
	.load = NULL,
	.save = NULL,
	.template_default = NULL,
	.template_get = NULL,
	.template_set = NULL,
	.template_rm = NULL,
};
