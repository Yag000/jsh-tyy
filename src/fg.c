#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "internals.h"
#include "jobs.h"
#include "string_utils.h"

int fg_command(command_call *command_call) {
    if (command_call->argc == 1) {
        dprintf(command_call->stderr, "fg: missing argument\n");
        return 1;
    }

    if (command_call->argc > 2) {
        dprintf(command_call->stderr, "fg: too many arguments\n");
        return 1;
    }

    // Check if job id starts with %
    if (!starts_with(command_call->argv[1], "%")) {
        dprintf(command_call->stderr, "fg: invalid job id (missing %%)\n");
        return 1;
    }

    // Parse job id
    intmax_t parsed_value;
    if ((parse_intmax_t(command_call->argv[1] + 1, &parsed_value, command_call->stderr)) == 0) {
        return 1;
    }

    if (parsed_value < 0) {
        dprintf(command_call->stderr, "fg: %%%jd: invalid job id, job id can't be negative.\n", parsed_value);
        return 1;
    }

    size_t job_id = parsed_value;
    if (job_id < 1 || job_id > job_table_capacity || job_table_size == 0 || job_table == NULL) {
        dprintf(command_call->stderr, "fg: %%%zu: invalid job id.\n", job_id);
        return 0;
    }

    if (job_table[job_id - 1] == NULL) {
        dprintf(command_call->stderr, "fg: %%%zu: no such job.\n", job_id);
        return 0;
    }

    job *job = job_table[job_id - 1];

    return put_job_in_foreground(job) ? 0 : 1;
}