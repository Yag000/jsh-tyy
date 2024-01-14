#include "internals.h"

int last_exit_code_command(command_call *call) {
    if (call->argc != 1) {
        dprintf(call->stderr, "?: too many arguments\n");
        return 1;
    }
    dprintf(call->stdout, "%d\n", last_exit_code);
    return 0;
}
