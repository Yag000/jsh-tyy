#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test_core.h"

int check_command_call(command_call *actual, command_call *expected) {

    if (actual->argc != expected->argc) {
        return 0;
    }

    for (size_t i = 0; i < actual->argc; i++) {
        if (strcmp(actual->argv[i], expected->argv[i]) != 0) {
            return 0;
        }
    }

    if (strcmp(actual->command_string, expected->command_string) != 0) {
        return 0;
    }

    if (actual->stdin != expected->stdin) {
        return 0;
    }
    if (actual->stdout != expected->stdout) {
        return 0;
    }
    if (actual->stderr != expected->stderr) {
        return 0;
    }

    return 1;
}

int check_command(command *actual, command *expected) {
    if (actual->command_call_count != expected->command_call_count) {
        return 0;
    }

    for (size_t i = 0; i < actual->command_call_count; i++) {
        if (!check_command_call(actual->command_calls[i], expected->command_calls[i])) {
            return 0;
        }
    }

    if (actual->background != expected->background) {
        return 0;
    }

    if (actual->open_pipes_size != expected->open_pipes_size) {
        return 0;
    }

    for (size_t i = 0; i < actual->open_pipes_size; i++) {
        for (size_t j = 0; j < 2; j++) {
            if (actual->open_pipes[i][j] != expected->open_pipes[i][j]) {
                return 0;
            }
        }
    }

    return 1;
}
int check_command_result(command_result *actual, command_result *expected) {
    if (actual->exit_code != expected->exit_code) {
        return 0;
    }

    return 1;
}

int check_subjob(subjob *actual, subjob *expected) {

    if (actual->pid != expected->pid) {
        return 0;
    }

    if (actual->last_status != expected->last_status) {
        return 0;
    }

    if (strcmp(actual->command, expected->command) != 0) {
        return 0;
    }

    return 1;
}

int check_job(job *actual, job *expected) {

    if (actual->id != expected->id) {
        return 0;
    }

    if (actual->subjobs_size != expected->subjobs_size) {
        return 0;
    }

    for (size_t i = 0; i < actual->subjobs_size; i++) {
        if (!check_subjob(actual->subjobs[i], expected->subjobs[i])) {
            return 0;
        }
    }

    return 1;
}

int open_test_file_to_write(const char *filename) {
    char path[PATH_MAX];
    sprintf(path, "tmp/%s", filename);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int open_test_file_to_read(const char *filename) {
    char path[PATH_MAX];
    sprintf(path, "tmp/%s", filename);
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    return fd;
}
