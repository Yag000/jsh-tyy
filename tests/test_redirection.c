#include "../src/command.h"
#include "../src/internals.h"
#include "test_core.h"

#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

void test_redirection_stdin(test_info *info);
void test_redirection_stdout(test_info *info);
void test_redirection_stderr(test_info *info);

test_info *test_redirection() {

    // Test setup
    print_test_header("redirections");
    clock_t start = clock();
    test_info *info = create_test_info();

    test_redirection_stdin(info);
    test_redirection_stdout(info);
    test_redirection_stderr(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("redirections", info);
    return info;
}

void test_redirection_stdin(test_info *info) {
    if (!allow_slow) {
        return;
    }

    print_test_name("Testing input redirection");

    int write_fd = open_test_file_to_write("test_redirection_input.log");
    char *buf = "A simple sentence to be written.";
    write(write_fd, buf, strlen(buf));
    close(write_fd);

    int fd = open_test_file_to_read("test_redirection_input.log");
    int output_fd = open_test_file_to_write("test_redirection_input_out.log");

    command *command = parse_command("cat");
    command->call->stdin = fd;
    command->call->stdout = output_fd;
    command->background = 1;
    command_result *result = execute_command(command);

    sleep(1);

    destroy_command_result(result);

    int read_fd = open_test_file_to_read("test_redirection_input_out.log");

    char buffer[strlen(buf) + 1];
    read(read_fd, buffer, strlen(buf));
    buffer[strlen(buf)] = '\0';
    close(read_fd);

    handle_string_test(buf, buffer, __LINE__, __FILE__, info);
}

void test_redirection_stdout(test_info *info) {
    if (!allow_slow) {
        return;
    }
    print_test_name("Testing output redirection");

    int fd = open_test_file_to_write("test_redirection_output.log");

    command *command = parse_command("echo HelloWorld");
    command->call->stdout = fd;
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
    handle_string_test(expected, buffer, __LINE__, __FILE__, info);
}

void test_redirection_stderr(test_info *info) {
    if (!allow_slow) {
        return;
    }
    print_test_name("Testing output redirection");

    int fd = open_test_file_to_write("test_redirection_error.log");

    command *command = parse_command("mv .");
    command->call->stderr = fd;
    command->background = 1;
    command_result *result = execute_command(command);
    sleep(1);
    destroy_command_result(result);

    int read_fd = open_test_file_to_read("test_redirection_output.log");

    char buffer[1024];
    int read_bytes = read(read_fd, buffer, 1024);
    close(read_fd);

    handle_boolean_test(true, read_bytes > 0, __LINE__, __FILE__, info);
}
