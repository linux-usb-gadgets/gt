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
#include <endian.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/usb/ch9.h>
#include <linux/usb/functionfs.h>

#include "ffs_state.h"
#include "common.h"

#define read_and_advance(off, ptr, size) 				\
	do {								\
		if (*(off) + sizeof(*(ptr)) > (size)) {			\
			fprintf(stderr, "Unexpected end of file!\n");	\
			return -1;					\
		}							\
									\
		(ptr) = (buf) + *(off);					\
		*(off) += sizeof(*(ptr));				\
	} while (0)

static int verify_le32(void *buf, off_t *off, off_t size, uint32_t expected)
{
	uint32_t *ptr;

	read_and_advance(off, ptr, size);

	return le32toh(*ptr) == expected;
}

static int get_le32(void *buf, off_t *off, off_t size, uint32_t *result)
{
	uint32_t *ptr;

	read_and_advance(off, ptr, size);

	*result = le32toh(*ptr);

	return 0;
}

#define CONSUME32(expected, msg)						\
	do {								\
		int ret = verify_le32(buf, &off, size, (expected));	\
		if (ret < 0)						\
			goto out;					\
									\
		if (ret == 0) {						\
			fprintf(stderr, (msg));				\
			goto out;					\
		}							\
	} while (0)

#define READ32(result)							\
	do {								\
		int ret = get_le32(buf, &off, size, (&result));		\
		if (ret < 0)						\
			goto out;					\
	} while (0)

static int get_le8(void *buf, off_t *off, off_t size, uint8_t *result)
{
	uint8_t *ptr;

	read_and_advance(off, ptr, size);

	*result = *ptr;

	return 0;
}

#define READ8(result)							\
	do {								\
		int ret = get_le8(buf, off, size, (&result));		\
		if (ret < 0)						\
			goto out;					\
	} while (0)

static int get_le16(void *buf, off_t *off, off_t size, uint16_t *result)
{
	uint16_t *ptr;

	read_and_advance(off, ptr, size);

	*result = *ptr;

	return 0;
}

#define READ16(result)							\
	do {								\
		int ret = get_le16(buf, off, size, (&result));		\
		if (ret < 0)						\
			goto out;					\
	} while (0)

static int gt_ffs_parse_interface(void *buf, off_t *off, off_t size, struct gt_ffs_descs_state *state, struct gt_ffs_link **last_iface, int speed)
{
	/* number, cls, subcls, protocol, str_idx */
	struct gt_ffs_link *item, **ptr;
	struct gt_ffs_interface_create_data *data;
	uint8_t val8;

	item = zalloc(sizeof(*item));
	if (item == NULL)
		return -1;

	data = zalloc(sizeof(*data));
	if (data == NULL)
		goto out_data;
	item->type = USB_DT_INTERFACE;
	item->data = data;

	data->speed = speed;
	READ8(val8);
	data->number = val8;
	READ8(val8); /* alt */
	READ8(val8);
	data->eps = val8;
	READ8(val8);
	data->cls = val8;
	READ8(val8);
	data->subcls = val8;
	READ8(val8);
	data->protocol = val8;
	READ8(val8);
	data->str_idx = val8;

	if (speed == FS)
		ptr = &state->fs_descs;
	else if (speed == HS)
		ptr = &state->hs_descs;
	else if (speed == SS)
		ptr = &state->ss_descs;
	else
		goto out;

	while (*ptr) {
		if ((*ptr)->type == item->type) {
			struct gt_ffs_interface_create_data *d = (*ptr)->data;
			if (d->number == data->number) {
				fprintf(stderr, "Duplicate descriptor detected!\n");
				goto out;
			}
		}
		ptr = &(*ptr)->next;
	}
	*ptr = item;
	*last_iface = item;

	return 0;

out:
	free(data);
out_data:
	free(item);
	return -1;
}

static int gt_ffs_parse_endpoint(void *buf, off_t *off, off_t size, struct gt_ffs_descs_state *state, struct gt_ffs_link **last_iface, int speed)
{
	/* address, type, max, interval */
	struct gt_ffs_link *item, **ptr;
	struct gt_ffs_endpoint_create_data *data;
	uint8_t val8;
	uint16_t val16;

	if (*last_iface == NULL)
		return -1;

	item = zalloc(sizeof(*item));
	if (item == NULL)
		return -1;

	data = zalloc(sizeof(*data));
	if (data == NULL)
		goto out_data;
	item->type = USB_DT_ENDPOINT;
	item->data = data;

	data->speed = speed;
	READ8(val8);
	data->address = val8;
	READ8(val8);
	data->type = val8;
	READ16(val16);
	data->max = val16;
	READ8(val8);
	data->interval = val8;

	/* start from one past last_iface and loop until end or next iface */
	ptr = &(*last_iface)->next;
	while (*ptr && (*ptr)->type != USB_DT_INTERFACE) {
		if ((*ptr)->type == item->type) {
			struct gt_ffs_endpoint_create_data *d = (*ptr)->data;
			if (d->address == data->address) {
				fprintf(stderr, "Duplicate descriptor detected!\n");
				goto out;
			}
		}
		ptr = &(*ptr)->next;
	}
	item->next = *ptr;
	*ptr = item;

	return 0;

out:
	free(data);
out_data:
	free(item);
	return -1;
}

