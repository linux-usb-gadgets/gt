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
#include <unistd.h>
#include <getopt.h>
#include <glib.h>
#include <gio/gio.h>

#include "function.h"
#include "common.h"
#include "parser.h"
#include "backend.h"
#include <gio/gio.h>

struct gt_func_create_data {
	const char *gadget;
	const char *type;
	const char *name;
	int opts;
	struct gt_setting *attrs;
};

static void gt_func_create_destructor(void *data)
{
	struct gt_func_create_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_func_create_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_func_create_func(void *data)
{
	struct gt_func_create_data *dt;

	dt = (struct gt_func_create_data *)data;

	if (dt->attrs->variable) {
		/* TODO add support for attributes */
		printf("Attributes are not supported now\n");
		return -1;
	}

	if (backend_ctx.backend == GT_BACKEND_GADGETD) {
		gchar *gadget_objpath = NULL;
		GError *err = NULL;
		GVariant *v;

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

	} else if (backend_ctx.backend == GT_BACKEND_LIBUSBG) {
		usbg_gadget *g;
		usbg_function_type f_type;
		usbg_function *f;
		int r;

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

	return -1;
}

static int gt_func_create_help(void *data)
{
	printf("usage: %s func create GADGET_NAME FUNCTION_TYPE FUNCTION_NAME\n"
	       "Create new function of specified type (refer to `gt func list-types')\n",
		program_name);
	return 0;
}

static void gt_parse_func_create(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_create_data *dt = NULL;
	int ind;
	int tmp;
	int avaible_opts = GT_FORCE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (argc - ind < 3)
		goto out;

	dt->gadget = argv[ind++];
	dt->type = argv[ind++];
	dt->name = argv[ind++];

	tmp = gt_parse_setting_list(&dt->attrs, argc - ind, argv + ind);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, gt_func_create_func, (void *)dt,
			gt_func_create_destructor);

	return;
out:
	gt_func_create_destructor(dt);
	executable_command_set(exec, gt_func_create_help, data, NULL);
}

struct gt_func_rm_data {
	const char *gadget;
	const char *type;
	const char *name;
	int opts;
};

static void gt_func_rm_destructor(void *data)
{
	struct gt_func_rm_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_func_rm_data *)data;
	free(dt);
}

static int gt_func_rm_func(void *data)
{
	struct gt_func_rm_data *dt;

	dt = (struct gt_func_rm_data *)data;
	printf("Func rm called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, name=%s, recursive=%d, force=%d\n",
		dt->gadget, dt->type, dt->name, !!(dt->opts & GT_RECURSIVE),
		!!(dt->opts & GT_FORCE));

	return 0;
}

static int gt_func_rm_help(void *data)
{
	printf("Func rm help.\n");
	return -1;
}

static void gt_parse_func_rm(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_rm_data *dt = NULL;
	int ind;
	int avaible_opts = GT_FORCE | GT_RECURSIVE;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (argc - ind < 3)
		goto out;

	dt->gadget = argv[ind++];
	dt->type = argv[ind++];
	dt->name = argv[ind++];

	if (ind != argc)
		goto out;

	executable_command_set(exec, gt_func_rm_func, (void *)dt,
			gt_func_rm_destructor);

	return;
out:
	gt_func_rm_destructor(dt);
	executable_command_set(exec, gt_func_rm_help, data, NULL);
}


static int gt_func_list_types_func(void *data)
{
	int i;

	if (backend_ctx.backend == GT_BACKEND_GADGETD) {
		GError *err = NULL;
		GVariantIter *iter;
		GVariant *v;
		gchar *s;

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

		printf("Discovered functions:\n");
		g_variant_get(v, "(as)", &iter);
		while (g_variant_iter_loop(iter, "s", &s))
		       printf("  %s\n", s);

		g_variant_iter_free(iter);
		g_variant_unref(v);

		return 0;

	} else if (backend_ctx.backend == GT_BACKEND_LIBUSBG) {
		printf("Functions known by library:\n");
		for (i = USBG_FUNCTION_TYPE_MIN; i < USBG_FUNCTION_TYPE_MAX; i++)
			printf("  %s\n", usbg_get_function_type_str(i));

		return 0;
	}

	return -1;
}

static int gt_func_list_types_help(void *data)
{
	printf("%s func list-types\n"
	       "List available function types.\n",
		program_name);
	return 0;
}

static void gt_parse_func_list_types(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	if (argc == 0)
		executable_command_set(exec, gt_func_list_types_func, NULL, NULL);
	else
		executable_command_set(exec, gt_func_list_types_help, NULL, NULL);

}

