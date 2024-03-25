#include "../src/command.h"
#include "../src/internals.h"
#include "../src/utils.h"
#include "test_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_TEST 3

static void test_case_home(test_info *);
static void test_case_deeper(test_info *);
static void test_invalid_arguments(test_info *);

test_info *test_pwd() {
    test_case cases[NUM_TEST] = {QUICK_CASE("Testing `cd && pwd`", test_case_home),
                                 QUICK_CASE("Testing `cd tmp/dir && pwd`", test_case_deeper),
                                 QUICK_CASE("Testing `pwd([:whitespace:]+.+)+`", test_invalid_arguments)};

    return run_cases("pwd", cases, NUM_TEST);
}

static void test_case_home(test_info *info) {
    command *command, *cd_command;

    // Get $HOME env var
    char *expected_path = getenv("HOME");

    int fd = open_test_file_to_write("test_pwd.log");
    cd_command = parse_command("cd");
    destroy_command_result(execute_command(cd_command));

    // Go to $HOME
    command = parse_command("pwd");
    command->command_calls[0]->stdout = fd;
    destroy_command_result(execute_command(command));

    // Go back to previous wd
    cd_command = parse_command("cd -");
    destroy_command_result(execute_command(cd_command));

    // Check log file
    int read_fd = open_test_file_to_read("test_pwd.log");
    char buffer[strlen(expected_path)];
    buffer[strlen(expected_path)] = '\0';
    read(read_fd, buffer, strlen(expected_path));

    // Check that there is nothing more than a newline char
    char buffer_newline[2];
    buffer_newline[1] = '\0';
    int eof = read(read_fd, buffer_newline, 2);
    CINTA_ASSERT_INT(1, eof, info);
    close(read_fd);

    CINTA_ASSERT_STRING(expected_path, buffer, info);
}

static void test_case_deeper(test_info *info) {
    command *command, *cd_command;

    int fd = open_test_file_to_write("test_pwd.log");
    cd_command = parse_command("cd tmp/dir");
    destroy_command_result(execute_command(cd_command));

    // Go to $HOME
    command = parse_command("pwd");
    command->command_calls[0]->stdout = fd;
    destroy_command_result(execute_command(command));

    char *expected_path = get_current_wd();

    // Go back to previous wd
    cd_command = parse_command("cd -");
    destroy_command_result(execute_command(cd_command));

    // Check log file
    int read_fd = open_test_file_to_read("test_pwd.log");
    char buffer[strlen(lwd)];
    buffer[strlen(lwd)] = '\0';

    read(read_fd, buffer, strlen(lwd));

    // Check that there is nothing more than a newline char
    char buffer_newline[2];
    buffer_newline[1] = '\0';
    int eof = read(read_fd, buffer_newline, 2);
    CINTA_ASSERT_INT(1, eof, info);
    close(read_fd);

    CINTA_ASSERT_STRING(expected_path, buffer, info);
    free(expected_path);
}

void test_invalid_arguments(test_info *info) {
    command *command;
    command_result *result;

    init_internals();

    command = parse_command("pwd test");
    int error_fd = open_test_file_to_write("test_last_exit_code_command_invalid_arguments.log");
    command->command_calls[0]->stderr = error_fd;
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 1, info);

    destroy_command_result(result);
}
