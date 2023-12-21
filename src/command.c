#include "command.h"
#include "internals.h"
#include "jobs.h"
#include "string_utils.h"
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char internal_commands[INTERNAL_COMMANDS_COUNT][100] = {"cd", "exit", "pwd", "?", "jobs", "fg", "bg", "kill"};

const char *redirection_caret_symbols[REDIRECTION_CARET_SYMBOLS_COUNT] = {">", "<", ">|", ">>", "2>", "2>|", "2>>"};

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
    command_call->stdin = STDIN_FILENO;
    command_call->stdout = STDOUT_FILENO;
    command_call->stderr = STDERR_FILENO;
    return command_call;
}

/** Frees the memory allocated for the command call.
 * Calls for `close_unused_file_descriptors`.
 */
void destroy_command_call(command_call *command_call) {
    if (command_call == NULL) {
        return;
    }

    for (size_t index = 0; index < command_call->argc; index++) {
        free(command_call->argv[index]);
    }

    free(command_call->argv);
    close_unused_file_descriptors(command_call);
    free(command_call);
}

/** Prints the command call. */
void command_call_print(command_call *command_call, int fd) {
    size_t i;
    for (i = 0; i < command_call->argc; i++) {
        if (i == command_call->argc - 1) {
            dprintf(fd, "%s", command_call->argv[i]);
        } else {
            dprintf(fd, "%s ", command_call->argv[i]);
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

int parse_redirections(int *fds, char *redirection_symbol, char *filename) {
    if (strcmp(redirection_symbol, "<") == 0) {
        int fd = open(filename, O_RDONLY);
        if (fd < 0) {
            dprintf(STDERR_FILENO, "jsh: %s: %s\n", filename, strerror(errno));
            return -1;
        }
        if (fds[0] > 2) {
            close(fds[0]);
        }
        fds[0] = fd;
        return 0;
    } else if (strcmp(redirection_symbol, ">") == 0) {
        int fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
        if (fd < 0) {
            dprintf(STDERR_FILENO, "jsh: %s: cannot overwrite existing file.\n", filename);
            return -1;
        }
        if (fds[1] > 2) {
            close(fds[1]);
        }
        fds[1] = fd;
        return 0;

    } else if (strcmp(redirection_symbol, ">|") == 0) {
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) {
            return -1;
        }
        if (fds[1] > 2) {
            close(fds[1]);
        }
        fds[1] = fd;
        return 0;

    } else if (strcmp(redirection_symbol, ">>") == 0) {
        int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd < 0) {
            return -1;
        }
        if (fds[1] > 2) {
            close(fds[1]);
        }
        fds[1] = fd;
        return 0;

    } else if (strcmp(redirection_symbol, "2>") == 0) {
        int fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
        if (fd < 0) {
            dprintf(STDERR_FILENO, "jsh: %s: cannot overwrite existing file.\n", filename);
            return -1;
        }
        if (fds[2] > 2) {
            close(fds[2]);
        }
        fds[2] = fd;
        return 0;

    } else if (strcmp(redirection_symbol, "2>|") == 0) {
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) {
            return -1;
        }
        if (fds[2] > 2) {
            close(fds[2]);
        }
        fds[2] = fd;
        return 0;

    } else if (strcmp(redirection_symbol, "2>>") == 0) {
        int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd < 0) {
            return -1;
        }
        if (fds[2] > 2) {
            close(fds[2]);
        }
        fds[2] = fd;
        return 0;
    }
    return -1;
}

command_call *parse_command(char *command_string) {

    size_t argc;
    char **parsed_command_string = split_string(command_string, COMMAND_SEPARATOR, &argc);
    if (argc == 0) {
        free(parsed_command_string);
        return NULL;
    }

    int fds[3];

    fds[0] = -1;
    fds[1] = -1;
    fds[2] = -1;

    /*
     * For each argument,
     *  - if it is not a caret symbol, pass
     *  - if the symbol isn't followed by a correct argument; return NULL
     *          -> A wrong argument could be NULL or also a caret symbol
     *
     *  - Then parse caret symbol and filename to file descriptor
     *  - If correctly parsed, map both to NULL arguments
     */
    for (size_t index = 0; index < argc; ++index) {
        if (!contains_string(redirection_caret_symbols, REDIRECTION_CARET_SYMBOLS_COUNT,
                             parsed_command_string[index])) {
            continue;
        }

        if (index == argc - 1 || parsed_command_string[index + 1] == NULL) {
            dprintf(STDERR_FILENO, "jsh: parse error\n");
            free(parsed_command_string);
            return NULL;
        }

        if (contains_string(redirection_caret_symbols, REDIRECTION_CARET_SYMBOLS_COUNT,
                            parsed_command_string[index + 1])) {
            dprintf(STDERR_FILENO, "jsh: parse error near %s\n", parsed_command_string[index]);
            free(parsed_command_string);
            return NULL;
        }

        int status = parse_redirections(fds, parsed_command_string[index], parsed_command_string[index + 1]);

        if (status < 0) {
            free(parsed_command_string);
            return NULL;
        }

        free(parsed_command_string[index]);
        free(parsed_command_string[index + 1]);

        parsed_command_string[index] = NULL;
        parsed_command_string[index + 1] = NULL;
        ++index;
    }

    int not_null_arguments = 0;

    // Filter non NULL arguments
    for (size_t index = 0; index < argc; ++index) {
        if (parsed_command_string[index] != NULL) {
            not_null_arguments += 1;
        }
    }

    char **redirection_parsed_command_string = malloc((not_null_arguments + 1) * sizeof(char *));
    if (redirection_parsed_command_string == NULL) {
        perror("malloc");
        free(parsed_command_string);
        return NULL;
    }
    redirection_parsed_command_string[not_null_arguments] = NULL;

    for (size_t index = 0, new_index = 0; index < argc; ++index) {
        if (parsed_command_string != NULL && parsed_command_string[index] != NULL) {
            redirection_parsed_command_string[new_index++] = parsed_command_string[index];
        }
    }

    free(parsed_command_string);

    command_call *command = new_command_call(not_null_arguments, redirection_parsed_command_string);

    if (fds[0] >= 0) {
        command->stdin = fds[0];
    }
    if (fds[1] >= 0) {
        command->stdout = fds[1];
    }
    if (fds[2] >= 0) {
        command->stderr = fds[2];
    }

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
                last_exit_code = 1;
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

    if (*total_commands == 1) {
        free(commands);
        return NULL;
    }

    commands = reallocarray(commands, --*total_commands, sizeof(command_call));

    if (commands == NULL) {
        perror("reallocarray");
        return NULL;
    }

    return commands;
}
