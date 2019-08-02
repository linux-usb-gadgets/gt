/*
 * Copyright (c) 2019 Collabora Ltd
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
#include <errno.h>
#include <ctype.h>

#include "ffs.h"
#include "ffs_state.h"
#include "common.h"
#include "parser.h"
#include "backend.h"
#include "settings.h"

#define GET_EXECUTABLE(func) \
	(backend_ctx.backend->ffs->func ? \
	 backend_ctx.backend->ffs->func : \
	 gt_ffs_backend_not_implemented.func)

int gt_ffs_help(void *data)
{
	printf("FFS help func. Not implemented yet.\n");
	return -1;
}

static int gt_ffs_interface_help(void *data)
{
	printf("FFS interface help func. Not implemented yet.\n");

	return -1;
}

static int gt_ffs_endpoint_help(void *data)
{
	printf("FFS endpoint help func. Not implemented yet.\n");

	return -1;
}

static int gt_ffs_language_help(void *data)
{
	printf("FFS language help func. Not implemented yet.\n");

	return -1;
}

static int gt_ffs_string_help(void *data)
{
	printf("FFS string help func. Not implemented yet.\n");

	return -1;
}

static int gt_ffs_descriptors_help(void *data)
{
	printf("FFS descriptors help func. Not implemented yet.\n");

	return -1;
}

static int gt_ffs_strings_help(void *data)
{
	printf("FFS strings help func. Not implemented yet.\n");

	return -1;
}

static void gt_ffs_interface_create_destructor(void *data)
{
	struct gt_ffs_interface_create_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_ffs_interface_create_data *)data;
	if (dt->state != NULL)
		gt_ffs_cleanup_descs_state(dt->state, dt->state_file);

	free(dt);
}

static int gt_ffs_interface_create_help(void *data)
{
	printf("FFS interface create help func. Not implemented yet.\n");

	return -1;
}

unsigned long int *find_interface_attr(struct gt_ffs_interface_create_data *dt, const char *name, struct find_interface_attr *f)
{
	if (streq(name, "speed")) {
		f->speed_found = true;
		return &dt->speed;
	} else if (streq(name, "number")) {
		f->number_found = true;
		return &dt->number;
	} else if (streq(name, "cls")) {
		f->cls_found = true;
		return &dt->cls;
	} else if (streq(name, "subcls")) {
		f->subcls_found = true;
		return &dt->subcls;
	} else if (streq(name, "protocol")) {
		f->protocol_found = true;
		return &dt->protocol;
	} else if (streq(name, "str_idx")) {
		f->str_idx_found = true;
		return &dt->str_idx;
	}

	return NULL;
}

static int gt_parse_ffs_interface_create_attrs(struct gt_setting *attrs, struct gt_ffs_interface_create_data *dt)
{
	struct gt_setting *setting;
	struct find_interface_attr f = { false, false, false, false, false, false, false };

	for (setting = attrs; setting->variable; ++setting) {
		unsigned long int *field;
		unsigned long int val;
		char *endptr = NULL;

		field = find_interface_attr(dt, setting->variable, &f);
		if (field == NULL) {
			fprintf(stderr, "Unknown attribute passed: %s\n", setting->variable);
			return -1;
		}

		errno = 0;
		if (streq(setting->variable, "speed")) {
			char buf[3] = { '\0'};
			if (strlen(setting->value) != 2) {
				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				return -1;
			}
			buf[0] = toupper(setting->value[0]);
			buf[1] = toupper(setting->value[1]);
			if (streq(buf, "LS"))
				val = LS;
			else if (streq(buf, "FS"))
				val = FS;
			else if (streq(buf, "HS"))
				val = HS;
			else if (streq(buf, "SS"))
				val = SS;
			else {
				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				return -1;
			}
		} else { /* number, cls, subcls, protocol, str_idx */
			val = strtoul(setting->value, &endptr, 0);
			if (errno
			    || *setting->value == 0
			    || (endptr && *endptr != 0)
			    || !(val <= UINT8_MAX)) {

				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				return -1;
			}
		}
		*field = val;
	}

	if (!f.speed_found) {
		fprintf(stderr, "Interface speed=<ls|fs|hs|ss> missing\n");
		return -1;
	}

	return 0;
}

