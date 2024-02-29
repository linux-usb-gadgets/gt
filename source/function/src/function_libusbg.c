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
#include <usbg/usbg.h>
#include <usbg/function/ms.h>
#include <usbg/function/net.h>
#include <usbg/function/ffs.h>
#include <usbg/function/phonet.h>
#include <usbg/function/midi.h>
#include <usbg/function/loopback.h>

#include "function.h"
#include "common.h"
#include "backend.h"

static int create_func(void *data)
{
	struct gt_func_create_data *dt;
	usbg_gadget *g;
	usbg_function_type f_type;
	usbg_function *f;
	int r;

	dt = (struct gt_func_create_data *)data;

	if (dt->attrs->variable) {
		/* TODO add support for attributes */
		printf("Attributes are not supported now\n");
		return -1;
	}


	f_type = usbg_lookup_function_type(dt->type);
	if (f_type < 0) {
		fprintf(stderr, "Unable to find function %s: %s\n",
			dt->type, usbg_strerror(f_type));
		return -1;
	}

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (!g) {
		fprintf(stderr, "Unable to find gadget %s\n",
			dt->gadget);
		return -1;
	}

	r = usbg_create_function(g, f_type, dt->name, NULL, &f);
	if (r < 0) {
		fprintf(stderr, "Unable to create function: %s\n",
			usbg_strerror(r));
		return -1;
	}

	return 0;
}

static int list_types_func(void *data)
{
	int i;
	struct gt_func_list_types_data *dt;

	dt = (struct gt_func_list_types_data *)data;

	if (!(dt->opts & GT_QUIET))
		printf("Functions known by library:\n");

	for (i = USBG_FUNCTION_TYPE_MIN; i < USBG_FUNCTION_TYPE_MAX; i++)
		printf("  %s\n", usbg_get_function_type_str(i));

	return 0;
}

int gt_print_function_libusbg(usbg_function *f, int opts)
{
	const char *instance;
	usbg_function_type type;
	int usbg_ret;
	union {
		struct usbg_f_net_attrs net;
		char *ffs_dev_name;
		struct usbg_f_ms_attrs ms;
		struct usbg_f_midi_attrs midi;
		int serial_port_num;
		char *phonet_ifname;
		struct usbg_f_loopback_attrs loopback;
	} f_attrs;

	instance = usbg_get_function_instance(f);
	if (!instance) {
		fprintf(stderr, "Unable to get function instance name\n");
		return -1;
	}

	type = usbg_get_function_type(f);

	if (opts & GT_INSTANCE)
		printf("  %s\n", instance);
	else if (opts & GT_TYPE)
		printf("  %s\n", usbg_get_function_type_str(type));
	else
		fprintf(stdout, "  %s.%s\n",
			usbg_get_function_type_str(type), instance);

	usbg_ret = usbg_get_function_attrs(f, &f_attrs);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
				usbg_strerror(usbg_ret));
		return -1;
	}

	if (!(opts & GT_VERBOSE))
		return 0;

	switch (type) {
	case USBG_F_ACM:
	case USBG_F_OBEX:
	case USBG_F_SERIAL:
		fprintf(stdout, "    port_num\t\t%d\n",
			f_attrs.serial_port_num);
		break;

	case USBG_F_ECM:
	case USBG_F_SUBSET:
	case USBG_F_NCM:
	case USBG_F_EEM:
	case USBG_F_RNDIS:
		fprintf(stdout, "    dev_addr\t\t%s\n",
			ether_ntoa(&f_attrs.net.dev_addr));
		fprintf(stdout, "    host_addr\t\t%s\n",
			ether_ntoa(&f_attrs.net.host_addr));
		fprintf(stdout, "    ifname\t\t%s\n", f_attrs.net.ifname);
		fprintf(stdout, "    qmult\t\t%d\n", f_attrs.net.qmult);
		break;

	case USBG_F_PHONET:
		fprintf(stdout, "    ifname\t\t%s\n", f_attrs.phonet_ifname);
		break;

	case USBG_F_FFS:
		fprintf(stdout, "    dev_name\t\t%s\n", f_attrs.ffs_dev_name);
		break;

	case USBG_F_MASS_STORAGE:
	{
		struct usbg_f_ms_attrs *attrs = &f_attrs.ms;
		int i;

		fprintf(stdout, "    stall\t\t%d\n", attrs->stall);
		fprintf(stdout, "    nluns\t\t%d\n", attrs->nluns);
		for (i = 0; i < attrs->nluns; ++i) {
			fprintf(stdout, "    lun %d:\n", attrs->luns[i]->id);
			fprintf(stdout, "      cdrom\t\t%d\n", attrs->luns[i]->cdrom);
			fprintf(stdout, "      ro\t\t%d\n", attrs->luns[i]->ro);
			fprintf(stdout, "      nofua\t\t%d\n", attrs->luns[i]->nofua);
			fprintf(stdout, "      removable\t\t%d\n", attrs->luns[i]->removable);
			fprintf(stdout, "      file\t\t%s\n", attrs->luns[i]->file);
			fprintf(stdout, "      inquiry_string\t%s\n", attrs->luns[i]->inquiry_string);
		}
		break;
	}

	case USBG_F_MIDI:
	{
		struct usbg_f_midi_attrs *attrs = &f_attrs.midi;

		fprintf(stdout, "    index\t\t%d\n", attrs->index);
		fprintf(stdout, "    id\t\t\t%s\n", attrs->id);
		fprintf(stdout, "    in_ports\t\t%d\n", attrs->in_ports);
		fprintf(stdout, "    out_ports\t\t%d\n", attrs->out_ports);
		fprintf(stdout, "    buflen\t\t%d\n", attrs->buflen);
		fprintf(stdout, "    qlen\t\t%d\n", attrs->qlen);
		break;
	}

	case USBG_F_LOOPBACK:
	{
		struct usbg_f_loopback_attrs *attrs = &f_attrs.loopback;

		fprintf(stdout, "    buflen\t\t%d\n", attrs->buflen);
		fprintf(stdout, "    qlen\t\t%d\n", attrs->qlen);
		break;
	}

	default:
		fprintf(stdout, "    UNKNOWN\n");
	}

	usbg_cleanup_function_attrs(f, &f_attrs);
	return 0;
}

