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
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include "gadget.h"
#include "common.h"
#include "parser.h"
#include "backend.h"

static const struct {
	char *name;
	int (*set_fn)(usbg_gadget *, int, const char *);
} strs[] = {
	{ "product", usbg_set_gadget_product },
	{ "manufacturer", usbg_set_gadget_manufacturer },
	{ "serialnumber", usbg_set_gadget_product },
};

struct gt_gadget_create_data {
	const char *name;
	struct gt_setting *attrs;
	int opts;
};

static void gt_gadget_create_destructor(void *data)
{
	struct gt_gadget_create_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_create_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static char *attr_type_get(usbg_gadget_attr a)
{
	switch (a) {
	case B_DEVICE_CLASS:
	case B_DEVICE_SUB_CLASS:
	case B_DEVICE_PROTOCOL:
	case B_MAX_PACKET_SIZE_0:
		return "y";

	case BCD_USB:
	case ID_VENDOR:
	case ID_PRODUCT:
	case BCD_DEVICE:
		return "q";

	default:
		return NULL;
	}
}

static int attr_val_in_range(usbg_gadget_attr a, unsigned long int val)
{
	char *type = attr_type_get(a);

	if (!type)
		return 0;

	if (type[0] == 'y')
		return val <= UINT8_MAX;
	else if (type[0] == 'q')
		return val <= UINT16_MAX;
	else
		return 0;
}


static int gt_gadget_create_func(void *data)
{
	struct gt_gadget_create_data *dt;
	struct gt_setting *setting;
	int attr_val[USBG_GADGET_ATTR_MAX];
	char *str_val[G_N_ELEMENTS(strs)];
	_Bool iter;
	int i;
	int r;

	for (i = 0; i < G_N_ELEMENTS(attr_val); i++)
		attr_val[i] = -1;
	memset(str_val, 0, sizeof(str_val));

	dt = (struct gt_gadget_create_data *)data;

	for (setting = dt->attrs; setting->variable; setting++) {
		int attr_id;

		iter = TRUE;

		attr_id = usbg_lookup_gadget_attr(setting->variable);
		if (attr_id >= 0) {
			unsigned long int val;
			char *endptr = NULL;

			errno = 0;
			val = strtoul(setting->value, &endptr, 0);
			if (errno
			    || *setting->value == 0
			    || (endptr && *endptr != 0)
			    || attr_val_in_range(attr_id, val) == 0) {

				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				return -1;
			}

			attr_val[attr_id] = val;
			iter = FALSE;
		}
		for (i = 0; iter && i < G_N_ELEMENTS(strs); i++) {
			if (streq(setting->variable, strs[i].name)) {
				str_val[i] = setting->value;
				iter = FALSE;
				break;
			}
		}

		if (iter) {
			fprintf(stderr, "Unknown attribute passed: %s\n", setting->variable);
			return -1;
		}
	}

	if (backend_ctx.backend == GT_BACKEND_GADGETD) {

		GVariantBuilder *b;
		GVariant *gattrs;
		GVariant *gstrs;
		GVariant *v;
		GError *err = NULL;

		b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
		for (i = USBG_GADGET_ATTR_MIN; i < USBG_GADGET_ATTR_MAX; i++) {
			if (attr_val[i] == -1)
				continue;

			g_variant_builder_add(b, "{sv}",
					      usbg_get_gadget_attr_str(i),
					      g_variant_new(attr_type_get(i), attr_val[i]));
		}
		gattrs = g_variant_builder_end(b);

		b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
		for (i = 0; i < G_N_ELEMENTS(strs); i++) {
			if (str_val[i] == NULL)
				continue;

			g_variant_builder_add(b, "{sv}",
					      strs[i].name,
					      g_variant_new("s", str_val[i]));
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

	} else if (backend_ctx.backend == GT_BACKEND_LIBUSBG) {
		usbg_gadget *g;

		r = usbg_create_gadget(backend_ctx.libusbg_state,
				       dt->name,
				       NULL,
				       NULL,
				       &g);
		if (r != USBG_SUCCESS) {
			fprintf(stderr, "Unable to create gadget %s: %s\n",
				dt->name, usbg_strerror(r));
			return -1;
		}

		for (i = USBG_GADGET_ATTR_MIN; i < USBG_GADGET_ATTR_MAX; i++) {
			if (attr_val[i] == -1)
				continue;

			r = usbg_set_gadget_attr(g, i, attr_val[i]);
			if (r != USBG_SUCCESS) {
				fprintf(stderr, "Unable to set attribute %s: %s\n",
					usbg_get_gadget_attr_str(i),
					usbg_strerror(r));
				goto err_usbg;
			}
		}

		for (i = 0; i < G_N_ELEMENTS(str_val); i++) {
			if (str_val[i] == NULL)
				continue;

			r = strs[i].set_fn(g, LANG_US_ENG, str_val[i]);
			if (r != USBG_SUCCESS) {
				fprintf(stderr, "Unable to set string %s: %s\n",
					strs[i].name,
					usbg_strerror(r));
				goto err_usbg;
			}
		}

		return 0;

	err_usbg:
		usbg_rm_gadget(g, USBG_RM_RECURSE);
		return -1;
	}

	return -1;
}

static int gt_gadget_create_help(void *data)
{
	int i;

	printf("usage: %s create NAME [attr=value]...\n"
	       "Create new gadget of specified name, attributes and language strings.\n"
	       "\n"
	       "Attributes:\n",
	       program_name);

	for (i = USBG_GADGET_ATTR_MIN; i < USBG_GADGET_ATTR_MAX; i++)
		printf("  %s\n", usbg_get_gadget_attr_str(i));

	printf("Device strings (en_US locale):\n");

	for (i = 0; i < G_N_ELEMENTS(strs); i++)
		printf("  %s\n", strs[i].name);

	return 0;
}

static void gt_parse_gadget_create(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_gadget_create_data *dt;
	int ind;
	int c;
	int avaible_opts = GT_FORCE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (ind == argc)
		goto out;

	dt->name = argv[ind++];

	c = gt_parse_setting_list(&dt->attrs, argc - ind, argv + ind);
	if (c < 0)
		goto out;

	executable_command_set(exec, gt_gadget_create_func,
				(void *)dt, gt_gadget_create_destructor);
	return;
out:
	gt_gadget_create_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_rm_data {
	const char *name;
	int opts;
};

static int gt_gadget_rm_help(void *data)
{
	printf("Gadget rm help.\n");
	return -1;
}

static int gt_gadget_rm_func(void *data)
{
	struct gt_gadget_rm_data *dt;

	dt = (struct gt_gadget_rm_data *)data;
	printf("Gadget rm called successfully. Not implemented.\n");
	printf("name = %s, force = %d, recursive = %d\n",
		dt->name, !!(dt->opts & GT_FORCE), !!(dt->opts & GT_RECURSIVE));
	return 0;
}

static void gt_parse_gadget_rm(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_gadget_rm_data *dt;
	int ind;
	int avaible_opts = GT_FORCE | GT_RECURSIVE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (argc - ind != 1)
		goto out;
	dt->name = argv[ind];
	executable_command_set(exec, gt_gadget_rm_func, (void *)dt,
			free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_get_data {
	const char *name;
	const char **attrs;
};

static void gt_gadget_get_destructor(void *data)
{
	struct gt_gadget_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_get_data *)data;

	free(dt->attrs);
	free(dt);
}

static int gt_gadget_get_func(void *data)
{
	struct gt_gadget_get_data *dt;
	const char **ptr;

	dt = (struct gt_gadget_get_data *)data;
	printf("Gadget get called successfully. Not implemented yet.\n");
	printf("name = %s, attrs = ", dt->name);

	ptr = dt->attrs;
	while (*ptr) {
		printf("%s, ", *ptr);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int gt_gadget_get_help(void *data)
{
	printf("Gadget get help.\n");
	return -1;
}

static void gt_parse_gadget_get(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void * data)
{
	struct gt_gadget_get_data *dt = NULL;
	int i;

	if (argc == 0)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->name = argv[0];
	dt->attrs = calloc(argc, sizeof(char *));
	if (dt->attrs == NULL)
		goto out;

	argv++;
	for (i = 0; i < argc; i++) {
		dt->attrs[i] = argv[i];
	}

	executable_command_set(exec, gt_gadget_get_func, (void *)dt,
			gt_gadget_get_destructor);

	return;
out:
	gt_gadget_get_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_set_data {
	const char *name;
	struct gt_setting *attrs;
};

static void gt_gadget_set_destructor(void *data)
{
	struct gt_gadget_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_set_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_gadget_set_func(void *data)
{
	struct gt_gadget_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_gadget_set_data *)data;
	printf("Gadget set called successfully. Not implemented.\n");
	printf("name = %s", dt->name);
	ptr = dt->attrs;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}
	putchar('\n');
	return 0;
}

static int gt_gadget_set_help(void *data)
{
	printf("Gadget set help.\n");
	return -1;
}

static void gt_parse_gadget_set(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void * data)
{
	struct gt_gadget_set_data *dt = NULL;
	int tmp;

	if (argc < 2)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->name = argv[0];
	tmp = gt_parse_setting_list(&dt->attrs, argc-1, argv+1);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, gt_gadget_set_func, (void *)dt,
			gt_gadget_set_destructor);
	return;
out:
	gt_gadget_set_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_enable_data {
	const char *gadget;
	const char *udc;
};

static int gt_gadget_enable_func(void *data)
{
	struct gt_gadget_enable_data *dt;

	dt = (struct gt_gadget_enable_data *)data;
	printf("Gadget enable called successfully. Not implemented.\n");
	if (dt->gadget)
		printf("gadget = %s, ", dt->gadget);
	if (dt->udc)
		printf("udc = %s", dt->udc);
	putchar('\n');
	return 0;
}

static int gt_gadget_enable_help(void *data)
{
	printf("Gadget enable help.\n");
	return -1;
}

static void gt_parse_gadget_enable(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_gadget_enable_data *dt;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	switch (argc) {
	case 1:
		dt->gadget = argv[0];
		break;
	case 2:
		dt->gadget = argv[0];
		dt->udc = argv[1];
		break;
	default:
		goto out;
	}

	executable_command_set(exec, gt_gadget_enable_func,
				(void *)dt, free);
	return;
out:
	free((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_disable_data {
	const char *gadget;
	const char *udc;
};

static int gt_gadget_disable_func(void *data)
{
	struct gt_gadget_disable_data *dt;

	dt = (struct gt_gadget_disable_data *)data;
	printf("Gadget disable called successfully. Not implemented.\n");
	if (dt->gadget)
		printf("gadget = %s, ", dt->gadget);
	if (dt->udc)
		printf("udc = %s", dt->udc);
	putchar('\n');
	return 0;
}

static int gt_gadget_disable_help(void *data)
{
	printf("Gadget disable help.\n");
	return -1;
}

static void gt_parse_gadget_disable(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_gadget_disable_data *dt;
	char c;
	struct option opts[] = {
		{"udc", required_argument, 0, 'u'},
		{0, 0, 0, 0}
	};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;
	argv--;
	argc++;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "u:", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 'u':
			dt->udc = optarg;
			break;
		default:
			goto out;
		}
	}

	if (optind < argc - 1 || (dt->udc && optind < argc)) {
		printf("Too many arguments\n");
		goto out;
	}

	dt->gadget = argv[optind];
	executable_command_set(exec, gt_gadget_disable_func, (void *)dt, free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_gadget_data {
	const char *name;
	int opts;
};

static int gt_gadget_gadget_func(void *data)
{
	struct gt_gadget_gadget_data *dt;

	dt = (struct gt_gadget_gadget_data *)data;
	printf("Gadget gadget called successfully. Not implemented.\n");
	if (dt->name)
		printf("name = %s, ", dt->name);
	printf("recursive = %d, verbose = %d\n",
		!!(dt->opts & GT_RECURSIVE), !!(dt->opts & GT_VERBOSE));
	return 0;
}

static int gt_gadget_gadget_help(void *data)
{
	printf("Gadget gadget help.\n");
	return -1;
}

static void gt_parse_gadget_gadget(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_gadget_gadget_data *dt;
	int ind;
	int avaible_opts = GT_RECURSIVE | GT_VERBOSE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	switch (argc - ind) {
	case 0:
		break;
	case 1:
		dt->name = argv[ind];
		break;
	default:
		goto out;
	}

	executable_command_set(exec, gt_gadget_gadget_func, (void *)dt,
		free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_load_data {
	const char *name;
	const char *gadget_name;
	const char *file;
	const char *path;
	int opts;
};

static int gt_gadget_load_func(void *data)
{
	struct gt_gadget_load_data *dt;

	dt = (struct gt_gadget_load_data *)data;
	printf("Gadget load called successfully. Not implemented.\n");
	if (dt->name)
		printf("name = %s, ", dt->name);
	if (dt->gadget_name)
		printf("gadget = %s, ", dt->gadget_name);
	if (dt->file)
		printf("file %s, ", dt->file);
	if (dt->path)
		printf("path = %s, ", dt->path);

	printf("off = %d, stdin = %d\n",
		!!(dt->opts & GT_OFF), !!(dt->opts & GT_STDIN));

	return 0;
}

static int gt_gadget_load_help(void *data)
{
	printf("Gadget load help\n");
	return -1;
}

static void gt_parse_gadget_load(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	char c;
	struct gt_gadget_load_data *dt;
	struct option opts[] = {
		{"off", no_argument, 0, 'o'},
		{"file", required_argument, 0, 1},
		{"stdin", no_argument, 0, 2},
		{"path", required_argument, 0, 3},
		{0, 0, 0, 0}
	};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;
	argv--;
	argc++;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "o", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 'o':
			dt->opts |= GT_OFF;
			break;
		case 1:
			if (dt->path || dt->opts & GT_STDIN)
				goto out;
			dt->file = optarg;
			break;
		case 2:
			if (dt->path || dt->file)
				goto out;
			dt->opts |= GT_STDIN;
			break;
		case 3:
			if (dt->file || dt->opts & GT_STDIN)
				goto out;
			dt->path = optarg;
			break;
		default:
			goto out;
		}
	}

	if (optind == argc || optind < argc - 2)
		goto out;

	dt->name = argv[optind++];
	if (dt->opts & GT_STDIN || dt->file) {
		dt->gadget_name = dt->name;
		dt->name = NULL;
		if (optind < argc)
			goto out;
	}

	if (optind < argc)
		dt->gadget_name = argv[optind++];

	executable_command_set(exec, gt_gadget_load_func, (void *)dt, free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_save_data {
	const char *gadget;
	const char *name;
	const char *file;
	const char *path;
	struct gt_setting *attrs;
	int opts;
};

static void gt_gadget_save_destructor(void *data)
{
	struct gt_gadget_save_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_save_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_gadget_save_func(void *data)
{
	struct gt_gadget_save_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_gadget_save_data *)data;
	printf("Gadget save called successfully. Not implemented\n");
	if (dt->gadget)
		printf("gadget = %s, ", dt->gadget);
	if (dt->name)
		printf("name = %s, ", dt->name);
	if (dt->file)
		printf("file = %s, ", dt->file);
	if (dt->path)
		printf("path = %s, ", dt->path);
	printf("force = %d, stdout = %d",
		!!(dt->opts & GT_FORCE), !!(dt->opts & GT_STDOUT));

	ptr = dt->attrs;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int gt_gadget_save_help(void *data)
{
	printf("Gadget save help.\n");
	return -1;
}

static void gt_parse_gadget_save(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int c;
	struct gt_gadget_save_data *dt;
	struct option opts[] = {
		{"force", no_argument, 0, 'f'},
		{"file", required_argument, 0, 1},
		{"stdout", no_argument, 0, 2},
		{"path", required_argument, 0, 3},
		{0, 0, 0, 0}
	};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;
	argv--;
	argc++;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "f", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 'f':
			dt->opts |= GT_FORCE;
			break;
		case 1:
			if (dt->path || dt->opts & GT_STDOUT)
				goto out;
			dt->file = optarg;
			break;
		case 2:
			if (dt->file || dt->path)
				goto out;
			dt->opts |= GT_STDOUT;
			break;
		case 3:
			if (dt->file || dt->opts & GT_STDOUT)
				goto out;
			dt->path = optarg;
			break;
		default:
			goto out;
		}
	}

	if (optind == argc)
		goto out;

	dt->gadget = argv[optind++];

	if(optind < argc
	  && !dt->file
	  && !(dt->opts & GT_STDOUT)
	  && strchr(argv[optind], '=') == NULL)
		dt->name = argv[optind++];

	c = gt_parse_setting_list(&dt->attrs, argc - optind,
			argv + optind);
	if (c < 0)
		goto out;

	executable_command_set(exec, gt_gadget_save_func, (void *)dt,
		gt_gadget_save_destructor);

	return;
out:
	gt_gadget_save_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_template_data {
	const char *name;
	int opts;
};

static int gt_gadget_template_func(void *data)
{
	struct gt_gadget_template_data *dt;

	dt = (struct gt_gadget_template_data *)data;
	printf("Gadget template called successfully. Not implemented.\n");
	if (dt->name)
		printf("name = %s, ", dt->name);
       	printf("verbose = %d, recursive = %d\n",
		!!(dt->opts & GT_VERBOSE), !!(dt->opts & GT_RECURSIVE));
	return 0;
}

static int gt_gadget_template_help(void *data)
{
	printf("Gadget template help.\n");
	return -1;
}

static void gt_parse_gadget_template(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int ind;
	struct gt_gadget_template_data *dt;
	int avaible_opts = GT_VERBOSE | GT_RECURSIVE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;
	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (argc - ind > 1)
		goto out;

	if (argv[ind])
		dt->name = argv[ind];

	executable_command_set(exec, gt_gadget_template_func, (void *)dt,
		free);

	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_gadget_template_rm_func(void *data)
{
	char *dt;

	dt = (char *)data;
	printf("Gadget template rm called successfully. Not implemented.\n");
	printf("name = %s\n", dt);
	return 0;
}

static int gt_gadget_template_rm_help(void *data)
{
	printf("Gadget template rm help.\n");
	return -1;
}

static void gt_parse_gadget_template_rm(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	const char *dt = NULL;

	if (argc != 1)
		goto out;

	dt = argv[0];
	executable_command_set(exec, gt_gadget_template_rm_func, (void *)dt,
		NULL);

	return;
out:
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_template_set_data {
	const char *name;
	struct gt_setting *attr;
};

static void gt_gadget_template_set_destructor(void *data)
{
	struct gt_gadget_template_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_template_set_data *)data;
	gt_setting_list_cleanup(dt->attr);
	free(dt);
}

static int gt_gadget_template_set_func(void *data)
{
	struct gt_gadget_template_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_gadget_template_set_data *)data;
	printf("Gadget template set called successfully. Not implemened.\n");
	printf("name = %s", dt->name);
	ptr = dt->attr;
	while (ptr->variable) {
		printf(", %s = %s", ptr->variable, ptr->value);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int gt_gadget_template_set_help(void *data)
{
	printf("Gadget template set help.\n");
	return -1;
}

static void gt_parse_gadget_template_set(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int tmp;
	struct gt_gadget_template_set_data *dt;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	if (argc != 2)
		goto out;

	dt->name = argv[0];
	tmp = gt_parse_setting_list(&dt->attr, argc-1, argv+1);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, gt_gadget_template_set_func, (void *)dt,
		gt_gadget_template_set_destructor);

	return;
out:
	gt_gadget_template_set_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_gadget_template_get_data {
	const char *name;
	const char **attr;
};

static void gt_gadget_template_get_destructor(void *data)
{
	struct gt_gadget_template_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_gadget_template_get_data *)data;
	free(dt->attr);
	free(dt);
}

static int gt_gadget_template_get_func(void *data)
{
	struct gt_gadget_template_get_data *dt;
	const char **ptr;

	dt = (struct gt_gadget_template_get_data *)data;
	printf("Gadget template get called successfully. Not implemented.\n");
	printf("name = %s, attr = ", dt->name);
	ptr = dt->attr;
	while (*ptr) {
		printf("%s, ", *ptr);
		ptr++;
	}

	putchar('\n');
	return 0;
}

static int gt_gadget_template_get_help(void *data)
{
	printf("Gadget template get help.\n");
	return -1;
}

static void gt_parse_gadget_template_get(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	int i;
	struct gt_gadget_template_get_data *dt;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	if (argc < 1)
		goto out;

	dt->name = argv[0];
	dt->attr = calloc(argc, sizeof(char *));
	if (dt->attr == NULL)
		goto out;

	argv++;
	argc--;
	for (i = 0; i < argc; i++)
		dt->attr[i] = argv[i];

	executable_command_set(exec, gt_gadget_template_get_func, (void *)dt,
		gt_gadget_template_get_destructor);

	return;
out:
	gt_gadget_template_get_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *get_gadget_template_children(const Command *cmd)
{
	static Command commands[] = {
		{"get", NEXT, gt_parse_gadget_template_get, NULL,
			gt_gadget_template_get_help},
		{"set", NEXT, gt_parse_gadget_template_set, NULL,
			gt_gadget_template_set_help},
		{"rm", NEXT, gt_parse_gadget_template_rm, NULL,
			gt_gadget_template_rm_help},
		{NULL, AGAIN, gt_parse_gadget_template, NULL,
			gt_gadget_template_help},
		CMD_LIST_END
	};

	return commands;
}

const Command *get_gadget_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_gadget_create, NULL,
			gt_gadget_create_help},
		{"rm", NEXT, gt_parse_gadget_rm, NULL, gt_gadget_rm_help},
		{"get", NEXT, gt_parse_gadget_get, NULL, gt_gadget_get_help},
		{"set", NEXT, gt_parse_gadget_set, NULL, gt_gadget_set_help},
		{"enable", NEXT, gt_parse_gadget_enable, NULL,
			gt_gadget_enable_help},
		{"disable", NEXT, gt_parse_gadget_disable, NULL,
			gt_gadget_disable_help},
		{"gadget", NEXT, gt_parse_gadget_gadget, NULL,
			gt_gadget_gadget_help},
		{"template", NEXT, command_parse, get_gadget_template_children,
			gt_gadget_template_help},
		{"load", NEXT, gt_parse_gadget_load, NULL,
			gt_gadget_load_help},
		{"save", NEXT, gt_parse_gadget_save, NULL,
			gt_gadget_save_help},
		CMD_LIST_END
	};

	return commands;
}

int gt_gadget_help(void *data)
{
	printf("Gadget help function\n");
	return -1;
}
