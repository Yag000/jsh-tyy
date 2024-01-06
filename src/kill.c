#include <asm-generic/errno-base.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "internals.h"
#include "jobs.h"
#include "string_utils.h"

int call_stderr;

int kill_query(pid_t pid, int sig) {
    if ((kill(pid, sig)) == -1) {
        switch (errno) {
            case EINVAL:
                dprintf(call_stderr, "kill: %d: invalid signal.\n", sig);
                return 0;
            case ESRCH:
                dprintf(call_stderr, "kill: %d: no such process.\n", sig);
                return 0;
            default:
                dprintf(call_stderr, "kill: kill failure. (%s)\n", strerror(errno));
                return 0;
        }
    }

    return 1;
}

int kill_job(size_t job_id, int sig) {
    // Invalid job_id
    if (job_id < 1 || job_id > job_table_capacity || job_table_size == 0 || job_table == NULL) {
        dprintf(call_stderr, "kill: %%%zu: invalid job_id.\n", job_id);
        return 0;
    }

    // Job doesn't exist
    if (job_table[job_id - 1] == NULL) {
        dprintf(call_stderr, "kill: %%%zu: no such job.", job_id);
        return 0;
    }

    pid_t to_kill = job_table[job_id - 1]->pgid;

    return kill_query(-to_kill, sig); // Kill the group
}

int kill_command(command_call *command_call) {
    call_stderr = command_call->stderr;

    if (command_call->argc < 2 || command_call->argc > 3) {
        dprintf(call_stderr, "kill: incorrect usage of kill command.\n");
        dprintf(call_stderr, "kill: correct usage: kill [-sig] %%job|pid\n");
        return 1;
    }

    int sig = SIGTERM;           // Default signal
    size_t identifier_index = 1; // index of the pid/job in argv
    intmax_t parsed_value;

    // Signal given by user
    if (starts_with(command_call->argv[1], "-")) {
        if ((parse_intmax_t(command_call->argv[1] + 1, &parsed_value, call_stderr)) == 0) {
            return 1;
        }

        sig = parsed_value;
        ++identifier_index; // We have a signal, we expect a %job|pid later
    }

    // Missing %job|pid
    if ((identifier_index == 2 && command_call->argc != 3)) {
        dprintf(call_stderr, "kill: missing %%job|pid argument.\n");
        return 0;
    }

    // Check job or pid
    if (starts_with(command_call->argv[identifier_index], "%")) { // job

        if ((parse_intmax_t(command_call->argv[identifier_index] + 1, &parsed_value, call_stderr)) == 0) {
            return 1;
        }

        if (parsed_value < 0) {
            dprintf(call_stderr, "kill: %ju:%%job can't be negative.\n", parsed_value);
            return 1;
        }

        size_t job_id = parsed_value;
        return kill_job(job_id, sig) ? 0 : 1;
    } else { // pid

        if ((parse_intmax_t(command_call->argv[identifier_index], &parsed_value, call_stderr)) == 0) {
            return 1;
        }

        pid_t pid = parsed_value;
        return kill_query(pid, sig) ? 0 : 1;
    }
}
