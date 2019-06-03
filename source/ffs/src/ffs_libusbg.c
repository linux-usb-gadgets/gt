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
#include <string.h>
#include <linux/usb/ch9.h>
#include <usbg/usbg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libconfig.h>

#include "common.h"
#include "backend.h"
#include "ffs.h"
#include "ffs_state.h"
#include "settings.h"

static int interface_create_func(void *data)
{
	struct gt_ffs_descs_state *state;
	struct gt_ffs_interface_create_data *dt, *dt_new;
	struct gt_ffs_link *item, **ptr;
	uint32_t *count;

	dt = (struct gt_ffs_interface_create_data *)data;
	state = dt->state;
	if (dt->speed == FS) {
		ptr = &state->fs_descs;
		count = &state->fs_count;
	} else if (dt->speed == HS) {
		ptr = &state->hs_descs;
		count = &state->hs_count;
	} else if (dt->speed == SS) {
		ptr = &state->ss_descs;
		count = &state->ss_count;
	} else
		return -1;

	while (*ptr) {
		if ((*ptr)->type == USB_DT_INTERFACE) {
			struct gt_ffs_interface_create_data *d = (*ptr)->data;
			if (d->number == dt->number) {
				fprintf(stderr, "Duplicate interface detected!\n");
				return -1;
			}
		}
		ptr = &(*ptr)->next;
	}

	item = zalloc(sizeof(*item));
	if (item == NULL)
		return -1;

	dt_new = zalloc(sizeof(*dt_new));
	if (dt_new == NULL)
		goto out_data;

	memcpy(dt_new, dt, sizeof(*dt_new));
	item->data = dt_new;
	item->type = USB_DT_INTERFACE;

	*ptr = item;
	++(*count);
	state->modified = true;

	return 0;

out_data:
	free(item);
	return -1;
}

static int endpoint_create_func(void *data)
{
	struct gt_ffs_descs_state *state;
	struct gt_ffs_endpoint_create_data *dt, *dt_new;
	struct gt_ffs_interface_create_data *d;
	struct gt_ffs_link *item, **last_iface, **ptr;
	uint32_t *count;

	dt = (struct gt_ffs_endpoint_create_data *)data;
	state = dt->state;
	if (dt->speed == FS) {
		ptr = &state->fs_descs;
		count = &state->fs_count;
	} else if (dt->speed == HS) {
		ptr = &state->hs_descs;
		count = &state->hs_count;
	} else if (dt->speed == SS) {
		ptr = &state->ss_descs;
		count = &state->ss_count;
	} else
		return -1;

	/* search for interface with matching number */
	last_iface = ptr;
	while (*last_iface) {
		if ((*last_iface)->type == USB_DT_INTERFACE) {
			d = (*last_iface)->data;
			if (d->number == dt->number)
				break;
		}
		last_iface = &(*last_iface)->next;
	}
	if (*last_iface == NULL) {
		fprintf(stderr, "Cannot find specified interface!\n");
		return -1;
	}

	/* start from one past last_iface and loop until end or next iface */
	ptr = &(*last_iface)->next;
	while (*ptr && (*ptr)->type != USB_DT_INTERFACE) {
		if ((*ptr)->type == USB_DT_ENDPOINT) {
			struct gt_ffs_endpoint_create_data *d = (*ptr)->data;
			if (d->address == dt->address) {
				fprintf(stderr, "Duplicate descriptor detected!\n");
				return -1;
			}
		}
		ptr = &(*ptr)->next;
	}

	item = zalloc(sizeof(*item));
	if (item == NULL)
		return -1;

	dt_new = zalloc(sizeof(*dt_new));
	if (dt_new == NULL)
		goto out_data;

	memcpy(dt_new, dt, sizeof(*dt_new));
	item->data = dt_new;
	item->type = USB_DT_ENDPOINT;

	item->next = *ptr;
	*ptr = item;
	++d->eps;
	++(*count);
	state->modified = true;

	return 0;

out_data:
	free(item);
	return -1;
}

