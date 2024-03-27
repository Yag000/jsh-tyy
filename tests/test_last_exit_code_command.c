#include "../src/command.h"
#include "../src/internals.h"
#include "test_core.h"
#include <unistd.h>

#define NUM_TEST 4

void test_invalid_arguments(test_info *);
void test_as_first_command(test_info *);
void test_after_failed_command(test_info *);
void test_after_successful_command(test_info *);

test_info *test_last_exit_code_command() {
    test_case cases[NUM_TEST] = {QUICK_CASE("? command with invalid arguments", test_invalid_arguments),
                                 QUICK_CASE("? command as first command", test_as_first_command),
                                 QUICK_CASE("? command after failed command", test_after_failed_command),
                                 QUICK_CASE("? command after successful command", test_after_successful_command)};

    return cinta_run_cases("?", cases, NUM_TEST);
}

void test_invalid_arguments(test_info *info) {
    command *command;

    command_result *result;

    int error_fd;

    init_internals();

    command = parse_command("? test");
    error_fd = open_test_file_to_write("test_last_exit_code_command_invalid_arguments.log");
    command->command_calls[0]->stderr = error_fd;
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 1, info);

    destroy_command_result(result);
}

void test_as_first_command(test_info *info) {
    command *command;

    command_result *result;

    int out_fd;
    init_internals();

    command = parse_command("?");
    out_fd = open_test_file_to_write("test_last_exit_code_command_as_first_command.log");
    command->command_calls[0]->stdout = out_fd;
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 0, info);

    destroy_command_result(result);
}

void test_after_failed_command(test_info *info) {
    command *command;

    command_result *result;

    int out_fd;
    int error_fd;

    init_internals();

    command = parse_command("? no arguments should be allowed");
    error_fd = open_test_file_to_write("test_last_exit_code_command_after_failed_command_errors.log");
    command->command_calls[0]->stderr = error_fd;
    result = execute_command(command);

    destroy_command_result(result);

    command = parse_command("?");
    out_fd = open_test_file_to_write("test_last_exit_code_command_after_failed_command.log");
    command->command_calls[0]->stdout = out_fd;
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 0, info);

    destroy_command_result(result);

    int read_fd = open_test_file_to_read("test_last_exit_code_command_after_failed_command.log");
    char *error_code = malloc(2);
    read(read_fd, error_code, 1);
    error_code[1] = '\0';
    close(read_fd);
    CINTA_ASSERT_STRING(error_code, "1", info);
    free(error_code);
}

void test_after_successful_command(test_info *info) {
    command *command;

    command_result *result;

    int out_fd = open_test_file_to_write("test_last_exit_code_command_after_successful_command.log");

    init_internals();

    command = parse_command("?");
    command->command_calls[0]->stdout = out_fd;
    result = execute_command(command);

    destroy_command_result(result);

    out_fd = open_test_file_to_write("test_last_exit_code_command_after_successful_command.log");

    command = parse_command("?");
    command->command_calls[0]->stdout = out_fd;
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 0, info);

    destroy_command_result(result);

    int read_fd = open_test_file_to_read("test_last_exit_code_command_after_successful_command.log");
    char *error_code = malloc(2);
    read(read_fd, error_code, 1);
    error_code[1] = '\0';
    close(read_fd);
    CINTA_ASSERT_STRING(error_code, "0", info);
    free(error_code);
}
