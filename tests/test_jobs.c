#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/internals.h"
#include "../src/jobs.h"
#include "test_core.h"
#include "utils.h"

#define NUM_TEST 18

void test_case_new_job(test_info *);

void test_case_print_job(test_info *);

void test_case_init_job_table(test_info *);

void test_case_add_first_job(test_info *);
void test_case_add_null_job(test_info *);
void test_case_expand_job_table(test_info *);
void test_case_expands_twice_job_table(test_info *);
void test_case_job_table_only_expands_when_needed(test_info *);

void test_case_remove_job_only_one(test_info *);
void test_case_remove_non_existent_job(test_info *);
void test_case_remove_job_does_not_shift_jobs(test_info *);
void test_case_remove_job_at_the_end_of_table(test_info *);
void test_empty_job_table_with_remove_job(test_info *);
void remove_job_2_with_3_jobs(test_info *);

void test_case_add_job_fills_null_position(test_info *);
void test_case_filling_job_table_after_deleting_it(test_info *);

void test_jobs_command_without_jobs_running(test_info *);
void test_jobs_command_with_jobs_running(test_info *);

test_info *test_jobs() {

    test_case cases[NUM_TEST] = {
        QUICK_CASE("Testing job_from_command", test_case_new_job),
        QUICK_CASE("Testing print_job", test_case_print_job),
        QUICK_CASE("Testing init_job_table", test_case_init_job_table),
        QUICK_CASE("Testing add_job - First job", test_case_add_first_job),
        QUICK_CASE("Testing add_job - NULL job", test_case_add_null_job),
        QUICK_CASE("Testing add_job - Expand job table", test_case_expand_job_table),
        QUICK_CASE("Testing add_job - Expand job table twice", test_case_expands_twice_job_table),
        QUICK_CASE("Testing add_job - Only expands when needed", test_case_job_table_only_expands_when_needed),
        QUICK_CASE("Testing remove_job - Only one job", test_case_remove_job_only_one),
        QUICK_CASE("Testing remove_job - Non existant job", test_case_remove_non_existent_job),
        QUICK_CASE("Testing remove_job - Shifts jobs", test_case_remove_job_does_not_shift_jobs),
        QUICK_CASE("Testing remove_job - At then end of table", test_case_remove_job_at_the_end_of_table),
        QUICK_CASE("Testing remove_job - Empty job table", test_empty_job_table_with_remove_job),
        QUICK_CASE("Testing remove_job - Remove job 2 with 3 jobs", remove_job_2_with_3_jobs),
        QUICK_CASE("Testing add_job - Fills NULL position", test_case_add_job_fills_null_position),
        QUICK_CASE("Testing add_job - Filling job table after deleting it",
                   test_case_filling_job_table_after_deleting_it),
        QUICK_CASE("Testing jobs command - without jobs running", test_jobs_command_without_jobs_running),
        SLOW_CASE("Testing jobs command - with jobs running", test_jobs_command_with_jobs_running)};

    test_info *info = cinta_run_cases("jobs", cases, NUM_TEST);

    // End of tests
    init_job_table();
    return info;
}

void test_case_new_job(test_info *info) {

    command *command = parse_command("pwd");
    job *job = job_from_command(command, 100, RUNNING);
    destroy_command(command);

    CINTA_ASSERT_INT(job->subjobs[0]->pid, 100, info);
    CINTA_ASSERT_INT(job->id, 0, info);

    destroy_job(job);
}

void test_case_print_job(test_info *info) {
    command *command = parse_command("pwd yes");
    job *job = new_single_command_job(command->command_calls[0], 100, RUNNING);
    job->pgid = 100;
    destroy_command(command);

    job->id = 1;

    int fd = open_test_file_to_write("test_print_job.log");

    print_job(job, fd);

    close(fd);

    fd = open_test_file_to_read("test_print_job.log");

    char buffer[1024];
    read(fd, buffer, 1024);
    close(fd);

    char *expected = "[1]\t100\tRunning\tpwd yes\n";
    buffer[strlen(expected)] = '\0';
    CINTA_ASSERT_STRING(buffer, expected, info);

    destroy_job(job);
}

void test_case_init_job_table(test_info *info) {
    init_job_table();

    CINTA_ASSERT_INT(job_table_size, 0, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, info);
    CINTA_ASSERT(job_table != NULL, info);
}

void test_case_add_null_job(test_info *info) {
    init_job_table();

    CINTA_ASSERT_INT(add_job(NULL), -1, info);

    CINTA_ASSERT_INT(job_table_size, 0, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, info);

    init_job_table();
}

void test_case_add_first_job(test_info *info) {
    init_job_table();

    command *command = parse_command("pwd");

    job *job = job_from_command(command, 100, RUNNING);
    destroy_command(command);

    CINTA_ASSERT_INT(add_job(job), 1, info);

    job->id = 1;

    CINTA_ASSERT_INT(job_table_size, 1, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, info);

    ASSERT_JOB(job, job_table[0], info);

    init_job_table();
}

