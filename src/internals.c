#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "command.h"
#include "internals.h"
#include "jobs.h"
#include "signals.h"
#include "utils.h"

int last_exit_code;
char lwd[PATH_MAX];
const char *COMMAND_SEPARATOR = " ";
const char *BACKGROUND_FLAG = "&";
const size_t LIMIT_PROMPT_SIZE = 30;

int should_exit;

int execute_internal_command(command_call *command_call);
command_result *execute_external_command(command *command_call);
void close_unused_file_descriptors(command *command, command_call *command_call);

void init_internals() {
    last_exit_code = 0;
    should_exit = 0;
    init_job_table();
}

command_result *execute_command(command *command) {
    command_result *result;
    command_call *command_call = command->call;

    if (is_internal_command(command_call) && command_call->dependencies_count == 0) {
        int exit_code = execute_internal_command(command_call);
        result = new_command_result(exit_code, command);
    } else {
        result = execute_external_command(command);
    }

    close_unused_file_descriptors(command, command->call);

    update_command_history(result);
    return result;
}

/** Executes an internal command call. */
int execute_internal_command(command_call *command_call) {
    int exit_code = 0;
    /** TODO: Implement the remaining internal commands. */

    if (strcmp(command_call->name, "cd") == 0) {
        exit_code = cd(command_call);
    } else if (strcmp(command_call->name, "?") == 0) {
        exit_code = last_exit_code_command(command_call);
    } else if (strcmp(command_call->name, "exit") == 0) {
        exit_code = exit_command(command_call);
    } else if (strcmp(command_call->name, "pwd") == 0) {
        exit_code = pwd(command_call);
    } else if (strcmp(command_call->name, "jobs") == 0) {
        exit_code = jobs_command(command_call);
    } else if (strcmp(command_call->name, "kill") == 0) {
        exit_code = kill_command(command_call);
    }

    return exit_code;
}

void close_reading_pipes(command *command, command_call *command_call) {
    if (command == NULL || command->open_pipes == NULL || command->open_pipes_size == 0 || command_call == NULL) {
        return;
    }

    for (size_t i = 0; i < command->open_pipes_size; i++) {
        if (contains(command_call->owned_pipe_indexes, command_call->dependencies_count, i)) {
            continue;
        }
        if (command->open_pipes[i] == NULL) {
            continue;
        }

        close(command->open_pipes[i][0]);
    }
}

void close_writing_pipes(command *command, command_call *command_call, size_t dependency_id) {
    if (command == NULL || command->open_pipes == NULL || command->open_pipes_size == 0 || command_call == NULL) {
        return;
    }

    for (size_t i = 0; i < command->open_pipes_size; i++) {
        if (i + 1 == dependency_id) {
            continue;
        }
        if (command->open_pipes[i] == NULL) {
            continue;
        }
        close(command->open_pipes[i][1]);
    }
}

