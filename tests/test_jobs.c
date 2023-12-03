#include "../src/jobs.h"
#include "test_core.h"
#include "utils.h"

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

test_info *test_jobs() {

    // Test setup
    print_test_header("jobs");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add tests here
    test_case_new_job(info);

    test_case_print_job(info);

    test_case_init_job_table(info);

    test_case_add_first_job(info);
    test_case_add_null_job(info);
    test_case_expand_job_table(info);
    test_case_expands_twice_job_table(info);
    test_case_job_table_only_expands_when_needed(info);

    test_case_remove_job_only_one(info);
    test_case_remove_non_existent_job(info);
    test_case_remove_job_does_not_shift_jobs(info);
    test_case_remove_job_at_the_end_of_table(info);
    test_empty_job_table_with_remove_job(info);
    remove_job_2_with_3_jobs(info);

    test_case_add_job_fills_null_position(info);
    test_case_filling_job_table_after_deleting_it(info);

    // End of tests
    init_job_table();
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("jobs", info);
    return info;
}

void test_case_new_job(test_info *info) {
    print_test_name("Testing new_job");

    command_call *command_call = parse_command("pwd");

    job *job = new_job(command_call, 100, RUNNING, BACKGROUND);

    handle_int_test(job->pid, 100, __LINE__, __FILE__, info);
    handle_int_test(job->id, 0, __LINE__, __FILE__, info);

    destroy_job(job);
}

void test_case_print_job(test_info *info) {
    print_test_name("Testing print_job");

    command_call *command_call = parse_command("pwd yes");

    job *job = new_job(command_call, 100, RUNNING, BACKGROUND);
    job->id = 1;

    int fd = open_test_file_to_write("test_print_job.log");

    int current_stdout = dup(STDOUT_FILENO);

    dup2(fd, STDOUT_FILENO);

    print_job(job);

    dup2(current_stdout, STDOUT_FILENO);

    close(fd);

    fd = open_test_file_to_read("test_print_job.log");

    char buffer[1024];
    read(fd, buffer, 1024);
    close(fd);

    char *expected = "[1]\t100\tRunning\tpwd yes\n";
    buffer[strlen(expected)] = '\0';
    handle_string_test(buffer, expected, __LINE__, __FILE__, info);

    destroy_job(job);
}

void test_case_init_job_table(test_info *info) {
    print_test_name("Testing init_job_table");

    init_job_table();

    handle_int_test(job_table_size, 0, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);
    handle_boolean_test(true, job_table != NULL, __LINE__, __FILE__, info);
}

void test_case_add_null_job(test_info *info) {
    print_test_name("Testing add_job - NULL job");

    init_job_table();

    handle_int_test(add_job(NULL), -1, __LINE__, __FILE__, info);

    handle_int_test(job_table_size, 0, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);

    init_job_table();
}

void test_case_add_first_job(test_info *info) {
    print_test_name("Testing add_job - First job");

    init_job_table();

    command_call *command_call = parse_command("pwd");

    job *job = new_job(command_call, 100, RUNNING, BACKGROUND);

    handle_int_test(add_job(job), 1, __LINE__, __FILE__, info);

    job->id = 1;

    handle_int_test(job_table_size, 1, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);

    handle_job_test(job, job_table[0], __LINE__, __FILE__, info);

    init_job_table();
}