static int show_func(void *data)
{
	struct gt_func_show_data *dt;
	usbg_gadget *g;
	usbg_function *f;

	dt = (struct gt_func_show_data *)data;

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (g == NULL) {
		fprintf(stderr, "Unable to find gadget %s\n",
			dt->gadget);
		return -1;
	}

	if (dt->instance) {
		f = usbg_get_function(g, dt->type, dt->instance);
		if (f == NULL) {
			fprintf(stderr, "Unable to find function: %s.%s\n",
					usbg_get_function_type_str(dt->type),
					dt->instance);
			return -1;
		}

		gt_print_function_libusbg(f, dt->opts);
	} else if (dt->type >= 0) {
		usbg_for_each_function(f, g) {
			if (dt->type == usbg_get_function_type(f))
				gt_print_function_libusbg(f, dt->opts);
		}
	} else {
		usbg_for_each_function(f, g) {
			gt_print_function_libusbg(f, dt->opts);
		}
	}

	return 0;
}

static int rm_func(void *data)
{
	struct gt_func_rm_data *dt;
	usbg_gadget *g;
	usbg_function *f;
	usbg_udc *u;
	int ret, opts = 0;

	dt = (struct gt_func_rm_data *)data;

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (g == NULL) {
		fprintf(stderr, "Unable to find gadget %s\n",
			dt->gadget);
		return -1;
	}

	f = usbg_get_function(g, dt->type, dt->instance);
	if (f == NULL) {
		fprintf(stderr, "Unable to find function: %s.%s\n",
				usbg_get_function_type_str(dt->type),
				dt->instance);
		return -1;
	}

	if (dt->opts & GT_RECURSIVE)
		opts |= USBG_RM_RECURSE;

	if (dt->opts & GT_FORCE) {
		u = usbg_get_gadget_udc(g);
		if (u != NULL) {
			ret = usbg_disable_gadget(g);
			if (ret < 0) {
				fprintf(stderr, "Error disabling gadget: %s\n",
						usbg_strerror(ret));
				return -1;
			}
		}
	}

	ret = usbg_rm_function(f, opts);
	if (ret < 0) {
		fprintf(stderr, "Error removing function: %s\n",
				usbg_strerror(ret));
		return -1;
	}

	return ret;
}

struct gt_function_backend gt_function_backend_libusbg = {
	.create = create_func,
	.rm = rm_func,
	.list_types = list_types_func,
	.get = NULL,
	.set = NULL,
	.show = show_func,
	.load = NULL,
	.save = NULL,
	.template_default = NULL,
	.template_get = NULL,
	.template_set = NULL,
	.template_rm = NULL,
};
