#include "../src/command.h"
#include "test_core.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#define NUM_TEST 14

void test_destroy_command_call_null(test_info *info);
void test_destroy_command_result_null(test_info *info);
void test_no_arguments_command_call_print(test_info *info);
void test_command_call_print_with_arguments(test_info *info);
void test_case_parse_command(test_info *info);
void test_invalid_background_parsing(test_info *info);
void test_simple_background_parsing_no_args_no_spaces(test_info *info);
void test_simple_background_parsing_spaces_no_args(test_info *info);
void test_simple_background_parsing_args_no_spaces(test_info *info);
void test_simple_background_parsing_args_spaces(test_info *info);
void test_complex_background_parsing_no_args_no_spaces(test_info *info);
void test_complex_background_parsing_args_spaces(test_info *info);
void test_complex_background_parsing_args_no_spaces(test_info *info);
void test_complex_full_background_parsing_args_spaces(test_info *info);

test_info *test_command() {

    test_case cases[NUM_TEST] = {
        QUICK_CASE("Testing `destroy_command_call` with NULL", test_destroy_command_call_null),
        QUICK_CASE("Testing `destroy_command_result` with NULL", test_destroy_command_result_null),
        QUICK_CASE("Testing command print with no arguments", test_no_arguments_command_call_print),
        QUICK_CASE("Testing command print with arguments", test_command_call_print_with_arguments),
        QUICK_CASE("Testing `parse_command`", test_case_parse_command),
        QUICK_CASE("& Parsing | Invalid", test_invalid_background_parsing),
        QUICK_CASE("& Parsing | Simple - No arguments - No spaces", test_simple_background_parsing_no_args_no_spaces),
        QUICK_CASE("& Parsing | Simple - No arguments - With spaces", test_simple_background_parsing_spaces_no_args),
        QUICK_CASE("& Parsing | Simple - With arguments - No spaces", test_simple_background_parsing_args_no_spaces),
        QUICK_CASE("& Parsing | Simple - With arguments - With spaces", test_simple_background_parsing_args_spaces),
        QUICK_CASE("& Parsing | a & b - No args - No spaces", test_complex_background_parsing_no_args_no_spaces),
        QUICK_CASE("& Parsing | a & b - With args - With spaces", test_complex_background_parsing_args_spaces),
        QUICK_CASE("& Parsing | a & b & - With args - No spaces", test_complex_background_parsing_args_no_spaces),
        QUICK_CASE("& Parsing | a & b - With args - With spaces", test_complex_full_background_parsing_args_spaces)};
    return cinta_run_cases("command", cases, NUM_TEST);
}

void test_destroy_command_call_null(test_info *info) {
    destroy_command_call(NULL);
    CINTA_ASSERT(1, info);
}

void test_destroy_command_result_null(test_info *info) {
    destroy_command_result(NULL);
    CINTA_ASSERT(1, info);
}

void test_no_arguments_command_call_print(test_info *info) {
    int fd = open_test_file_to_write("test_command_call_print.log");

    command *command = parse_command("pwd");
    command_call_print(command->command_calls[0], fd);

    close(fd);

    int read_fd = open_test_file_to_read("test_command_call_print.log");
    char buffer[4];
    buffer[3] = '\0';
    read(read_fd, buffer, 3);
    close(read_fd);

    char *expected = "pwd";
    CINTA_ASSERT_STRING(buffer, expected, info);

    destroy_command(command);
}

void test_command_call_print_with_arguments(test_info *info) {
    int fd = open_test_file_to_write("test_command_call_print.log");

    command *command = parse_command("pwd test");
    command_call_print(command->command_calls[0], fd);

    close(fd);

    int read_fd = open_test_file_to_read("test_command_call_print.log");
    char buffer[9];
    buffer[8] = '\0';

    read(read_fd, buffer, 8);
    close(read_fd);

    char *expected = "pwd test";
    CINTA_ASSERT_STRING(buffer, expected, info);

    destroy_command(command);

    fd = open_test_file_to_write("test_command_call_print.log");

    command = parse_command("pwd test test2");
    command_call_print(command->command_calls[0], fd);

    close(fd);

    read_fd = open_test_file_to_read("test_command_call_print.log");
    char buffer2[15];
    buffer2[14] = '\0';

    read(read_fd, buffer2, 14);
    close(read_fd);

    expected = "pwd test test2";
    CINTA_ASSERT_STRING(buffer2, expected, info);

    destroy_command(command);
}

