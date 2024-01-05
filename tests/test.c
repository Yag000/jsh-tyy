#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/jobs.h"
#include "../src/signals.h"
#include "test_core.h"

bool debug;
bool allow_slow = true;

static void update_test_info(test_info *, test_info *);

/** This is the main function for the test program. Every test should be
 * called from here and the results will be printed.
 */
int main(int argc, char *argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                debug = true;
            }
            if (strcmp(argv[i], "-q") == 0) {
                allow_slow = false;
            }
        }
    }

    ignore_signals();

    // Create the test info
    test_info *info = create_test_info();
    clock_t start = clock();

    // Add tests here
    update_test_info(info, test_string_utils());
    update_test_info(info, test_utils());
    update_test_info(info, test_command());
    update_test_info(info, test_cd());
    update_test_info(info, test_last_exit_code_command());
    update_test_info(info, test_pwd());
    update_test_info(info, test_exit());
    update_test_info(info, test_jobs());
    update_test_info(info, test_running_jobs());
    update_test_info(info, test_prompt());
    update_test_info(info, test_background_jobs());
    update_test_info(info, test_redirection());
    update_test_info(info, test_redirection_parsing());

    // End of tests
    clock_t end = clock();
    info->time = clock_ticks_to_seconds(end - start);
    bool success = info->passed == info->total;

    printf("\nTotal: ");
    print_test_info(info);
    destroy_test_info(info);

    destroy_job_table();

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void update_test_info(test_info *target_info, test_info *origin_info) {
    target_info->passed += origin_info->passed;
    target_info->failed += origin_info->failed;
    target_info->total += origin_info->total;

    destroy_test_info(origin_info);
}