void test_case_expand_job_table(test_info *info) {
    init_job_table();

    job **jobs = malloc((INITIAL_JOB_TABLE_CAPACITY + 1) * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        sprintf(buffer, "pwd %zu", i);

        command *command = parse_command(buffer);
        jobs[i] = job_from_command(command, 100 + i, RUNNING);
        destroy_command(command);

        CINTA_ASSERT_INT(add_job(jobs[i]), i + 1, info);
    }

    CINTA_ASSERT_INT(job_table_size, INITIAL_JOB_TABLE_CAPACITY + 1, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY * 2, info);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        ASSERT_JOB(jobs[i], job_table[i], info);
    }

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_case_expands_twice_job_table(test_info *info) {
    init_job_table();

    job **jobs = malloc((2 * INITIAL_JOB_TABLE_CAPACITY + 1) * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < 2 * INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        sprintf(buffer, "pwd %zu", i);

        command *command = parse_command(buffer);
        jobs[i] = job_from_command(command, 100 + i, RUNNING);
        destroy_command(command);

        add_job(jobs[i]);
    }

    CINTA_ASSERT_INT(job_table_size, 2 * INITIAL_JOB_TABLE_CAPACITY + 1, info);
    CINTA_ASSERT_INT(job_table_capacity, 3 * INITIAL_JOB_TABLE_CAPACITY, info);

    for (size_t i = 0; i < 2 * INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        ASSERT_JOB(jobs[i], job_table[i], info);
    }

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_case_job_table_only_expands_when_needed(test_info *info) {
    init_job_table();

    char *buffer = malloc(1024);
    job *job;
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        sprintf(buffer, "pwd %zu", i);

        command *command = parse_command(buffer);
        job = job_from_command(command, 100 + i, RUNNING);
        destroy_command(command);

        add_job(job);
    }

    CINTA_ASSERT_INT(job_table_size, INITIAL_JOB_TABLE_CAPACITY, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, info);

    free(buffer);

    init_job_table();
}

void test_case_remove_job_only_one(test_info *info) {
    init_job_table();

    command *command = parse_command("pwd");

    job *job = job_from_command(command, 100, RUNNING);
    destroy_command(command);

    add_job(job);

    CINTA_ASSERT_INT(remove_job(1), 0, info);

    CINTA_ASSERT_INT(job_table_size, 0, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, info);

    init_job_table();
}

void test_case_remove_non_existent_job(test_info *info) {
    init_job_table();

    command *command = parse_command("pwd");

    job *job = job_from_command(command, 100, RUNNING);
    destroy_command(command);

    add_job(job);

    CINTA_ASSERT_INT(remove_job(2), 1, info);

    CINTA_ASSERT_INT(job_table_size, 1, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, info);

    ASSERT_JOB(job, job_table[0], info);

    init_job_table();
}

void test_case_remove_job_does_not_shift_jobs(test_info *info) {
    init_job_table();

    job **jobs = malloc((INITIAL_JOB_TABLE_CAPACITY + 1) * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        sprintf(buffer, "pwd %zu", i);

        command *command = parse_command(buffer);
        jobs[i] = job_from_command(command, 100 + i, RUNNING);
        destroy_command(command);

        add_job(jobs[i]);
    }

    remove_job(1);

    CINTA_ASSERT_INT(job_table_size, INITIAL_JOB_TABLE_CAPACITY, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY * 2, info);

    CINTA_ASSERT_NULL(job_table[0], info);
    for (size_t i = 1; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        ASSERT_JOB(jobs[i], job_table[i], info);
    }

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_case_remove_job_at_the_end_of_table(test_info *info) {
    init_job_table();

    job **jobs = malloc((INITIAL_JOB_TABLE_CAPACITY) * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        sprintf(buffer, "pwd %zu", i);

        command *command = parse_command(buffer);
        jobs[i] = job_from_command(command, 100 + i, RUNNING);
        destroy_command(command);

        add_job(jobs[i]);
    }

    remove_job(INITIAL_JOB_TABLE_CAPACITY);

    CINTA_ASSERT_INT(job_table_size, INITIAL_JOB_TABLE_CAPACITY - 1, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, info);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY - 1; i++) {
        ASSERT_JOB(jobs[i], job_table[i], info);
    }

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_empty_job_table_with_remove_job(test_info *info) {
    init_job_table();

    char *buffer = malloc(1024);
    job *job;
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        sprintf(buffer, "pwd %zu", i);

        command *command = parse_command(buffer);
        job = job_from_command(command, 100 + i, RUNNING);
        destroy_command(command);

        add_job(job);
    }

    free(buffer);

    CINTA_ASSERT_INT(job_table_size, INITIAL_JOB_TABLE_CAPACITY + 1, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY * 2, info);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        remove_job(i + 1);
    }

    CINTA_ASSERT_INT(job_table_size, 0, info);
    CINTA_ASSERT_INT(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY * 2, info);

    init_job_table();
}

