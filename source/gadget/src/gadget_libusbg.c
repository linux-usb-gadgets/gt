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

#include <usbg/usbg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

#include "gadget.h"
#include "backend.h"
#include "common.h"
#include "settings.h"
#include "function.h"
#include "configuration.h"

#ifndef WITH_GADGETD
#define G_N_ELEMENTS(arr)	(sizeof(arr) / sizeof((arr)[0]))
#endif

/**
 * @brief Get implicite gadget
 * @param[in] s Usbg state
 * @return Default gadget if exists, only gadget when only one gadget exists,
 * or NULL if cannot select an implicite gadget.
 */
static usbg_gadget *get_implicite_gadget(usbg_state *s) {
	usbg_gadget *g;

	g = usbg_get_first_gadget(s);
	if (usbg_get_next_gadget(g) == NULL)
		return g;

	if (gt_settings.default_gadget)
		g = usbg_get_gadget(backend_ctx.libusbg_state,
				gt_settings.default_gadget);

	return g;
}

static int create_func(void *data)
{
	struct gt_gadget_create_data *dt;
	int i;
	int r;

	usbg_gadget *g;

	dt = (struct gt_gadget_create_data *)data;

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
		if (dt->attr_val[i] == -1)
			continue;

		r = usbg_set_gadget_attr(g, i, dt->attr_val[i]);
		if (r != USBG_SUCCESS) {
			fprintf(stderr, "Unable to set attribute %s: %s\n",
				usbg_get_gadget_attr_str(i),
				usbg_strerror(r));
			goto err_usbg;
		}
	}

	for (i = 0; i < G_N_ELEMENTS(dt->str_val); i++) {
		if (dt->str_val[i] == NULL)
			continue;

		r = gadget_strs[i].set_fn(g, LANG_US_ENG, dt->str_val[i]);
		if (r != USBG_SUCCESS) {
			fprintf(stderr, "Unable to set string %s: %s\n",
				gadget_strs[i].name,
				usbg_strerror(r));
			goto err_usbg;
		}
	}

	return 0;

err_usbg:
	usbg_rm_gadget(g, USBG_RM_RECURSE);
	return -1;
}

static int rm_func(void *data)
{
	struct gt_gadget_rm_data *dt;
	int usbg_ret;
	usbg_udc *u;
	usbg_gadget *g;
	int opts = 0;

	dt = (struct gt_gadget_rm_data *)data;

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->name);
	if (g == NULL) {
		fprintf(stderr, "Gadget '%s' not found\n", dt->name);
		goto err;
	}

	u = usbg_get_gadget_udc(g);

	if (u) {
		if (!(dt->opts & GT_FORCE)) {
			fprintf(stderr, "Gadget is enabled, disable it first or use --force option\n");
			goto err;
		}

		usbg_ret = usbg_disable_gadget(g);
		if (usbg_ret != USBG_SUCCESS) {
			fprintf(stderr, "Error on disable gadget: %s : %s\n",
				usbg_error_name(usbg_ret), usbg_strerror(usbg_ret));
			goto err;
		}
	}

	if (dt->opts & GT_RECURSIVE)
		opts |= USBG_RM_RECURSE;

	usbg_ret = usbg_rm_gadget(g, opts);
	if (usbg_ret != USBG_SUCCESS){
		fprintf(stderr, "Error on gadget remove: %s : %s\n",
			usbg_error_name(usbg_ret), usbg_strerror(usbg_ret));
		goto err;
	}

	return 0;
err:
	return -1;
}

static int enable_func(void *data)
{
	struct gt_gadget_enable_data *dt;

	dt = (struct gt_gadget_enable_data *)data;

	usbg_gadget *g;
	usbg_udc *udc = NULL;
	int usbg_ret;

	if (dt->udc != NULL) {
		udc = usbg_get_udc(backend_ctx.libusbg_state, dt->udc);
		if (udc == NULL) {
			fprintf(stderr, "Failed to get udc\n");
			return -1;
		}
	}

	if (dt->gadget) {
		g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
		if (g == NULL) {
			fprintf(stderr, "Failed to get gadget\n");
			return -1;
		}
	} else {
		g = get_implicite_gadget(backend_ctx.libusbg_state);
		if (g == NULL) {
			fprintf(stderr, "Gadget not specified\n");
			return -1;
		}
	}

	usbg_ret = usbg_enable_gadget(g, udc);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Failed to enable gadget %s\n", usbg_strerror(usbg_ret));
		return -1;
	}

	return 0;
}

