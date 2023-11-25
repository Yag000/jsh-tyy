#include <stdio.h>
#include <stdlib.h>

#include "command.h"
#include "internals.h"

int exit_command(command_call *command_call) {
    /** TODO: Handle jobs running in the background */

    if (command_call == NULL) {
        return 1;
    }

    if (command_call->argc > 2) {
        dprintf(command_call->stderr, "exit: too many arguments\n");
        return 1;
    }

    if (command_call->argc == 1) {
        should_exit = 1;
        exit_code = last_exit_code;
        return 0;
    }

    if (command_call->argc == 2) {
        char *endptr;
        int converted_exit_code = strtol(command_call->argv[1], &endptr, 10);
        if (*endptr != '\0') {
            dprintf(command_call->stderr, "exit: %s: numeric argument required\n", command_call->argv[1]);
            return 1;
        }
        if (converted_exit_code < 0 || converted_exit_code > 255) {
            dprintf(command_call->stderr, "exit: %s: number between 0 and 255 (inclusive) required\n",
                    command_call->argv[1]);
            return 1;
        }
        exit_code = converted_exit_code;
        should_exit = 1;
    }

    return 0;
}