static void gt_parse_ffs_interface_create(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_ffs_interface_create_data *dt;
	struct gt_setting *attrs;
	int c;
	struct option opts[] = {
		{"state", required_argument, 0, 1},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	argv--;
	argc++;
	dt->state_file = gt_settings.default_ffs_descs;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "h", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 1:
			dt->state_file = optarg;
			break;
		case 'h':
			goto out;
			break;
		default:
			goto out;
		}
	}

	/* only speed=ls|fs|hs|ss is mandatory */
	if (argc - optind < 1)
		goto out;

	c = gt_parse_setting_list(&attrs, argc - optind, argv + optind);
	if (c < 0)
		goto out;

	c = gt_parse_ffs_interface_create_attrs(attrs, dt);
	if (c < 0)
		goto out;

	dt->state = gt_ffs_build_descs_state(dt->state_file);
	if (dt->state == NULL)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(interface_create),
				(void *)dt, gt_ffs_interface_create_destructor);

	return;
out:
	gt_ffs_interface_create_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *gt_ffs_interface_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_ffs_interface_create, NULL, gt_ffs_interface_create_help},
		CMD_LIST_END
	};

	return commands;
}

static void gt_ffs_endpoint_create_destructor(void *data)
{
	struct gt_ffs_endpoint_create_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_ffs_endpoint_create_data *)data;
	if (dt->state != NULL)
		gt_ffs_cleanup_descs_state(dt->state, dt->state_file);

	free(dt);
}

static int gt_ffs_endpoint_create_help(void *data)
{
	printf("FFS endpoint create help func. Not implemented yet.\n");

	return -1;
}

unsigned long int *find_endpoint_attr(struct gt_ffs_endpoint_create_data *dt, const char *name, struct find_endpoint_attr *f)
{
	if (streq(name, "speed")) {
		f->speed_found = true;
		return &dt->speed;
	} else if (streq(name, "number")){
		f->number_found = true;
		return &dt->number;
	} else if (streq(name, "address")) {
		f->address_found = true;
		return &dt->address;
	} else if (streq(name, "type")) {
		f->type_found = true;
		return &dt->type;
	} else if (streq(name, "max")) {
		f->max_found = true;
		return &dt->max;
	} else if (streq(name, "interval")) {
		f->interval_found = true;
		return &dt->interval;
	}

	return NULL;
}

static int gt_parse_ffs_endpoint_create_attrs(struct gt_setting *attrs, struct gt_ffs_endpoint_create_data *dt)
{
	struct gt_setting *setting;
	struct find_endpoint_attr f = { false, false, false, false, false, false };

	for (setting = attrs; setting->variable; ++setting) {
		unsigned long int *field;
		unsigned long int val;
		char *endptr = NULL;

		field = find_endpoint_attr(dt, setting->variable, &f);
		if (field == NULL) {
			fprintf(stderr, "Unknown attribute passed: %s\n", setting->variable);
			return -1;
		}

		errno = 0;
		if (streq(setting->variable, "speed")) {
			char buf[3] = { '\0'};
			if (strlen(setting->value) != 2) {
				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				return -1;
			}
			buf[0] = toupper(setting->value[0]);
			buf[1] = toupper(setting->value[1]);
			if (streq(buf, "LS"))
				val = LS;
			else if (streq(buf, "FS"))
				val = FS;
			else if (streq(buf, "HS"))
				val = HS;
			else if (streq(buf, "SS"))
				val = SS;
			else {
				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				return -1;
			}
		} else if (streq(setting->variable, "type")) {
			char buf[5] = { '\0'};
			unsigned char c;
			if (strlen(setting->value) != 4) {
				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				return -1;
			}
			for (c = 0; c < 4; ++c)
				buf[c] = toupper(setting->value[c]);
			if (streq(buf, "BULK"))
				val = BULK;
			else if (streq(buf, "ISOC"))
				val = ISOC;
			else if (streq(buf, "INTR"))
				val = INTR;
			else {
				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				return -1;
			}
		} else { /* number, address, max, interval */
			val = strtoul(setting->value, &endptr, 0);
			if (errno
			    || *setting->value == 0
			    || (endptr && *endptr != 0)
			    || !(val <= (streq(setting->variable, "max") ? UINT16_MAX : UINT8_MAX))) {

				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				return -1;
			}
		}
		*field = val;
	}

	if (!f.speed_found) {
		fprintf(stderr, "Endpoint speed=<ls|fs|hs|ss> missing\n");
		return -1;
	}

	if (!f.number_found) {
		fprintf(stderr, "Number of associated interface missing\n");
		return -1;
	}

	if (!f.address_found) {
		fprintf(stderr, "Endpoint address=<val> missing\n");
		return -1;
	}

	if (!f.type_found) {
		fprintf(stderr, "Endpoint type=<bulk|int|iso> missing\n");
		return -1;
	}

	if (!f.max_found) {
		fprintf(stderr, "Endpoint max=<max packet size> missing\n");
		return -1;
	}

	if (!f.interval_found) {
		fprintf(stderr, "Endpoint interval=<val> missing\n");
		return -1;
	}

	return 0;
}