static int disable_func(void *data)
{
	struct gt_gadget_disable_data *dt;

	usbg_gadget *g;
	usbg_udc *u;
	int usbg_ret;

	dt = (struct gt_gadget_disable_data *)data;

	if (dt->gadget) {
		g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
		if (g == NULL) {
			fprintf(stderr, "Gadget '%s' not found\n", dt->gadget);
			return -1;
		}
	} else if (dt->udc) {
		u = usbg_get_udc(backend_ctx.libusbg_state, dt->udc);
		if (u == NULL) {
			fprintf(stderr, "UDC '%s' not found\n", dt->udc);
			return -1;
		}

		g = usbg_get_udc_gadget(u);
		if (g == NULL) {
			fprintf(stderr, "No gadget enabled on this UDC\n");
			return -1;
		}
	} else {
		g = get_implicite_gadget(backend_ctx.libusbg_state);
		if (g == NULL) {
			fprintf(stderr, "Gadget not specified\n");
			return -1;
		}
	}

	usbg_ret = usbg_disable_gadget(g);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error on disable gadget: %s : %s\n",
			usbg_error_name(usbg_ret), usbg_strerror(usbg_ret));
		return -1;
	}

	return 0;
}

static void print_gadget_attrs(struct usbg_gadget_attrs *g_attrs, int *mask) {
	if (!mask || mask[USBG_BCD_USB])
		printf("  bcdUSB\t\t%x.%02x\n",
			g_attrs->bcdUSB >> 8,
			g_attrs->bcdUSB & 0x00ff);

	if (!mask || mask[USBG_B_DEVICE_CLASS])
		printf("  bDeviceClass\t\t0x%02x\n", g_attrs->bDeviceClass);
	if (!mask || mask[USBG_B_DEVICE_SUB_CLASS])
		printf("  bDeviceSubClass\t0x%02x\n", g_attrs->bDeviceSubClass);
	if (!mask || mask[USBG_B_DEVICE_PROTOCOL])
		printf("  bDeviceProtocol\t0x%02x\n", g_attrs->bDeviceProtocol);
	if (!mask || mask[USBG_B_MAX_PACKET_SIZE_0])
		printf("  bMaxPacketSize0\t%d\n", g_attrs->bMaxPacketSize0);
	if (!mask || mask[USBG_ID_VENDOR])
		printf("  idVendor\t\t0x%04x\n", g_attrs->idVendor);
	if (!mask || mask[USBG_ID_PRODUCT])
		printf("  idProduct\t\t0x%04x\n", g_attrs->idProduct);
	if (!mask || mask[USBG_BCD_DEVICE])
		printf("  bcdDevice\t\t%x.%02x\n",
			g_attrs->bcdDevice >> 8,
			g_attrs->bcdDevice & 0x00ff);
}

static int get_func(void *data)
{
	struct gt_gadget_get_data *dt;

	usbg_gadget *g;
	struct usbg_gadget_attrs g_attrs;
	int usbg_ret;

	dt = (struct gt_gadget_get_data *)data;

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->name);
	if (g == NULL) {
		fprintf(stderr, "Gadget '%s' not found\n", dt->name);
		return -1;
	}

	usbg_ret = usbg_get_gadget_attrs(g, &g_attrs);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
			usbg_strerror(usbg_ret));
		return -1;
	}

	print_gadget_attrs(&g_attrs, dt->attrs);

	return 0;
}

