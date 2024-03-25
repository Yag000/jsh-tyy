#include "../src/jobs.h"
#include "../src/prompt.h"
#include "test_core.h"
#include "utils.h"
#include <string.h>

#define NUM_TEST 4

void test_promt_string_no_jobs(test_info *);
void test_promt_string_with_one_jobs(test_info *);
void test_promt_string_with_jobs(test_info *);
void test_promt_string_remove_jobs(test_info *);

test_info *test_prompt() {
    test_case cases[NUM_TEST] = {
        QUICK_CASE("Testing prompt string with no jobs", test_promt_string_no_jobs),
        QUICK_CASE("Testing prompt string with one job", test_promt_string_with_one_jobs),
        QUICK_CASE("Testing prompt string with jobs", test_promt_string_with_jobs),
        QUICK_CASE("Testing prompt string with jobs and removing them", test_promt_string_remove_jobs)};

    return run_cases("prompt", cases, NUM_TEST);
}

void test_promt_string_no_jobs(test_info *info) {

    init_job_table();

    job_table_size = 0;
    char *prompt_string = get_prompt_string();

    char *expected = ("\001\033[0;33m\002");
    prompt_string[strlen(expected)] = '\0';
    CINTA_ASSERT_STRING(prompt_string, expected, info);

    free(prompt_string);
}

void test_promt_string_with_one_jobs(test_info *info) {

    init_job_table();

    command *command = parse_command("pwd");
    job *job = new_single_command_job(command->command_calls[0], 100, RUNNING);
    destroy_command(command);
    add_job(job);

    char *prompt_string = get_prompt_string();

    char *expected = ("\001\033[0;33m\002[1]");
    prompt_string[strlen(expected)] = '\0';
    CINTA_ASSERT_STRING(prompt_string, expected, info);

    free(prompt_string);

    init_job_table();
}

void test_promt_string_with_jobs(test_info *info) {

    init_job_table();

    char *expected = malloc(100);

    for (int i = 0; i < 100; i++) {

        command *command = parse_command("pwd");
        job *job = new_single_command_job(command->command_calls[0], 100, RUNNING);
        add_job(job);
        destroy_command(command);

        snprintf(expected, 100, "\001\033[0;33m\002[%d]", i + 1);

        char *prompt_string = get_prompt_string();
        prompt_string[strlen(expected)] = '\0';

        CINTA_ASSERT_STRING(prompt_string, expected, info);
        free(prompt_string);
    }

    free(expected);

    init_job_table();
}

void test_promt_string_remove_jobs(test_info *info) {

    init_job_table();

    for (int i = 0; i < 100; i++) {

        command *command = parse_command("pwd");
        job *job = new_single_command_job(command->command_calls[0], 100, RUNNING);
        destroy_command(command);

        add_job(job);
    }

    char *expected = malloc(100);
    for (int i = 0; i < 100; i++) {
        remove_job(i + 1);

        snprintf(expected, 100, "\001\033[0;33m\002[%d]", 100 - i - 1);

        char *prompt_string = get_prompt_string();
        prompt_string[strlen(expected)] = '\0';

        CINTA_ASSERT_STRING(prompt_string, expected, info);
        free(prompt_string);
    }

    free(expected);

    init_job_table();
}