struct gt_func_get_data {
	const char *gadget;
	const char *type;
	const char *name;
	const char **attrs;
};

static void gt_func_get_destructor(void *data)
{
	struct gt_func_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_get_data *)data;
	free(dt->attrs);
	free(dt);
}

static int gt_func_get_func(void *data)
{
	struct gt_func_get_data *dt;
	const char **ptr;

	dt = (struct gt_func_get_data *)data;
	printf("Func get called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, name=%s, attrs=",
		dt->gadget, dt->type, dt->name);

	for (ptr = dt->attrs; *ptr; ptr++)
		printf("%s, ", *ptr);
	putchar('\n');

	return 0;
}

static int gt_func_get_help(void *data)
{
	printf("Func get help.\n");
	return -1;
}

static void gt_parse_func_get(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_get_data *dt = NULL;
	int ind = 0;
	int i;

	if (argc < 3)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->gadget = argv[ind++];
	dt->type = argv[ind++];
	dt->name = argv[ind++];

	/* One more for NULL */
	dt->attrs = calloc(argc - ind + 1, sizeof(char *));
	if (dt->attrs == NULL)
		goto out;

	argv += ind;
	for (i = 0; argv[i]; i++)
		dt->attrs[i] = argv[i];

	executable_command_set(exec, gt_func_get_func, (void *)dt,
			gt_func_get_destructor);

	return;
out:
	gt_func_get_destructor(dt);
	executable_command_set(exec, gt_func_get_help, data, NULL);
}

struct gt_func_set_data {
	const char *gadget;
	const char *type;
	const char *name;
	struct gt_setting *attrs;
};

static void gt_func_set_destructor(void *data)
{
	struct gt_func_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_set_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_func_set_func(void *data)
{
	struct gt_func_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_func_set_data *)data;
	printf("Func set called successfully. Not implemented.\n");
	printf("gadget=%s, type=%s, name=%s", dt->gadget, dt->type, dt->name);

	for (ptr = dt->attrs; ptr->variable; ptr++)
		printf(", %s=%s", ptr->variable, ptr->value);
	putchar('\n');

	return 0;
}

static int gt_func_set_help(void *data)
{
	printf("Func set help.\n");
	return -1;
}

static void gt_parse_func_set(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_set_data *dt = NULL;
	int ind = 0;
	int tmp;

	if (argc < 3)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->gadget = argv[ind++];
	dt->type = argv[ind++];
	dt->name = argv[ind++];

	argc -= ind;
	argv += ind;
	tmp = gt_parse_setting_list(&dt->attrs, argc, argv);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, gt_func_set_func, (void *)dt,
			gt_func_set_destructor);

	return;
out:
	gt_func_set_destructor(dt);
	executable_command_set(exec, gt_func_get_help, data, NULL);
}

struct gt_func_func_data {
	const char *gadget;
	const char *type;
	const char *name;
	int opts;
};

static void gt_func_func_destructor(void *data)
{
	struct gt_func_func_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_func_data *)data;
	free(dt);
}

static int gt_func_func_func(void *data)
{
	struct gt_func_func_data *dt;

	dt = (struct gt_func_func_data *)data;
	printf("Func func called successfully. Not implemented.\n");
	printf("gadget=%s", dt->gadget);
	if (dt->type)
		printf(", type=%s", dt->type);
	if (dt->name)
		printf(", name=%s", dt->name);
	printf(", verbose=%d\n", !!(dt->opts & GT_VERBOSE));

	return 0;
}

static int gt_func_func_help(void *data)
{
	printf("usage: %s func COMMAND\n"
	       "Manipulate USB function - both in-kernel and functionfs-based\n\n",
		program_name);

	printf("Command:\n"
	       "  create\n"
	       "  rm\n"
	       "  get\n"
	       "  set\n"
	       "  list-types\n"
	       "  load\n"
	       "  save\n"
	       "  template\n");

	return 0;
}

static void gt_parse_func_func(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_func_data *dt = NULL;
	int ind;
	int avaible_opts = GT_VERBOSE;

	if (argc < 1)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	dt->gadget = argv[ind++];
	if (argc > ind) {
		if (argc - ind != 2)
			goto out;

		dt->type = argv[ind++];
		dt->name = argv[ind++];
	}

	executable_command_set(exec, gt_func_func_func, (void *)dt,
			gt_func_func_destructor);
	return;
out:
	gt_func_func_destructor(dt);
	executable_command_set(exec, gt_func_func_help, data, NULL);
}

struct gt_func_load_data {
	const char *name;
	const char *gadget;
	const char *func;
	const char *file;
	const char *path;
	int opts;
};

