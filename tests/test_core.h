// Original code developed by Yago Iglesias for a
// c project. Github: https://github.com/Yag000/l2_s4_c-project
#ifndef TEST_CORE_H
#define TEST_CORE_H

#include <stdbool.h>
#include <time.h>

extern bool debug;

/** Structure to hold test information. */
typedef struct test_info {
    int passed;
    int failed;
    int total;
    double time;
} test_info;

test_info *create_test_info();
void destroy_test_info(test_info *);
void print_test_info(const test_info *);

// Test utils
double clock_ticks_to_seconds(clock_t);

void print_test_header(const char *);
void print_test_footer(const char *, const test_info *);
void print_test_name(const char *);

void handle_string_test(const char *, const char *, int, const char *, test_info *);
void handle_boolean_test(bool, bool, int, const char *, test_info *);
void handle_int_test(int, int, int, const char *, test_info *);

// All the tests
test_info *test_example();

#endif
