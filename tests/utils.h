#ifndef UTILS_H
#define UTILS_H

#include "../src/command.h"
#include "../src/jobs.h"
#include "test_core.h"


void helper_mute_update_jobs(char *file_name);

command_result *mute_command_execution(command *);

job *new_single_command_job(command_call *, pid_t, job_status);

/* Returns a new job with the given command, pid, status and type.
 * All the subjobs will have the same status, type and pid.
 */
job *job_from_command(command *command, pid_t pid, job_status status);

#endif // UTILS_H
