#include "../src/command.h"
#include "../src/internals.h"
#include "../src/utils.h"
#include "test_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void test_case_home(test_info *);
static void test_case_deeper(test_info *);
static void test_invalid_arguments(test_info *);

test_info *test_pwd() {

    // Test setup
    print_test_header("pwd");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add tests here
    test_case_home(info);
    test_case_deeper(info);
    test_invalid_arguments(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("pwd", info);
    return info;
}

static void test_case_home(test_info *info) {
    command_call *command, *cd_command;

    print_test_name("Testing `cd && pwd`");

    // Get $HOME env var
    char *expected_path = getenv("HOME");

    // Open log file
    int fd = open_test_file_to_write("test_pwd.log");
    cd_command = parse_command("cd");
    cd(cd_command);
    destroy_command_call(cd_command);

    // Go to $HOME
    command = parse_command("pwd");
    command->stdout = fd;
    pwd(command);
    destroy_command_call(command);

    // Go back to previous wd
    cd_command = parse_command("cd -");
    cd(cd_command);
    destroy_command_call(cd_command);

    // Check log file
    int read_fd = open_test_file_to_read("test_pwd.log");
    char buffer[strlen(expected_path)];
    buffer[strlen(expected_path)] = '\0';
    read(read_fd, buffer, strlen(expected_path));

    // Check that there is nothing more than a newline char
    char buffer_newline[2];
    buffer_newline[1] = '\0';
    int eof = read(read_fd, buffer_newline, 2);
    handle_int_test(1, eof, __LINE__, __FILE__, info);
    close(read_fd);

    handle_string_test(expected_path, buffer, __LINE__, __FILE__, info);
}

static void test_case_deeper(test_info *info) {
    command_call *command, *cd_command;

    print_test_name("Testing `cd tmp/dir && pwd`");

    // Open log file
    int fd = open_test_file_to_write("test_pwd.log");
    cd_command = parse_command("cd tmp/dir");
    cd(cd_command);
    destroy_command_call(cd_command);

    // Go to $HOME
    command = parse_command("pwd");
    command->stdout = fd;
    pwd(command);
    destroy_command_call(command);

    char *expected_path = get_current_wd();

    // Go back to previous wd
    cd_command = parse_command("cd -");
    cd(cd_command);
    destroy_command_call(cd_command);

    // Check log file
    int read_fd = open_test_file_to_read("test_pwd.log");
    char buffer[strlen(lwd)];
    buffer[strlen(lwd)] = '\0';

    read(read_fd, buffer, strlen(lwd));

    // Check that there is nothing more than a newline char
    char buffer_newline[2];
    buffer_newline[1] = '\0';
    int eof = read(fd, buffer_newline, 2);
    handle_int_test(1, eof, __LINE__, __FILE__, info);
    close(read_fd);

    handle_string_test(expected_path, buffer, __LINE__, __FILE__, info);
    free(expected_path);
}

void test_invalid_arguments(test_info *info) {
    command_call *command;
    command_result *result;

    print_test_name("Testing `pwd([:whitespace:]+.+)+`");

    init_internals();

    command = parse_command("pwd test");
    int error_fd = open_test_file_to_write("test_last_exit_code_command_invalid_arguments.log");
    command->stderr = error_fd;
    result = execute_command_call(command);

    handle_int_test(result->exit_code, 1, __LINE__, __FILE__, info);

    destroy_command_result(result);
}
