#include "../src/jobs.h"
#include "../src/prompt.h"
#include "test_core.h"
#include <string.h>

void test_promt_string_no_jobs(test_info *);
void test_promt_string_with_one_jobs(test_info *);
void test_promt_string_with_jobs(test_info *);
void test_promt_string_remove_jobs(test_info *);

test_info *test_prompt() {
    // Test setup
    print_test_header("prompt");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add tests here
    test_promt_string_no_jobs(info);
    test_promt_string_with_one_jobs(info);
    test_promt_string_with_jobs(info);
    test_promt_string_remove_jobs(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("prompt", info);
    return info;
}

void test_promt_string_no_jobs(test_info *info) {
    print_test_name("Testing prompt string with no jobs");

    init_job_table();

    job_table_size = 0;
    char *prompt_string = get_prompt_string();

    char *expected = ("\001\033[0;33m\002");
    prompt_string[strlen(expected)] = '\0';
    handle_string_test(prompt_string, expected, __LINE__, __FILE__, info);

    free(prompt_string);
}

void test_promt_string_with_one_jobs(test_info *info) {
    print_test_name("Testing prompt string with jobs");

    init_job_table();

    command_call *command = parse_command("pwd");
    job *job = new_job(command, 100, RUNNING, FOREGROUND);
    add_job(job);

    char *prompt_string = get_prompt_string();

    char *expected = ("\001\033[0;33m\002[1]");
    prompt_string[strlen(expected)] = '\0';
    handle_string_test(prompt_string, expected, __LINE__, __FILE__, info);

    free(prompt_string);

    init_job_table();
}

void test_promt_string_with_jobs(test_info *info) {
    print_test_name("Testing prompt string with jobs");

    init_job_table();

    char *expected = malloc(100);

    for (int i = 0; i < 100; i++) {

        command_call *command = parse_command("pwd");
        job *job = new_job(command, 100, RUNNING, FOREGROUND);
        add_job(job);

        snprintf(expected, 100, "\001\033[0;33m\002[%d]", i + 1);

        char *prompt_string = get_prompt_string();
        prompt_string[strlen(expected)] = '\0';

        handle_string_test(prompt_string, expected, __LINE__, __FILE__, info);
        free(prompt_string);
    }

    free(expected);

    init_job_table();
}

void test_promt_string_remove_jobs(test_info *info) {
    print_test_name("Testing prompt string with jobs");

    init_job_table();

    for (int i = 0; i < 100; i++) {

        command_call *command = parse_command("pwd");
        job *job = new_job(command, 100, RUNNING, FOREGROUND);
        add_job(job);
    }

    char *expected = malloc(100);
    for (int i = 0; i < 100; i++) {
        remove_job(i + 1);

        snprintf(expected, 100, "\001\033[0;33m\002[%d]", 100 - i - 1);

        char *prompt_string = get_prompt_string();
        prompt_string[strlen(expected)] = '\0';

        handle_string_test(prompt_string, expected, __LINE__, __FILE__, info);
        free(prompt_string);
    }

    free(expected);

    init_job_table();
}
