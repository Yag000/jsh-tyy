#include "command.h"
#include "internals.h"
#include "jobs.h"
#include "string_utils.h"
#include "utils.h"

#include <errno.h>
#include <fcntl.h>

const char internal_commands[INTERNAL_COMMANDS_COUNT][100] = {"cd", "exit", "pwd", "?", "jobs", "fg", "bg", "kill"};

const char *redirection_caret_symbols[REDIRECTION_CARET_SYMBOLS_COUNT] = {
    ">", "<", ">|", ">>", "2>", "2>|", "2>>", COMMAND_SUBSTITUTION_START, COMMAND_SUBSTITUTION_END, PIPE_SYMBOL};

#define UNINITIALIZED_FD -1

// This variable represents the last command call on the main pipeline
// for example in a | b | c it should be c
// and in a | b | c <( d ) it should also be c
// The call to `parse_command` resets it.
command_call *last_parsed_command_call = NULL;

// The same as the previous one but this is limited to a particular
// substitution. It gets reset every time we parse a new substitution.
command_call *last_parsed_command_call_substitution = NULL;

command_call *parse_command_call(command *, char *, int, int);

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
    command_call->reading_pipes = NULL;
    command_call->writing_pipes = NULL;
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

command *new_command(command_call **call, size_t command_call_count, int **open_pipes, size_t open_pipes_size,
                     char *command_string) {
    command *command = malloc(sizeof(*command));

    if (command == NULL) {
        perror("malloc");
        return NULL;
    }

    command->command_calls = call;
    command->command_call_count = command_call_count;
    command->background = 0;
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

command *new_empty_command(char *command_string) {
    if (command_string == NULL) {
        return NULL;
    }
    if (strlen(command_string) == 0) {
        return NULL;
    }
    if (is_only_composed_of(command_string, COMMAND_SEPARATOR)) {
        return NULL;
    }

    command *command = malloc(sizeof(*command));

    if (command == NULL) {
        perror("malloc");
        return NULL;
    }

    command->command_calls = NULL;
    command->command_call_count = 0;
    command->command_string = NULL;
    command->background = 0;
    command->open_pipes = NULL;
    command->open_pipes_size = 0;

    return command;
}

void destroy_command(command *command) {
    if (command == NULL) {
        return;
    }

    for (size_t i = 0; i < command->command_call_count; i++) {
        destroy_command_call(command->command_calls[i]);
    }
    free(command->command_calls);

    for (size_t i = 0; i < command->open_pipes_size; i++) {
        free(command->open_pipes[i]);
    }

    free(command->open_pipes);
    free(command->command_string);
    free(command);
}

typedef struct command_call_builder {
    int *fds;
    pipe_info *reading_pipes;
    pipe_info *writing_pipes;

} command_call_builder;

command_call_builder *new_command_call_builder() {
    command_call_builder *c = malloc(sizeof(command_call));

    if (c == NULL) {
        perror("malloc");
        return NULL;
    }

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

    fds[0] = UNINITIALIZED_FD;
    fds[1] = UNINITIALIZED_FD;
    fds[2] = UNINITIALIZED_FD;

    c->fds = fds;
    c->reading_pipes = reading_pipe_info;
    c->writing_pipes = writing_pipe_info;

    return c;
}

void soft_destroy_command_call_builder(command_call_builder *builder) {
    free(builder->fds);
    free(builder);
}

void destroy_command_call_builder(command_call_builder *builder) {
    if (builder == NULL) {
        return;
    }

    destroy_pipe_info(builder->reading_pipes);
    destroy_pipe_info(builder->writing_pipes);
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

int add_pipe(command *command, int *pipe) {
    if (command == NULL) {
        return -1;
    }

    command->open_pipes = reallocarray(command->open_pipes, command->open_pipes_size + 1, sizeof(int *));

    if (command->open_pipes == NULL) {
        perror("reallocarray");
        return -1;
    }

    command->open_pipes[command->open_pipes_size] = pipe;
    command->open_pipes_size++;

    return 0;
}

void add_pipe_pipe_info(pipe_info *pi, int index) {
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

void add_call_at_the_beginning(command *command, command_call *call) {
    if (command == NULL || call == NULL) {
        return;
    }

    command_call **new_command_calls = malloc((command->command_call_count + 1) * sizeof(command_call *));
    if (new_command_calls == NULL) {
        perror("malloc");
        return;
    }

    new_command_calls[0] = call;

    for (size_t index = 0; index < command->command_call_count; ++index) {
        new_command_calls[index + 1] = command->command_calls[index];
    }

    free(command->command_calls);
    command->command_calls = new_command_calls;
    command->command_call_count++;
}

void add_call_at_the_end(command *command, command_call *call) {
    if (command == NULL || call == NULL) {
        return;
    }

    command->command_calls =
        reallocarray(command->command_calls, command->command_call_count + 1, sizeof(command_call *));
    if (command->command_calls == NULL) {
        perror("reallocarray");
        return;
    }

    command->command_calls[command->command_call_count] = call;
    command->command_call_count++;
}

command_call *build_command(command_call_builder *builder, size_t argc, char **argv, char *command_string) {
    command_call *command = new_command_call(argc, argv, command_string);
    if (command == NULL) {
        return NULL;
    }

    if (builder->fds[0] != UNINITIALIZED_FD) {
        command->stdin = builder->fds[0];
    }
    if (builder->fds[1] != UNINITIALIZED_FD) {
        command->stdout = builder->fds[1];
    }
    if (builder->fds[2] != UNINITIALIZED_FD) {
        command->stderr = builder->fds[2];
    }

    command->writing_pipes = builder->writing_pipes;
    command->reading_pipes = builder->reading_pipes;

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

    add_pipe_pipe_info(builder->reading_pipes, index);
    add_pipe_pipe_info(command_call->writing_pipes, index);
}

int parse_command_substitution(command *command, command_call_builder *builder, char **command_string,
                               size_t positions_left, size_t *index) {

    last_parsed_command_call_substitution = NULL;

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

    int pipe_pos = command->open_pipes_size;

    int *fd = malloc(2 * sizeof(int));

    if (fd == NULL) {
        perror("malloc");
        return -1;
    }

    if (pipe(fd) == -1) {
        perror("pipe");
        return -1;
    }

    add_pipe(command, fd);

    char *joined_command = join_strings(command_string, i, " ");
    if (joined_command == NULL) {
        return -1;
    }
    command_call *call = parse_command_call(command, joined_command, 1, 0);
    free(joined_command);

    if (last_parsed_command_call_substitution == NULL) {
        return -1;
    }

    if (call == NULL) {
        return -1;
    }

    // The last element of process substitution should not redirect stdout
    if (last_parsed_command_call_substitution->stdout != STDOUT_FILENO) {
        destroy_command_call(call);
        int fds[3] = {call->stdin, call->stdout, call->stderr};
        close_unused_file_descriptors_from_array(fds, 3);
        dprintf(STDERR_FILENO, "jsh: parse error\n");
        return -1;
    }

    last_parsed_command_call_substitution->stdout = fd[1];

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

    update_dependencies(builder, last_parsed_command_call_substitution, pipe_pos);

    last_parsed_command_call_substitution = NULL;

    add_call_at_the_end(command, call);

    return 0;
}

int link_pipelines(command *command, command_call_builder *prev, command_call *call) {

    int *fd = malloc(2 * sizeof(int));

    if (fd == NULL) {
        perror("malloc");
        return -1;
    }

    if (pipe(fd) == -1) {
        perror("pipe");
        return -1;
    }

    add_pipe_pipe_info(prev->writing_pipes, command->open_pipes_size);
    add_pipe_pipe_info(call->reading_pipes, command->open_pipes_size);

    add_pipe(command, fd);

    prev->fds[1] = fd[1];
    call->stdin = fd[0];

    return 0;
}

char *sanitize_command_string(char *command_string) {
    char *result = trim_spaces(command_string);
    return result;
}

command *parse_command(char *command_string) {
    command *command = new_empty_command(command_string);

    if (command == NULL) {
        return NULL;
    }

    char *sanitized_command_string = sanitize_command_string(command_string);
    if (sanitized_command_string == NULL) {
        destroy_command(command);
        return NULL;
    }
    command->command_string = sanitized_command_string;

    last_parsed_command_call = NULL;

    command_call *call = parse_command_call(command, command_string, 0, 0);

    if (call == NULL) {
        if (last_parsed_command_call != NULL) {
            // We add it to let command take care of closing the file descriptors and destroying the command call
            add_call_at_the_end(command, last_parsed_command_call);
        }
        goto error;
    }

    if (last_parsed_command_call == NULL) {
        // We add it to let command take care of closing the file descriptors and destroying the command call
        add_call_at_the_end(command, call);
        goto error;
    }

    if (last_parsed_command_call != call) {
        add_call_at_the_end(command, call);
    }

    add_call_at_the_beginning(command, last_parsed_command_call);

    last_parsed_command_call = NULL;

    return command;

error:
    close_unused_file_descriptors(command);
    if (command->open_pipes != NULL) {
        for (size_t index = 0; index < command->open_pipes_size; ++index) {
            if (command->open_pipes[index] == NULL) {
                continue;
            }
            close(command->open_pipes[index][0]);
            close(command->open_pipes[index][1]);
        }
    }
    destroy_command(command);
    return NULL;
}

char *remove_extra_pipes(char *command_string) {
    char *result = malloc((strlen(command_string) + 1) * sizeof(char));
    if (result == NULL) {
        perror("malloc");
        return NULL;
    }

    int found_substitutions = 0;
    size_t i;

    for (i = 0; i < strlen(command_string); i++) {
        // I'd rather avoid this kind of hacks but for this time it's ok
        if (command_string[i] == '(') {
            found_substitutions++;
        } else if (command_string[i] == ')') {
            found_substitutions--;
        } else {
            // This means that we have a pipe that it's not inside a substitution
            // so we ignore the rest of the string
            if (command_string[i] == '|' && found_substitutions == 0) {
                break;
            }
        }
        result[i] = command_string[i];
    }

    result[i] = '\0';
    result = reallocarray(result, i + 1, sizeof(char));
    if (result == NULL) {
        perror("reallocarray");
        return NULL;
    }

    return result;
}

command_call *parse_command_call(command *command, char *command_string, int inside_substitution, int inside_pipeline) {
    size_t argc;
    char **parsed_command_string = split_string(command_string, COMMAND_SEPARATOR, &argc);
    if (argc == 0 || parsed_command_string == NULL) {
        free(parsed_command_string);
        return NULL;
    }

    command_call_builder *command_builder = new_command_call_builder();
    if (command_builder == NULL) {
        for (size_t index = 0; index < argc; ++index) {
            free(parsed_command_string[index]);
        }
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

        if (index == argc - 1 || parsed_command_string[index + 1] == NULL || index == 0) {
            dprintf(STDERR_FILENO, "jsh: parse error\n");
            goto error;
        }

        if (contains_string(redirection_caret_symbols, REDIRECTION_CARET_SYMBOLS_COUNT,
                            parsed_command_string[index + 1])) {
            dprintf(STDERR_FILENO, "jsh: parse error near %s\n", parsed_command_string[index]);
            goto error;
        }

        if (strcmp(parsed_command_string[index], COMMAND_SUBSTITUTION_START) == 0) {
            free(parsed_command_string[index]);
            parsed_command_string[index] = NULL;

            if (parse_command_substitution(command, command_builder, parsed_command_string + index + 1,
                                           argc - index - 1, &index) == -1) {

                dprintf(STDERR_FILENO, "jsh: parse error near %s\n", parsed_command_string[index]);
                goto error;
            }
        } else if (strcmp(parsed_command_string[index], PIPE_SYMBOL) == 0) {
            // If we find a pipe symbol we need to make sure that we do not have set an stdout, since
            // we need to pipe the output of this command to the next one.
            if (command_builder->fds[1] != UNINITIALIZED_FD) {
                dprintf(STDERR_FILENO, "jsh: parse error near %s\n", parsed_command_string[index]);
                goto error;
            }
            char *joined = join_strings(parsed_command_string + index + 1, argc - index - 1, " ");

            command_call *pipped_command_call = parse_command_call(command, joined, inside_substitution, 1);
            free(joined);
            if (pipped_command_call == NULL) {
                dprintf(STDERR_FILENO, "jsh: parse error near %s\n", parsed_command_string[index]);
                goto error;
            }

            if (link_pipelines(command, command_builder, pipped_command_call) == -1) {
                dprintf(STDERR_FILENO, "jsh: parse error near %s\n", parsed_command_string[index]);
                goto error;
            }

            if (last_parsed_command_call != pipped_command_call || inside_substitution) {
                add_call_at_the_end(command, pipped_command_call);
            }

            // A pipe indicated the end of a command, so no need to parse the rest since
            // `pipped_command_call` has already done it.
            for (size_t i = index; i < argc; i++) {
                free(parsed_command_string[i]);
                parsed_command_string[i] = NULL;
            }

            break;
        } else {
            int status = parse_redirections(command_builder->fds, parsed_command_string[index],
                                            parsed_command_string[index + 1]);

            if (status < 0) {
                goto error;
            }

            free(parsed_command_string[index]);
            free(parsed_command_string[index + 1]);

            parsed_command_string[index] = NULL;
            parsed_command_string[index + 1] = NULL;
            ++index;
        }
    }

    if (inside_pipeline) {
        if (command_builder->fds[0] != UNINITIALIZED_FD) {
            dprintf(STDERR_FILENO, "jsh: parse error\n");
            goto error;
        }

        // This condition means that we are inside a pipeline
        // but not at the end, which means that we are not allowed to redirect stdout.
        if (!(last_parsed_command_call == NULL || last_parsed_command_call_substitution == NULL)) {
            if (command_builder->fds[1] != UNINITIALIZED_FD) {
                dprintf(STDERR_FILENO, "jsh: parse error\n");
                goto error;
            }
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
        goto error;
    }
    redirection_parsed_command_string[not_null_arguments] = NULL;

    for (size_t index = 0, new_index = 0; index < argc; ++index) {
        if (parsed_command_string != NULL && parsed_command_string[index] != NULL) {
            redirection_parsed_command_string[new_index] = parsed_command_string[index];
            new_index++;
        }
    }

    free(parsed_command_string);

    char *trimmed_command_string = sanitize_command_string(command_string);
    if (trimmed_command_string == NULL) {
        free(redirection_parsed_command_string);
        return NULL;
    }
    char *removed_extra_pipes = remove_extra_pipes(trimmed_command_string);
    if (removed_extra_pipes == NULL) {
        free(trimmed_command_string);
        free(redirection_parsed_command_string);
        return NULL;
    }
    free(trimmed_command_string);
    command_call *command_call =
        build_command(command_builder, not_null_arguments, redirection_parsed_command_string, removed_extra_pipes);
    free(removed_extra_pipes);
    soft_destroy_command_call_builder(command_builder);

    if (last_parsed_command_call == NULL && !inside_substitution) {
        last_parsed_command_call = command_call;
    }

    if (last_parsed_command_call_substitution == NULL && inside_substitution) {
        last_parsed_command_call_substitution = command_call;
    }

    return command_call;

error:
    for (size_t index = 0; index < argc; ++index) {
        free(parsed_command_string[index]);
    }
    free(parsed_command_string);
    close_unused_file_descriptors_from_array(command_builder->fds, 3);
    destroy_command_call_builder(command_builder);

    return NULL;
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

    *total_commands -= 1;
    commands = reallocarray(commands, *total_commands, sizeof(command_call));

    if (commands == NULL) {
        perror("reallocarray");
        return NULL;
    }

    return commands;
}
