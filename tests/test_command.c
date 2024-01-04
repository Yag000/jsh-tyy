#include "../src/command.h"
#include "test_core.h"
#include "utils.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

void test_destroy_command_call_null();
void test_destroy_command_result_null();

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
void test_complex_background_parsing_args_spaces(test_info *info);

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
    test_invalid_background_parsing(info);
    test_simple_background_parsing_no_args_no_spaces(info);
    test_simple_background_parsing_spaces_no_args(info);
    test_simple_background_parsing_args_no_spaces(info);
    test_simple_background_parsing_args_spaces(info);
    test_complex_background_parsing_no_args_no_spaces(info);
    test_complex_background_parsing_args_spaces(info);
    test_complex_background_parsing_args_no_spaces(info);
    test_complex_background_parsing_args_spaces(info);

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

    command *command = parse_command("pwd");
    command_call_print(command->call, fd);

    close(fd);

    int read_fd = open_test_file_to_read("test_command_call_print.log");
    char buffer[4];
    buffer[3] = '\0';
    read(read_fd, buffer, 3);
    close(read_fd);

    char *expected = "pwd";
    handle_string_test(buffer, expected, __LINE__, __FILE__, info);

    destroy_command(command);
}

void test_command_call_print_with_arguments(test_info *info) {
    print_test_name("Testing command print with arguments");

    int fd = open_test_file_to_write("test_command_call_print.log");

    command *command = parse_command("pwd test");
    command_call_print(command->call, fd);

    close(fd);

    int read_fd = open_test_file_to_read("test_command_call_print.log");
    char buffer[9];
    buffer[8] = '\0';

    read(read_fd, buffer, 8);
    close(read_fd);

    char *expected = "pwd test";
    handle_string_test(buffer, expected, __LINE__, __FILE__, info);

    destroy_command(command);

    fd = open_test_file_to_write("test_command_call_print.log");

    command = parse_command("pwd test test2");
    command_call_print(command->call, fd);

    close(fd);

    read_fd = open_test_file_to_read("test_command_call_print.log");
    char buffer2[15];
    buffer2[14] = '\0';

    read(read_fd, buffer2, 14);
    close(read_fd);

    expected = "pwd test test2";
    handle_string_test(buffer2, expected, __LINE__, __FILE__, info);

    destroy_command(command);
}

void test_case_parse_command(test_info *info) {
    command *command;
    command_call *command_call;

    print_test_name("Testing `parse_command`");

    // Empty command string
    command = parse_command("");
    handle_null_test(command, __LINE__, __FILE__, info);

    // Only spaces
    command = parse_command("     ");
    handle_null_test(command, __LINE__, __FILE__, info);

    // Command call with no arguments
    command = parse_command("^mv^rm");
    command_call = command->call;
    char *expected_1[1] = {"^mv^rm"};
    handle_string_test(expected_1[0], command_call->name, __LINE__, __FILE__, info);
    handle_int_test(1, command_call->argc, __LINE__, __FILE__, info);
    for (size_t index = 0; index < command_call->argc; ++index) {
        handle_string_test(expected_1[index], command_call->argv[index], __LINE__, __FILE__, info);
    }
    handle_null_test(command_call->argv[command_call->argc], __LINE__, __FILE__, info);
    destroy_command(command);

    // Command call with few arguments
    command = parse_command("nvim -A /absolute/path/to/launch/in/neovim");
    command_call = command->call;
    char *expected_2[3] = {"nvim", "-A", "/absolute/path/to/launch/in/neovim"};
    handle_string_test(expected_2[0], command_call->name, __LINE__, __FILE__, info);
    handle_int_test(3, command_call->argc, __LINE__, __FILE__, info);
    for (size_t index = 0; index < command_call->argc; ++index) {
        handle_string_test(expected_2[index], command_call->argv[index], __LINE__, __FILE__, info);
    }
    handle_int_test(0, command_call->background, __LINE__, __FILE__, info);
    handle_int_test(0, command_call->stdin, __LINE__, __FILE__, info);
    handle_int_test(1, command_call->stdout, __LINE__, __FILE__, info);
    handle_int_test(2, command_call->stderr, __LINE__, __FILE__, info);
    handle_null_test(command_call->argv[command_call->argc], __LINE__, __FILE__, info);
    destroy_command(command);

    // Another command call with few arguments
    command = parse_command("rm -rf --no-preserve-root /");
    command_call = command->call;
    char *expected_3[4] = {"rm", "-rf", "--no-preserve-root", "/"};
    handle_string_test(expected_3[0], command_call->name, __LINE__, __FILE__, info);
    handle_int_test(4, command_call->argc, __LINE__, __FILE__, info);
    for (size_t index = 0; index < command_call->argc; ++index) {
        handle_string_test(expected_3[index], command_call->argv[index], __LINE__, __FILE__, info);
    }
    handle_null_test(command_call->argv[command_call->argc], __LINE__, __FILE__, info);
    destroy_command(command);
}

