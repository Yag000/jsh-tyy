#include <errno.h>
#include <signal.h>

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
    pid_t id_to_kill;
    if (starts_with(command_call->argv[identifier_index], "%")) { // job
        job *job = get_job(command_call->argv[identifier_index], call_stderr);
        if (job == NULL) {
            return 1;
        }

        id_to_kill = -job->pgid; // -pgid to send the signal to the whole process group

    } else { // pid
        if ((parse_intmax_t(command_call->argv[identifier_index], &parsed_value, call_stderr)) == 0) {
            return 1;
        }

        id_to_kill = parsed_value;
    }

    return kill_query(id_to_kill, sig) ? 0 : 1;
}