static int language_create_func(void *data)
{
	struct gt_ffs_strs_state *state;
	struct gt_ffs_language_create_data *dt;
	struct gt_ffs_lang *dt_new;
	struct gt_ffs_link *item, **ptr;

	dt = (struct gt_ffs_language_create_data *)data;
	state = dt->state;

	ptr = &state->langs;
	while (*ptr) {
		struct gt_ffs_lang *l = (*ptr)->data;
		if (l->code == dt->code) {
			fprintf(stderr, "Duplicate language detected!\n");
			return -1;
		}
		ptr = &(*ptr)->next;
	}

	item = zalloc(sizeof(*item));
	if (item == NULL)
		return -1;

	dt_new = zalloc(sizeof(*dt_new));
	if (dt_new == NULL)
		goto out_data;

	dt_new->code = dt->code;
	item->data = dt_new;

	item->next = *ptr;
	*ptr = item;
	++state->lang_count;
	state->modified = true;

	return 0;
out_data:
	free(item);
	return -1;
}

static int string_create_func(void *data)
{
	struct gt_ffs_strs_state *state;
	struct gt_ffs_string_create_data *dt;
	struct gt_ffs_link *item, **ptr;
	struct gt_ffs_lang *lang;
	char *string;

	dt = (struct gt_ffs_string_create_data *)data;
	state = dt->state;

	ptr = &state->langs;
	while (*ptr) {
		lang = (*ptr)->data;
		if (lang->code == dt->lang)
			break;
		ptr = &(*ptr)->next;
	}
	if (*ptr == NULL) {
		fprintf(stderr, "Cannot find specified language!\n");
		return -1;
	}
	if ((*ptr)->next != NULL) {
		fprintf(stderr, "Only last language can be modified!\n");
		return -1;
	}
	/* The first language defines the number of strings */
	if (state->lang_count > 1 && lang->count >= state->str_count) {
		fprintf(stderr, "Cannot add string! Language full.\n");
		return -1;
	}

	ptr = &lang->strs;
	while (*ptr)
		ptr = &(*ptr)->next;

	item = zalloc(sizeof(*item));
	if (item == NULL)
		return -1;

	string = strdup(dt->str);
	if (string == NULL)
		goto out_data;

	item->data = string;
	*ptr = item;

	++lang->count;
	/* The first language defines the number of strings */
	if (state->lang_count == 1)
		++state->str_count;
	state->modified = true;

	return 0;
out_data:
	free(item);
	return -1;
}

static inline bool interface_attrs_equal(struct find_interface_attr *f1, struct find_interface_attr *f2)
{
	return f1->number_found == f2->number_found &&
		f1->cls_found == f2->cls_found &&
		f1->subcls_found == f2->subcls_found &&
		f1->protocol_found == f2->protocol_found &&
		f1->str_idx_found == f2->str_idx_found;
}

static inline bool endpoint_attrs_equal(struct find_endpoint_attr *f1, struct find_endpoint_attr *f2)
{
	return f1->number_found == f2->number_found &&
		f1->address_found == f2->address_found &&
		f1->type_found == f2->type_found &&
		f1->max_found == f2->max_found &&
		f1->interval_found == f2->interval_found;
}

