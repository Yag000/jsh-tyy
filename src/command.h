#ifndef COMMAND_H
#define COMMAND_H

#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define REDIRECTION_CARET_SYMBOLS_COUNT 7
#define INTERNAL_COMMANDS_COUNT 8
#define UNINITIALIZED_PID -2

/** Array of internal command names. */
extern const char internal_commands[INTERNAL_COMMANDS_COUNT][100];

/** Array of possible caret symbols. */
extern const char *redirection_caret_symbols[REDIRECTION_CARET_SYMBOLS_COUNT];

/** Structure that represents a command call. */
typedef struct command_call {
    char *name;
    size_t argc;
    char **argv;
    char *command_string;
    int background; // 1 if the command is to be executed in background, 0 otherwise
    int stdin;
    int stdout;
    int stderr;
} command_call;

/** Returns a new command call with the given name, argc and argv. */
command_call *new_command_call(size_t argc, char **argv, char *command_string);

/** Frees the memory allocated for the command call.
 * Calls for `close_unused_file_descriptors`.
 */
void destroy_command_call(command_call *command_call);

/** Prints the command call to `fd`, following the format:
 *  name argv[0] argv[1] ... argv[argc - 1]
 */
void command_call_print(command_call *command_call, int fd);

/** Returns 1 if the command call is an internal command, 0 otherwise. */
int is_internal_command(command_call *command_call);

/** Parse unique command string to `command_call`.
 *  Returns `command_call` if correctly written, NULL otherwise.
 *
 *  Example :
 *      -> "echo abcdefghijkl"
 *      -> "cat >> output file0"
 *      -> "sleep 1000 2>> error.log > random 2>| true_error.log"
 */
command_call *parse_command(char *command_string);

/** Parses the command string and returns an array of command calls. */
command_call **parse_read_line(char *command, size_t *total_commands);

/** Structure that represents the result of a command.
 *  - If the command is an internal command or it is ran on foreground then
 *  `pid` is set to `UNINITIALIZED_PID` and `job_id` to `UNINITIALIZED_JOB_ID`.
 *  - If the `pid` is set to something else than `UNINITIALIZED_PID`, then the `command_call` will
 *  be `NULL`, as that information will be stored in the `job_table`, using
 *  `job_id - 1` as key.
 * */
typedef struct command_result {
    int exit_code;
    command_call *call;
    pid_t pid;
    size_t job_id;
} command_result;

/** Returns a new command result with the given exit code. */
command_result *new_command_result(int exit_code, command_call *call);

/** Frees the memory allocated for the command result.
 *  Calls `destroy_command_call` to free `command_result-> call`.
 */
void destroy_command_result(command_result *command_result);

#endif // COMMAND_H
