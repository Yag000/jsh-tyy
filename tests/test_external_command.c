#include "../src/command.h"
#include "../src/internals.h"
#include "test_core.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static void test_case_successful_command(test_info *info);
static void test_case_unsuccessful_command(test_info *info);
static void test_case_non_existent_command(test_info *info);
static void test_case_existent_file_command(test_info *info);
static void test_case_non_existent_file_command(test_info *info);

test_info *test_external_commands() {
    // Test setup
    print_test_header("external commands");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Test body
    test_case_successful_command(info);
    test_case_unsuccessful_command(info);
    test_case_non_existent_command(info);
    test_case_existent_file_command(info);
    test_case_non_existent_file_command(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("external commands", info);
    return info;
}

static void test_case_successful_command(test_info *info) {
    command_call **commands;
    command_result *command_result;
    size_t total_commands = 0;

    print_test_name("Testing successful external commands");

    fflush(stdout);
    fflush(stderr);
    commands = parse_command("true", &total_commands);
    command_result = execute_command_call(commands[0]);
    handle_int_test(0, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
    free(commands);
}

static void test_case_unsuccessful_command(test_info *info) {
    command_call **commands;
    command_result *command_result;
    size_t total_commands = 0;

    print_test_name("Testing unsuccessful external commands");

    fflush(stdout);
    fflush(stderr);
    commands = parse_command("false", &total_commands);
    command_result = execute_command_call(commands[0]);
    handle_int_test(1, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
    free(commands);
}

static void test_case_non_existent_command(test_info *info) {
    command_call **commands;
    command_result *command_result;
    size_t total_commands = 0;

    print_test_name("Testing non existent external commands");

    int bin_fd = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, 0666);

    fflush(stdout);
    fflush(stderr);
    commands = parse_command("idonotexistasacommand", &total_commands);
    commands[0]->stderr = bin_fd;
    command_result = execute_command_call(commands[0]);
    handle_int_test(1, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
    free(commands);
}

static void test_case_existent_file_command(test_info *info) {
    command_call **commands;
    command_result *command_result;
    size_t total_commands = 0;

    print_test_name("Testing existent file external commands");

    fflush(stdout);
    fflush(stderr);
    commands = parse_command("./tests/scripts/always_true.sh", &total_commands);
    command_result = execute_command_call(commands[0]);
    handle_int_test(0, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
    free(commands);

    fflush(stdout);
    fflush(stderr);
    commands = parse_command("./tests/scripts/always_false.sh", &total_commands);
    command_result = execute_command_call(commands[0]);
    handle_int_test(1, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
    free(commands);
}

static void test_case_non_existent_file_command(test_info *info) {
    command_call **commands;
    command_result *command_result;
    size_t total_commands = 0;

    print_test_name("Testing non existent file external commands");

    int bin_fd = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, 0666);

    fflush(stdout);
    fflush(stderr);
    commands = parse_command("./tmp/iamadirectory", &total_commands);
    commands[0]->stderr = bin_fd;
    command_result = execute_command_call(commands[0]);
    handle_int_test(1, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
    free(commands);

    fflush(stdout);
    fflush(stderr);
    commands = parse_command("./tmp/idonotexist", &total_commands);
    commands[0]->stderr = bin_fd;
    command_result = execute_command_call(commands[0]);
    handle_int_test(1, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
    free(commands);
}
