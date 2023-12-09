#include "command.h"
#include "internals.h"
#include "jobs.h"
#include "string_utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char internal_commands[INTERNAL_COMMANDS_COUNT][100] = {"cd", "exit", "pwd", "?", "jobs", "fg", "bg", "kill"};

/** Returns a new command call with the given name, argc and argv. */
command_call *new_command_call(size_t argc, char **argv) {

    command_call *command_call = malloc(sizeof(*command_call));
    if (command_call == NULL) {
        perror("malloc");
        return NULL;
    }

    command_call->name = argv[0];
    command_call->argc = argc;
    command_call->argv = argv;
    command_call->background = 0;
    command_call->stdin = 0;
    command_call->stdout = 1;
    command_call->stderr = 2;
    return command_call;
}

/** Frees the memory allocated for the command call. */
void destroy_command_call(command_call *command_call) {
    if (command_call == NULL) {
        return;
    }
    for (size_t index = 0; index < command_call->argc; index++) {
        free(command_call->argv[index]);
    }
    free(command_call->argv);
    free(command_call);
}

/** Prints the command call. */
void command_call_print(command_call *command_call) {
    size_t i;
    for (i = 0; i < command_call->argc; i++) {
        if (i == command_call->argc - 1) {
            dprintf(STDOUT_FILENO, "%s", command_call->argv[i]);
        } else {
            dprintf(STDOUT_FILENO, "%s ", command_call->argv[i]);
        }
    }
}

int is_internal_command(command_call *command_call) {
    int i;
    for (i = 0; i < 8; i++) {
        if (strcmp(command_call->name, internal_commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

command_result *new_command_result(int exit_code, command_call *command_call) {
    if (command_call == NULL) {
        return NULL;
    }
    command_result *command_result = malloc(sizeof(*command_result));
    if (command_result == NULL) {
        perror("malloc");
        return NULL;
    }

    command_result->exit_code = exit_code;
    command_result->call = command_call;
    command_result->pid = UNINITIALIZED_PID;
    command_result->job_id = UNINITIALIZED_JOB_ID;
    return command_result;
}

void destroy_command_result(command_result *command_result) {
    if (command_result == NULL) {
        return;
    }
    destroy_command_call(command_result->call);
    free(command_result);
}

command_call *parse_command(char *command_string) {

    size_t argc;
    char **parsed_command_string = split_string(command_string, COMMAND_SEPARATOR, &argc);
    if (argc == 0) {
        free(parsed_command_string);
        return NULL;
    }

    parsed_command_string = realloc(parsed_command_string, sizeof(char *) * (argc + 1));
    if (parsed_command_string == NULL) {
        perror("realloc");
        return NULL;
    }
    parsed_command_string[argc] = NULL;

    command_call *command = new_command_call(argc, parsed_command_string);

    return command;
}

command_call **parse_read_line(char *command_string, size_t *total_commands) {

    if (starts_with(command_string, BACKGROUND_FLAG)) {
        *total_commands = 0;
        return NULL;
    }

    // Split with BACKGROUND_FLAG
    int last_background_command_index = -1;
    char **bg_flag_parsed =
        split_string_keep_trace(command_string, BACKGROUND_FLAG, total_commands, &last_background_command_index);
    if (bg_flag_parsed == NULL) {
        return NULL;
    }

    command_call **commands = malloc(*total_commands * sizeof(command_call));
    if (commands == NULL) {
        perror("malloc");
        for (size_t index = 0; index < *total_commands; ++index) {
            free(bg_flag_parsed[index]);
        }
        free(bg_flag_parsed);
        return NULL;
    }

    // Parse each command
    int abort = 0, trim_last = 0;
    for (size_t index = 0; index < *total_commands; ++index) {
        commands[index] = NULL;
        if (!abort) {
            commands[index] = parse_command(bg_flag_parsed[index]);

            if (commands[index] == NULL) {
                if (index == *total_commands - 1) {
                    trim_last = 1;
                } else {
                    abort = 1;
                }
            } else {
                if ((int)((ssize_t)index) <= last_background_command_index) {
                    commands[index]->background = 1;
                }
            }
        }
    }

    for (size_t index = 0; index < *total_commands; ++index) {
        free(bg_flag_parsed[index]);
    }
    free(bg_flag_parsed);

    if (abort) {
        for (size_t to_free = 0; to_free < *total_commands; ++to_free) {
            free(commands[to_free]);
        }
        free(commands);
        return NULL;
    }

    if (!trim_last) {
        return commands;
    }

    return reallocarray(commands, --*total_commands, sizeof(command_call));
}
