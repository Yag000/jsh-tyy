#include "internals.h"
#include "command.h"
#include <stddef.h>

int last_exit_code;
command_call *last_command_call;
char cwd[PATH_MAX];

command_result *execute_internal_command(command_call *command_call);
command_result *execute_external_command(command_call *command_call);

void init_internals() {
    last_exit_code = 0;
    last_command_call = NULL;
}

command_result *execute_command(command_call *command_call) {
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
    /** TODO: Implement internal commands. */
    return NULL;
}

/** Executes an external command call. */
command_result *execute_external_command(command_call *command_call) {
    /** TODO: Implement external commands. */
    return NULL;
}

/** Updates the command history with the given result. */
void update_command_history(command_result *result) {
    last_exit_code = result->exit_code;
    last_command_call = result->call;
}