static int gt_ffs_parse_speed(void *buf, off_t *off, off_t size, struct gt_ffs_descs_state *state, struct gt_ffs_link **last_iface, int speed)
{
	uint8_t val8;

	READ8(val8);
	READ8(val8);
	if (val8 == USB_DT_INTERFACE)
		return gt_ffs_parse_interface(buf, off, size, state, last_iface, speed);
	else if (val8 == USB_DT_ENDPOINT)
		return gt_ffs_parse_endpoint(buf, off, size, state, last_iface, speed);
	
	fprintf(stderr, "Unsupported descriptor type: %02x\n", val8);

out:
	return -1;
}

static int gt_ffs_parse_desc_file(int fd, off_t size, struct gt_ffs_descs_state *state)
{
	void *buf;
	off_t off = 0;
	int ret = -1;
	uint32_t val32;
	uint32_t cnt[] = {0, 0, 0};
	struct gt_ffs_link *last_iface = NULL;
	int i;
	
	buf = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (buf == MAP_FAILED)
		return -1;
	
	CONSUME32(FUNCTIONFS_DESCRIPTORS_MAGIC_V2, "Incorrect magic value!\n");
	CONSUME32(size, "Incorrect file length!\n");
	READ32(val32);
	if (val32 & ~(FUNCTIONFS_HAS_FS_DESC | FUNCTIONFS_HAS_HS_DESC | FUNCTIONFS_HAS_SS_DESC)) {
		fprintf(stderr, "Unsupported FunctionFS flags detected!\n");

		goto out;
	}
	state->flags = val32;

	for (i = 0; i < ARRAY_SIZE(cnt); ++i) {
		if (!(state->flags & (1 << i)))
			continue;

		READ32(val32);
		cnt[i] = val32;
	}
	state->fs_count = cnt[0];
	state->hs_count = cnt[1];
	state->ss_count = cnt[2];

	for (i = 0; i < state->fs_count; ++i)
		if (gt_ffs_parse_speed(buf, &off, size, state, &last_iface, FS))
			goto out;

	for (i = 0; i < state->hs_count; ++i)
		if (gt_ffs_parse_speed(buf, &off, size, state, &last_iface, HS))
			goto out;

	for (i = 0; i < state->ss_count; ++i)
		if (gt_ffs_parse_speed(buf, &off, size, state, &last_iface, SS))
			goto out;

	ret = 0;
out:	
	munmap(buf, size);
	return ret;
}

#undef READ8
#undef READ32
#undef CONSUME32

#if 0
static void print_state(struct gt_ffs_state *state)
{
	struct gt_ffs_link *ptr, *arr[3];
	int i;

	printf("flags:%02x\n", state->flags);
	printf("fs_count:%d\n", state->fs_count);
	printf("hs_count:%d\n", state->hs_count);
	printf("ss_count:%d\n", state->ss_count);

	arr[0] = state->fs_descs;
	arr[1] = state->hs_descs;
	arr[2] = state->ss_descs;

	for (i = 0; i < ARRAY_SIZE(arr); ++i) {
		ptr = arr[i];
		while (ptr) {
			if (ptr->type == USB_DT_INTERFACE) {
				struct gt_ffs_interface_create_data *d;

				d = ptr->data;
				printf("%02x|%02x %02x %02x %02x %02x %02x %02x\n", d->speed, ptr->type, d->number, d->eps, d->cls, d->subcls, d->protocol, d->str_idx);
			} else {
				struct gt_ffs_endpoint_create_data *d;

				d = ptr->data;
				printf("%02x|%02x %02x %02x %04x %02x\n", d->speed, ptr->type, d->address, d->type, d->max, d->interval);
			}
			ptr = ptr->next;
		}
	}
}
#endif

struct gt_ffs_descs_state *gt_ffs_build_descs_state(const char *descs)
{
	struct gt_ffs_descs_state *state;
	struct stat st;
	int ret, fd;

