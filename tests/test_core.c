#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_core.h"

static void print_green() { printf("\033[0;32m"); }

static void print_red() { printf("\033[0;31m"); }

static void print_no_color() { printf("\033[0m"); }

/** Creates a new test_info. */
test_info *create_test_info() {
    test_info *info = malloc(sizeof(test_info));
    assert(info != NULL);
    info->passed = 0;
    info->failed = 0;
    info->total = 0;
    info->time = 0;

    return info;
}

/** Destroys the test_info . */
void destroy_test_info(test_info *info) { free(info); }

/** Prints the test info. */
void print_test_info(const test_info *info) {
    if (info->failed > 0) {
        print_red();
    } else {
        print_green();
    }
    printf("passed: %d, failed: %d, total: %d, time: %f seconds\n", info->passed, info->failed, info->total,
           info->time);
    print_no_color();
}

double clock_ticks_to_seconds(clock_t ticks) { return (double)ticks / CLOCKS_PER_SEC; }

void print_test_header(const char *name) {
    if (debug) {
        printf("\n----------------------- Testing %s -----------------------\n", name);
    }
}

void print_test_footer(const char *name, const test_info *info) {
    if (debug) {
        printf("\n");
    }

    printf("Test %s: ", name);
    print_test_info(info);

    if (debug) {
        printf("\n----------------------- End test %s -----------------------\n", name);
    }
}

void print_test_name(const char *name) {
    if (debug) {
        printf("\n-> %s\n", name);
    }
}

void handle_string_test(const char *expected, const char *actual, int line, const char *file, test_info *info) {
    info->total++;
    if (strcmp(expected, actual) != 0) {
        print_red();
        printf("Error: %s != %s at line %d in file %s\n", expected, actual, line, file);
        print_no_color();
        info->failed++;
        return;
    }
    if (debug) {
        print_green();
        printf("Passed: %s == %s at line %d in file %s\n", expected, actual, line, file);
        print_no_color();
    }
    info->passed++;
}

void handle_boolean_test(bool expected, bool actual, int line, const char *file, test_info *info) {
    info->total++;
    if (expected != actual) {
        print_red();
        printf("Error: %d != %d at line %d in file %s\n", expected, actual, line, file);
        print_no_color();
        info->failed++;
        return;
    }
    if (debug) {
        print_green();
        printf("Passed: %d == %d at line %d in file %s\n", expected, actual, line, file);
        print_no_color();
    }
    info->passed++;
}

void handle_int_test(int expected, int actual, int line, const char *file, test_info *info) {
    info->total++;
    if (expected != actual) {
        print_red();
        printf("Error: %d != %d at line %d in file %s\n", expected, actual, line, file);
        print_no_color();
        info->failed++;
    }
    if (debug) {
        print_green();
        printf("Passed: %d == %d at line %d in file %s\n", expected, actual, line, file);
        print_no_color();
    }
    info->passed++;
}
