#include "../src/string_utils.h"
#include "test_core.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_case_starts_with(test_info *);
static void test_case_trim_start(test_info *);
static void test_case_get_number_of_words_left(test_info *info);
static void test_case_has_next_word(test_info *info);
static void test_case_new_string_iterator(test_info *info);
static void test_case_next_word(test_info *info);
static void test_case_split_string(test_info *info);
static void test_case_join_string(test_info *info);

test_info *test_string_utils() {
    // Test setup
    print_test_header("string_utils");
    clock_t start = clock();
    test_info *info = create_test_info();

    test_case_new_string_iterator(info);
    test_case_starts_with(info);
    test_case_trim_start(info);
    test_case_get_number_of_words_left(info);
    test_case_has_next_word(info);
    test_case_next_word(info);
    test_case_split_string(info);
    test_case_join_string(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("string_utils", info);
    return info;
}

static void test_case_new_string_iterator(test_info *info) {
    string_iterator *str_iter;

    // Classic String iterator
    str_iter = new_string_iterator("sentence", "word");
    handle_string_test("sentence", str_iter->string, __LINE__, __FILE__, info);
    handle_string_test("word", str_iter->separator, __LINE__, __FILE__, info);
    handle_int_test(0, str_iter->index, __LINE__, __FILE__, info);
    free(str_iter);

    // Cannot have an empty separator
    str_iter = new_string_iterator("sentence", "");
    handle_null_test(str_iter, __LINE__, __FILE__, info);
    free(str_iter);
}

static void test_case_starts_with(test_info *info) {
    int result;

    print_test_name("Testing `starts_with`");

    // Empty character
    result = starts_with("sentence", "");
    handle_boolean_test(true, result, __LINE__, __FILE__, info);

    // Simple
    result = starts_with("ex4mple", "ex4");
    handle_boolean_test(true, result, __LINE__, __FILE__, info);

    // Single character
    result = starts_with("word", "w");
    handle_boolean_test(true, result, __LINE__, __FILE__, info);

    // Does not starts with
    result = starts_with("starts_with", "with");
    handle_boolean_test(false, result, __LINE__, __FILE__, info);

    // With non alphanumeric characters
    result = starts_with("  _ randomizer", "  _ r");
    handle_boolean_test(true, result, __LINE__, __FILE__, info);

    // With tabulations
    result = starts_with("\ttabulation      ", "\ttab");
    handle_boolean_test(true, result, __LINE__, __FILE__, info);
}

static void test_case_trim_start(test_info *info) {
    string_iterator *str_iter;

    print_test_name("Testing `trim_start`");

    // Starting with one pattern
    str_iter = new_string_iterator(" test", " ");
    trim_start(str_iter);
    handle_string_test("test", str_iter->string, __LINE__, __FILE__, info);
    free(str_iter);

    // Same with another pattern
    str_iter = new_string_iterator("\ttest", "\tt");
    trim_start(str_iter);
    handle_string_test("est", str_iter->string, __LINE__, __FILE__, info);
    free(str_iter);

    // Match complete word
    str_iter = new_string_iterator("complete word", "complete word");
    trim_start(str_iter);
    handle_string_test("", str_iter->string, __LINE__, __FILE__, info);
    free(str_iter);

    // Non present pattern
    str_iter = new_string_iterator("word", " ");
    trim_start(str_iter);
    handle_string_test("word", str_iter->string, __LINE__, __FILE__, info);
    free(str_iter);

    // Repeating pattern
    str_iter = new_string_iterator("ABCABCABC", "ABC");
    trim_start(str_iter);
    handle_string_test("", str_iter->string, __LINE__, __FILE__, info);
    free(str_iter);

    // 3/2 times pattern
    str_iter = new_string_iterator("...example", "..");
    trim_start(str_iter);
    handle_string_test(".example", str_iter->string, __LINE__, __FILE__, info);
    free(str_iter);

    // Longer than the word itself
    str_iter = new_string_iterator("comp", "complete");
    trim_start(str_iter);
    handle_string_test("comp", str_iter->string, __LINE__, __FILE__, info);
    free(str_iter);
}

static void test_case_get_number_of_words_left(test_info *info) {
    int result;
    string_iterator *str_iter;

    print_test_name("Testing `get_number_of_words_left`");

    // Empty word
    str_iter = new_string_iterator("aaaa", "a");
    result = get_number_of_words_left(str_iter);
    free(str_iter);
    handle_int_test(0, result, __LINE__, __FILE__, info);

    // Non present separator
    str_iter = new_string_iterator("test", " ");
    result = get_number_of_words_left(str_iter);
    free(str_iter);
    handle_int_test(1, result, __LINE__, __FILE__, info);

    // Consecutive separator
    str_iter = new_string_iterator("it     works??", " ");
    result = get_number_of_words_left(str_iter);
    free(str_iter);
    handle_int_test(2, result, __LINE__, __FILE__, info);

    // Simple sentence
    str_iter = new_string_iterator("this is a simple sentence", " ");
    result = get_number_of_words_left(str_iter);
    free(str_iter);
    handle_int_test(5, result, __LINE__, __FILE__, info);

    // 3/2 times a separator
    //                             "[].O.M.G[][].I.think[].this[][].is[][].working now!![]."
    str_iter = new_string_iterator("...O.M.G.....I.think...this.....is.....working now!!...", "..");
    result = get_number_of_words_left(str_iter);
    free(str_iter);
    handle_int_test(6, result, __LINE__, __FILE__, info);
}

static void test_case_has_next_word(test_info *info) {
    int result;
    string_iterator *str_iter;

    print_test_name("Testing `has_next_word`");

    // Non present separator
    str_iter = new_string_iterator("test", "?");
    result = has_next_word(str_iter);
    free(str_iter);
    handle_boolean_test(1, result, __LINE__, __FILE__, info);

    // Repeating separator at start
    str_iter = new_string_iterator("aaaaaaaaaaatest", "aaa");
    result = has_next_word(str_iter);
    free(str_iter);
    handle_boolean_test(1, result, __LINE__, __FILE__, info);

    // Word only composed of repeating separator
    str_iter = new_string_iterator("ABCABCABCABC", "ABC");
    result = has_next_word(str_iter);
    free(str_iter);
    handle_boolean_test(0, result, __LINE__, __FILE__, info);
}

static void test_case_next_word(test_info *info) {
    char *result;
    string_iterator *str_iter;

    print_test_name("Testing `next_word`");

    // Non present separator
    str_iter = new_string_iterator("test", "?");
    result = next_word(str_iter);
    handle_string_test("test", result, __LINE__, __FILE__, info);
    handle_string_test("", str_iter->string, __LINE__, __FILE__, info);
    handle_int_test(1, str_iter->index, __LINE__, __FILE__, info);
    free(str_iter);
    free(result);

    // Repeating separator
    str_iter = new_string_iterator("first..word", ".");
    handle_int_test(0, str_iter->index, __LINE__, __FILE__, info);
    result = next_word(str_iter);
    handle_string_test("first", result, __LINE__, __FILE__, info);
    handle_int_test(1, str_iter->index, __LINE__, __FILE__, info);
    free(result);

    // Continue with previous test
    result = next_word(str_iter);
    handle_string_test("word", result, __LINE__, __FILE__, info);
    handle_int_test(2, str_iter->index, __LINE__, __FILE__, info);
    free(result);

    // Continue again. No more words.
    result = next_word(str_iter);
    handle_string_test("", result, __LINE__, __FILE__, info);
    handle_int_test(2, str_iter->index, __LINE__, __FILE__, info);
    free(str_iter);

    // 3/2 times separator
    str_iter = new_string_iterator("###word", "##");
    result = next_word(str_iter);
    handle_string_test("#word", result, __LINE__, __FILE__, info);
    free(str_iter);
    free(result);
}

static void test_case_split_string(test_info *info) {
    char **result;
    size_t size;

    print_test_name("Testing `split_string`");

    // Empty string
    result = split_string("", " ", &size);
    handle_int_test(0, size, __LINE__, __FILE__, info);
    handle_null_test(result, __LINE__, __FILE__, info);

    // Only separator
    result = split_string("    ", " ", &size);
    handle_int_test(0, size, __LINE__, __FILE__, info);
    handle_null_test(result, __LINE__, __FILE__, info);

    // Simple sentence
    result = split_string("this is a sentence", " ", &size);
    handle_int_test(4, size, __LINE__, __FILE__, info);
    char *expected_1[4] = {"this", "is", "a", "sentence"};
    for (size_t index = 0; index < size; ++index) {
        handle_string_test(expected_1[index], result[index], __LINE__, __FILE__, info);
        free(result[index]);
    }
    free(result);

    // Repeating separator
    result = split_string("this____is____also___a____sentence____", "_", &size);
    handle_int_test(5, size, __LINE__, __FILE__, info);
    char *expected_2[5] = {"this", "is", "also", "a", "sentence"};
    for (size_t index = 0; index < size; ++index) {
        handle_string_test(expected_2[index], result[index], __LINE__, __FILE__, info);
        free(result[index]);
    }
    free(result);

    // 3/2 times separator
    result = split_string("...O.M.G.....I.think...this.....is.....working now!!...", "..", &size);
    handle_int_test(6, size, __LINE__, __FILE__, info);
    char *expected_3[6] = {".O.M.G", ".I.think", ".this", ".is", ".working now!!", "."};
    for (size_t index = 0; index < size; ++index) {
        handle_string_test(expected_3[index], result[index], __LINE__, __FILE__, info);
        free(result[index]);
    }
    free(result);

    // Trimming needed
    result = split_string("-----this-is-also-a-sentence------", "-", &size);
    handle_int_test(5, size, __LINE__, __FILE__, info);
    char *expected_4[5] = {"this", "is", "also", "a", "sentence"};
    for (size_t index = 0; index < size; ++index) {
        handle_string_test(expected_4[index], result[index], __LINE__, __FILE__, info);
        free(result[index]);
    }
    free(result);
}

static void test_case_join_string(test_info *info) {
    char **splitted;
    char *result;
    size_t size;

    print_test_name("Testing `join_string`");

    // Empty string
    splitted = split_string("", "_", &size);
    result = join_strings(splitted, size, " ");
    handle_string_test("", result, __LINE__, __FILE__, info);
    free(splitted);

    // Simple sentence
    splitted = split_string("this is a really complex sentence.", " ", &size);
    result = join_strings(splitted, size, "-");
    handle_string_test("this-is-a-really-complex-sentence.", result, __LINE__, __FILE__, info);
    for (size_t index = 0; index < size; ++index) {
        free(splitted[index]);
    }
    free(splitted);
    free(result);

    // Trimming needed
    splitted = split_string("______with______some____spaces_____", "_", &size);
    result = join_strings(splitted, size, "@");
    handle_string_test("with@some@spaces", result, __LINE__, __FILE__, info);
    for (size_t index = 0; index < size; ++index) {
        free(splitted[index]);
    }
    free(splitted);
    free(result);

    // Multiple characters separator
    splitted = split_string("I  do not have    enough   inspiration ...", " ", &size);
    result = join_strings(splitted, size, "][");
    handle_string_test("I][do][not][have][enough][inspiration][...", result, __LINE__, __FILE__, info);
    for (size_t index = 0; index < size; ++index) {
        free(splitted[index]);
    }
    free(splitted);
    free(result);

    // Single word due to an absent separator
    splitted = split_string(" [ @  Single word  @  ]  ", "%", &size);
    result = join_strings(splitted, size, "+");
    handle_string_test(" [ @  Single word  @  ]  ", result, __LINE__, __FILE__, info);
    for (size_t index = 0; index < size; ++index) {
        free(splitted[index]);
    }
    free(splitted);
    free(result);
}
