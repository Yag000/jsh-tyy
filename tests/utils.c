#include "utils.h"
#include "../src/command.h"
#include "../src/internals.h"

#include <assert.h>
#include <fcntl.h>


command_result *mute_command_execution(command *command) {
    int fd = open("/dev/null", O_WRONLY);
    assert(fd != -1);
    int old_stdout = dup(STDOUT_FILENO);
    assert(old_stdout != -1);
    dup2(fd, STDERR_FILENO);

    command_result *result = execute_command(command);

    dup2(old_stdout, STDOUT_FILENO);

    close(fd);
    close(old_stdout);

    return result;
}

job *new_single_command_job(command_call *command_call, pid_t pid, job_status status) {
    job *job = new_job(1, command_call->command_string);
    subjob *subjob = new_subjob(command_call, pid, status);
    job->subjobs[0] = subjob;

    return job;
}

job *job_from_command(command *command, pid_t pid, job_status status) {
    job *job = new_job(command->command_call_count, command->command_string);
    for (size_t i = 0; i < command->command_call_count; i++) {
        subjob *subjob = new_subjob(command->command_calls[i], pid, status);
        job->subjobs[i] = subjob;
    }
    return job;
}
