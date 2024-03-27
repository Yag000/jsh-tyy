#include "../src/command.h"
#include "../src/jobs.h"
#include "test_core.h"
#include "utils.h"

#define NUM_TEST 2

void test_launching_one_bg_job(test_info *);
void test_launching_multiple_bg_jobs(test_info *);

test_info *test_background_jobs() {
    test_case cases[NUM_TEST] = {QUICK_CASE("Testing launching one background job", test_launching_one_bg_job),
                          QUICK_CASE("Testing launching multiple background jobs", test_launching_multiple_bg_jobs)};
    return cinta_run_cases("test", cases, NUM_TEST);
}

void test_launching_one_bg_job(test_info *info) {

    init_job_table();

    int fd = open_test_file_to_write("test_launching_one_bg_job.log");

    command *command = parse_command("ls");
    command->background = 1;
    command->command_calls[0]->stdout = fd;
    command_result *result = mute_command_execution(command);

    CINTA_ASSERT(result->pid > 0, info);
    CINTA_ASSERT_INT(result->job_id, 1, info);

    CINTA_ASSERT_INT(job_table_size, 1, info);
    CINTA_ASSERT_STRING(job_table[0]->subjobs[0]->command, command->command_calls[0]->name, info);

    destroy_command_result(result);
}

void test_launching_multiple_bg_jobs(test_info *info) {
    init_job_table();

    int fd = open_test_file_to_write("test_launching_multiple_bg_jobs.log");

    for (int i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 2; i++) {
        command *command = parse_command("ls");
        command->background = 1;
        command->command_calls[0]->stdout = fd;
        command_result *result = mute_command_execution(command);

        CINTA_ASSERT(result->pid > 0, info);
        CINTA_ASSERT_INT(result->job_id, i + 1, info);

        CINTA_ASSERT_INT(job_table_size, i + 1, info);
        CINTA_ASSERT_STRING(job_table[i]->subjobs[0]->command, command->command_calls[0]->name, info);

        destroy_command_result(result);
    }
}
