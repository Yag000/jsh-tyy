#include "../src/command.h"
#include "../src/internals.h"
#include "test_core.h"
#include "utils.h"

#define NUM_TEST 7

void test_exit_no_arguments_no_previous_command(test_info *);
void test_exit_no_arguments_previous_command(test_info *);
void test_exit_multiple_arguments(test_info *);
void test_exit_correct_argument(test_info *);
void test_exit_string_argument(test_info *);
void test_exit_out_of_range_argument(test_info *);
void test_exit_with_running_jobs(test_info *);

test_info *test_exit() {

    test_case cases[NUM_TEST] = {
        QUICK_CASE("exit no arguments | No previous command", test_exit_no_arguments_no_previous_command),
        QUICK_CASE("exit no arguments | With 1 previous command failed", test_exit_no_arguments_previous_command),
        QUICK_CASE("exit multiple arguments", test_exit_multiple_arguments),
        QUICK_CASE("exit correct argument", test_exit_correct_argument),
        QUICK_CASE("exit string argument", test_exit_string_argument),
        QUICK_CASE("exit out of range argument", test_exit_out_of_range_argument),
        SLOW_CASE("exit with running jobs", test_exit_with_running_jobs)};

    test_info *info = run_cases("exit", cases, NUM_TEST);

    init_internals();
    return info;
}

void test_exit_no_arguments_no_previous_command(test_info *info) {
    command *command;
    command_result *result;

    init_internals();

    command = parse_command("exit");
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 0, info);
    CINTA_ASSERT_INT(should_exit, 1, info);
    CINTA_ASSERT_INT(last_exit_code, 0, info);

    destroy_command_result(result);
}

void test_exit_no_arguments_previous_command(test_info *info) {
    command *command;
    command_result *result;

    init_internals();

    int stderr_fd = open_test_file_to_write("test_exit_no_arguments_previous_command.log");
    command = parse_command("? e423423 423 423 42");
    command->command_calls[0]->stderr = stderr_fd;
    result = execute_command(command);
    destroy_command_result(result);

    command = parse_command("exit");
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 1, info);
    CINTA_ASSERT_INT(should_exit, 1, info);
    CINTA_ASSERT_INT(last_exit_code, 1, info);

    destroy_command_result(result);
}

void test_exit_multiple_arguments(test_info *info) {
    command *command;
    command_result *result;
    int error_fd;

    init_internals();

    command = parse_command("exit 1 2 3");
    error_fd = open_test_file_to_write("test_exit_multiple_arguments.log");
    command->command_calls[0]->stderr = error_fd;
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 1, info);
    CINTA_ASSERT_INT(should_exit, 0, info);
    CINTA_ASSERT_INT(last_exit_code, 1, info);

    destroy_command_result(result);
}

void test_exit_correct_argument(test_info *info) {
    command *command;
    command_result *result;

    init_internals();

    for (int i = 0; i <= 255; i++) {
        init_internals();
        char *command_string = malloc(10);
        sprintf(command_string, "exit %d", i);
        command = parse_command(command_string);
        result = execute_command(command);

        CINTA_ASSERT_INT(result->exit_code, i, info);
        CINTA_ASSERT_INT(should_exit, 1, info);
        CINTA_ASSERT_INT(last_exit_code, i, info);

        destroy_command_result(result);
        free(command_string);
    }
}

void test_exit_string_argument(test_info *info) {
    command *command;
    command_result *result;

    int error_fd;

    init_internals();

    command = parse_command("exit test");

    error_fd = open_test_file_to_write("test_exit_string_argument.log");
    command->command_calls[0]->stderr = error_fd;
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 1, info);
    CINTA_ASSERT_INT(should_exit, 0, info);
    CINTA_ASSERT_INT(last_exit_code, 1, info);

    destroy_command_result(result);
}

void test_exit_out_of_range_argument(test_info *info) {
    command *command;
    command_result *result;

    int error_fd;

    init_internals();

    command = parse_command("exit 256");

    error_fd = open_test_file_to_write("test_exit_out_of_range_argument_256.log");
    command->command_calls[0]->stderr = error_fd;
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 1, info);
    CINTA_ASSERT_INT(should_exit, 0, info);
    CINTA_ASSERT_INT(last_exit_code, 1, info);

    destroy_command_result(result);

    command = parse_command("exit -1");

    error_fd = open_test_file_to_write("test_exit_out_of_range_argument_negative.log");
    command->command_calls[0]->stderr = error_fd;
    result = execute_command(command);

    CINTA_ASSERT_INT(result->exit_code, 1, info);
    CINTA_ASSERT_INT(should_exit, 0, info);
    CINTA_ASSERT_INT(last_exit_code, 1, info);

    destroy_command_result(result);
}

void test_exit_with_running_jobs(test_info *info) {

    init_internals();

    int fd = open_test_file_to_write("test_exit_with_running_jobs_out.log");
    command *background_job = parse_command("sleep 1");
    background_job->background = 1;
    background_job->command_calls[0]->stderr = fd;

    command *exit_fail = parse_command("exit");
    exit_fail->command_calls[0]->stderr = dup(fd);

    command_result *result_job = mute_command_execution(background_job);
    command_result *result_exit_fail = execute_command(exit_fail);

    CINTA_ASSERT_INT(1, result_exit_fail->exit_code, info);
    CINTA_ASSERT_INT(0, should_exit, info);
    CINTA_ASSERT_INT(1, last_exit_code, info);

    // Let the job finish
    sleep(2);

    helper_mute_update_jobs("test_exit_with_running_jobs.log");

    command *exit_success = parse_command("exit");
    command_result *result_exit_success = execute_command(exit_success);

    CINTA_ASSERT_INT(1, result_exit_success->exit_code, info);
    CINTA_ASSERT_INT(1, should_exit, info);
    CINTA_ASSERT_INT(1, last_exit_code, info);

    destroy_command_result(result_job);
    destroy_command_result(result_exit_fail);
    destroy_command_result(result_exit_success);

    init_internals();
}
