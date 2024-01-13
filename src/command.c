#include "command.h"
#include "internals.h"
#include "jobs.h"
#include "string_utils.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char internal_commands[INTERNAL_COMMANDS_COUNT][100] = {"cd", "exit", "pwd", "?", "jobs", "fg", "bg", "kill"};

const char *redirection_caret_symbols[REDIRECTION_CARET_SYMBOLS_COUNT] = {
    ">", "<", ">|", ">>", "2>", "2>|", "2>>", COMMAND_SUBSTITUTION_START, COMMAND_SUBSTITUTION_END};

command_call *parse_command_call(char *, int **, size_t *);

/** Returns a new command call with the given name, argc and argv. */
command_call *new_command_call(size_t argc, char **argv, char *command_string) {
    command_call *command_call = malloc(sizeof(*command_call));
    if (command_call == NULL) {
        perror("malloc");
        return NULL;
    }

    command_call->name = argv[0];
    command_call->argc = argc;
    command_call->argv = argv;
    command_call->background = 0;
    command_call->reading_pipes = NULL;
    command_call->writing_pipes = NULL;
    command_call->dependencies = NULL;
    command_call->dependencies_count = 0;
    command_call->stdin = STDIN_FILENO;
    command_call->stdout = STDOUT_FILENO;
    command_call->stderr = STDERR_FILENO;

    command_call->command_string = malloc((strlen(command_string) + 1) * sizeof(char));
    if (command_string == NULL) {
        return NULL;
    }

    memmove(command_call->command_string, command_string, (strlen(command_string) + 1) * sizeof(char));

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

    if (command_call->dependencies != NULL) {
        for (size_t i = 0; i < command_call->dependencies_count; i++) {
            destroy_command_call(command_call->dependencies[i]);
        }
        free(command_call->dependencies);
    }

    free(command_call->argv);
    free(command_call->command_string);
    destroy_pipe_info(command_call->reading_pipes);
    destroy_pipe_info(command_call->writing_pipes);
    free(command_call);
}