	state = zalloc(sizeof(*state));
	if (state == NULL)
		goto out;

	errno = 0;
	ret = stat(descs, &st);
	if (errno != ENOENT) {
		if (ret)
			goto out;
		/* no error, file exists */
		fd = open(descs, O_RDONLY);
		if (fd < 0)
			goto out;
		ret = gt_ffs_parse_desc_file(fd, st.st_size, state);
		close(fd);
		if (ret)
			goto out;
	} /* else file does not exist, so empty descriptors state */

	return state;

out:
	gt_ffs_cleanup_descs_state(state, descs);
	return NULL;
}

static uint32_t gt_ffs_get_state_length(struct gt_ffs_descs_state *state)
{
	struct gt_ffs_link *ptr, *arr[3];
	uint32_t result;
	int i;

	result = 0;
	result += 4; /* MAGIC */
	result += 4; /* length */
	result += 4; /* flags */
	if (state->fs_count)
		result += 4;
	if (state->hs_count)
		result += 4;
	if (state->ss_count)
		result += 4;

	arr[0] = state->fs_descs;
	arr[1] = state->hs_descs;
	arr[2] = state->ss_descs;

	for (i = 0; i < ARRAY_SIZE(arr); ++i) {
		ptr = arr[i];
		while (ptr) {
			if (ptr->type == USB_DT_INTERFACE)
				result += 9;
			else
				result += 7;
			ptr = ptr->next;
		}
	}

	return result;
}

void gt_ffs_cleanup_descs_state(struct gt_ffs_descs_state *state, const char *descs)
{
	struct gt_ffs_link *ptr, *next, *arr[3];
	int i, fd = -1;

	arr[0] = state->fs_descs;
	arr[1] = state->hs_descs;
	arr[2] = state->ss_descs;

	if (state->modified) {
		fd = open(descs, O_WRONLY | O_TRUNC | O_CREAT);
		if (fd < 0)
			fprintf(stderr, "Cannot write descriptors!\n");
	}

	if (fd >= 0) {
		uint32_t v;

		v = htole32(FUNCTIONFS_DESCRIPTORS_MAGIC_V2);
		write(fd, &v, sizeof(v));
		v = htole32(gt_ffs_get_state_length(state));
		write(fd, &v, sizeof(v));
		v = 0;
		if (state->fs_count)
			v |= FUNCTIONFS_HAS_FS_DESC;
		if (state->hs_count)
			v |= FUNCTIONFS_HAS_HS_DESC;
		if (state->ss_count)
			v |= FUNCTIONFS_HAS_SS_DESC;
		v = htole32(v);
		write(fd, &v, sizeof(v));
		if (state->fs_count) {
			v = htole32(state->fs_count);
			write(fd, &v, sizeof(v));
		}
		if (state->hs_count) {
			v = htole32(state->hs_count);
			write(fd, &v, sizeof(v));
		}
		if (state->ss_count) {
			v = htole32(state->ss_count);
			write(fd, &v, sizeof(v));
		}
	}
	for (i = 0; i < ARRAY_SIZE(arr); ++i) {
		ptr = arr[i];
		while (ptr) {
			if (fd >= 0) {
				uint8_t v;
				uint16_t v16;

				if (ptr->type == USB_DT_INTERFACE) {
					struct gt_ffs_interface_create_data *d;

					d = ptr->data;
					v = 9; /* len */
					write(fd, &v, sizeof(v));
					write(fd, &ptr->type, sizeof(ptr->type));
					v = d->number;
					write(fd, &v, sizeof(v));
					v = 0; /* alt */
					write(fd, &v, sizeof(v));
					v = d->eps;
					write(fd, &v, sizeof(v));
					v = d->cls;
					write(fd, &v, sizeof(v));
					v = d->subcls;
					write(fd, &v, sizeof(v));
					v = d->protocol;
					write(fd, &v, sizeof(v));
					v = d->str_idx;
					write(fd, &v, sizeof(v));
				} else {
					struct gt_ffs_endpoint_create_data *d;

					d = ptr->data;
					v = 7; /* len */
					write(fd, &v, sizeof(v));
					write(fd, &ptr->type, sizeof(ptr->type));
					v = d->address;
					write(fd, &v, sizeof(v));
					v = d->type;
					write(fd, &v, sizeof(v));
					v16 = htole16(d->max);
					write(fd, &v16, sizeof(v16));
					v = d->interval;
					write(fd, &v, sizeof(v));
				}
			}
			free(ptr->data);
			next = ptr->next;
			free(ptr);
			ptr = next;
		}
	}
	if (fd >= 0)
		close(fd);

	free(state);
}