void test_case_parse_command(test_info *info) {
    command *command;
    command_call *command_call;

    // Empty command string
    command = parse_command("");
    CINTA_ASSERT_NULL(command, info);

    // Only spaces
    command = parse_command("     ");
    CINTA_ASSERT_NULL(command, info);

    // Command call with no arguments
    command = parse_command("^mv^rm");
    command_call = command->command_calls[0];
    char *expected_1[1] = {"^mv^rm"};
    CINTA_ASSERT_STRING(expected_1[0], command_call->name, info);
    CINTA_ASSERT_INT(1, command_call->argc, info);
    for (size_t index = 0; index < command_call->argc; ++index) {
        CINTA_ASSERT_STRING(expected_1[index], command_call->argv[index], info);
    }
    CINTA_ASSERT_NULL(command_call->argv[command_call->argc], info);
    destroy_command(command);

    // Command call with few arguments
    command = parse_command("nvim -A /absolute/path/to/launch/in/neovim");
    command_call = command->command_calls[0];
    char *expected_2[3] = {"nvim", "-A", "/absolute/path/to/launch/in/neovim"};
    CINTA_ASSERT_STRING(expected_2[0], command_call->name, info);
    CINTA_ASSERT_INT(3, command_call->argc, info);
    for (size_t index = 0; index < command_call->argc; ++index) {
        CINTA_ASSERT_STRING(expected_2[index], command_call->argv[index], info);
    }
    CINTA_ASSERT_INT(0, command->background, info);
    CINTA_ASSERT_INT(0, command_call->stdin, info);
    CINTA_ASSERT_INT(1, command_call->stdout, info);
    CINTA_ASSERT_INT(2, command_call->stderr, info);
    CINTA_ASSERT_NULL(command_call->argv[command_call->argc], info);
    destroy_command(command);

    // Another command call with few arguments
    command = parse_command("rm -rf --no-preserve-root /");
    command_call = command->command_calls[0];
    char *expected_3[4] = {"rm", "-rf", "--no-preserve-root", "/"};
    CINTA_ASSERT_STRING(expected_3[0], command_call->name, info);
    CINTA_ASSERT_INT(4, command_call->argc, info);
    for (size_t index = 0; index < command_call->argc; ++index) {
        CINTA_ASSERT_STRING(expected_3[index], command_call->argv[index], info);
    }
    CINTA_ASSERT_NULL(command_call->argv[command_call->argc], info);
    destroy_command(command);
}

void test_invalid_background_parsing(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    // Single Ampersand
    commands = parse_read_line("&", &total_commands);
    CINTA_ASSERT_NULL(commands, info);

    // Empty at start
    commands = parse_read_line("    &", &total_commands);
    CINTA_ASSERT_NULL(commands, info);

    // Empty at the end
    commands = parse_read_line("&      ", &total_commands);
    CINTA_ASSERT_NULL(commands, info);

    // Empty at start and end
    commands = parse_read_line("        &      ", &total_commands);
    CINTA_ASSERT_NULL(commands, info);

    // Multiples &
    commands = parse_read_line("&&", &total_commands);
    CINTA_ASSERT_NULL(commands, info);
    commands = parse_read_line("&&&", &total_commands);
    CINTA_ASSERT_NULL(commands, info);
    commands = parse_read_line("  &  &  ", &total_commands);
    CINTA_ASSERT_NULL(commands, info);
    commands = parse_read_line("  &  &  &&  ", &total_commands);
    CINTA_ASSERT_NULL(commands, info);
}

void test_simple_background_parsing_no_args_no_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    commands = parse_read_line("pwd &", &total_commands);
    CINTA_ASSERT_FALSE(commands == NULL, info);
    CINTA_ASSERT_INT(1, total_commands, info);

    command *expected = parse_command("pwd");
    expected->background = 1;
    ASSERT_COMMAND(commands[0], expected, info);

    destroy_command(expected);
    destroy_command(commands[0]);
    free(commands);
}

