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
#include "ffs.h"

static int interface_create_func(void *data)
{
	printf("gt interface_create called successfully. Not implemented yet.\n");
	return 0;
}

static int endpoint_create_func(void *data)
{
	printf("gt endpoint_create called successfully. Not implemented yet.\n");
	return 0;
}

static int language_create_func(void *data)
{
	printf("gt language_create called successfully. Not implemented yet.\n");
	return 0;
}

static int string_create_func(void *data)
{
	printf("gt string_create called successfully. Not implemented yet.\n");
	return 0;
}

struct gt_ffs_backend gt_ffs_backend_not_implemented = {
	.interface_create = interface_create_func,
	.endpoint_create = endpoint_create_func,
	.language_create = language_create_func,
	.string_create = string_create_func,
};