void test_invalid_background_parsing(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    print_test_name("& Parsing | Invalid");

    // Single Ampersand
    commands = parse_read_line("&", &total_commands);
    handle_null_test(commands, __LINE__, __FILE__, info);

    // Empty at start
    commands = parse_read_line("    &", &total_commands);
    handle_null_test(commands, __LINE__, __FILE__, info);

    // Empty at the end
    commands = parse_read_line("&      ", &total_commands);
    handle_null_test(commands, __LINE__, __FILE__, info);

    // Empty at start and end
    commands = parse_read_line("        &      ", &total_commands);
    handle_null_test(commands, __LINE__, __FILE__, info);

    // Multiples &
    commands = parse_read_line("&&", &total_commands);
    handle_null_test(commands, __LINE__, __FILE__, info);
    commands = parse_read_line("&&&", &total_commands);
    handle_null_test(commands, __LINE__, __FILE__, info);
    commands = parse_read_line("  &  &  ", &total_commands);
    handle_null_test(commands, __LINE__, __FILE__, info);
    commands = parse_read_line("  &  &  &&  ", &total_commands);
    handle_null_test(commands, __LINE__, __FILE__, info);
}

void test_simple_background_parsing_no_args_no_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    print_test_name("& Parsing | Simple - No arguments - No spaces");

    commands = parse_read_line("pwd &", &total_commands);
    handle_boolean_test(false, commands == NULL, __LINE__, __FILE__, info);
    handle_int_test(1, total_commands, __LINE__, __FILE__, info);

    command *expected = parse_command("pwd");
    expected->call->background = 1;
    handle_command_test(commands[0], expected, __LINE__, __FILE__, info);

    destroy_command(expected);
    destroy_command(commands[0]);
    free(commands);
}

void test_simple_background_parsing_spaces_no_args(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    print_test_name("& Parsing | Simple - No arguments - With spaces");

    commands = parse_read_line("    pwd  &    ", &total_commands);
    handle_boolean_test(false, commands == NULL, __LINE__, __FILE__, info);
    handle_int_test(1, total_commands, __LINE__, __FILE__, info);

    command *expected = parse_command("pwd");
    expected->call->background = 1;
    handle_command_test(commands[0], expected, __LINE__, __FILE__, info);

    destroy_command(expected);
    destroy_command(commands[0]);
    free(commands);
}

void test_simple_background_parsing_args_no_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    print_test_name("& Parsing | Simple - With arguments - No spaces");

    commands = parse_read_line("aoc -d 24 -y 2003 submit &", &total_commands);
    handle_boolean_test(false, commands == NULL, __LINE__, __FILE__, info);
    handle_int_test(1, total_commands, __LINE__, __FILE__, info);

    command *expected = parse_command("aoc -d 24 -y 2003 submit");
    expected->call->background = 1;
    handle_command_test(commands[0], expected, __LINE__, __FILE__, info);

    destroy_command(expected);
    destroy_command(commands[0]);
    free(commands);
}

