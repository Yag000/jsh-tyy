#include "internals.h"
#include "utils.h"

#include <errno.h>

int pwd(command_call *command_call) {

    if (command_call->argc > 1) {
        dprintf(command_call->stderr, "pwd: too many arguments\n");
        return 1;
    }

    char *cwd = get_current_wd();

    if (cwd == NULL) {
        dprintf(command_call->stderr, "pwd: getcwd failure. (%s)\n", strerror(errno));
        return 1;
    }

    dprintf(command_call->stdout, "%s\n", cwd);
    free(cwd);

    return 0;
}