void test_case_expand_job_table(test_info *info) {
    print_test_name("Testing add_job - Expand job table");

    init_job_table();

    job **jobs = malloc((INITIAL_JOB_TABLE_CAPACITY + 1) * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        sprintf(buffer, "pwd %zu", i);
        command_call *command_call = parse_command(buffer);
        jobs[i] = new_job(command_call, 100 + i, RUNNING, BACKGROUND);

        handle_int_test(add_job(jobs[i]), i + 1, __LINE__, __FILE__, info);
    }

    handle_int_test(job_table_size, INITIAL_JOB_TABLE_CAPACITY + 1, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY * 2, __LINE__, __FILE__, info);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        handle_job_test(jobs[i], job_table[i], __LINE__, __FILE__, info);
    }

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_case_expands_twice_job_table(test_info *info) {
    print_test_name("Testing add_job - Expand job table twice");

    init_job_table();

    job **jobs = malloc((2 * INITIAL_JOB_TABLE_CAPACITY + 1) * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < 2 * INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        sprintf(buffer, "pwd %zu", i);
        command_call *command_call = parse_command(buffer);
        jobs[i] = new_job(command_call, 100 + i, RUNNING, BACKGROUND);

        add_job(jobs[i]);
    }

    handle_int_test(job_table_size, 2 * INITIAL_JOB_TABLE_CAPACITY + 1, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, 3 * INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);

    for (size_t i = 0; i < 2 * INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        handle_job_test(jobs[i], job_table[i], __LINE__, __FILE__, info);
    }

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_case_job_table_only_expands_when_needed(test_info *info) {
    print_test_name("Testing add_job - Only expands when needed");

    init_job_table();

    char *buffer = malloc(1024);
    job *job;
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        sprintf(buffer, "pwd %zu", i);
        command_call *command_call = parse_command(buffer);
        job = new_job(command_call, 100 + i, RUNNING, BACKGROUND);

        add_job(job);
    }

    handle_int_test(job_table_size, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);

    free(buffer);

    init_job_table();
}

void test_case_remove_job_only_one(test_info *info) {
    print_test_name("Testing remove_job - Only one job");

    init_job_table();

    command_call *command_call = parse_command("pwd");

    job *job = new_job(command_call, 100, RUNNING, BACKGROUND);

    add_job(job);

    handle_int_test(remove_job(1), 0, __LINE__, __FILE__, info);

    handle_int_test(job_table_size, 0, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);

    init_job_table();
}

void test_case_remove_non_existent_job(test_info *info) {
    print_test_name("Testing remove_job - Non existant job");

    init_job_table();

    command_call *command_call = parse_command("pwd");

    job *job = new_job(command_call, 100, RUNNING, BACKGROUND);

    add_job(job);

    handle_int_test(remove_job(2), 1, __LINE__, __FILE__, info);

    handle_int_test(job_table_size, 1, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);

    handle_job_test(job, job_table[0], __LINE__, __FILE__, info);

    init_job_table();
}

void test_case_remove_job_does_not_shift_jobs(test_info *info) {
    print_test_name("Testing remove_job - Shifts jobs");

    init_job_table();

    job **jobs = malloc((INITIAL_JOB_TABLE_CAPACITY + 1) * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        sprintf(buffer, "pwd %zu", i);
        command_call *command_call = parse_command(buffer);
        jobs[i] = new_job(command_call, 100 + i, RUNNING, BACKGROUND);

        add_job(jobs[i]);
    }

    remove_job(1);

    handle_int_test(job_table_size, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY * 2, __LINE__, __FILE__, info);

    handle_null_test(job_table[0], __LINE__, __FILE__, info);
    for (size_t i = 1; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        handle_job_test(jobs[i], job_table[i], __LINE__, __FILE__, info);
    }

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_case_remove_job_at_the_end_of_table(test_info *info) {
    print_test_name("Testing remove_job - At then end of table");

    init_job_table();

    job **jobs = malloc((INITIAL_JOB_TABLE_CAPACITY) * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        sprintf(buffer, "pwd %zu", i);
        command_call *command_call = parse_command(buffer);
        jobs[i] = new_job(command_call, 100 + i, RUNNING, BACKGROUND);

        add_job(jobs[i]);
    }

    remove_job(INITIAL_JOB_TABLE_CAPACITY);

    handle_int_test(job_table_size, INITIAL_JOB_TABLE_CAPACITY - 1, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY - 1; i++) {
        handle_job_test(jobs[i], job_table[i], __LINE__, __FILE__, info);
    }

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_empty_job_table_with_remove_job(test_info *info) {
    print_test_name("Testing remove_job - Empty job table");

    init_job_table();

    char *buffer = malloc(1024);
    job *job;
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        sprintf(buffer, "pwd %zu", i);
        command_call *command_call = parse_command(buffer);
        job = new_job(command_call, 100 + i, RUNNING, BACKGROUND);

        add_job(job);
    }

    free(buffer);

    handle_int_test(job_table_size, INITIAL_JOB_TABLE_CAPACITY + 1, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY * 2, __LINE__, __FILE__, info);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY + 1; i++) {
        remove_job(i + 1);
    }

    handle_int_test(job_table_size, 0, __LINE__, __FILE__, info);
    handle_int_test(job_table_capacity, INITIAL_JOB_TABLE_CAPACITY * 2, __LINE__, __FILE__, info);

    init_job_table();
}

