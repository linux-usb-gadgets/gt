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

#endif //__GADGET_TOOL_PARSER_H__