static void gt_parse_ffs_endpoint_create(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_ffs_endpoint_create_data *dt;
	struct gt_setting *attrs;
	int c;
	struct option opts[] = {
		{"state", required_argument, 0, 1},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};


	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	argv--;
	argc++;
	dt->state_file = gt_settings.default_ffs_descs;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "h", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 1:
			dt->state_file = optarg;
			break;
		case 'h':
			goto out;
			break;
		default:
			goto out;
		}
	}

	/* speed=ls|fs|hs|ss, address, type=bulk|int|iso, max, interval - all mandatory */
	if (argc - optind < 5)
		goto out;

	c = gt_parse_setting_list(&attrs, argc - optind, argv + optind);
	if (c < 0)
		goto out;

	c = gt_parse_ffs_endpoint_create_attrs(attrs, dt);
	if (c < 0)
		goto out;

	dt->state = gt_ffs_build_descs_state(dt->state_file);
	if (dt->state == NULL)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(endpoint_create),
				(void *)dt, gt_ffs_endpoint_create_destructor);

	return;
out:
	gt_ffs_endpoint_create_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *gt_ffs_endpoint_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_ffs_endpoint_create, NULL, gt_ffs_endpoint_create_help},
		CMD_LIST_END
	};

	return commands;
}

static void gt_ffs_language_create_destructor(void *data)
{
	struct gt_ffs_language_create_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_ffs_language_create_data *)data;
	if (dt->state != NULL)
		gt_ffs_cleanup_strs_state(dt->state, dt->state_file);

	free(dt);
}

static int gt_ffs_language_create_help(void *data)
{
	printf("FFS language create help func. Not implemented yet.\n");

	return -1;
}

static int gt_parse_ffs_language_create_attrs(struct gt_setting *attrs, struct gt_ffs_language_create_data *dt)
{
	struct gt_setting *setting;
	bool code_found = false;

	for (setting = attrs; setting->variable; ++setting) {
		unsigned long int val;
		char *endptr = NULL;

		if (!streq(setting->variable, "code")) {
			fprintf(stderr, "Unknown attribute passed: %s\n", setting->variable);
			return -1;
		}

		errno = 0;
		val = strtoul(setting->value, &endptr, 0);
		if (errno
		    || *setting->value == 0
		    || (endptr && *endptr != 0)
		    || !(val <= UINT16_MAX)) {

			fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
				setting->value, setting->variable);
			return -1;
		}
		dt->code = val;
		code_found = true;
	}

	if (!code_found) {
		fprintf(stderr, "Language code=<val> missing\n");
		return -1;
	}

	return 0;
}

static void gt_parse_ffs_language_create(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_ffs_language_create_data *dt;
	struct gt_setting *attrs;
	int c;
	struct option opts[] = {
		{"state", required_argument, 0, 1},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	argv--;
	argc++;
	dt->state_file = gt_settings.default_ffs_descs;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "h", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 1:
			dt->state_file = optarg;
			break;
		case 'h':
			goto out;
			break;
		default:
			goto out;
		}
	}

	/* only code=val is mandatory */
	if (argc - optind < 1)
		goto out;

	c = gt_parse_setting_list(&attrs, argc - optind, argv + optind);
	if (c < 0)
		goto out;

	c = gt_parse_ffs_language_create_attrs(attrs, dt);
	if (c < 0)
		goto out;

	dt->state = gt_ffs_build_strs_state(dt->state_file);
	if (dt->state == NULL)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(language_create),
				(void *)dt, gt_ffs_language_create_destructor);

	return;
