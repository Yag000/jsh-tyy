#include "../src/command.h"
#include "../src/internals.h"
#include "test_core.h"

#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#define NUM_TEST 3

void test_redirection_stdin(test_info *info);
void test_redirection_stdout(test_info *info);
void test_redirection_stderr(test_info *info);

test_info *test_redirection() {
    test_case cases[NUM_TEST] = {SLOW_CASE("Testing input redirection", test_redirection_stdin),
                                 SLOW_CASE("Testing output redirection", test_redirection_stdout),
                                 SLOW_CASE("Testing error redirection", test_redirection_stderr)};

    return run_cases("redirection", cases, NUM_TEST);
}

void test_redirection_stdin(test_info *info) {
    int write_fd = open_test_file_to_write("test_redirection_input.log");
    char *buf = "A simple sentence to be written.";
    write(write_fd, buf, strlen(buf));
    close(write_fd);

    int fd = open_test_file_to_read("test_redirection_input.log");
    int output_fd = open_test_file_to_write("test_redirection_input_out.log");

    command *command = parse_command("cat");
    command->command_calls[0]->stdin = fd;
    command->command_calls[0]->stdout = output_fd;
    command->background = 1;
    command_result *result = execute_command(command);

    sleep(1);

    destroy_command_result(result);

    int read_fd = open_test_file_to_read("test_redirection_input_out.log");

    char buffer[strlen(buf) + 1];
    read(read_fd, buffer, strlen(buf));
    buffer[strlen(buf)] = '\0';
    close(read_fd);

    CINTA_ASSERT_STRING(buf, buffer, info);
}

void test_redirection_stdout(test_info *info) {
    int fd = open_test_file_to_write("test_redirection_output.log");

    command *command = parse_command("echo HelloWorld");
    command->command_calls[0]->stdout = fd;
    command->background = 1;
    command_result *result = execute_command(command);
    sleep(1);
    destroy_command_result(result);

    int read_fd = open_test_file_to_read("test_redirection_output.log");

    char buffer[1024];
    read(read_fd, buffer, 1024);
    close(read_fd);

    char *expected = "HelloWorld";
    buffer[strlen(expected)] = '\0';
    CINTA_ASSERT_STRING(expected, buffer, info);
}

void test_redirection_stderr(test_info *info) {
    int fd = open_test_file_to_write("test_redirection_error.log");

    command *command = parse_command("mv .");
    command->command_calls[0]->stderr = fd;
    command->background = 1;
    command_result *result = execute_command(command);
    sleep(1);
    destroy_command_result(result);

    int read_fd = open_test_file_to_read("test_redirection_output.log");

    char buffer[1024];
    int read_bytes = read(read_fd, buffer, 1024);
    close(read_fd);

    CINTA_ASSERT(read_bytes > 0, info);
}
