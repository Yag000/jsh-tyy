#include "internals.h"
#include "command.h"
#include <stddef.h>
#include <string.h>

int last_exit_code;
command_call *last_command_call;
char lwd[PATH_MAX];
const char *COMMAND_SEPARATOR = " ";

command_result *execute_internal_command(command_call *command_call);
command_result *execute_external_command(command_call *command_call);

void init_internals() {
    last_exit_code = 0;
    last_command_call = NULL;
}

command_result *execute_command_call(command_call *command_call) {
    command_result *result;

    if (is_internal_command(command_call)) {
        result = execute_internal_command(command_call);
    } else {
        result = execute_external_command(command_call);
    }

    update_command_history(result);
    return result;
}

/** Executes an internal command call. */
command_result *execute_internal_command(command_call *command_call) {
    command_result *command_result = new_command_result(0, command_call);

    /** TODO: Implement the remaining internal commands. */

    if (strcmp(command_call->name, "cd") == 0) {
        command_result->exit_code = cd(command_call);
    }

    return command_result;
}

/** Executes an external command call. */
command_result *execute_external_command(command_call *command_call) {
    /** TODO: Implement external commands. */
    return new_command_result(1, command_call);
}

/** Updates the command history with the given result. */
void update_command_history(command_result *result) {
    if (result != NULL) {
        last_exit_code = result->exit_code;
        last_command_call = result->call;
    }
}
