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
 * @file command.h
 * @brief Definition of Command type used for parsing command line options
 */

#ifndef __GADGET_TOOL_COMMAND_H__
#define __GADGET_TOOL_COMMAND_H__

#include "executable_command.h"

struct command;

typedef void (*ParseFunc)(const struct command *cmd, int, char **,
		ExecutableCommand *, void *);
typedef const struct command *(*GetChildrenFunc)(const struct command *cmd);

/**
 * @brief Representation of a single Command.
 */
typedef struct command
{
	/**
	 * @brief String of this command.
	 */
	const char *name;
	/**
	 * @brief Indicates if subcommand should be parsed from the same string
	 * or from the next one
	 */
	enum
	{
		NEXT = 1,
		AGAIN = 0
	} distance;
	/**
	 * @brief Parses this command.
	 * @details If this param is NULL the whole structure is invalid
	 */
	ParseFunc parse;
	/**
	 * @brief Gets the children of this command if any
	 */
	GetChildrenFunc getChildren;
	/**
	 * @brief Function used to show help if parsing fails.
	 */
	ExecutableFunc printHelp;
} Command;

/* Macro indicating end of list of commands */
#define CMD_LIST_END {NULL, AGAIN, NULL, NULL, NULL}

/**
 * @brief Looks for a suitable child and recursively parses it.
 * @details Used convention says that when string is NULL it matches
 * all other strings
 *
 * @param[in] cmd which children should be considered
 * @param[in] argc number of commands left till end of input
 * @param[in] argv place to start parse user input
 * @param[out] exec structure to be filled in with suitable command
 * @param[in] data additional data
 */
void command_parse(const Command *cmd, int argc, char **argv,
		ExecutableCommand *exec, void *data);

#endif //__GADGET_TOOL_COMMAND_H__
