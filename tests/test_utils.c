#include "../src/utils.h"
#include "test_core.h"
#include <stdlib.h>

void test_case_add_to_set(test_info *info);
void test_case_remove_from_set(test_info *info);
void test_case_contains(test_info *info);

test_info *test_utils() {
    // Test setup
    print_test_header("test_utils");
    clock_t start = clock();
    test_info *info = create_test_info();

    test_case_add_to_set(info);
    test_case_remove_from_set(info);
    test_case_contains(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("test_utils", info);
    return info;
}

void test_case_add_to_set(test_info *info) {

    int *set = malloc(100 * sizeof(int));

    for (size_t index = 0; index < 100; ++index) {
        set[index] = -1;
    }

    for (size_t index = 0; index < 100; ++index) {
        int status = add_set(set, 100, (int)index);

        handle_int_test(0, status, __LINE__, __FILE__, info);
        handle_int_test((int)index, set[index], __LINE__, __FILE__, info);
    }

    for (size_t index = 0; index < 101; ++index) {
        int status = add_set(set, 100, (int)index);

        handle_int_test(-1, status, __LINE__, __FILE__, info);
    }

    free(set);
}

void test_case_remove_from_set(test_info *info) {

    int *set = malloc(100 * sizeof(int));

    for (size_t index = 0; index < 100; ++index) {
        set[index] = -1;
    }

    for (size_t index = 0; index < 100; ++index) {
        add_set(set, 100, (int)index);
    }

    for (size_t index = 100; index < 200; ++index) {
        int status = remove_set(set, 100, (int)index);

        handle_int_test(-1, status, __LINE__, __FILE__, info);
    }

    for (size_t index = 0; index < 100; ++index) {
        int status = remove_set(set, 100, (int)index);

        handle_int_test(0, status, __LINE__, __FILE__, info);
    }

    free(set);
}

void test_case_contains(test_info *info) {

    int *set = malloc(100 * sizeof(int));

    for (size_t index = 0; index < 100; ++index) {
        set[index] = -1;
        add_set(set, 100, (int)index * 2);
    }

    for (size_t index = 0; index < 200; ++index) {
        int status = contains(set, 100, (int)index);

        handle_boolean_test(index % 2 == 0, status, __LINE__, __FILE__, info);
    }

    free(set);
}