static int print_gadget(usbg_gadget *g, int opts)
{
	struct usbg_gadget_attrs g_attrs;
	usbg_udc *u;
	usbg_function *f;
	usbg_config *c;
	int usbg_ret;
	const char *name;

	usbg_ret = usbg_get_gadget_attrs(g, &g_attrs);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error: %s : %s\n",
			usbg_error_name(usbg_ret), usbg_strerror(usbg_ret));
		return -1;
	}

	name = usbg_get_gadget_name(g);
	if (name == NULL) {
		fprintf(stderr, "Unable to get gadget name\n");
		return -1;
	}

	if (opts & GT_QUIET) {
		printf("%s\n", name);
	} else {
		printf("ID %04x:%04x '%s'", g_attrs.idVendor,
				g_attrs.idProduct, name);

		u = usbg_get_gadget_udc(g);
		if (u) {
			printf("\t%s", usbg_get_udc_name(u));
		}

		putchar('\n');
	}

	if (opts & GT_VERBOSE)
		print_gadget_attrs(&g_attrs, NULL);

	if (opts & GT_RECURSIVE) {
		usbg_for_each_function(f, g) {
			gt_print_function_libusbg(f, opts);
		}

		usbg_for_each_config(c, g) {
			gt_print_config_libusbg(c, opts);
		}
	}

	return 0;
}

static int gadget_func(void *data)
{
	struct gt_gadget_gadget_data *dt;

	usbg_gadget *g;
	int usbg_ret = 0;

	dt = (struct gt_gadget_gadget_data *)data;

	if (dt->name) {
		g = usbg_get_gadget(backend_ctx.libusbg_state, dt->name);
		if (g == NULL) {
			fprintf(stderr, "Gadget '%s' not found\n", dt->name);
			return -1;
		}

		usbg_ret = print_gadget(g, dt->opts);
		if (usbg_ret < 0)
			goto out;

	} else {
		usbg_for_each_gadget(g, backend_ctx.libusbg_state) {
			usbg_ret = print_gadget(g, dt->opts);
			if (usbg_ret < 0)
				goto out;
		}
	}

out:
	return usbg_ret;
}

static int load_func(void *data)
{
	FILE *fp = NULL;
	struct gt_gadget_load_data *dt;
	const char *filename = NULL;
	const char **ptr;
	struct stat st;
	char buf[PATH_MAX];
	usbg_gadget *g;
	int ret;

	dt = (struct gt_gadget_load_data *)data;

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
		fprintf(stderr, "Could not find matching gadget file.\n");
		return -1;
	}

	if (fp == NULL) {
		fp = fopen(filename, "r");
		if (fp == NULL) {
			perror("Error opening file");
			return -1;
		}
	}

	ret = usbg_import_gadget(backend_ctx.libusbg_state, fp, dt->gadget_name, &g);
	if (ret != USBG_SUCCESS) {
		fprintf(stderr, "Error on import gadget\n");
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(ret),
				usbg_strerror(ret));
		if (ret == USBG_ERROR_INVALID_FORMAT)
			fprintf(stderr, "Line: %d. Error: %s\n",
				usbg_get_gadget_import_error_line(backend_ctx.libusbg_state),
				usbg_get_gadget_import_error_text(backend_ctx.libusbg_state));
		goto out;
	}

	if (!(dt->opts & GT_OFF)) {
		ret = usbg_enable_gadget(g, NULL);
		if (ret != USBG_SUCCESS) {
			fprintf(stderr, "Failed to enable gadget %s\n", usbg_strerror(ret));
			goto out;
		}
	}

out:
	if (fp != stdin)
		fclose(fp);

	return ret;
}