void remove_job_2_with_3_jobs(test_info *info) {
    init_job_table();

    job **jobs = malloc(3 * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < 3; i++) {
        sprintf(buffer, "pwd %zu", i);

        command *command = parse_command(buffer);
        jobs[i] = job_from_command(command, 100 + i, RUNNING);
        destroy_command(command);

        add_job(jobs[i]);
    }

    CINTA_ASSERT_INT(job_table_size, 3, info);

    remove_job(2);

    CINTA_ASSERT_INT(job_table_size, 2, info);

    ASSERT_JOB(jobs[0], job_table[0], info);
    CINTA_ASSERT_NULL(job_table[1], info);
    ASSERT_JOB(jobs[2], job_table[2], info);

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_case_add_job_fills_null_position(test_info *info) {
    init_job_table();

    job **jobs = malloc(3 * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < 3; i++) {
        sprintf(buffer, "pwd %zu", i);

        command *command = parse_command(buffer);
        jobs[i] = job_from_command(command, 100 + i, RUNNING);
        destroy_command(command);

        add_job(jobs[i]);
    }

    CINTA_ASSERT_INT(job_table_size, 3, info);

    remove_job(2);

    CINTA_ASSERT_INT(job_table_size, 2, info);

    command *command = parse_command("pwd new is here :)");
    job *job = job_from_command(command, 103, RUNNING);
    destroy_command(command);

    CINTA_ASSERT_INT(add_job(job), 2, info);

    CINTA_ASSERT_INT(job_table_size, 3, info);

    ASSERT_JOB(jobs[0], job_table[0], info);
    ASSERT_JOB(job, job_table[1], info);
    ASSERT_JOB(jobs[2], job_table[2], info);

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_case_filling_job_table_after_deleting_it(test_info *info) {
    init_job_table();

    char *buffer = malloc(1024);
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        sprintf(buffer, "pwd %zu", i);

        command *command = parse_command(buffer);
        job *job = job_from_command(command, 100 + i, RUNNING);
        destroy_command(command);

        add_job(job);
    }

    CINTA_ASSERT_INT(job_table_size, INITIAL_JOB_TABLE_CAPACITY, info);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        remove_job(i + 1);
    }

    CINTA_ASSERT_INT(job_table_size, 0, info);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        sprintf(buffer, "pwd %zu", i);

        command *command = parse_command(buffer);
        job *job = job_from_command(command, 100 + i, RUNNING);
        destroy_command(command);

        add_job(job);
    }

    CINTA_ASSERT_INT(job_table_size, INITIAL_JOB_TABLE_CAPACITY, info);

    free(buffer);

    init_job_table();
}

void test_jobs_command_without_jobs_running(test_info *info) {
    init_job_table();

    int fd = open_test_file_to_write("test_jobs_command_without_jobs_running.log");

    int current_stdout = dup(STDOUT_FILENO);

    dup2(fd, STDOUT_FILENO);

    command *command = parse_command("jobs");
    command_result *result = execute_command(command);

    dup2(current_stdout, STDOUT_FILENO);
    close(current_stdout);
    close(fd);

    int read_fd = open_test_file_to_read("test_jobs_command_without_jobs_running.log");

    char buffer[1024];
    int nb = read(read_fd, buffer, 1024);
    close(read_fd);

    CINTA_ASSERT_INT(0, nb, info);

    destroy_command_result(result);
    init_job_table();
}

void test_jobs_command_with_jobs_running(test_info *info) {
    init_job_table();

    // Init the expected string
    char *expected = calloc(1024 * INITIAL_JOB_TABLE_CAPACITY, sizeof(char));
    char *line = calloc(1024, sizeof(char));

    // Create and add jobs
    char *command_buffer = malloc(1024);
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        sprintf(command_buffer, "sleep %zu", i + 10);

        command *command = parse_command(command_buffer);
        command->background = 1;
        command_result *result = mute_command_execution(command);

        job *job = job_table[result->job_id - 1];
        subjob *subjob = job->subjobs[0];

        // Build the expected string
        sprintf(line, "[%ld]\t%d\t%s\t", job->id, subjob->pid, job_status_to_string(subjob->last_status));
        strcat(expected, line);
        strcat(expected, command_buffer);
        strcat(expected, "\n");

        destroy_command_result(result);
    }

    // Wait for the jobs to finish
    sleep(1);

    int current_stdout = dup(STDOUT_FILENO);

    int fd = open_test_file_to_write("test_jobs_command_with_jobs_running.log");

    dup2(fd, STDOUT_FILENO);

    command *command = parse_command("jobs");
    command_result *result = execute_command(command);

    dup2(current_stdout, STDOUT_FILENO);
    close(current_stdout);
    close(fd);

    int read_fd = open_test_file_to_read("test_jobs_command_with_jobs_running.log");

    char buffer[1024 * INITIAL_JOB_TABLE_CAPACITY];
    read(read_fd, buffer, 1024 * INITIAL_JOB_TABLE_CAPACITY);
    close(read_fd);
    buffer[strlen(expected)] = '\0';
    CINTA_ASSERT_STRING(buffer, expected, info);

    free(expected);
    free(line);
    free(command_buffer);
    destroy_command_result(result);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        kill(job_table[i]->subjobs[0]->pid, SIGKILL);
    }

    init_job_table();
}
