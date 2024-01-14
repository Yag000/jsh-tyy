#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>

#include "internals.h"
#include "jobs.h"
#include "signals.h"
#include "utils.h"

int last_exit_code;
char lwd[PATH_MAX];
const size_t LIMIT_PROMPT_SIZE = 30;

int should_exit;

int execute_internal_command(command_call *command_call);
command_result *execute_external_command(command *command_call);

#define UNINITIALIZED_EXIT_CODE -1

typedef struct internal_exit_info {
    pid_t pid;
    int exit_code;
} internal_exit_info;

internal_exit_info *new_internal_exit_info(pid_t pid, int exit_code) {
    internal_exit_info *info = malloc(sizeof(internal_exit_info));
    if (info == NULL) {
        perror("malloc");
        return NULL;
    }

    info->pid = pid;
    info->exit_code = exit_code;
    return info;
}

void destroy_internal_exit_info(internal_exit_info *info) {
    free(info);
}

int has_exit_code(internal_exit_info *info) {
    if (info == NULL) {
        return 0;
    }

    return info->exit_code != UNINITIALIZED_EXIT_CODE;
}

void init_internals() {
    last_exit_code = 0;
    should_exit = 0;
    init_job_table();
}

command_result *execute_command(command *command) {
    command_result *result;

    if (command->command_call_count == 1 && is_internal_command(command->command_calls[0])) {
        int exit_code = execute_internal_command(command->command_calls[0]);
        result = new_command_result(exit_code, command);
    } else {
        result = execute_external_command(command);
    }

    close_unused_file_descriptors(command);

    update_command_history(result);
    return result;
}

/** Executes an internal command call. */
int execute_internal_command(command_call *command_call) {
    int exit_code = 0;

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
    } else if (strcmp(command_call->name, "fg") == 0) {
        exit_code = fg_command(command_call);
    } else if (strcmp(command_call->name, "bg") == 0) {
        exit_code = bg_command(command_call);
    }

    return exit_code;
}

void close_reading_pipes(command *command, command_call *command_call) {
    if (command == NULL || command->open_pipes == NULL || command->open_pipes_size == 0 || command_call == NULL) {
        return;
    }

    for (size_t i = 0; i < command->open_pipes_size; i++) {
        if (contains(command_call->reading_pipes->pipes, command_call->reading_pipes->pipe_count, i)) {
            continue;
        }
        if (command->open_pipes[i] == NULL) {
            continue;
        }
        close(command->open_pipes[i][0]);
    }
}

void close_writing_pipes(command *command, command_call *command_call) {
    if (command == NULL || command->open_pipes == NULL || command->open_pipes_size == 0 || command_call == NULL) {
        return;
    }

    for (size_t i = 0; i < command->open_pipes_size; i++) {
        if (contains(command_call->writing_pipes->pipes, command_call->writing_pipes->pipe_count, i)) {
            continue;
        }
        if (command->open_pipes[i] == NULL) {
            continue;
        }

        close(command->open_pipes[i][1]);
    }
}

internal_exit_info *execute_single_command(command *command, command_call *command_call, job *job) {
    if (is_internal_command(command_call)) {
        int exit_code = execute_internal_command(command_call);
        return new_internal_exit_info(UNINITIALIZED_PID, exit_code);
    }

    // These 2 pipes ensure that the group is created at the right moment.
    // The parent tells the child which pgid they should use,
    // the child sets it and the tells the parent it's ok.
    int fd[2];
    int fd_2[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return NULL;
    }
    if (pipe(fd_2) == -1) {
        perror("pipe");
        return NULL;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return NULL;
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
        close_writing_pipes(command, command_call);

        // Putting the process to the foreground
        if (command->background == 0) {
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
        kill(pid, SIGKILL); // It is important to kill the child if the parent fails to write
                            // since the child would never know its pgid
        return NULL;
    }

    close(fd[1]);

    int dump;

    // Waiting for the child to set the pgid
    if (read(fd_2[0], &dump, sizeof(int)) != sizeof(int)) {
        perror("read");
        kill(pid, SIGKILL); // It is important to kill the child if the parent fails to read
                            // since the child would never know that teh parent is waiting for it
        return NULL;
    }

    close(fd_2[0]);

    return new_internal_exit_info(pid, UNINITIALIZED_EXIT_CODE);
}

/**
 * Executes a command call as a job, it assumes the job has enough room to fit
 * all the calls.
 *
 * Returns the pid of the first executed command_call.
 */
internal_exit_info *execute_as_job(command *command, job *job) {
    internal_exit_info *info = NULL;

    for (size_t i = 0; i < command->command_call_count; i++) {

        internal_exit_info *info_ = execute_single_command(command, command->command_calls[i], job);

        if (info_ == NULL) {
            if (job->pgid != 0) {
                kill(-job->pgid, SIGKILL); // If there was an error, kill the whole group
            }
            destroy_internal_exit_info(info);
            return NULL;
        }

        if (i == 0) {
            info = new_internal_exit_info(info_->pid, info_->exit_code);
        }

        if (info_->pid != UNINITIALIZED_PID) { // Avoid internal commands
            subjob *subjob = new_subjob(command->command_calls[i], info_->pid, RUNNING);
            job->subjobs[i] = subjob;
        }

        destroy_internal_exit_info(info_);
    }

    return info;
}

job *setup_job(command *command) {
    size_t dependencies_count = command->command_call_count;
    job *job = new_job(dependencies_count, command->command_string);

    return job;
}

/** Executes an external command call. */
command_result *execute_external_command(command *command) {
    int background = command->background;
    command_result *command_result = new_command_result(1, command);
    job *job = setup_job(command);

    internal_exit_info *info = execute_as_job(command, job);

    for (size_t i = 0; i < command->open_pipes_size; i++) {
        if (command->open_pipes[i] == NULL) {
            continue;
        }
        close(command->open_pipes[i][0]);
        close(command->open_pipes[i][1]);
    }

    if (info == NULL) {
        destroy_job(job);
        return command_result;
    }

    if (background == 0) {
        int exit_code = blocking_wait_for_job(job);

        if (tcsetpgrp(STDERR_FILENO, getpgrp()) == -1) {
            perror("tcsetpgrp");
            exit(1);
        }

        if (exit_code == -1) {
            perror("waitpid");
            destroy_job(job);
            return command_result;
        }

        if (job->status == DONE || job->status == KILLED || job->status == DETACHED) {
            if (has_exit_code(info)) {
                // If the chief was an internal command, we need to use its exit code
                command_result->exit_code = info->exit_code;
            } else {
                command_result->exit_code = exit_code;
            }
            destroy_job(job);
        } else {
            int job_id = add_job(job);

            print_job(job, STDERR_FILENO);

            command_result->job_id = job_id;
            command_result->pid = info->pid;
            command_result->exit_code = 0;
        }

        destroy_internal_exit_info(info);

        return command_result;
    }

    int job_id = add_job(job);

    print_job(job, STDERR_FILENO);

    command_result->job_id = job_id;
    command_result->pid = info->pid;
    command_result->exit_code = 0;

    destroy_internal_exit_info(info);

    return command_result;
}

/** Updates the command history with the given result. */
void update_command_history(command_result *result) {
    if (result != NULL) {
        last_exit_code = result->exit_code;
    }
}

void close_unused_file_descriptors_helper(command *command, command_call *command_call) {
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

    free(fds_to_close);
}

void close_unused_file_descriptors(command *command) {
    if (command == NULL) {
        return;
    }

    for (size_t index = 0; index < command->command_call_count; ++index) {
        close_unused_file_descriptors_helper(command, command->command_calls[index]);
    }
}