void test_simple_background_parsing_args_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    print_test_name("& Parsing | Simple - With arguments - With spaces");

    commands = parse_read_line("    cargo  build     --release   &   ", &total_commands);
    handle_boolean_test(false, commands == NULL, __LINE__, __FILE__, info);
    handle_int_test(1, total_commands, __LINE__, __FILE__, info);

    command *expected = parse_command("    cargo  build     --release ");
    expected->call->background = 1;
    handle_command_test(commands[0], expected, __LINE__, __FILE__, info);

    destroy_command(expected);
    destroy_command(commands[0]);
    free(commands);
}

void test_complex_background_parsing_no_args_no_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    print_test_name("& Parsing | a & b - No args - No spaces");

    commands = parse_read_line("pwd & ls", &total_commands);
    handle_boolean_test(false, commands == NULL, __LINE__, __FILE__, info);
    handle_int_test(2, total_commands, __LINE__, __FILE__, info);

    command *expected_0 = parse_command("pwd");
    expected_0->call->background = 1;
    handle_command_test(commands[0], expected_0, __LINE__, __FILE__, info);

    destroy_command(expected_0);
    destroy_command(commands[0]);

    command *expected_1 = parse_command("ls");
    expected_1->call->background = 0;
    handle_command_test(commands[1], expected_1, __LINE__, __FILE__, info);

    destroy_command(expected_1);
    destroy_command(commands[1]);
    free(commands);
}

void test_complex_background_parsing_args_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    print_test_name("& Parsing | a & b - With args - With spaces");

    commands = parse_read_line("   sleep  10   &     ls   ", &total_commands);
    handle_boolean_test(false, commands == NULL, __LINE__, __FILE__, info);
    handle_int_test(2, total_commands, __LINE__, __FILE__, info);

    command *expected_0 = parse_command("sleep  10");
    expected_0->call->background = 1;
    handle_command_test(commands[0], expected_0, __LINE__, __FILE__, info);

    destroy_command(expected_0);
    destroy_command(commands[0]);

    command *expected_1 = parse_command("ls");
    handle_command_test(commands[1], expected_1, __LINE__, __FILE__, info);

    destroy_command(expected_1);
    destroy_command(commands[1]);
    free(commands);
}

void test_complex_background_parsing_args_no_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    print_test_name("& Parsing | a & b & - With args - No spaces");

    commands = parse_read_line("sleep 666& ls&", &total_commands);
    handle_boolean_test(false, commands == NULL, __LINE__, __FILE__, info);
    handle_int_test(2, total_commands, __LINE__, __FILE__, info);

    command *expected_0 = parse_command("sleep 666");
    expected_0->call->background = 1;
    handle_command_test(commands[0], expected_0, __LINE__, __FILE__, info);

    destroy_command(expected_0);
    destroy_command(commands[0]);

    command *expected_1 = parse_command("ls");
    expected_1->call->background = 1;
    handle_command_test(commands[1], expected_1, __LINE__, __FILE__, info);

    destroy_command(expected_1);
    destroy_command(commands[1]);
    free(commands);
}
void test_complex_full_background_parsing_args_spaces(test_info *info) {
    size_t total_commands = 0;
    command **commands;

    print_test_name("& Parsing | a & b & c & - With args - With spaces");

    commands = parse_read_line("  sleep   1000d   &  ls   -l   -a  &   ", &total_commands);
    handle_boolean_test(false, commands == NULL, __LINE__, __FILE__, info);
    handle_int_test(2, total_commands, __LINE__, __FILE__, info);

    command *expected_0 = parse_command("sleep 1000d");
    expected_0->call->background = 1;
    handle_command_test(commands[0], expected_0, __LINE__, __FILE__, info);

    destroy_command(expected_0);
    destroy_command(commands[0]);

    command *expected_1 = parse_command("ls -l   -a");
    expected_1->call->background = 1;
    handle_command_test(commands[1], expected_1, __LINE__, __FILE__, info);

    destroy_command(expected_1);
    destroy_command(commands[1]);
    free(commands);
}
