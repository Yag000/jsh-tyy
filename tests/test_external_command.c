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
    print_test_header("external command");
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
    print_test_footer("external command", info);
    return info;
}

static void test_case_successful_command(test_info *info) {
    command_call *command;
    command_result *command_result;

    print_test_name("Testing successful external command");

    fflush(stdout);
    fflush(stderr);
    command = parse_command("true");
    command_result = execute_command_call(command);
    handle_int_test(0, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
}

static void test_case_unsuccessful_command(test_info *info) {
    command_call *command;
    command_result *command_result;

    print_test_name("Testing unsuccessful external command");

    fflush(stdout);
    fflush(stderr);
    command = parse_command("false");
    command_result = execute_command_call(command);
    handle_int_test(1, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
}

static void test_case_non_existent_command(test_info *info) {
    command_call *command;
    command_result *command_result;

    print_test_name("Testing non existent external command");

    int bin_fd = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, 0666);

    fflush(stdout);
    fflush(stderr);
    command = parse_command("idonotexistasacommand");
    command->stderr = bin_fd;
    command_result = execute_command_call(command);
    handle_int_test(1, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
}

static void test_case_existent_file_command(test_info *info) {
    command_call *command;
    command_result *command_result;

    print_test_name("Testing existent file external command");

    fflush(stdout);
    fflush(stderr);
    command = parse_command("./tests/scripts/always_true.sh");
    command_result = execute_command_call(command);
    handle_int_test(0, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);

    fflush(stdout);
    fflush(stderr);
    command = parse_command("./tests/scripts/always_false.sh");
    command_result = execute_command_call(command);
    handle_int_test(1, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
}

static void test_case_non_existent_file_command(test_info *info) {
    command_call *command;
    command_result *command_result;

    print_test_name("Testing non existent file external command");

    int bin_fd = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, 0666);

    fflush(stdout);
    fflush(stderr);
    command = parse_command("./tmp/iamadirectory");
    command->stderr = bin_fd;
    command_result = execute_command_call(command);
    handle_int_test(1, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);

    bin_fd = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, 0666);

    fflush(stdout);
    fflush(stderr);
    command = parse_command("./tmp/idonotexist");
    command->stderr = bin_fd;
    command_result = execute_command_call(command);
    handle_int_test(1, command_result->exit_code, __LINE__, __FILE__, info);
    destroy_command_result(command_result);
}
