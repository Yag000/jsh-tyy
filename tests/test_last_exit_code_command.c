#include "../src/command.h"
#include "../src/internals.h"
#include "test_core.h"
#include <unistd.h>

void test_invalid_arguments(test_info *);
void test_as_first_command(test_info *);
void test_after_failed_command(test_info *);
void test_after_successful_command(test_info *);

test_info *test_last_exit_code_command() {
    // Test setup
    print_test_header("? command");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Test body
    test_invalid_arguments(info);
    test_as_first_command(info);
    test_after_failed_command(info);
    test_after_successful_command(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("? command", info);
    return info;
}

void test_invalid_arguments(test_info *info) {
    command *command;

    command_result *result;

    int error_fd;
    print_test_name("? command with invalid arguments");

    init_internals();

    command = parse_command("? test");
    error_fd = open_test_file_to_write("test_last_exit_code_command_invalid_arguments.log");
    command->call->stderr = error_fd;
    result = execute_command(command);

    handle_int_test(result->exit_code, 1, __LINE__, __FILE__, info);

    destroy_command_result(result);
}

void test_as_first_command(test_info *info) {
    command *command;

    command_result *result;

    int out_fd;
    print_test_name("? command as first command");

    init_internals();

    command = parse_command("?");
    out_fd = open_test_file_to_write("test_last_exit_code_command_as_first_command.log");
    command->call->stdout = out_fd;
    result = execute_command(command);

    handle_int_test(result->exit_code, 0, __LINE__, __FILE__, info);

    destroy_command_result(result);
}

void test_after_failed_command(test_info *info) {
    command *command;

    command_result *result;

    int out_fd;
    int error_fd;
    print_test_name("? command after failed command");

    init_internals();

    command = parse_command("? no arguments should be allowed");
    error_fd = open_test_file_to_write("test_last_exit_code_command_after_failed_command_errors.log");
    command->call->stderr = error_fd;
    result = execute_command(command);

    destroy_command_result(result);

    command = parse_command("?");
    out_fd = open_test_file_to_write("test_last_exit_code_command_after_failed_command.log");
    command->call->stdout = out_fd;
    result = execute_command(command);

    handle_int_test(result->exit_code, 0, __LINE__, __FILE__, info);

    destroy_command_result(result);

    int read_fd = open_test_file_to_read("test_last_exit_code_command_after_failed_command.log");
    char *error_code = malloc(2);
    read(read_fd, error_code, 1);
    error_code[1] = '\0';
    close(read_fd);
    handle_string_test(error_code, "1", __LINE__, __FILE__, info);
    free(error_code);
}

void test_after_successful_command(test_info *info) {
    command *command;

    command_result *result;

    print_test_name("? command after successful command");
    int out_fd = open_test_file_to_write("test_last_exit_code_command_after_successful_command.log");

    init_internals();

    command = parse_command("?");
    command->call->stdout = out_fd;
    result = execute_command(command);

    destroy_command_result(result);

    out_fd = open_test_file_to_write("test_last_exit_code_command_after_successful_command.log");

    command = parse_command("?");
    command->call->stdout = out_fd;
    result = execute_command(command);

    handle_int_test(result->exit_code, 0, __LINE__, __FILE__, info);

    destroy_command_result(result);

    int read_fd = open_test_file_to_read("test_last_exit_code_command_after_successful_command.log");
    char *error_code = malloc(2);
    read(read_fd, error_code, 1);
    error_code[1] = '\0';
    close(read_fd);
    handle_string_test(error_code, "0", __LINE__, __FILE__, info);
    free(error_code);
}
