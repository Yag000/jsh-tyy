#include "command.h"
#include "internals.h"
#include "utils.h"
#include <errno.h>
#include <stdio.h>

int pwd(command_call *command_call) {

    if (command_call->argc > 1) {
        dprintf(command_call->stderr, "pwd: too many arguments\n");
        return 1;
    }

    char *cwd = get_cwd_raw();

    if (cwd == NULL) {
        dprintf(command_call->stderr, "pwd: getcwd failure. (%s)\n", strerror(errno));
        return 1;
    }

    dprintf(command_call->stdout, "%s\n", cwd);
    if (cwd != NULL) {
        free(cwd);
    }

    return 0;
}
