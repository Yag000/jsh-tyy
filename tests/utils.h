#ifndef UTILS_H
#define UTILS_H

#include "../src/command.h"
#include "../src/jobs.h"
#include "test_core.h"

void handle_command_call_test(command_call *actual, command_call *expected, int line, const char *file,
                              test_info *info);

void handle_command_test(command *actual, command *expected, int line, const char *file, test_info *info);

void handle_command_result_test(command_result *actual, command_result *expected, int line, const char *file,
                                test_info *info);
void handle_job_test(job *actual, job *expected, int line, const char *file, test_info *info);

void helper_mute_update_jobs(char *file_name);

command_result *mute_command_execution(command *);

job *new_single_command_job(command_call *, pid_t, job_status);

/* Returns a new job with the given command, pid, status and type.
 * All the subjobs will have the same status, type and pid.
 */
job *job_from_command(command *command, pid_t pid, job_status status);

#endif // UTILS_H