void remove_job_2_with_3_jobs(test_info *info) {
    print_test_name("Testing remove_job - Remove job 2 with 3 jobs");

    init_job_table();

    job **jobs = malloc(3 * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < 3; i++) {
        sprintf(buffer, "pwd %zu", i);
        command_call *command_call = parse_command(buffer);
        jobs[i] = new_job(command_call, 100 + i, RUNNING, BACKGROUND);

        add_job(jobs[i]);
    }

    handle_int_test(job_table_size, 3, __LINE__, __FILE__, info);

    remove_job(2);

    handle_int_test(job_table_size, 2, __LINE__, __FILE__, info);

    handle_job_test(jobs[0], job_table[0], __LINE__, __FILE__, info);
    handle_null_test(job_table[1], __LINE__, __FILE__, info);
    handle_job_test(jobs[2], job_table[2], __LINE__, __FILE__, info);

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_case_add_job_fills_null_position(test_info *info) {
    print_test_name("Testing add_job - Fills NULL position");

    init_job_table();

    job **jobs = malloc(3 * sizeof(job *));
    char *buffer = malloc(1024);
    for (size_t i = 0; i < 3; i++) {
        sprintf(buffer, "pwd %zu", i);
        command_call *command_call = parse_command(buffer);
        jobs[i] = new_job(command_call, 100 + i, RUNNING, BACKGROUND);

        add_job(jobs[i]);
    }

    handle_int_test(job_table_size, 3, __LINE__, __FILE__, info);

    remove_job(2);

    handle_int_test(job_table_size, 2, __LINE__, __FILE__, info);

    command_call *command_call = parse_command("pwd new is here :)");
    job *job = new_job(command_call, 103, RUNNING, BACKGROUND);
    handle_int_test(add_job(job), 2, __LINE__, __FILE__, info);

    handle_int_test(job_table_size, 3, __LINE__, __FILE__, info);

    handle_job_test(jobs[0], job_table[0], __LINE__, __FILE__, info);
    handle_job_test(job, job_table[1], __LINE__, __FILE__, info);
    handle_job_test(jobs[2], job_table[2], __LINE__, __FILE__, info);

    free(buffer);
    free(jobs);

    init_job_table();
}

void test_case_filling_job_table_after_deleting_it(test_info *info) {
    print_test_name("Testing add_job - Fills NULL position");

    init_job_table();

    char *buffer = malloc(1024);
    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        sprintf(buffer, "pwd %zu", i);
        command_call *command_call = parse_command(buffer);
        job *job = new_job(command_call, 100 + i, RUNNING, BACKGROUND);
        add_job(job);
    }

    handle_int_test(job_table_size, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        remove_job(i + 1);
    }

    handle_int_test(job_table_size, 0, __LINE__, __FILE__, info);

    for (size_t i = 0; i < INITIAL_JOB_TABLE_CAPACITY; i++) {
        sprintf(buffer, "pwd %zu", i);
        command_call *command_call = parse_command(buffer);
        job *job = new_job(command_call, 100 + i, RUNNING, BACKGROUND);
        add_job(job);
    }

    handle_int_test(job_table_size, INITIAL_JOB_TABLE_CAPACITY, __LINE__, __FILE__, info);

    free(buffer);

    init_job_table();
}
