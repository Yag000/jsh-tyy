#ifndef INTERNALS_H
#define INTERNALS_H

#include "command.h"
#include <linux/limits.h>
#include <stddef.h>

/** Last exit code. */
extern int last_exit_code;

/** Last working directory */
extern char lwd[PATH_MAX];

/** Separator used to split commands. In our case a single space character. */
extern const char *COMMAND_SEPARATOR;

/** Indicator of a background execution. */
extern const char *BACKGROUND_FLAG;

/** 1 if the shell should exit, 0 otherwise. */
extern int should_exit;

/** The maximum size a prompt can have. */
extern const size_t LIMIT_PROMPT_SIZE;

/** Executes the command call. */
command_result *execute_command_call(command_call *command_call);

/** Updates the command history with the given command result. */
void update_command_history(command_result *command_result);

/** Defines all the internal variables default values. */
void init_internals();

/**
 * Changes the current working directory
 *
 * @param command_call
 *
 * @return `0` if the change worked; `1` otherwise.
 */
int cd(command_call *command_call);

/**
 * Prints the last exit code
 *
 * @param command_call
 *
 * @return `0` if the cwd was printed; `1` otherwise
 */
int last_exit_code_command(command_call *command_call);

/**
 * Prints on stdout pwd.
 *
 * @param command_call
 *
 * @return `0` if worked, `1` otherwise.
 */
int pwd(command_call *command_call);

/** Exits the shell.
 *
 * @param command_call
 *
 * @return `0` if the she;l will be exited; `1` otherwise
 * */
int exit_command(command_call *command_call);

/**
 * Prints the list of jobs to STDOUT_FILENO.
 */
int jobs_command(command_call *command_call);

#endif // INTERNALS_H
