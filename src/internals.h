#ifndef INTERNALS_H
#define INTERNALS_H

#include "command.h"
#include <linux/limits.h>

/** Last exit code. */
extern int last_exit_code;

/** Last command call. */
extern command_call *last_command_call;

/** Current working directory. */
extern char cwd[PATH_MAX];

/** Executes the command call. */
command_result *execute_command_call(command_call *command_call);

/** Updates the command history with the given command result. */
void update_command_history(command_result *command_result);

/** Defines all the internal variables default values. */
void init_internals();

#endif // INTERNALS_H
