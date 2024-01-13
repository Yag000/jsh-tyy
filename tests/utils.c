#include "utils.h"
#include "../src/command.h"
#include "../src/internals.h"
#include "test_core.h"

#include <assert.h>
#include <fcntl.h>

void handle_command_call_test(command_call *actual, command_call *expected, int line, const char *file,
                              test_info *info) {
    info->total++;

    int failed = info->failed;

    handle_int_test(actual->argc, expected->argc, line, file, info);

    if (failed != info->failed) {
        info->failed++;
        return;
    }

    for (size_t i = 0; i < actual->argc; i++) {
        handle_string_test(actual->argv[i], expected->argv[i], line, file, info);
        if (failed != info->failed) {
            info->failed++;
            return;
        }
    }

    handle_int_test(actual->background, expected->background, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    handle_string_test(actual->command_string, expected->command_string, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    handle_int_test(actual->stdin, expected->stdin, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    handle_int_test(actual->stdout, expected->stdout, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    handle_int_test(actual->stderr, expected->stderr, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    info->passed++;
}

void handle_command_test(command *actual, command *expected, int line, const char *file, test_info *info) {
    info->total++;

    int failed = info->failed;

    handle_command_call_test(actual->call, expected->call, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    handle_int_test(actual->open_pipes_size, expected->open_pipes_size, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    for (size_t i = 0; i < actual->open_pipes_size; i++) {
        for (size_t j = 0; j < 2; j++) {
            handle_int_test(actual->open_pipes[i][j], expected->open_pipes[i][j], line, file, info);
            if (failed != info->failed) {
                info->failed++;
                return;
            }
        }
    }

    info->passed++;
}
void handle_command_result_test(command_result *actual, command_result *expected, int line, const char *file,
                                test_info *info) {
    info->total++;
    int failed = info->failed;

    handle_int_test(actual->exit_code, expected->exit_code, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    info->passed++;
}

void handle_subjob_test(subjob *actual, subjob *expected, int line, const char *file, test_info *info) {
    info->total++;
    int failed = info->failed;

    handle_int_test(actual->pid, expected->pid, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    handle_int_test(actual->last_status, expected->last_status, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    handle_string_test(actual->command, expected->command, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    info->passed++;
}
void handle_job_test(job *actual, job *expected, int line, const char *file, test_info *info) {
    info->total++;

    int failed = info->failed;

    handle_int_test(actual->id, expected->id, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    handle_int_test(actual->type, expected->type, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    handle_int_test(actual->subjobs_size, expected->subjobs_size, line, file, info);
    if (failed != info->failed) {
        info->failed++;
        return;
    }

    for (size_t i = 0; i < actual->subjobs_size; i++) {
        handle_subjob_test(actual->subjobs[i], expected->subjobs[i], line, file, info);
        if (failed != info->failed) {
            info->failed++;
            return;
        }
    }

    info->passed++;
}

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

job *new_single_command_job(command_call *command_call, pid_t pid, job_status status, job_type type) {
    job *job = new_job(1, type, command_call->command_string);
    subjob *subjob = new_subjob(command_call, pid, status);
    job->subjobs[0] = subjob;

    return job;
}
