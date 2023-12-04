#ifndef COMMAND_H
#define COMMAND_H

#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INTERNAL_COMMANDS_COUNT 8

/** Array of internal command names. */
extern const char internal_commands[INTERNAL_COMMANDS_COUNT][100];

/** Structure that represents a command call. */
typedef struct command_call {
    char *name;
    size_t argc;
    char **argv;
    int background; // 1 if the command is to be executed in background, 0 otherwise
    int stdin;
    int stdout;
    int stderr;
} command_call;

/** Returns a new command call with the given name, argc and argv. */
command_call *new_command_call(size_t argc, char **argv);

/** Frees the memory allocated for the command call. */
void destroy_command_call(command_call *command_call);

/** Prints the command call. */
void command_call_print(command_call *command_call);

/** Returns 1 if the command call is an internal command, 0 otherwise. */
int is_internal_command(command_call *command_call);

/** Parses the command string and returns a command call. */
command_call *parse_command(char *command);

/** Structure that represents the result of a command. */
typedef struct command_result {
    int exit_code;
    command_call *call;
} command_result;

/** Returns a new command result with the given exit code. */
command_result *new_command_result(int exit_code, command_call *call);

/** Frees the memory allocated for the command result.
 *  Calls `destroy_command_call` to free `command_result-> call`.
 */
void destroy_command_result(command_result *command_result);

#endif // COMMAND_H
