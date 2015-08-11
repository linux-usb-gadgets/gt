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
 * @file parser.h
 * @brief Declaration of methods related to parsing command line option.
 */

#ifndef __GADGET_TOOL_PARSER_H__
#define __GADGET_TOOL_PARSER_H__

#include "executable_command.h"

/**
 * @brief Parse the user input and determine a command to run.
 * @param[in] argc number of arguments provided by user
 * @param[in] argv user input
 * @param[out] exec which will be set with suitable values for parsed command
 * execution
 */
void gt_parse_commands(int argc, char **argv, ExecutableCommand *exec);

/**
 * @brief structure for storing name of variable and it's value
 **/
struct gt_setting {
	char *variable;
	char *value;
};

/**
 * @brief Parse string in format <variable>=<value> and fills given structure
 * with extracted values.
 * @param[out] dst Pointer to result
 * @param[in] str string to parse
 * @return Error code if failed or 0 if succeed
 **/
int gt_parse_setting(struct gt_setting *dst, const char *str);

/**
 * @brief Parse settings list from argv
 * @param[out] dst Pointer to null-terminated array of settings. Allocated
 * inside this function, should be freed by caller.
 * @param[in] argc Number of arguments passed in argv
 * @param[in] argv List of strings containg settings
 **/
int gt_parse_setting_list(struct gt_setting **dst, int argc, char **argv);

/**
 * @brief Cleanup function for setting structure.
 * @details Free only content of setting (i.e. strings)
 * @param[in] data pointer to structure which should be destroyed
 **/
void gt_setting_cleanup(void *data);

/**
 * @brief Destroy array of settings
 * @details Free content of array and array itself.
 * @param[in] data Null-terminated array of settings
 */
void gt_setting_list_cleanup(void *data);

/**
 * Definitions of options flags
 */
enum gt_option_flags {
	GT_FORCE = 1,
	GT_RECURSIVE = 1 << 1,
	GT_VERBOSE = 1 << 2,
	GT_OFF = 1 << 3,
	GT_STDIN = 1 << 4,
	GT_STDOUT = 1 << 5,
	GT_HELP = 1 << 6,
	GT_QUIET = 1 << 7,
	GT_INSTANCE = 1 << 8,
	GT_TYPE = 1 << 9,
	GT_NAME = 1 << 10,
	GT_ID = 1 << 11,
};

/**
 * @brief Get known options flags from given argv
 * @param[out] optmask mask containing all found options
 * @param[in] allowed_opts mask with allowed options
 * @param[in] argc number of given arguments
 * @param[in] argv list of arguments
 * @return index of first non-option argument, negative number otherwise
 **/
int gt_get_options(int *optmask, int allowed_opts, int argc, char **argv);

/**
 * @brief Split string in format <type>.<instance> into two strings
 * @param[out] type pointer to string with <type>
 * @param[out] instance pointer to string with <instance>
 * @param[in] str string to be parsed
 **/
int gt_parse_function_name(char **type, char **instance, const char *str);

#endif //__GADGET_TOOL_PARSER_H__