static int save_func(void *data)
{
	struct gt_gadget_save_data *dt;
	const char *filename = NULL;
	FILE *fp = NULL;
	char buf[PATH_MAX];
	usbg_gadget *g;
	struct stat st;
	int ret;

	dt = (struct gt_gadget_save_data *)data;

	if (dt->opts & GT_STDOUT) {
		fp = stdout;
	} else if (dt->file) {
		filename = dt->file;
	} else if (dt->path) {
		ret = snprintf(buf, sizeof(buf), "%s/%s", dt->path, dt->name);
		if (ret >= sizeof(buf)) {
			fprintf(stderr, "path too long\n");
			return -1;
		}

		filename = buf;
	} else if (gt_settings.default_template_path) {
		ret = snprintf(buf, sizeof(buf), "%s/%s",
				gt_settings.default_template_path, dt->name);
		if (ret >= sizeof(buf)) {
			fprintf(stderr, "path too long\n");
			return -1;
		}

		filename = buf;
	} else {
		fprintf(stderr, "Path not specified\n");
		return -1;
	}

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->gadget);
	if (!g) {
		fprintf(stderr, "Error on get gadget\n");
		return -1;
	}

	if (fp == NULL) {
		if (!(dt->opts & GT_FORCE) && stat(filename, &st) == 0) {
			fprintf(stderr, "File exists.\n");
			return -1;
		}

		fp = fopen(filename, "w");
		if (fp == NULL) {
			perror("Error opening file\n");
			return -1;
		}
	}

	ret = usbg_export_gadget(g, fp);
	if (ret != USBG_SUCCESS) {
		fprintf(stderr, "Error on export gadget\n");
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(ret),
				usbg_strerror(ret));
		goto out;
	}

out:
	fclose(fp);
	return ret;
}

static int set_func(void *data)
{
	struct gt_gadget_set_data *dt;
	int i;
	usbg_gadget *g;
	int ret;

	dt = (struct gt_gadget_set_data *)data;

	g = usbg_get_gadget(backend_ctx.libusbg_state, dt->name);
	if (!g) {
		fprintf(stderr, "Error on get gadget\n");
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(dt->attr_val); ++i) {
		if (dt->attr_val[i] < 0)
			continue;

		ret = usbg_set_gadget_attr(g, i, dt->attr_val[i]);
		if (ret != USBG_SUCCESS) {
			fprintf(stderr, "Unable to set attribute %s: %s\n",
				usbg_get_gadget_attr_str(i),
				usbg_strerror(ret));
			return -1;
		}
	}

	for (i = 0; i < G_N_ELEMENTS(dt->str_val); i++) {
		if (dt->str_val[i] == NULL)
			continue;

		ret = gadget_strs[i].set_fn(g, LANG_US_ENG, dt->str_val[i]);
		if (ret != USBG_SUCCESS) {
			fprintf(stderr, "Unable to set string %s: %s\n",
				gadget_strs[i].name,
				usbg_strerror(ret));
			return -1;
		}
	}

	return 0;
}

int template_filter(const struct dirent *entry)
{
	if (entry->d_name[0] == '.')
		return 0;

	return 1;
}

static int template_func(void *data)
{
	struct gt_gadget_template_data *dt;
	struct dirent **dentry;
	char buf[PATH_MAX];
	int ret;
	const char **ptr;

	dt = (struct gt_gadget_template_data *)data;

	if (gt_settings.lookup_path != NULL) {
		ptr = gt_settings.lookup_path;
		while (*ptr) {
			ret = snprintf(buf, sizeof(buf), "%s/%s", *ptr, dt->name);
			if (ret >= sizeof(buf)) {
				fprintf(stderr, "path too long\n");
				return -1;
			}

			ret = scandir(*ptr, &dentry, template_filter, alphasort);
			if (ret < 0) {
				perror("Error reading directory");
				return -1;
			}

			while (ret--) {
				printf("%s\n", dentry[ret]->d_name);
				free(dentry[ret]);
			}

			free(dentry);

			ptr++;
		}
	}

	return 0;
}

struct gt_gadget_backend gt_gadget_backend_libusbg = {
	.create = create_func,
	.rm = rm_func,
	.get = get_func,
	.set = set_func,
	.enable = enable_func,
	.disable = disable_func,
	.gadget = gadget_func,
	.load = load_func,
	.save = save_func,
	.template_default = template_func,
	.template_get = NULL,
	.template_set = NULL,
	.template_rm = NULL,
};
