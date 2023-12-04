#include "../src/command.h"
#include "../src/internals.h"
#include "test_core.h"

void test_exit_no_arguments_no_previous_command(test_info *);
void test_exit_no_arguments_previous_command(test_info *);
void test_exit_multiple_arguments(test_info *);
void test_exit_correct_argument(test_info *);
void test_exit_string_argument(test_info *);
void test_exit_out_of_range_argument(test_info *);

test_info *test_exit() {
    // Test setup
    print_test_header("exit");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add tests here
    test_exit_no_arguments_no_previous_command(info);
    test_exit_no_arguments_previous_command(info);
    test_exit_multiple_arguments(info);
    test_exit_correct_argument(info);
    test_exit_string_argument(info);
    test_exit_out_of_range_argument(info);

    // End of tests
    init_internals();
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("exit", info);
    return info;
}

void test_exit_no_arguments_no_previous_command(test_info *info) {
    command_call *command;
    command_result *result;

    print_test_name("exit no arguments | No previous command");

    init_internals();

    command = parse_command("exit");
    result = execute_command_call(command);

    handle_int_test(result->exit_code, 0, __LINE__, __FILE__, info);
    handle_int_test(should_exit, 1, __LINE__, __FILE__, info);
    handle_int_test(last_exit_code, 0, __LINE__, __FILE__, info);

    destroy_command_result(result);
}

void test_exit_no_arguments_previous_command(test_info *info) {
    command_call *command;
    command_result *result;

    print_test_name("exit no arguments | With 1 previous command failed");

    init_internals();

    int stderr_fd = open_test_file_to_write("test_exit_no_arguments_previous_command.log");
    command = parse_command("? e423423 423 423 42");
    command->stderr = stderr_fd;
    result = execute_command_call(command);
    destroy_command_result(result);
    close(stderr_fd);

    command = parse_command("exit");
    result = execute_command_call(command);

    handle_int_test(result->exit_code, 1, __LINE__, __FILE__, info);
    handle_int_test(should_exit, 1, __LINE__, __FILE__, info);
    handle_int_test(last_exit_code, 1, __LINE__, __FILE__, info);

    destroy_command_result(result);
}

void test_exit_multiple_arguments(test_info *info) {
    command_call *command;
    command_result *result;
    int error_fd;
    print_test_name("exit multiple arguments");

    init_internals();

    command = parse_command("exit 1 2 3");
    error_fd = open_test_file_to_write("test_exit_multiple_arguments.log");
    command->stderr = error_fd;
    result = execute_command_call(command);

    handle_int_test(result->exit_code, 1, __LINE__, __FILE__, info);
    handle_int_test(should_exit, 0, __LINE__, __FILE__, info);
    handle_int_test(last_exit_code, 1, __LINE__, __FILE__, info);

    destroy_command_result(result);
    close(error_fd);
}

void test_exit_correct_argument(test_info *info) {
    command_call *command;
    command_result *result;
    print_test_name("exit correct argument");

    init_internals();

    for (int i = 0; i <= 255; i++) {
        init_internals();
        char *command_string = malloc(10);
        sprintf(command_string, "exit %d", i);
        command = parse_command(command_string);
        result = execute_command_call(command);

        handle_int_test(result->exit_code, i, __LINE__, __FILE__, info);
        handle_int_test(should_exit, 1, __LINE__, __FILE__, info);
        handle_int_test(last_exit_code, i, __LINE__, __FILE__, info);

        destroy_command_result(result);
        free(command_string);
    }
}

void test_exit_string_argument(test_info *info) {
    command_call *command;
    command_result *result;
    int errorfd;
    print_test_name("exit string argument");

    init_internals();

    command = parse_command("exit test");

    errorfd = open_test_file_to_write("test_exit_string_argument.log");
    command->stderr = errorfd;
    result = execute_command_call(command);

    handle_int_test(result->exit_code, 1, __LINE__, __FILE__, info);
    handle_int_test(should_exit, 0, __LINE__, __FILE__, info);
    handle_int_test(last_exit_code, 1, __LINE__, __FILE__, info);

    destroy_command_result(result);
    close(errorfd);
}

void test_exit_out_of_range_argument(test_info *info) {
    command_call *command;
    command_result *result;
    int errorfd;
    print_test_name("exit out of range argument: 256");

    init_internals();

    command = parse_command("exit 256");

    errorfd = open_test_file_to_write("test_exit_out_of_range_argument_256.log");
    command->stderr = errorfd;
    result = execute_command_call(command);

    handle_int_test(result->exit_code, 1, __LINE__, __FILE__, info);
    handle_int_test(should_exit, 0, __LINE__, __FILE__, info);
    handle_int_test(last_exit_code, 1, __LINE__, __FILE__, info);

    destroy_command_result(result);
    close(errorfd);

    print_test_name("exit out of range argument: -1");
    command = parse_command("exit -1");

    errorfd = open_test_file_to_write("test_exit_out_of_range_argument_negative.log");
    command->stderr = errorfd;
    result = execute_command_call(command);

    handle_int_test(result->exit_code, 1, __LINE__, __FILE__, info);
    handle_int_test(should_exit, 0, __LINE__, __FILE__, info);
    handle_int_test(last_exit_code, 1, __LINE__, __FILE__, info);

    destroy_command_result(result);
    close(errorfd);
}
