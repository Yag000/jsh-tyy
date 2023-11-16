#include "../src/example.h"
#include "test_core.h"

static void test_case_example(test_info *);

test_info *test_example() {
    // Test setup
    print_test_header("pwd");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add tests here
    test_case_example(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("pwd", info);
    return info;
}

static void test_case_example(test_info *info) {
    print_test_name("Testing command pwd");

    this_is_an_example();

    handle_boolean_test(true, true, __LINE__, __FILE__, info);
}
