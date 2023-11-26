#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test_core.h"

bool debug;

static void update_test_info(test_info *, test_info *);

/** This is the main function for the test program. Every test should be
 * called from here and the results will be printed.
 */
int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        debug = true;
        printf("Debug mode enabled\n");
    }

    // Create the test info
    test_info *info = create_test_info();
    clock_t start = clock();

    // Add tests here
    update_test_info(info, test_string_utils());
    update_test_info(info, test_command());
    update_test_info(info, test_cd());
    update_test_info(info, test_last_exit_code_command());
    update_test_info(info, test_pwd());
    update_test_info(info, test_exit());
    update_test_info(info, test_external_commands());

    // End of tests
    clock_t end = clock();
    info->time = clock_ticks_to_seconds(end - start);
    bool success = info->passed == info->total;

    printf("\nTotal: ");
    print_test_info(info);
    destroy_test_info(info);

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void update_test_info(test_info *target_info, test_info *origin_info) {
    target_info->passed += origin_info->passed;
    target_info->failed += origin_info->failed;
    target_info->total += origin_info->total;

    destroy_test_info(origin_info);
}
