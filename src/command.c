#include "command.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char internal_commands[INTERNAL_COMMANDS_COUNT][100] = {"cd", "exit", "pwd", "?", "jobs", "fg", "bg", "kill"};

/** Returns a new command call with the given name, argc and argv. */
command_call *new_command_call(char *name, int argc, char **argv) {
    command_call *command_call = malloc(sizeof(*command_call));
    command_call->name = name;
    command_call->argc = argc;
    command_call->argv = argv;
    command_call->stdin = 0;
    command_call->stdout = 1;
    command_call->stderr = 2;
    return command_call;
}

/** Frees the memory allocated for the command call. */
void destroy_command_call(command_call *command_call) {
    free(command_call);
}

/** Prints the command call. */
void command_call_print(command_call *command_call) {
    int i;
    // This takes care of whitespace size
    int command_call_length = command_call->argc;
    for (i = 0; i < command_call->argc; i++) {
        command_call_length += strlen(command_call->argv[i]);
    }
    char *buffer = malloc(sizeof(char) * command_call_length);
    for (i = 0; i < command_call->argc; i++) {
        if (i == command_call->argc - 1) {
            sprintf(buffer, "%s", command_call->argv[i]);
            write(command_call->stdout, buffer, strlen(buffer));
        } else {
            sprintf(buffer, "%s ", command_call->argv[i]);
            write(command_call->stdout, buffer, strlen(buffer));
        }
    }
    free(buffer);
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
