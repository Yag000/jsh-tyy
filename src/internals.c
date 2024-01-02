#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "command.h"
#include "internals.h"
#include "jobs.h"
#include "utils.h"

int last_exit_code;
char lwd[PATH_MAX];
const char *COMMAND_SEPARATOR = " ";
const char *BACKGROUND_FLAG = "&";
const size_t LIMIT_PROMPT_SIZE = 30;

int should_exit;

command_result *execute_internal_command(command_call *command_call);
command_result *execute_external_command(command_call *command_call);

void init_internals() {
    last_exit_code = 0;
    should_exit = 0;
    init_job_table();
}

command_result *execute_command_call(command_call *command_call) {
    command_result *result;

    if (is_internal_command(command_call) && command_call->dependencies_count == 0) {
        result = execute_internal_command(command_call);
    } else {
        result = execute_external_command(command_call);
    }

    update_command_history(result);
    return result;
}

/** Executes an internal command call. */
command_result *execute_internal_command(command_call *command_call) {
    command_result *command_result = new_command_result(1, command_call);

    /** TODO: Implement the remaining internal commands. */

    if (strcmp(command_call->name, "cd") == 0) {
        command_result->exit_code = cd(command_call);
    } else if (strcmp(command_call->name, "?") == 0) {
        command_result->exit_code = last_exit_code_command(command_call);
    } else if (strcmp(command_call->name, "exit") == 0) {
        command_result->exit_code = exit_command(command_call);
    } else if (strcmp(command_call->name, "pwd") == 0) {
        command_result->exit_code = pwd(command_call);
    } else if (strcmp(command_call->name, "jobs") == 0) {
        command_result->exit_code = jobs_command(command_call);
    } else if (strcmp(command_call->name, "kill") == 0) {
        command_result->exit_code = kill_command(command_call);
    }

    return command_result;
}

pid_t execute_single_command(command_call *command_call, job *job) {
    // TODO: Solve this gracefully and properly. When implementing pipelines, this could be
    // a problem. (see !65)
    if (is_internal_command(command_call)) {
        destroy_command_result(execute_internal_command(command_call));
        return 0;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        dup2(command_call->stdin, STDIN_FILENO);
        dup2(command_call->stdout, STDOUT_FILENO);
        dup2(command_call->stderr, STDERR_FILENO);

        execvp(command_call->name, command_call->argv);
        dprintf(command_call->stderr, "jsh: %s: %s\n", command_call->name, strerror(errno));
        exit(1);
    }

    if (job->pgid == 0) {
        job->pgid = pid;
    }

    // Every process in the job should be in the same process group.
    if ((setpgid(pid, job->pgid)) == -1) {
        perror("setpgid");
        exit(1);
    }

    return pid;
}

/**
 * Executes a command call as a job, it assumes the job has enough room to fit
 * all the calls.
 *
 * Returns the pid of the executed command_call (not it's dependencies).
 */
pid_t execute_as_job(command_call *command_call, job *job) {
    size_t index;

    for (index = 0; index < job->subjobs_size; index++) {
        if (job->subjobs[index] == NULL) {
            break;
        }
    }

    // This should never happen, if it's the case then there is a huge
    // issue with the code (this should maybe be changed to something
    // more graceful).
    assert(index < job->subjobs_size);

    pid_t pid = execute_single_command(command_call, job);
    if (pid != 0) {
        subjob *subjob = new_subjob(command_call, pid, RUNNING);
        job->subjobs[index] = subjob;
    }

    for (size_t i = 0; i < command_call->dependencies_count; i++) {
        execute_as_job(command_call->dependencies[i], job);
    }

    return pid;
}

size_t count_dependencies(command_call *command_call) {
    int dependencies_count = 1;

    for (size_t i = 0; i < command_call->dependencies_count; i++) {
        dependencies_count += count_dependencies(command_call->dependencies[i]);
    }

    return dependencies_count;
}

job *setup_job(command_call *command_call) {
    job_type job_type = command_call->background ? BACKGROUND : FOREGROUND;
    size_t dependencies_count = count_dependencies(command_call);
    job *job = new_job(dependencies_count, job_type);

    return job;
}

/** Executes an external command call. */
command_result *execute_external_command(command_call *command_call) {
    command_result *command_result = new_command_result(1, command_call);
    pid_t pid;
    int status;

    job *job = setup_job(command_call);

    pid = execute_as_job(command_call, job);

    if (command_call->background == 0) {
        int pid_ = waitpid(pid, &status, WUNTRACED);
        if (pid_ == -1) {
            perror("waitpid");
            exit(1);
        }

        if (WIFEXITED(status)) {
            command_result->exit_code = WEXITSTATUS(status);
            soft_destroy_job(job);
        } else if (WIFSTOPPED(status)) {
            int job_id = add_job(job);

            job->subjobs[0]->last_status = STOPPED;

            print_job(job, STDERR_FILENO);

            command_result->job_id = job_id;
            command_result->pid = pid;
            command_result->exit_code = 0;
            command_result->call = NULL;
        }

        return command_result;
    }

    int job_id = add_job(job);

    print_job(job, STDERR_FILENO);

    command_result->job_id = job_id;
    command_result->pid = pid;
    command_result->exit_code = 0;
    command_result->call = NULL;

    return command_result;
}

/** Updates the command history with the given result. */
void update_command_history(command_result *result) {
    if (result != NULL) {
        last_exit_code = result->exit_code;
    }
}

void close_unused_file_descriptors(command_call *command) {
    if (command == NULL) {
        return;
    }

    size_t nb_fds_to_close = 3;

    int *fds_to_close = malloc(nb_fds_to_close * sizeof(int));
    if (fds_to_close == NULL) {
        perror("malloc");
        return;
    }

    for (size_t index = 0; index < nb_fds_to_close; ++index) {
        fds_to_close[index] = -1;
    }

    add_set(fds_to_close, nb_fds_to_close, command->stdin);
    add_set(fds_to_close, nb_fds_to_close, command->stdout);
    add_set(fds_to_close, nb_fds_to_close, command->stderr);

    for (size_t index = 0; index < nb_fds_to_close; ++index) {
        if (fds_to_close[index] > 2) {
            close(fds_to_close[index]);
        }
    }

    free(fds_to_close);
}
