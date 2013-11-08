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

/**
 * @file executable_command.h
 * @brief Definition of ExecutableCommand type used for executing actions.
 */

#ifndef __GADGET_TOOL_EXECUTABLE_COMMAND_H__
#define __GADGET_TOOL_EXECUTABLE_COMMAND_H__

typedef int (*ExecutableFunc)(void *);
typedef void (*CleanupFunc)(void *);

typedef struct
{
	ExecutableFunc exec;
	void *data;
	CleanupFunc destructor;
} ExecutableCommand;

/**
 * @brief Fills to_set with given values.
 * @param[out] to_set structure to be filled in
 * @param[in] exec function to be set for execution
 * @param[in] data to be passed to function
 * @param[in] destructor to clean up data
 * @note If to_set is NULL function does nothing
 */
void executable_command_set(ExecutableCommand *to_set, ExecutableFunc exec,
		void *data, CleanupFunc destructor);

/**
 * @brief Cleans the stored data with destructor and sets to_clean fields
 * to NULL.
 *
 * @param[in, out] to_clean structure to be cleaned
 * @note If to_set is NULL function does nothing
 */
void executable_command_clean(ExecutableCommand *to_clean);

/**
 * @brief Executes the command.
 * @param[in] cmd to be executed
 * @return Value returned from executed function or -1 if function not set.
 */
int executable_command_exec(ExecutableCommand *cmd);

#endif //__GADGET_TOOL_EXECUTABLE_COMMAND_H__