out:
	gt_ffs_language_create_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *gt_ffs_language_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_ffs_language_create, NULL, gt_ffs_language_create_help},
		CMD_LIST_END
	};

	return commands;
}

static void gt_ffs_string_create_destructor(void *data)
{
	struct gt_ffs_string_create_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_ffs_string_create_data *)data;
	if (dt->state != NULL)
		gt_ffs_cleanup_strs_state(dt->state, dt->state_file);

	free(dt->str);
	free(dt);
}

static int gt_ffs_string_create_help(void *data)
{
	printf("FFS string create help func. Not implemented yet.\n");

	return -1;
}

static int gt_parse_ffs_string_create_attrs(struct gt_setting *attrs, struct gt_ffs_string_create_data *dt)
{
	struct gt_setting *setting;
	bool lang_found = false, str_found = false;

	for (setting = attrs; setting->variable; ++setting) {
		unsigned long int val;
		char *endptr = NULL;

		if (streq(setting->variable, "str")) {
			dt->str = strdup(setting->value);
			if (dt->str == NULL) {
				fprintf(stderr, "Cannot allocate memory for %s\n", setting->value);
				return -1;
			}
			str_found = true;
		} else if (streq(setting->variable, "lang")) {
			errno = 0;
			val = strtoul(setting->value, &endptr, 0);
			if (errno
			    || *setting->value == 0
			    || (endptr && *endptr != 0)
			    || !(val <= UINT16_MAX)) {

				fprintf(stderr, "Invalid value '%s' for attribute '%s'\n",
					setting->value, setting->variable);
				return -1;
			}
			dt->lang = val;
			lang_found = true;
		} else {
			fprintf(stderr, "Unknown attribute passed: %s\n", setting->variable);
			return -1;
		}
	}

	if (!lang_found) {
		fprintf(stderr, "String lang=<val> missing\n");
		return -1;
	}

	if (!str_found) {
		fprintf(stderr, "String str=<val> missing\n");
		return -1;
	}

	return 0;
}

static void gt_parse_ffs_string_create(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_ffs_string_create_data *dt;
	struct gt_setting *attrs;
	int c;
	struct option opts[] = {
		{"state", required_argument, 0, 1},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	argv--;
	argc++;
	dt->state_file = gt_settings.default_ffs_descs;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "h", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 1:
			dt->state_file = optarg;
			break;
		case 'h':
			goto out;
			break;
		default:
			goto out;
		}
	}

	/* code=val str=string mandatory */
	if (argc - optind < 2)
		goto out;

	c = gt_parse_setting_list(&attrs, argc - optind, argv + optind);
	if (c < 0)
		goto out;

	c = gt_parse_ffs_string_create_attrs(attrs, dt);
	if (c < 0)
		goto out;

	dt->state = gt_ffs_build_strs_state(dt->state_file);
	if (dt->state == NULL)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(string_create),
				(void *)dt, gt_ffs_string_create_destructor);

	return;
out:
	gt_ffs_string_create_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *gt_ffs_string_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"create", NEXT, gt_parse_ffs_string_create, NULL, gt_ffs_string_create_help},
		CMD_LIST_END
	};

	return commands;
}

static void gt_ffs_descriptors_load_destructor(void *data)
{
	struct gt_ffs_descriptors_load_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_ffs_descriptors_load_data *)data;
	if (dt->state != NULL)
		gt_ffs_cleanup_descs_state(dt->state, dt->state_file);

	free(dt);
}

static int gt_ffs_descriptors_load_help(void *data)
{
	printf("FFS descriptors load help func. Not implemented yet.\n");

	return -1;
}

