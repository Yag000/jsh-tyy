#include "../src/command.h"
#include "../src/internals.h"
#include "../src/jobs.h"
#include "test_core.h"
#include "utils.h"
#include <stdbool.h>

void test_launching_one_bg_job(test_info *);
void test_launching_multiple_bg_jobs(test_info *);

test_info *test_background_jobs() {
    // Test setup
    print_test_header("background jobs");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add tests here
    test_launching_one_bg_job(info);
    test_launching_multiple_bg_jobs(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("background jobs", info);
    return info;
}

void test_launching_one_bg_job(test_info *info) {
    print_test_name("Testing launching one background job");

    init_job_table();

    int old_stdout = dup(STDOUT_FILENO);
    int fd = open_test_file_to_write("test_launching_one_bg_job.log");
    dup2(fd, STDOUT_FILENO);

    size_t total_commands;
    command_call **commands = parse_command("ls", &total_commands);
    commands[0]->background = 1;
    command_result *result = execute_command_call(commands[0]);

    dup2(old_stdout, STDOUT_FILENO);

    close(fd);

    handle_boolean_test(result->pid > 0, true, __LINE__, __FILE__, info);
    handle_int_test(result->job_id, 1, __LINE__, __FILE__, info);

    handle_int_test(job_table_size, 1, __LINE__, __FILE__, info);
    handle_command_call_test(job_table[0]->command, commands[0], __LINE__, __FILE__, info);

    destroy_command_result(result);
    free(commands);

    init_job_table();
}

void test_launching_multiple_bg_jobs(test_info *info) {
    print_test_name("Testing launching multiple background jobs");

    init_job_table();

    int old_stdout = dup(STDOUT_FILENO);
    int fd = open_test_file_to_write("test_launching_multiple_bg_jobs.log");
    size_t total_commands;

    for (int i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 2; i++) {
        dup2(fd, STDOUT_FILENO);
        command_call **commands = parse_command("ls", &total_commands);
        commands[0]->background = 1;
        command_result *result = execute_command_call(commands[0]);

        dup2(old_stdout, STDOUT_FILENO);

        handle_boolean_test(result->pid > 0, true, __LINE__, __FILE__, info);
        handle_int_test(result->job_id, i + 1, __LINE__, __FILE__, info);

        handle_int_test(job_table_size, i + 1, __LINE__, __FILE__, info);
        handle_command_call_test(job_table[i]->command, commands[0], __LINE__, __FILE__, info);

        destroy_command_result(result);
        free(commands);
    }

    close(fd);

    init_job_table();
}
