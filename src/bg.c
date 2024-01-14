#include "internals.h"
#include "jobs.h"

int bg_command(command_call *command_call) {
    if (command_call->argc == 1) {
        dprintf(command_call->stderr, "bg: missing argument\n");
        return 1;
    }

    if (command_call->argc > 2) {
        dprintf(command_call->stderr, "bg: too many arguments\n");
        return 1;
    }

    job *job = get_job(command_call->argv[1], command_call->stderr);
    if (job == NULL) {
        return 1;
    }

    return continue_job_in_background(job) ? 0 : 1;
}
