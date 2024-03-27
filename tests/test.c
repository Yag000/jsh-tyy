#include "test_core.h"

#define NUM_TESTS 13

test tests[NUM_TESTS] = {test_string_utils,
                         test_utils,
                         test_command,
                         test_cd,
                         test_last_exit_code_command,
                         test_pwd,
                         test_exit,
                         test_jobs,
                         test_running_jobs,
                         test_prompt,
                         test_background_jobs,
                         test_redirection,
                         test_redirection_parsing};

/** This is the main function for the test program. Every test should be
 * called from here and the results will be printed.
 */
int main(int argc, char *argv[]) {
    init_job_table();
    int res = cinta_main(argc, argv, tests, NUM_TESTS);
    destroy_job_table();

    return res;
}