/** Prints the command call. */
void command_call_print(command_call *command_call, int fd) {
    dprintf(fd, "%s", command_call->command_string);
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

command_result *new_command_result(int exit_code, command *command) {
    command_result *command_result = malloc(sizeof(*command_result));
    if (command_result == NULL) {
        perror("malloc");
        return NULL;
    }

    command_result->exit_code = exit_code;
    command_result->pid = UNINITIALIZED_PID;
    command_result->job_id = UNINITIALIZED_JOB_ID;
    command_result->command = command;
    return command_result;
}

void destroy_command_result(command_result *command_result) {
    if (command_result == NULL) {
        return;
    }

    destroy_command(command_result->command);

    free(command_result);
}

command *new_command(command_call *call, int **open_pipes, size_t open_pipes_size, char *command_string) {
    command *command = malloc(sizeof(*command));

    if (command == NULL) {
        perror("malloc");
        return NULL;
    }

    command->call = call;
    command->open_pipes = open_pipes;
    command->open_pipes_size = open_pipes_size;

    char *command_string_copy = malloc((strlen(command_string) + 1) * sizeof(char));
    if (command_string_copy == NULL) {
        perror("malloc");
        return NULL;
    }
    strcpy(command_string_copy, command_string);
    command->command_string = command_string_copy;

    return command;
}

void destroy_command(command *command) {
    destroy_command_call(command->call);

    for (size_t i = 0; i < command->open_pipes_size; i++) {
        free(command->open_pipes[i]);
    }

    free(command->open_pipes);
    free(command->command_string);
    free(command);
}

typedef struct command_call_builder {
    command_call **dependencies;
    int dependencies_count;

    int *fds;
    pipe_info *reading_pipes;
    pipe_info *writing_pipes;

} command_call_builder;

command_call_builder *new_command_call_builder(size_t argc) {
    command_call_builder *c = malloc(sizeof(command_call));

    if (c == NULL) {
        perror("malloc");
        return NULL;
    }

    command_call **dependencies = malloc(argc * sizeof(command_call *));
    if (dependencies == NULL) {
        perror("malloc");
        return NULL;
    }
    size_t dependencies_count = 0;

    pipe_info *reading_pipe_info = new_pipe_info();
    if (reading_pipe_info == NULL) {
        return NULL;
    }

    pipe_info *writing_pipe_info = new_pipe_info();
    if (writing_pipe_info == NULL) {
        return NULL;
    }

    int *fds = malloc(3 * sizeof(int));
    if (fds == NULL) {
        perror("malloc");
        return NULL;
    }

    fds[0] = -1;
    fds[1] = -1;
    fds[2] = -1;

    c->dependencies = dependencies;
    c->dependencies_count = dependencies_count;

    c->fds = fds;
    c->reading_pipes = reading_pipe_info;
    c->writing_pipes = writing_pipe_info;

    return c;
}

void destroy_command_call_builder(command_call_builder *builder) {
    free(builder->fds);
    free(builder);
}

pipe_info *new_pipe_info() {
    pipe_info *pi = malloc(sizeof(pipe_info));

    if (pi == NULL) {
        perror("malloc");
        return NULL;
    }

    pi->pipes = NULL;
    pi->pipe_count = 0;
    return pi;
}

void destroy_pipe_info(pipe_info *pi) {
    if (pi == NULL) {
        return;
    }

    if (pi->pipes != NULL) {
        free(pi->pipes);
    }
    free(pi);
}

void add_pipe(pipe_info *pi, int index) {
    if (pi == NULL) {
        return;
    }

    if (pi->pipes == NULL) {
        pi->pipes = malloc(sizeof(int));

        if (pi->pipes == NULL) {
            perror("malloc");
            return;
        }

        pi->pipes[0] = index;
        pi->pipe_count++;
        return;
    }

    pi->pipes = reallocarray(pi->pipes, pi->pipe_count + 1, sizeof(int));

    if (pi->pipes == NULL) {
        perror("reallocarray");
        return;
    }

    pi->pipes[pi->pipe_count] = index;
    pi->pipe_count++;
    return;
}

int trim_arrays(command_call_builder *builder) {
    builder->dependencies = reallocarray(builder->dependencies, builder->dependencies_count, sizeof(command_call *));

    if (builder->dependencies == NULL) {
        perror("reallocarray");
        return -1;
    }

    return 0;
}

command_call *build_command(command_call_builder *builder, size_t argc, char **argv, char *command_string) {
    command_call *command = new_command_call(argc, argv, command_string);
    if (command == NULL) {
        return NULL;
    }

    if (builder->fds[0] >= 0) {
        command->stdin = builder->fds[0];
    }
    if (builder->fds[1] >= 0) {
        command->stdout = builder->fds[1];
    }
    if (builder->fds[2] >= 0) {
        command->stderr = builder->fds[2];
    }

    command->writing_pipes = builder->writing_pipes;
    command->reading_pipes = builder->reading_pipes;

    if (builder->dependencies_count == 0) {
        free(builder->dependencies);
        return command;
    }

    int error = trim_arrays(builder);

    if (error == -1) {
        return NULL;
    }

    command->dependencies_count = builder->dependencies_count;
    command->dependencies = builder->dependencies;

    return command;
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

void update_dependencies(command_call_builder *builder, command_call *command_call, int index) {
    if (builder == NULL) {
        return;
    }

    builder->dependencies[builder->dependencies_count] = command_call;
    add_pipe(builder->reading_pipes, index);
    add_pipe(command_call->writing_pipes, index);
    builder->dependencies_count++;
}

int parse_command_substitution(command_call_builder *builder, char **command_string, size_t positions_left,
                               size_t *index, int **open_pipes, size_t *total_pipes) {
    size_t i;

    int starts_found = 0;

    for (i = 0; i < positions_left; i++) {

        if (strcmp(command_string[i], COMMAND_SUBSTITUTION_START) == 0) {
            starts_found++;
        }
        if (strcmp(command_string[i], COMMAND_SUBSTITUTION_END) == 0) {
            if (starts_found == 0) {
                break;
            }
            starts_found--;
        }
    }

    // If we are at the end of the string or we have not found enough
    // closing characters that means that the command is malformed.
    if (i == positions_left || starts_found != 0) {
        return -1;
    }

    int pipe_pos = *total_pipes;

    int *fd = malloc(2 * sizeof(int));

    if (fd == NULL) {
        perror("malloc");
        return -1;
    }

    if (pipe(fd) == -1) {
        perror("pipe");
        return -1;
    }

    open_pipes[*total_pipes] = fd;
    *total_pipes += 1;

    char *joined_command = join_strings(command_string, i, " ");
    command_call *command = parse_command_call(joined_command, open_pipes, total_pipes);
    free(joined_command);

    if (command == NULL) {
        return -1;
    }

    command->stdout = fd[1];

    char *dev_file = malloc(PATH_MAX * sizeof(char));

    if (dev_file == NULL) {
        perror("malloc");
        return -1;
    }

    snprintf(dev_file, PATH_MAX, "/dev/fd/%d", fd[0]);

    free(command_string[i]);
    command_string[i] = dev_file;

    for (size_t j = 0; j < i; j++) {
        free(command_string[j]);
        command_string[j] = NULL;
    }

    *index = *index + i + 1;

    update_dependencies(builder, command, pipe_pos);

    return 0;
}

/**
 * Add `dep` as a dependency of `call`.
 */
int add_depencencies(command_call *call, command_call *dep) {
    if (call == NULL || dep == NULL) {
        return -1;
    }

    if (call->dependencies == NULL) {

        call->dependencies = malloc(sizeof(command_call *));
        if (call->dependencies == NULL) {
            perror("malloc");
            return -1;
        }

        call->dependencies[0] = dep;
        call->dependencies_count++;
        return 0;
    }

    call->dependencies = reallocarray(call->dependencies, call->dependencies_count + 1, sizeof(command_call *));

    if (call->dependencies == NULL) {
        perror("reallocarray");
        return -1;
    }

    call->dependencies[call->dependencies_count] = dep;
    call->dependencies_count++;
    return 0;
}

command_call *parse_command_call_with_pipes(char *command_string, int **open_pipes, size_t *total_pipes) {

    size_t total_commands = 0;

    char **pipes_splitted_str = split_string(command_string, PIPE_SYMBOL, &total_commands);

    if (pipes_splitted_str == NULL) {
        return NULL;
    }

    command_call *prev = NULL;
    command_call *main_command = NULL;

    // Compute commands from last to first; last command being the main one.
    for (int index = total_commands - 1; index >= 0; --index) {

        command_call *call = parse_command_call(pipes_splitted_str[index], open_pipes, total_pipes);

        if (prev == NULL) {
            prev = call;
            main_command = call;
            continue;
        }

        int result = add_depencencies(prev, call);

        if (result < 0) {
            return NULL;
        }

        int *fd = malloc(2 * sizeof(int));

        if (fd == NULL) {
            perror("malloc");
            return NULL;
        }

        if (pipe(fd) == -1) {
            perror("pipe");
            return NULL;
        }

        add_pipe(prev->reading_pipes, *total_pipes);
        add_pipe(call->writing_pipes, *total_pipes);

        open_pipes[*total_pipes] = fd;
        *total_pipes += 1;

        prev->stdin = fd[0];
        call->stdout = fd[1];

        prev = call;
    }

    for (size_t index = 0; index < total_commands; ++index) {
        free(pipes_splitted_str[index]);
    }
    free(pipes_splitted_str);

    return main_command;
}

char *sanitize_command_string(char *command_string) {
    char *result = trim_spaces(command_string);

    size_t size = 0;
    char **split = split_string(result, "<( ", &size); // Bash removes this space
    free(result);
    result = join_strings(split, size, "<(");
    for (size_t i = 0; i < size; i++) {
        free(split[i]);
    }
    free(split);

    split = split_string(result, " )", &size); // Bash removes this space
    free(result);
    result = join_strings(split, size, ")");
    for (size_t i = 0; i < size; i++) {
        free(split[i]);
    }
    free(split);

    if (size > 0 && strcmp(result + (strlen(result) - 1), ")") == 0) {
        // This was removed and needs to be added back
        result = reallocarray(result, strlen(result) + 2, sizeof(char));
        if (result == NULL) {
            perror("reallocarray");
            return NULL;
        }
        strcat(result, ")");
    }

    return result;
}

command *parse_command(char *command_string) {

    string_iterator *iterator = new_string_iterator(command_string, COMMAND_SEPARATOR);
    if (iterator == NULL) {
        return NULL;
    }
    int max_open_pipes_size = get_number_of_words_left(iterator);
    destroy_string_iterator(iterator);

    int **open_pipes = malloc(max_open_pipes_size * sizeof(int *));
    if (open_pipes == NULL) {
        perror("malloc");
        return NULL;
    }
    size_t total_pipes = 0;

    command_call *call = parse_command_call_with_pipes(command_string, open_pipes, &total_pipes);

    if (call == NULL) {
        free(open_pipes);
        return NULL;
    }

    if (total_pipes == 0) {
        free(open_pipes);
        open_pipes = NULL;
    } else {
        open_pipes = reallocarray(open_pipes, total_pipes, sizeof(int *));

        if (open_pipes == NULL) {
            perror("reallocarray");
            return NULL;
        }
    }

    char *sanitized_command_string = sanitize_command_string(command_string);
    command *command = new_command(call, open_pipes, total_pipes, sanitized_command_string);
    free(sanitized_command_string);

    return command;
}

command_call *parse_command_call(char *command_string, int **open_pipes, size_t *total_pipes) {
    size_t argc;
    char **parsed_command_string = split_string(command_string, COMMAND_SEPARATOR, &argc);
    if (argc == 0) {
        free(parsed_command_string);
        return NULL;
    }

    command_call_builder *command_builder = new_command_call_builder(argc);
    if (command_builder == NULL) {
        free(parsed_command_string);
        return NULL;
    }

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

        if (strcmp(parsed_command_string[index], COMMAND_SUBSTITUTION_START) == 0) {
            free(parsed_command_string[index]);
            parsed_command_string[index] = NULL;

            if (parse_command_substitution(command_builder, parsed_command_string + index + 1, argc - index - 1, &index,
                                           open_pipes, total_pipes) == -1) {

                dprintf(STDERR_FILENO, "jsh: parse error near %s\n", parsed_command_string[index]);
                free(parsed_command_string);
                return NULL;
            }

        } else {
            int status = parse_redirections(command_builder->fds, parsed_command_string[index],
                                            parsed_command_string[index + 1]);

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
            redirection_parsed_command_string[new_index] = parsed_command_string[index];
            new_index++;
        }
    }

    free(parsed_command_string);

    char *trimmed_command_string = trim_spaces(command_string);
    command_call *command =
        build_command(command_builder, not_null_arguments, redirection_parsed_command_string, trimmed_command_string);
    free(trimmed_command_string);
    destroy_command_call_builder(command_builder);

    return command;
}

command **parse_read_line(char *command_string, size_t *total_commands) {

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

    command **commands = malloc(*total_commands * sizeof(command_call));
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
                    commands[index]->call->background = 1;
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

    *total_commands -= 1;
    commands = reallocarray(commands, *total_commands, sizeof(command_call));

    if (commands == NULL) {
        perror("reallocarray");
        return NULL;
    }

    return commands;
}
