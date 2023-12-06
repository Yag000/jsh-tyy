#include "../src/command.h"
#include "test_core.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

void test_destroy_command_call_null();
void test_destroy_command_result_null();

void test_no_arguments_command_call_print(test_info *info);
void test_command_call_print_with_arguments(test_info *info);
void test_case_parse_command(test_info *info);

test_info *test_command() {

    // Test setup
    print_test_header("command");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add tests here
    test_destroy_command_call_null();
    test_destroy_command_result_null();

    test_no_arguments_command_call_print(info);
    test_command_call_print_with_arguments(info);
    test_case_parse_command(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("command", info);
    return info;
}

void test_destroy_command_call_null() {
    print_test_name("Testing `destroy_command_call` with NULL");

    destroy_command_call(NULL);
}

void test_destroy_command_result_null() {
    print_test_name("Testing `destroy_command_result` with NULL");

    destroy_command_result(NULL);
}

void test_no_arguments_command_call_print(test_info *info) {
    print_test_name("Testing command print with no arguments");

    int fd = open_test_file_to_write("test_command_call_print.log");

    int current_stdout = dup(STDOUT_FILENO);

    dup2(fd, STDOUT_FILENO);

    command_call *command = parse_command("pwd");

    command_call_print(command);

    dup2(current_stdout, STDOUT_FILENO);

    close(fd);

    fd = open_test_file_to_read("test_command_call_print.log");
    char buffer[4];
    buffer[3] = '\0';
    read(fd, buffer, 3);
    close(fd);

    char *expected = "pwd";
    handle_string_test(buffer, expected, __LINE__, __FILE__, info);

    destroy_command_call(command);
}

void test_command_call_print_with_arguments(test_info *info) {
    print_test_name("Testing command print with arguments");

    int fd;
    fd = open_test_file_to_write("test_command_call_print.log");
    int current_stdout = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);

    command_call *command = parse_command("pwd test");

    command_call_print(command);

    dup2(current_stdout, STDOUT_FILENO);

    close(fd);

    fd = open_test_file_to_read("test_command_call_print.log");
    char buffer[9];
    buffer[8] = '\0';

    read(fd, buffer, 8);
    close(fd);

    char *expected = "pwd test";
    handle_string_test(buffer, expected, __LINE__, __FILE__, info);

    destroy_command_call(command);

    fd = open_test_file_to_write("test_command_call_print.log");
    current_stdout = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);
    command = parse_command("pwd test test2");

    command_call_print(command);

    dup2(current_stdout, STDOUT_FILENO);

    close(fd);

    fd = open_test_file_to_read("test_command_call_print.log");
    char buffer2[15];
    buffer2[14] = '\0';

    read(fd, buffer2, 14);
    close(fd);

    expected = "pwd test test2";
    handle_string_test(buffer2, expected, __LINE__, __FILE__, info);

    destroy_command_call(command);
}

void test_case_parse_command(test_info *info) {
    command_call *command;

    print_test_name("Testing `parse_command`");

    // Empty command string
    command = parse_command("");
    handle_null_test(command, __LINE__, __FILE__, info);

    // Only spaces
    command = parse_command("     ");
    handle_null_test(command, __LINE__, __FILE__, info);

    // Command call with no arguments
    command = parse_command("^mv^rm");
    char *expected_1[1] = {"^mv^rm"};
    handle_string_test(expected_1[0], command->name, __LINE__, __FILE__, info);
    handle_int_test(1, command->argc, __LINE__, __FILE__, info);
    for (size_t index = 0; index < command->argc; ++index) {
        handle_string_test(expected_1[index], command->argv[index], __LINE__, __FILE__, info);
    }
    handle_null_test(command->argv[command->argc], __LINE__, __FILE__, info);
    destroy_command_call(command);

    // Command call with few arguments
    command = parse_command("nvim -A /absolute/path/to/launch/in/neovim");
    char *expected_2[3] = {"nvim", "-A", "/absolute/path/to/launch/in/neovim"};
    handle_string_test(expected_2[0], command->name, __LINE__, __FILE__, info);
    handle_int_test(3, command->argc, __LINE__, __FILE__, info);
    for (size_t index = 0; index < command->argc; ++index) {
        handle_string_test(expected_2[index], command->argv[index], __LINE__, __FILE__, info);
    }
    handle_int_test(0, command->background, __LINE__, __FILE__, info);
    handle_int_test(0, command->stdin, __LINE__, __FILE__, info);
    handle_int_test(1, command->stdout, __LINE__, __FILE__, info);
    handle_int_test(2, command->stderr, __LINE__, __FILE__, info);
    handle_null_test(command->argv[command->argc], __LINE__, __FILE__, info);
    destroy_command_call(command);

    // Another command call with few arguments
    command = parse_command("rm -rf --no-preserve-root /");
    char *expected_3[4] = {"rm", "-rf", "--no-preserve-root", "/"};
    handle_string_test(expected_3[0], command->name, __LINE__, __FILE__, info);
    handle_int_test(4, command->argc, __LINE__, __FILE__, info);
    for (size_t index = 0; index < command->argc; ++index) {
        handle_string_test(expected_3[index], command->argv[index], __LINE__, __FILE__, info);
    }
    handle_null_test(command->argv[command->argc], __LINE__, __FILE__, info);
    destroy_command_call(command);
}
