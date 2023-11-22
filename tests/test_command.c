#include "../src/command.h"
#include "../src/string_utils.h"
#include "test_core.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void test_no_arguments_command_call_print(test_info *info);
void test_command_call_print_with_arguments(test_info *info);
void test_case_parse_command(test_info *info);

test_info *test_command() {

    // Test setup
    print_test_header("command");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add tests here
    test_no_arguments_command_call_print(info);
    test_command_call_print_with_arguments(info);
    test_case_parse_command(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("command", info);
    return info;
}

void test_no_arguments_command_call_print(test_info *info) {
    print_test_name("Testing command print with no arguments");

    int fd = open_test_file_to_write("test_command_call_print.log");
    size_t size;
    char **argv = split_string("pwd", " ", &size);
    command_call *command_call = new_command_call(1, argv);
    command_call->stdout = fd;

    command_call_print(command_call);

    close(fd);

    fd = open_test_file_to_read("test_command_call_print.log");
    char buffer[4];
    buffer[3] = '\0';
    read(fd, buffer, 3);
    close(fd);

    char *expected = join_strings(argv, size, " ");
    handle_string_test(buffer, expected, __LINE__, __FILE__, info);
    free(expected);

    destroy_command_call(command_call);
}

void test_command_call_print_with_arguments(test_info *info) {
    size_t size;
    char **argv;
    char *expected;

    print_test_name("Testing command print with arguments");

    int fd;
    fd = open_test_file_to_write("test_command_call_print.log");
    argv = split_string("pwd test", " ", &size);
    command_call *command_call = new_command_call(size, argv);
    command_call->stdout = fd;

    command_call_print(command_call);

    close(fd);

    fd = open_test_file_to_read("test_command_call_print.log");
    char buffer[9];
    buffer[8] = '\0';

    read(fd, buffer, 8);
    close(fd);

    expected = join_strings(argv, size, " ");
    handle_string_test(buffer, expected, __LINE__, __FILE__, info);
    free(expected);

    destroy_command_call(command_call);

    fd = open_test_file_to_write("test_command_call_print.log");
    argv = split_string("pwd test test2", " ", &size);
    command_call = new_command_call(3, argv);
    command_call->stdout = fd;

    command_call_print(command_call);

    close(fd);

    fd = open_test_file_to_read("test_command_call_print.log");
    char buffer2[15];
    buffer2[14] = '\0';

    read(fd, buffer2, 14);
    close(fd);

    expected = join_strings(argv, size, " ");
    handle_string_test(buffer2, expected, __LINE__, __FILE__, info);
    free(expected);

    destroy_command_call(command_call);
}

void test_case_parse_command(test_info *info) {
    command_call *command_call;

    print_test_name("Testing `parse_command`");

    // Empty command string
    command_call = parse_command("");
    handle_null_test(command_call, __LINE__, __FILE__, info);

    // Command call with no arguments
    command_call = parse_command("^mv^rm");
    char *expected_1[1] = {"^mv^rm"};
    handle_string_test(expected_1[0], command_call->name, __LINE__, __FILE__, info);
    handle_int_test(1, command_call->argc, __LINE__, __FILE__, info);
    for (size_t index = 0; index < command_call->argc; ++index) {
        handle_string_test(expected_1[index], command_call->argv[index], __LINE__, __FILE__, info);
    }
    destroy_command_call(command_call);

    // Command call with few arguments
    command_call = parse_command("nvim -A /absolute/path/to/launch/in/neovim");
    char *expected_2[3] = {"nvim", "-A", "/absolute/path/to/launch/in/neovim"};
    handle_string_test(expected_2[0], command_call->name, __LINE__, __FILE__, info);
    handle_int_test(3, command_call->argc, __LINE__, __FILE__, info);
    for (size_t index = 0; index < command_call->argc; ++index) {
        handle_string_test(expected_2[index], command_call->argv[index], __LINE__, __FILE__, info);
    }
    handle_int_test(0, command_call->stdin, __LINE__, __FILE__, info);
    handle_int_test(1, command_call->stdout, __LINE__, __FILE__, info);
    handle_int_test(2, command_call->stderr, __LINE__, __FILE__, info);
    destroy_command_call(command_call);

    // Another command call with few arguments
    command_call = parse_command("rm -rf --no-preserve-root /");
    char *expected_3[4] = {"rm", "-rf", "--no-preserve-root", "/"};
    handle_string_test(expected_3[0], command_call->name, __LINE__, __FILE__, info);
    handle_int_test(4, command_call->argc, __LINE__, __FILE__, info);
    for (size_t index = 0; index < command_call->argc; ++index) {
        handle_string_test(expected_3[index], command_call->argv[index], __LINE__, __FILE__, info);
    }
    destroy_command_call(command_call);
}
