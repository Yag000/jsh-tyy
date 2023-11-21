#include "../src/command.h"
#include "test_core.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void test_no_arguments_command_call_print(test_info *info);
void test_command_call_print_with_arguments(test_info *info);

test_info *test_command() {

    // Test setup
    print_test_header("command");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add tests here
    test_no_arguments_command_call_print(info);
    test_command_call_print_with_arguments(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("command", info);
    return info;
}

void test_no_arguments_command_call_print(test_info *info) {
    print_test_name("Testing command print with no arguments");

    int fd = open_test_file_to_write(".test_command_call_print");
    char *argv[1];
    argv[0] = "pwd";
    command_call *command_call = new_command_call("pwd", 1, argv);
    command_call->stdout = fd;

    command_call_print(command_call);

    close(fd);

    fd = open_test_file_to_read(".test_command_call_print");
    char buffer[4];
    buffer[3] = '\0';
    read(fd, buffer, 3);

    handle_string_test(buffer, "pwd", __LINE__, __FILE__, info);

    destroy_command_call(command_call);
}

void test_command_call_print_with_arguments(test_info *info) {
    print_test_name("Testing command print with arguments");

    int fd;
    fd = open_test_file_to_write(".test_command_call_print");
    char *argv[2];
    argv[0] = "pwd";
    argv[1] = "test";
    command_call *command_call = new_command_call("pwd", 2, argv);
    command_call->stdout = fd;

    command_call_print(command_call);

    close(fd);

    fd = open_test_file_to_read(".test_command_call_print");
    char buffer[9];
    buffer[8] = '\0';

    read(fd, buffer, 8);

    handle_string_test(buffer, "pwd test", __LINE__, __FILE__, info);

    destroy_command_call(command_call);

    fd = open_test_file_to_write(".test_command_call_print");
    char *argv2[3];
    argv2[0] = "pwd";
    argv2[1] = "test";
    argv2[2] = "test2";
    command_call = new_command_call("pwd", 3, argv2);
    command_call->stdout = fd;

    command_call_print(command_call);

    close(fd);

    fd = open_test_file_to_read(".test_command_call_print");
    char buffer2[15];
    buffer2[14] = '\0';

    read(fd, buffer2, 14);

    handle_string_test(buffer2, "pwd test test2", __LINE__, __FILE__, info);

    destroy_command_call(command_call);
}
