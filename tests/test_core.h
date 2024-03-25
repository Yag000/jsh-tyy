#ifndef TEST_CORE_H
#define TEST_CORE_H

#include <limits.h>
#include <stdbool.h>
#include <time.h>

#include "../cinta/cinta.h"
#include "../src/command.h"
#include "../src/jobs.h"

int check_command_call(command_call *actual, command_call *expected);

int check_command(command *actual, command *expected);

int check_command_result(command_result *actual, command_result *expected);

int check_job(job *actual, job *expected);

#define ASSERT_COMMAND_CALL(actual, expected, info) CINTA_CUSTOM_ASSERT(actual, expected, check_command_call, info)
#define ASSERT_COMMAND(actual, expected, info) CINTA_CUSTOM_ASSERT(actual, expected, check_command, info)
#define ASSERT_COMMAND_RESULT(actual, expected, info) CINTA_CUSTOM_ASSERT(actual, expected, check_command_result, info)
#define ASSERT_JOB(actual, expected, info) CINTA_CUSTOM_ASSERT(actual, expected, check_job, info)

int open_test_file_to_read(const char *);
int open_test_file_to_write(const char *);

void helper_mute_update_jobs(char *file_name);

// All the tests
test_info *test_command();
test_info *test_cd();
test_info *test_string_utils();
test_info *test_last_exit_code_command();
test_info *test_pwd();
test_info *test_exit();
test_info *test_utils();
test_info *test_jobs();
test_info *test_prompt();
test_info *test_background_jobs();
test_info *test_running_jobs();
test_info *test_redirection();
test_info *test_redirection_parsing();

#endif // TEST_CORE_H
