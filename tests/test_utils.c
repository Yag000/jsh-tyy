#include "../src/utils.h"
#include "test_core.h"
#include <stdlib.h>

#define NUM_TESTS 3

void test_case_add_to_set(test_info *info);
void test_case_remove_from_set(test_info *info);
void test_case_contains(test_info *info);

test_info *test_utils() {
    test_case test_cases[NUM_TESTS] = {QUICK_CASE("add_to_set", test_case_add_to_set),
                                       QUICK_CASE("remove_from_set", test_case_remove_from_set),
                                       QUICK_CASE("contains", test_case_contains)};

    return run_cases("utils", test_cases, NUM_TESTS);
}

void test_case_add_to_set(test_info *info) {

    int *set = malloc(100 * sizeof(int));

    for (size_t index = 0; index < 100; ++index) {
        set[index] = -1;
    }

    for (size_t index = 0; index < 100; ++index) {
        int status = add_set(set, 100, (int)index);

        CINTA_ASSERT_INT(0, status, info);
        CINTA_ASSERT_INT((int)index, set[index], info);
    }

    for (size_t index = 0; index < 101; ++index) {
        int status = add_set(set, 100, (int)index);

        CINTA_ASSERT_INT(-1, status, info);
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

        CINTA_ASSERT_INT(-1, status, info);
    }

    for (size_t index = 0; index < 100; ++index) {
        int status = remove_set(set, 100, (int)index);

        CINTA_ASSERT_INT(0, status, info);
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

        CINTA_ASSERT_INT(index % 2 == 0, status, info);
    }

    free(set);
}