void test_simple_background_parsing_spaces_no_args(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    commands = parse_read_line("    pwd  &    ", &total_commands);
    CINTA_ASSERT_FALSE(commands == NULL, info);
    CINTA_ASSERT_INT(1, total_commands, info);

    command *expected = parse_command("pwd");
    expected->background = 1;
    ASSERT_COMMAND(commands[0], expected, info);

    destroy_command(expected);
    destroy_command(commands[0]);
    free(commands);
}

void test_simple_background_parsing_args_no_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    commands = parse_read_line("aoc -d 24 -y 2003 submit &", &total_commands);
    CINTA_ASSERT_FALSE(commands == NULL, info);
    CINTA_ASSERT_INT(1, total_commands, info);

    command *expected = parse_command("aoc -d 24 -y 2003 submit");
    expected->background = 1;
    ASSERT_COMMAND(commands[0], expected, info);

    destroy_command(expected);
    destroy_command(commands[0]);
    free(commands);
}

void test_simple_background_parsing_args_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    commands = parse_read_line("    cargo  build     --release   &   ", &total_commands);
    CINTA_ASSERT_FALSE(commands == NULL, info);
    CINTA_ASSERT_INT(1, total_commands, info);

    command *expected = parse_command("    cargo  build     --release ");
    expected->background = 1;
    ASSERT_COMMAND(commands[0], expected, info);

    destroy_command(expected);
    destroy_command(commands[0]);
    free(commands);
}

void test_complex_background_parsing_no_args_no_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    commands = parse_read_line("pwd & ls", &total_commands);
    CINTA_ASSERT_FALSE(commands == NULL, info);
    CINTA_ASSERT_INT(2, total_commands, info);

    command *expected_0 = parse_command("pwd");
    expected_0->background = 1;
    ASSERT_COMMAND(commands[0], expected_0, info);

    destroy_command(expected_0);
    destroy_command(commands[0]);

    command *expected_1 = parse_command("ls");
    expected_1->background = 0;
    ASSERT_COMMAND(commands[1], expected_1, info);

    destroy_command(expected_1);
    destroy_command(commands[1]);
    free(commands);
}

void test_complex_background_parsing_args_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    commands = parse_read_line("   sleep  10   &     ls   ", &total_commands);
    CINTA_ASSERT_FALSE(commands == NULL, info);
    CINTA_ASSERT_INT(2, total_commands, info);

    command *expected_0 = parse_command("sleep  10");
    expected_0->background = 1;
    ASSERT_COMMAND(commands[0], expected_0, info);

    destroy_command(expected_0);
    destroy_command(commands[0]);

    command *expected_1 = parse_command("ls");
    ASSERT_COMMAND(commands[1], expected_1, info);

    destroy_command(expected_1);
    destroy_command(commands[1]);
    free(commands);
}

void test_complex_background_parsing_args_no_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    commands = parse_read_line("sleep 666& ls&", &total_commands);
    CINTA_ASSERT_FALSE(commands == NULL, info);
    CINTA_ASSERT_INT(2, total_commands, info);

    command *expected_0 = parse_command("sleep 666");
    expected_0->background = 1;
    ASSERT_COMMAND(commands[0], expected_0, info);

    destroy_command(expected_0);
    destroy_command(commands[0]);

    command *expected_1 = parse_command("ls");
    expected_1->background = 1;
    ASSERT_COMMAND(commands[1], expected_1, info);

    destroy_command(expected_1);
    destroy_command(commands[1]);
    free(commands);
}
void test_complex_full_background_parsing_args_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    commands = parse_read_line("  sleep   1000d   &  ls   -l   -a  &   ", &total_commands);
    CINTA_ASSERT_FALSE(commands == NULL, info);
    CINTA_ASSERT_INT(2, total_commands, info);

    command *expected_0 = parse_command("sleep   1000d");
    expected_0->background = 1;
    ASSERT_COMMAND(commands[0], expected_0, info);

    destroy_command(expected_0);
    destroy_command(commands[0]);

    command *expected_1 = parse_command("ls   -l   -a");
    expected_1->background = 1;
    ASSERT_COMMAND(commands[1], expected_1, info);

    destroy_command(expected_1);
    destroy_command(commands[1]);
    free(commands);
}
