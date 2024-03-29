#include "internals.h"
#include "jobs.h"

int fg_command(command_call *command_call) {
    if (command_call->argc == 1) {
        dprintf(command_call->stderr, "fg: missing argument\n");
        return 1;
    }

    if (command_call->argc > 2) {
        dprintf(command_call->stderr, "fg: too many arguments\n");
        return 1;
    }

    job *job = get_job(command_call->argv[1], command_call->stderr);
    if (job == NULL) {
        return 1;
    }

    return put_job_in_foreground(job) ? 0 : 1;
}