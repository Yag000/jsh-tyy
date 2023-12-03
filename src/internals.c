#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command.h"
#include "internals.h"
#include "jobs.h"

int last_exit_code;
command_call *last_command_call;
char lwd[PATH_MAX];
const char *COMMAND_SEPARATOR = " ";
const size_t LIMIT_PROMPT_SIZE = 30;

int should_exit;
int exit_code;

command_result *execute_internal_command(command_call *command_call);
command_result *execute_external_command(command_call *command_call);

void init_internals() {
    last_exit_code = 0;
    last_command_call = NULL;
    should_exit = 0;
    exit_code = 0;
    init_job_table();
}

command_result *execute_command_call(command_call *command_call) {
    command_result *result;

    if (is_internal_command(command_call)) {
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
    }

    return command_result;
}

/** Executes an external command call. */
command_result *execute_external_command(command_call *command_call) {
    command_result *command_result = new_command_result(1, command_call);

    pid_t pid;
    int status;
    switch (pid = fork()) {
        case 0:
            execvp(command_call->name, command_call->argv);
            dprintf(command_call->stderr, "jsh: %s: %s\n", command_call->name, strerror(errno));
            exit(1);
        default:
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
                exit(1);
            }
    }
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
        command_result->exit_code = WEXITSTATUS(status);
    }

    return command_result;
}

/** Updates the command history with the given result. */
void update_command_history(command_result *result) {
    if (result != NULL) {
        last_exit_code = result->exit_code;
        last_command_call = result->call;
    }
}