#define MSG_AND_EXIT(format, args...)				\
	do {							\
		fprintf(stderr, format, ##args);		\
		return -1;					\
	} while (0)

#define UNKNOWN_SETTING(name, info)				\
	MSG_AND_EXIT("Uknown setting %s, line %d!\n", name,	\
			config_setting_source_line(info))

#define INCORRECT_SETTING(name, info)				\
	MSG_AND_EXIT("Incorrect setting %s, line %d!\n", name,	\
			config_setting_source_line(info))

#define DUPLICATE_SETTING(name, info)				\
	MSG_AND_EXIT("Duplicate setting %s, line %d\n", name,	\
			config_setting_source_line(info))

static int interface_load_state(int speed, int number, struct config_setting_t *i_info, struct gt_ffs_descs_state *state)
{
	struct gt_ffs_interface_create_data idt = { 0, 0, 0, 0, 0, 0, 0, 0 };
	config_setting_t *iface, *e_info, *endpoint;
	int iface_count, eps_count, endpoint_count, i, j, ret;
	bool endpoints_found = false;

	iface = config_setting_get_elem(i_info, number);
	if (!config_setting_is_group(iface))
		INCORRECT_SETTING("(expected interface description)", iface);

	idt.speed = speed;
	iface_count = config_setting_length(iface);
	for (i = 0; i < iface_count; ++i) {
		config_setting_t *i_field;
		const char *name;
		unsigned long int *field;
		struct find_interface_attr fi = {
			false, false, false, false, false, false, false
		};
		struct find_interface_attr fi_tmp = {
			false, false, false, false, false, false, false
		};

		i_field = config_setting_get_elem(iface, i);
		name = config_setting_name(i_field);

		if (config_setting_is_list(i_field)) {
			if (streq(name, "endpoints")) {
				if (endpoints_found)
					DUPLICATE_SETTING(name, i_field);
				endpoints_found = true;
				continue;
			} else
				UNKNOWN_SETTING(name, i_field);
		} else if (config_setting_is_number(i_field)) {
			if (streq(name, "speed"))
				UNKNOWN_SETTING(name, i_field);
		} else
			INCORRECT_SETTING(name, i_field);
		fi_tmp = fi;
		field = find_interface_attr(&idt, name, &fi_tmp);
		if (field == NULL)
			UNKNOWN_SETTING(name, i_field);
		if (interface_attrs_equal(&fi_tmp, &fi))
			DUPLICATE_SETTING(name, i_field);
		fi = fi_tmp;
		*field = config_setting_get_int(i_field);
	}
	idt.state = state;
	ret = interface_create_func(&idt);
	state->modified = false;
	if (ret != 0)
		return -1;

	if (!endpoints_found)
		return 0;

	e_info = config_setting_lookup(iface, "endpoints");
	eps_count = config_setting_length(e_info);
	for (i = 0; i < eps_count; ++i) {
		struct gt_ffs_endpoint_create_data edt = { 0, 0, 0, 0, 0, 0, 0 };
		struct find_endpoint_attr fe = {
			false, false, false, false, false, false
		};
		struct find_endpoint_attr fe_tmp = {
			false, false, false, false, false, false
		};

		endpoint = config_setting_get_elem(e_info, i);
		if (!config_setting_is_group(endpoint))
			INCORRECT_SETTING("(expected endpoint description)", endpoint);

		edt.speed = speed;
		edt.number = idt.number;
		endpoint_count = config_setting_length(endpoint);
		for (j = 0; j < endpoint_count; ++j) {
			config_setting_t *e_field;
			const char *name;
			unsigned long int *field;

			e_field = config_setting_get_elem(endpoint, j);
			name = config_setting_name(e_field);

			if (!config_setting_is_number(e_field))
				INCORRECT_SETTING(name, e_field);
			if (streq(name, "number"))
				UNKNOWN_SETTING(name, e_field);
			fe_tmp = fe;
			field = find_endpoint_attr(&edt, name, &fe_tmp);
			if (field == NULL)
				UNKNOWN_SETTING(name, e_field);
			if (endpoint_attrs_equal(&fe_tmp, &fe))
				DUPLICATE_SETTING(name, e_field);
			fe = fe_tmp;
			*field = config_setting_get_int(e_field);
		}
		edt.state = state;
		ret = endpoint_create_func(&edt);
		state->modified = false;
		if (ret != 0)
			return -1;
	}
	return 0;
}

static int descriptors_load_state(config_setting_t *cfg_root, struct gt_ffs_descs_state *state)
{
	int count, i, j, k;
	char *s_names[] = { "fs", "hs", "ss" };
	int speeds[] = { FS, HS, SS };
	bool s_found[] = { false, false, false };

	if (cfg_root == NULL)
		return -1;

	count = config_setting_length(cfg_root);
	if (count < 1)
		return -1;

	for (i = 0; i < count; ++i) {
		const char *s_name;
		int s_count, speed;
		bool i_found = false;
		config_setting_t *s_info;

		s_info = config_setting_get_elem(cfg_root, i);
		s_name = config_setting_name(s_info);

		if (!config_setting_is_group(s_info))
			INCORRECT_SETTING(s_name, s_info);

		if (!(streq(s_name, "fs") || streq(s_name, "hs") || streq(s_name, "ss")))
			UNKNOWN_SETTING(s_name, s_info);

		for (j = 0; j < ARRAY_SIZE(s_names); ++j)
			if (streq(s_name, s_names[j])) {
				if (s_found[j])
					DUPLICATE_SETTING(s_names[j], s_info);
				s_found[j] = true;
				speed = speeds[j];
				break;
			}

		s_count = config_setting_length(s_info);
		for (j = 0; j < s_count; ++j) {
			const char *i_name;
			int i_count;
			config_setting_t *i_info;

			i_info = config_setting_get_elem(s_info, j);
			i_name = config_setting_name(i_info);

			if (!config_setting_is_list(i_info))
				INCORRECT_SETTING(i_name, i_info);

			if (!streq(i_name, "interfaces"))
				UNKNOWN_SETTING(i_name, i_info);

			if (i_found)
				DUPLICATE_SETTING(i_name, i_info);
			i_found = true;

			i_count = config_setting_length(i_info);
			for (k = 0; k < i_count; ++k)
				if (interface_load_state(speed, k, i_info, state))
					return -1;
		}
	}

	return 0;
}

static int language_load_state(config_setting_t *language, struct gt_ffs_strs_state *state)
{
	struct gt_ffs_language_create_data ldt = { 0, 0 };
	config_setting_t *s_info;
	int count, i, ret;
	bool strs_found = false, code_found = false;

	count = config_setting_length(language);
	if (count < 1)
		return -1;

	for (i = 0; i < count; ++i) {
		config_setting_t *l_field;
		const char *name;

		l_field = config_setting_get_elem(language, i);
		name = config_setting_name(l_field);

		if (config_setting_is_list(l_field)) {
			if (streq(name, "strs")) {
				if (strs_found)
					DUPLICATE_SETTING(name, l_field);
				strs_found = true;
				continue;
			} else
				UNKNOWN_SETTING(name, l_field);
		} else if (config_setting_is_number(l_field)) {
			if (!streq(name, "code"))
				UNKNOWN_SETTING(name, l_field);
			else if (code_found)
				DUPLICATE_SETTING(name, l_field);
			else
				code_found = true;
		} else
			INCORRECT_SETTING(name, l_field);
		ldt.code = config_setting_get_int(l_field);
	}
	ldt.state = state;
	ret = language_create_func(&ldt);
	state->modified = false;
	if (ret != 0)
		return -1;

	if (!strs_found)
		return 0;

	s_info = config_setting_lookup(language, "strs");
	count = config_setting_length(s_info);
	for (i = 0; i < count; ++i) {
		struct gt_ffs_string_create_data sdt = { 0, 0, NULL };
		config_setting_t *str;

		str = config_setting_get_elem(s_info, i);
		if (!config_setting_is_scalar(str))
			INCORRECT_SETTING("(string expected)", str);

		sdt.str = strdup(config_setting_get_string(str));
		if (sdt.str == NULL)
			return -1;

		sdt.state = state;
		sdt.lang = ldt.code;
		ret = string_create_func(&sdt);
		state->modified = false;
		free(sdt.str);
		if (ret != 0)
			return -1;
	}

	return 0;
}

static int strings_load_state(config_setting_t *cfg_root, struct gt_ffs_strs_state *state)
{
	int count, i, j;
	bool l_found = false;
	config_setting_t *l_info;

	count = config_setting_length(cfg_root);
	if (count < 1)
		return -1;

	for (i = 0; i < count; ++i) {
		const char *l_name;
		int l_count;

		l_info = config_setting_get_elem(cfg_root, i);
		l_name = config_setting_name(l_info);

		if (!config_setting_is_list(l_info))
			INCORRECT_SETTING(l_name, l_info);

		if (!(streq(l_name, "languages")))
			UNKNOWN_SETTING(l_name, l_info);

		if (l_found)
			DUPLICATE_SETTING(l_name, l_info);
		l_found = true;

		l_count = config_setting_length(l_info);
		for (j = 0; j < l_count; ++j) {
			config_setting_t *language;

			language = config_setting_get_elem(l_info, j);

			if (!config_setting_is_group(language))
				INCORRECT_SETTING("(language description expected)", language);

			if (language_load_state(language, state))
				return -1;
		}
	}

	return 0;
}

#undef DUPLICATE_SETTING
#undef INCORRECT_SETTING
#undef UNKNOWN_SETTING
#undef MSG_AND_EXIT

static int descriptors_load_func(void *data)
{
	FILE *fp = NULL;
	config_t cfg;
	struct gt_ffs_descriptors_load_data *dt;
	const char *filename = NULL;
	const char **ptr;
	struct stat st;
	char buf[PATH_MAX];
	int ret;

	dt = (struct gt_ffs_descriptors_load_data *)data;

	if (dt->opts & GT_STDIN) {
		fp = stdin;
	} else if (dt->file) {
		filename = dt->file;
	} else if (dt->path) {
		ret = snprintf(buf, sizeof(buf), "%s/%s", dt->path, dt->name);
		if (ret >= sizeof(buf)) {
			fprintf(stderr, "path too long\n");
			return -1;
		}

		filename = buf;
	} else {
		if (gt_settings.lookup_path != NULL) {
			ptr = gt_settings.lookup_path;
			while (*ptr) {
				ret = snprintf(buf, sizeof(buf), "%s/%s", *ptr, dt->name);
				if (ret >= sizeof(buf)) {
					fprintf(stderr, "path too long\n");
					return -1;
				}

				if (stat(buf, &st) == 0) {
					filename = buf;
					break;
				}

				ptr++;
			}
		}

		/* use current directory as path */
		if (filename == NULL && stat(dt->name, &st) == 0)
			filename = dt->name;
	}

	if (filename == NULL && fp == NULL) {
		fprintf(stderr, "Could not find matching descriptors file.\n");
		return -1;
	}

	if (fp == NULL) {
		fp = fopen(filename, "r");
		if (fp == NULL) {
			perror("Error opening file");
			return -1;
		}
	}

	config_init(&cfg);
	ret = config_read(&cfg, fp);
	if (ret == CONFIG_FALSE) {
		fprintf(stderr, "%d:%s\n",
			config_error_line(&cfg), config_error_text(&cfg));
		goto out;
	}

	ret = descriptors_load_state(config_root_setting(&cfg), dt->state);
	if (ret == 0)
		dt->state->modified = true;
	config_destroy(&cfg);

out:
	if (fp != stdin)
		fclose(fp);

	return ret;
}

static int strings_load_func(void *data)
{
	FILE *fp = NULL;
	config_t cfg;
	struct gt_ffs_strings_load_data *dt;
	const char *filename = NULL;
	const char **ptr;
	struct stat st;
	char buf[PATH_MAX];
	int ret;

	dt = (struct gt_ffs_strings_load_data *)data;

	if (dt->opts & GT_STDIN) {
		fp = stdin;
	} else if (dt->file) {
		filename = dt->file;
	} else if (dt->path) {
		ret = snprintf(buf, sizeof(buf), "%s/%s", dt->path, dt->name);
		if (ret >= sizeof(buf)) {
			fprintf(stderr, "path too long\n");
			return -1;
		}

		filename = buf;
	} else {
		if (gt_settings.lookup_path != NULL) {
			ptr = gt_settings.lookup_path;
			while (*ptr) {
				ret = snprintf(buf, sizeof(buf), "%s/%s", *ptr, dt->name);
				if (ret >= sizeof(buf)) {
					fprintf(stderr, "path too long\n");
					return -1;
				}

				if (stat(buf, &st) == 0) {
					filename = buf;
					break;
				}

				ptr++;
			}
		}

		/* use current directory as path */
		if (filename == NULL && stat(dt->name, &st) == 0)
			filename = dt->name;
	}

	if (filename == NULL && fp == NULL) {
		fprintf(stderr, "Could not find matching strings file.\n");
		return -1;
	}

	if (fp == NULL) {
		fp = fopen(filename, "r");
		if (fp == NULL) {
			perror("Error opening file");
			return -1;
		}
	}

	config_init(&cfg);
	ret = config_read(&cfg, fp);
	if (ret == CONFIG_FALSE) {
		fprintf(stderr, "%d:%s\n",
			config_error_line(&cfg), config_error_text(&cfg));
		goto out;
	}

	ret = strings_load_state(config_root_setting(&cfg), dt->state);
	if (ret == 0)
		dt->state->modified = true;
	config_destroy(&cfg);

out:
	if (fp != stdin)
		fclose(fp);

	return ret;
}

struct gt_ffs_backend gt_ffs_backend_libusbg = {
	.interface_create = interface_create_func,
	.endpoint_create = endpoint_create_func,
	.language_create = language_create_func,
	.string_create = string_create_func,
	.descriptors_load = descriptors_load_func,
	.strings_load = strings_load_func,
};