pid_t execute_single_command(command *command, command_call *command_call, job *job, int dependency_id) {
    // TODO: Solve this gracefully and properly. When implementing pipelines, this could be
    // a problem. (see !65)
    if (is_internal_command(command_call)) {
        execute_internal_command(command_call);
        return 0;
    }

    // These 2 pipes ensure that the group is created at the right moment.
    // The parent tells the child which pgid they should use,
    // the child sets it and the tells the parent it's ok.
    int fd[2];
    int fd_2[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return 0;
    }
    if (pipe(fd_2) == -1) {
        perror("pipe");
        return 0;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        close(fd[1]);
        close(fd_2[0]);

        // Getting the pgid for this process
        pid_t pgid;
        if (read(fd[0], &pgid, sizeof(int)) != sizeof(int)) {
            perror("read");
            exit(1);
        }
        close(fd[0]);

        // Every process in the job should be in the same process group.
        if ((setpgid(0, pgid)) == -1) {
            perror("setpgid");
            exit(1);
        }

        // Telling the parent that the pgid has been set
        if (write(fd_2[1], &pgid, sizeof(int)) != sizeof(int)) {
            perror("write");
            exit(1);
        }

        close(fd_2[1]);

        close_reading_pipes(command, command_call);
        close_writing_pipes(command, command_call, dependency_id);

        // Putting the process to the foreground
        if (command_call->background == 0) {
            if (tcsetpgrp(STDERR_FILENO, pgid) == -1) {
                perror("tcsetpgrp");
                exit(1);
            }
        }

        restore_signals();

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

    close(fd[0]);
    close(fd_2[1]);

    // Telling the child the pgid it should use
    if (write(fd[1], &job->pgid, sizeof(int)) != sizeof(int)) {
        perror("write");
        return 0;
    }

    close(fd[1]);

    int dump;

    // Waiting for the child to set the pgid
    if (read(fd_2[0], &dump, sizeof(int)) != sizeof(int)) {
        perror("read");
        return 0;
    }

    close(fd_2[0]);

    return pid;
}

/**
 * Executes a command call as a job, it assumes the job has enough room to fit
 * all the calls.
 *
 * Returns the pid of the executed command_call (not it's dependencies).
 */
pid_t execute_as_job(command *command, command_call *command_call, job *job) {
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

    pid_t pid = execute_single_command(command, command_call, job, index);

    if (pid != 0) {
        subjob *subjob = new_subjob(command_call, pid, RUNNING);
        job->subjobs[index] = subjob;
    }

    for (size_t i = 0; i < command_call->dependencies_count; i++) {
        execute_as_job(command, command_call->dependencies[i], job);
    }

    return pid;
}

size_t count_dependencies(command_call *command_call) {
    size_t dependencies_count = 1;

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
command_result *execute_external_command(command *command) {
    pid_t pid;
    int status;

    int background = command->call->background;
    command_result *command_result = new_command_result(1, command);
    job *job = setup_job(command->call);

    pid = execute_as_job(command, command->call, job);

    for (size_t i = 0; i < command->open_pipes_size; i++) {
        if (command->open_pipes[i] == NULL) {
            continue;
        }
        close(command->open_pipes[i][0]);
        close(command->open_pipes[i][1]);
    }

    if (background == 0) {
        int pid_ = waitpid(pid, &status, WUNTRACED);
        if (pid_ == -1) {
            perror("waitpid");
            exit(1);
        }

        if (tcsetpgrp(STDERR_FILENO, getpid()) == -1) {
            perror("tcsetpgrp");
            exit(1);
        }

        if (WIFEXITED(status)) {
            command_result->exit_code = WEXITSTATUS(status);
            destroy_job(job);
        } else if (WIFSTOPPED(status)) {
            int job_id = add_job(job);

            job->subjobs[0]->last_status = STOPPED;

            print_job(job, STDERR_FILENO);

            command_result->job_id = job_id;
            command_result->pid = pid;
            command_result->exit_code = 0;
        }

        return command_result;
    }

    int job_id = add_job(job);

    print_job(job, STDERR_FILENO);

    command_result->job_id = job_id;
    command_result->pid = pid;
    command_result->exit_code = 0;

    return command_result;
}

/** Updates the command history with the given result. */
void update_command_history(command_result *result) {
    if (result != NULL) {
        last_exit_code = result->exit_code;
    }
}

void close_unused_file_descriptors(command *command, command_call *command_call) {
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

    add_set(fds_to_close, nb_fds_to_close, command_call->stdin);
    add_set(fds_to_close, nb_fds_to_close, command_call->stdout);
    add_set(fds_to_close, nb_fds_to_close, command_call->stderr);

    for (size_t index = 0; index < nb_fds_to_close; ++index) {
        if (fds_to_close[index] > 2 &&
            !contains2(command->open_pipes, command->open_pipes_size, 2, fds_to_close[index])) {
            close(fds_to_close[index]);
        }
    }

    for (size_t i = 0; i < command_call->dependencies_count; i++) {
        close_unused_file_descriptors(command, command_call->dependencies[i]);
    }

    free(fds_to_close);
}