static void gt_parse_ffs_descriptors_load(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_ffs_descriptors_load_data *dt;
	int c;
	struct option opts[] = {
		{"file", required_argument, 0, 1},
		{"stdin", no_argument, 0, 2},
		{"path", required_argument, 0, 3},
		{"state", required_argument, 0, 4},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	argv--;
	argc++;
	dt->state_file = gt_settings.default_ffs_descs;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "h", opts, &opt_index);
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
		case 4:
			dt->state_file = optarg;
			break;
		case 'h':
			goto out;
			break;
		default:
			goto out;
		}
	}

	if (dt->file || dt->opts & GT_STDIN) {
		/* no more arguments allowed */
		if (optind != argc)
			goto out;
	} else {
		/* exactly one argument expected */
		if (optind + 1 != argc)
			goto out;
		dt->name = argv[optind++];
	}

	dt->state = gt_ffs_build_descs_state(dt->state_file);
	if (dt->state == NULL)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(descriptors_load),
				(void *)dt, gt_ffs_descriptors_load_destructor);

	return;
out:
	gt_ffs_descriptors_load_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *gt_ffs_descriptors_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"load", NEXT, gt_parse_ffs_descriptors_load, NULL, gt_ffs_descriptors_load_help},
		CMD_LIST_END
	};

	return commands;
}

static void gt_ffs_strings_load_destructor(void *data)
{
	struct gt_ffs_strings_load_data *dt;

	if (data == NULL)
		return;

	dt = (struct gt_ffs_strings_load_data *)data;
	if (dt->state != NULL)
		gt_ffs_cleanup_strs_state(dt->state, dt->state_file);

	free(dt);
}

static int gt_ffs_strings_load_help(void *data)
{
	printf("FFS strings load help func. Not implemented yet.\n");

	return -1;
}

static void gt_parse_ffs_strings_load(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data)
{
	struct gt_ffs_strings_load_data *dt;
	int c;
	struct option opts[] = {
		{"file", required_argument, 0, 1},
		{"stdin", no_argument, 0, 2},
		{"path", required_argument, 0, 3},
		{"state", required_argument, 0, 4},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	dt = zalloc(sizeof(*dt));
	if (dt == NULL)
		goto out;

	argv--;
	argc++;
	dt->state_file = gt_settings.default_ffs_descs;
	while (1) {
		int opt_index = 0;
		c = getopt_long(argc, argv, "h", opts, &opt_index);
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
		case 4:
			dt->state_file = optarg;
			break;
		case 'h':
			goto out;
			break;
		default:
			goto out;
		}
	}

	if (dt->file || dt->opts & GT_STDIN) {
		/* no more arguments allowed */
		if (optind != argc)
			goto out;
	} else {
		/* exactly one argument expected */
		if (optind + 1 != argc)
			goto out;
		dt->name = argv[optind++];
	}

	dt->state = gt_ffs_build_strs_state(dt->state_file);
	if (dt->state == NULL)
		goto out;

	executable_command_set(exec, GET_EXECUTABLE(strings_load),
				(void *)dt, gt_ffs_strings_load_destructor);

	return;
out:
	gt_ffs_strings_load_destructor((void *)dt);
	executable_command_set(exec, cmd->printHelp, data, NULL);
}

const Command *gt_ffs_strings_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"load", NEXT, gt_parse_ffs_strings_load, NULL, gt_ffs_strings_load_help},
		CMD_LIST_END
	};

	return commands;
}

const Command *gt_ffs_get_children(const Command *cmd)
{
	static Command commands[] = {
		{"interface", NEXT, command_parse, gt_ffs_interface_get_children, gt_ffs_interface_help},
		{"endpoint", NEXT, command_parse, gt_ffs_endpoint_get_children, gt_ffs_endpoint_help},
		{"language", NEXT, command_parse, gt_ffs_language_get_children, gt_ffs_language_help},
		{"string", NEXT, command_parse, gt_ffs_string_get_children, gt_ffs_string_help},
		{"descriptors", NEXT, command_parse, gt_ffs_descriptors_get_children, gt_ffs_descriptors_help},
		{"strings", NEXT, command_parse, gt_ffs_strings_get_children, gt_ffs_strings_help},
		CMD_LIST_END
	};

	return commands;
}