static int gt_func_load_func(void *data)
{
	struct gt_func_load_data *dt;

	dt = (struct gt_func_load_data *)data;
	printf("Func load called succesfully. Not implemented.\n");
	if (dt->name)
		printf("name=%s, ", dt->name);
	if (dt->gadget)
		printf("gadget=%s, ", dt->gadget);
	if (dt->func)
		printf("func=%s, ", dt->func);
	if (dt->file)
		printf("file=%s, ", dt->file);
	if (dt->path)
		printf("path=%s, ", dt->path);
	printf("force=%d, stdin=%d\n", !!(dt->opts & GT_FORCE),
			!!(dt->opts & GT_STDIN));

	return 0;
}

static int gt_func_load_help(void *data)
{
	printf("Func load help.\n");
	return -1;
}

static void gt_parse_func_load(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_load_data *dt = NULL;
	int c;
	struct option opts[] = {
			{"force", no_argument, 0, 'f'},
			{"recursive", no_argument, 0, 'r'},
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
		c = getopt_long(argc, argv, "f", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			dt->opts |= GT_FORCE;
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

	if (argc - optind < 2 || argc - optind > 3)
		goto out;

	dt->name = argv[optind++];
	dt->gadget = argv[optind++];
	if (dt->opts & GT_STDIN || dt->file) {
		dt->func = dt->name;
		dt->name = NULL;
		if (optind < argc)
			goto out;
	}

	if (optind < argc)
		dt->func = argv[optind];

	executable_command_set(exec, gt_func_load_func, (void *)dt, free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_func_save_data {
	const char *gadget;
	const char *func;
	const char *name;
	const char *file;
	const char *path;
	int opts;
	struct gt_setting *attrs;
};

static void gt_func_save_destructor(void *data)
{
	struct gt_func_save_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_func_save_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_func_save_func(void *data)
{
	struct gt_func_save_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_func_save_data *)data;

	printf("Func save called successfully. Not implemented.\n");
	if (dt->gadget)
		printf("gadget=%s, ", dt->gadget);
	if (dt->func)
		printf("func=%s, ", dt->func);
	if (dt->name)
		printf("name=%s, ", dt->name);
	if (dt->file)
		printf("file=%s, ", dt->file);
	if (dt->path)
		printf("path=%s, ", dt->path);
	printf("force=%d, stdout=%d", !!(dt->opts & GT_FORCE),
			!!(dt->opts & GT_STDOUT));

	for (ptr = dt->attrs; ptr->variable; ptr++)
		printf(", %s=%s", ptr->variable, ptr->value);
	putchar('\n');

	return 0;
}

static int gt_func_save_help(void *data)
{
	printf("Func save help.\n");
	return -1;
}

static void gt_parse_func_save(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_save_data *dt = NULL;
	int c;
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
			if (dt->path || dt->file)
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

	if (argc - optind < 2)
		goto out;

	dt->gadget = argv[optind++];
	dt->func = argv[optind++];
	if (optind < argc
	   && !dt->file
	   && !(dt->opts & GT_STDOUT)
	   && strchr(argv[optind], '=') == NULL)
		dt->name = argv[optind++];

	c = gt_parse_setting_list(&dt->attrs, argc - optind, argv + optind);
	if (c < 0)
		goto out;

	executable_command_set(exec, gt_func_save_func, (void *)dt,
			gt_func_save_destructor);

	return;
out:
	gt_func_save_destructor(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

int gt_func_help(void *data)
{
	printf("Function help function\n");
	return -1;
}

struct gt_func_template_data {
	const char *name;
	int opts;
};

static int gt_func_template_func(void *data)
{
	struct gt_func_template_data *dt;

	dt = (struct gt_func_template_data *)data;
	printf("Func template called successfully. Not implemented.\n");
	if (dt->name)
		printf("name=%s, ", dt->name);
	printf("verbose=%d\n", !!(dt->opts & GT_VERBOSE));

	return 0;
}

static int gt_func_template_help(void *data)
{
	printf("Func template help.\n");
	return -1;
}

static void gt_parse_func_template(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_template_data *dt;
	int avaible_opts = GT_VERBOSE;
	int ind;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	ind = gt_get_options(&dt->opts, avaible_opts, argc, argv);
	if (ind < 0)
		goto out;

	if (argc - ind > 1)
		goto out;

	dt->name = argv[ind++];

	executable_command_set(exec, gt_func_template_func, (void *)dt, free);
	return;
out:
	free(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_func_template_get_data {
	const char *name;
	const char **attrs;
};

static void gt_func_template_get_destructor(void *data)
{
	struct gt_func_template_get_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_template_get_data *)data;
	free(dt->attrs);
	free(dt);
}

static int gt_func_template_get_func(void *data)
{
	struct gt_func_template_get_data *dt;
	const char **ptr;

	dt = (struct gt_func_template_get_data *)data;
	printf("Func template get called successfully. Not implemented.\n");
	printf("name=%s, attrs=", dt->name);
	for (ptr = dt->attrs; *ptr; ptr++)
		printf("%s, ", *ptr);
	putchar('\n');

	return 0;
}

static int gt_func_template_get_help(void *data)
{
	printf("Func template get help called successfully.\n");
	return -1;
}

static void gt_parse_func_template_get(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_template_get_data *dt = NULL;
	int i;

	if (argc < 1)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->name = argv[0];

	dt->attrs = calloc(argc, sizeof(char *));
	if (dt->attrs == NULL)
		goto out;

	argv += 1;
	for (i = 0; argv[i]; i++)
		dt->attrs[i] = argv[i];

	executable_command_set(exec, gt_func_template_get_func, (void *)dt,
			gt_func_template_get_destructor);
	return;

out:
	gt_func_template_get_destructor(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

struct gt_func_template_set_data {
	const char *name;
	struct gt_setting *attrs;
};

static void gt_func_template_set_destructor(void *data)
{
	struct gt_func_template_set_data *dt;

	if (data == NULL)
		return;
	dt = (struct gt_func_template_set_data *)data;
	gt_setting_list_cleanup(dt->attrs);
	free(dt);
}

static int gt_func_template_set_func(void *data)
{
	struct gt_func_template_set_data *dt;
	struct gt_setting *ptr;

	dt = (struct gt_func_template_set_data *)data;
	printf("Func template set called successfully. Not implemented.\n");
	printf("name=%s", dt->name);

	for (ptr = dt->attrs; ptr->variable; ptr++)
		printf(", %s=%s", ptr->variable, ptr->value);
	putchar('\n');

	return 0;
}

static int gt_func_template_set_help(void *data)
{
	printf("Func template set help.\n");
	return -1;
}

static void gt_parse_func_template_set(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	struct gt_func_template_set_data *dt = NULL;
	int tmp;

	if (argc < 2)
		goto out;

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	dt->name = argv[0];
	tmp = gt_parse_setting_list(&dt->attrs, argc - 1, argv + 1);
	if (tmp < 0)
		goto out;

	executable_command_set(exec, gt_func_template_set_func, (void *)dt,
			gt_func_template_set_destructor);
	return;

out:
	gt_func_template_set_destructor(dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

static int gt_func_template_rm_func(void *data)
{
	const char *dt;

	dt = (const char *)data;
	printf("Func template rm called successfully. Not implemented.\n");
	printf("name=%s\n", dt);

	return 0;
}

static int gt_func_template_rm_help(void *data)
{
	printf("Func template rm help.\n");
	return -1;
}

static void gt_parse_func_template_rm(const Command *cmd, int argc,
		char **argv, ExecutableCommand *exec, void * data)
{
	const char *name;

	if (argc != 1)
		goto out;

	name = argv[0];

	executable_command_set(exec, gt_func_template_rm_func, (void *)name, NULL);

	return;
out:
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *gt_func_template_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"get", NEXT, gt_parse_func_template_get, NULL,
			gt_func_template_get_help},
		{"set", NEXT, gt_parse_func_template_set, NULL,
			gt_func_template_set_help},
		{"rm", NEXT, gt_parse_func_template_rm, NULL,
			gt_func_template_rm_help},
		{NULL, AGAIN, gt_parse_func_template, NULL,
			gt_func_template_help},
		{NULL, AGAIN, NULL, NULL, NULL}
	};
	return commands;
}

const Command *gt_func_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_func_create, NULL,
			gt_func_create_help},
		{"rm", NEXT, gt_parse_func_rm, NULL, gt_func_rm_help},
		{"get", NEXT, gt_parse_func_get, NULL, gt_func_get_help},
		{"set", NEXT, gt_parse_func_set, NULL, gt_func_set_help},
		{"list-types", NEXT, gt_parse_func_list_types, NULL, gt_func_list_types_help},
		{"load", NEXT, gt_parse_func_load, NULL, gt_func_load_help},
		{"save", NEXT, gt_parse_func_save, NULL, gt_func_save_help},
		{"template", NEXT, command_parse,
			gt_func_template_get_children, gt_func_template_help},
		{NULL, AGAIN, gt_parse_func_func, NULL, gt_func_func_help},
		{NULL, AGAIN, NULL, NULL, NULL}
	};
	return commands;
}
