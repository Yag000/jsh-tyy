#include "../src/string_utils.h"
#include "test_core.h"
#include <stddef.h>
#include <stdlib.h>

#define NUM_TEST 19

static void test_case_starts_with(test_info *);
static void test_case_trim_start(test_info *);
static void test_case_get_number_of_words_left(test_info *info);
static void test_case_has_next_word(test_info *info);
static void test_case_new_string_iterator(test_info *info);
static void test_case_next_word(test_info *info);
static void test_case_split_string(test_info *info);
static void test_case_join_string(test_info *info);
static void test_case_trunc_start(test_info *info);
static void test_case_split_string_keep_trace(test_info *info);

void test_case_null_trim_spaces(test_info *info);
void test_case_empty_trim_spaces(test_info *info);
void test_case_only_whitespace_trim_spaces(test_info *info);
void test_case_nothing_to_trim_trim_spaces(test_info *info);
void test_case_trim_start_trim_spaces(test_info *info);
void test_case_trim_end_trim_spaces(test_info *info);
void test_case_trim_both_trim_spaces(test_info *info);

void test_case_string_made_of(test_info *info);
void test_case_repeat(test_info *info);

test_info *test_string_utils() {
    test_case cases[NUM_TEST] = {
        QUICK_CASE("Testing `starts_with`", test_case_starts_with),
        QUICK_CASE("Testing `trim_start`", test_case_trim_start),
        QUICK_CASE("Testing `get_number_of_words_left`", test_case_get_number_of_words_left),
        QUICK_CASE("Testing `has_next_word`", test_case_has_next_word),
        QUICK_CASE("Testing `new_string_iterator`", test_case_new_string_iterator),
        QUICK_CASE("Testing `next_word`", test_case_next_word),
        QUICK_CASE("Testing `split_string`", test_case_split_string),
        QUICK_CASE("Testing `join_string`", test_case_join_string),
        QUICK_CASE("Testing `trunc_start`", test_case_trunc_start),
        QUICK_CASE("Testing `split_string_keep_trace`", test_case_split_string_keep_trace),
        QUICK_CASE("Testing `trim_spaces` with NULL string", test_case_null_trim_spaces),
        QUICK_CASE("Testing `trim_spaces` with empty string", test_case_empty_trim_spaces),
        QUICK_CASE("Testing `trim_spaces` with only spaces string", test_case_only_whitespace_trim_spaces),
        QUICK_CASE("Testing `trim_spaces` with nothing to trim", test_case_nothing_to_trim_trim_spaces),
        QUICK_CASE("Testing `trim_spaces` with only spaces at start", test_case_trim_start_trim_spaces),
        QUICK_CASE("Testing `trim_spaces` with only spaces at end", test_case_trim_end_trim_spaces),
        QUICK_CASE("Testing `trim_spaces` with spaces at start and end", test_case_trim_both_trim_spaces),
        QUICK_CASE("Testing `is_only_composed_of`", test_case_string_made_of),
        QUICK_CASE("Testing `repeat`", test_case_repeat)};

    return run_cases("String Utils", cases, NUM_TEST);
}

static void test_case_new_string_iterator(test_info *info) {
    string_iterator *str_iter;

    // Classic String iterator
    str_iter = new_string_iterator("sentence", "word");
    CINTA_ASSERT_STRING("sentence", str_iter->string, info);
    CINTA_ASSERT_STRING("word", str_iter->separator, info);
    CINTA_ASSERT_INT(0, str_iter->index, info);
    free(str_iter);

    // Cannot have an empty separator
    str_iter = new_string_iterator("sentence", "");
    CINTA_ASSERT_NULL(str_iter, info);
    free(str_iter);
}

static void test_case_starts_with(test_info *info) {
    int result;

    // Empty character
    result = starts_with("sentence", "");
    CINTA_ASSERT_INT(true, result, info);

    // Simple
    result = starts_with("ex4mple", "ex4");
    CINTA_ASSERT_INT(true, result, info);

    // Single character
    result = starts_with("word", "w");
    CINTA_ASSERT_INT(true, result, info);

    // Does not starts with
    result = starts_with("starts_with", "with");
    CINTA_ASSERT_INT(false, result, info);

    // With non alphanumeric characters
    result = starts_with("  _ randomizer", "  _ r");
    CINTA_ASSERT_INT(true, result, info);

    // With tabulations
    result = starts_with("\ttabulation      ", "\ttab");
    CINTA_ASSERT_INT(true, result, info);
}

static void test_case_trim_start(test_info *info) {
    string_iterator *str_iter;

    // Starting with one pattern
    str_iter = new_string_iterator(" test", " ");
    trim_start(str_iter);
    CINTA_ASSERT_STRING("test", str_iter->string, info);
    free(str_iter);

    // Same with another pattern
    str_iter = new_string_iterator("\ttest", "\tt");
    trim_start(str_iter);
    CINTA_ASSERT_STRING("est", str_iter->string, info);
    free(str_iter);

    // Match complete word
    str_iter = new_string_iterator("complete word", "complete word");
    trim_start(str_iter);
    CINTA_ASSERT_STRING("", str_iter->string, info);
    free(str_iter);

    // Non present pattern
    str_iter = new_string_iterator("word", " ");
    trim_start(str_iter);
    CINTA_ASSERT_STRING("word", str_iter->string, info);
    free(str_iter);

    // Repeating pattern
    str_iter = new_string_iterator("ABCABCABC", "ABC");
    trim_start(str_iter);
    CINTA_ASSERT_STRING("", str_iter->string, info);
    free(str_iter);

    // 3/2 times pattern
    str_iter = new_string_iterator("...example", "..");
    trim_start(str_iter);
    CINTA_ASSERT_STRING(".example", str_iter->string, info);
    free(str_iter);

    // Longer than the word itself
    str_iter = new_string_iterator("comp", "complete");
    trim_start(str_iter);
    CINTA_ASSERT_STRING("comp", str_iter->string, info);
    free(str_iter);
}

static void test_case_get_number_of_words_left(test_info *info) {
    int result;
    string_iterator *str_iter;

    // Empty word
    str_iter = new_string_iterator("aaaa", "a");
    result = get_number_of_words_left(str_iter);
    free(str_iter);
    CINTA_ASSERT_INT(0, result, info);

    // Non present separator
    str_iter = new_string_iterator("test", " ");
    result = get_number_of_words_left(str_iter);
    free(str_iter);
    CINTA_ASSERT_INT(1, result, info);

    // Consecutive separator
    str_iter = new_string_iterator("it     works??", " ");
    result = get_number_of_words_left(str_iter);
    free(str_iter);
    CINTA_ASSERT_INT(2, result, info);

    // Simple sentence
    str_iter = new_string_iterator("this is a simple sentence", " ");
    result = get_number_of_words_left(str_iter);
    free(str_iter);
    CINTA_ASSERT_INT(5, result, info);

    // 3/2 times a separator
    //                             "[].O.M.G[][].I.think[].this[][].is[][].working now!![]."
    str_iter = new_string_iterator("...O.M.G.....I.think...this.....is.....working now!!...", "..");
    result = get_number_of_words_left(str_iter);
    free(str_iter);
    CINTA_ASSERT_INT(6, result, info);
}

static void test_case_has_next_word(test_info *info) {
    int result;
    string_iterator *str_iter;

    // Non present separator
    str_iter = new_string_iterator("test", "?");
    result = has_next_word(str_iter);
    free(str_iter);
    CINTA_ASSERT_INT(1, result, info);

    // Repeating separator at start
    str_iter = new_string_iterator("aaaaaaaaaaatest", "aaa");
    result = has_next_word(str_iter);
    free(str_iter);
    CINTA_ASSERT_INT(1, result, info);

    // Word only composed of repeating separator
    str_iter = new_string_iterator("ABCABCABCABC", "ABC");
    result = has_next_word(str_iter);
    free(str_iter);
    CINTA_ASSERT_INT(0, result, info);
}

static void test_case_next_word(test_info *info) {
    char *result;
    string_iterator *str_iter;

    // Non present separator
    str_iter = new_string_iterator("test", "?");
    result = next_word(str_iter);
    CINTA_ASSERT_STRING("test", result, info);
    CINTA_ASSERT_STRING("", str_iter->string, info);
    CINTA_ASSERT_INT(1, str_iter->index, info);
    free(str_iter);
    free(result);

    // Repeating separator
    str_iter = new_string_iterator("first..word", ".");
    CINTA_ASSERT_INT(0, str_iter->index, info);
    result = next_word(str_iter);
    CINTA_ASSERT_STRING("first", result, info);
    CINTA_ASSERT_INT(1, str_iter->index, info);
    free(result);

    // Continue with previous test
    result = next_word(str_iter);
    CINTA_ASSERT_STRING("word", result, info);
    CINTA_ASSERT_INT(2, str_iter->index, info);
    free(result);

    // Continue again. No more words.
    result = next_word(str_iter);
    CINTA_ASSERT_STRING("", result, info);
    CINTA_ASSERT_INT(2, str_iter->index, info);
    free(str_iter);

    // 3/2 times separator
    str_iter = new_string_iterator("###word", "##");
    result = next_word(str_iter);
    CINTA_ASSERT_STRING("#word", result, info);
    free(str_iter);
    free(result);
}

static void test_case_split_string(test_info *info) {
    char **result;
    size_t size;

    // Empty string
    result = split_string("", " ", &size);
    CINTA_ASSERT_INT(0, size, info);
    CINTA_ASSERT_NULL(result, info);

    // Only separator
    result = split_string("    ", " ", &size);
    CINTA_ASSERT_INT(0, size, info);
    CINTA_ASSERT_NULL(result, info);

    // No separator
    result = split_string("word", " ", &size);
    CINTA_ASSERT_INT(1, size, info);
    CINTA_ASSERT_NOT_NULL(result, info);
    CINTA_ASSERT_STRING("word", result[0], info);
    free(result[0]);
    free(result);

    // Simple sentence
    result = split_string("this is a sentence", " ", &size);
    CINTA_ASSERT_INT(4, size, info);
    char *expected_1[4] = {"this", "is", "a", "sentence"};
    for (size_t index = 0; index < size; ++index) {
        CINTA_ASSERT_STRING(expected_1[index], result[index], info);
        free(result[index]);
    }
    free(result);

    // Repeating separator
    result = split_string("this____is____also___a____sentence____", "_", &size);
    CINTA_ASSERT_INT(5, size, info);
    char *expected_2[5] = {"this", "is", "also", "a", "sentence"};
    for (size_t index = 0; index < size; ++index) {
        CINTA_ASSERT_STRING(expected_2[index], result[index], info);
        free(result[index]);
    }
    free(result);

    // 3/2 times separator
    result = split_string("...O.M.G.....I.think...this.....is.....working now!!...", "..", &size);
    CINTA_ASSERT_INT(6, size, info);
    char *expected_3[6] = {".O.M.G", ".I.think", ".this", ".is", ".working now!!", "."};
    for (size_t index = 0; index < size; ++index) {
        CINTA_ASSERT_STRING(expected_3[index], result[index], info);
        free(result[index]);
    }
    free(result);

    // Trimming needed
    result = split_string("-----this-is-also-a-sentence------", "-", &size);
    CINTA_ASSERT_INT(5, size, info);
    char *expected_4[5] = {"this", "is", "also", "a", "sentence"};
    for (size_t index = 0; index < size; ++index) {
        CINTA_ASSERT_STRING(expected_4[index], result[index], info);
        free(result[index]);
    }
    free(result);
}

static void test_case_join_string(test_info *info) {
    char **splitted;
    char *result;
    size_t size;

    // Empty string
    splitted = split_string("", "_", &size);
    result = join_strings(splitted, size, " ");
    CINTA_ASSERT_STRING("", result, info);
    free(splitted);

    // Simple sentence
    splitted = split_string("this is a really complex sentence.", " ", &size);
    result = join_strings(splitted, size, "-");
    CINTA_ASSERT_STRING("this-is-a-really-complex-sentence.", result, info);
    for (size_t index = 0; index < size; ++index) {
        free(splitted[index]);
    }
    free(splitted);
    free(result);

    // Trimming needed
    splitted = split_string("______with______some____spaces_____", "_", &size);
    result = join_strings(splitted, size, "@");
    CINTA_ASSERT_STRING("with@some@spaces", result, info);
    for (size_t index = 0; index < size; ++index) {
        free(splitted[index]);
    }
    free(splitted);
    free(result);

    // Multiple characters separator
    splitted = split_string("I  do not have    enough   inspiration ...", " ", &size);
    result = join_strings(splitted, size, "][");
    CINTA_ASSERT_STRING("I][do][not][have][enough][inspiration][...", result, info);
    for (size_t index = 0; index < size; ++index) {
        free(splitted[index]);
    }
    free(splitted);
    free(result);

    // Single word due to an absent separator
    splitted = split_string(" [ @  Single word  @  ]  ", "%", &size);
    result = join_strings(splitted, size, "+");
    CINTA_ASSERT_STRING(" [ @  Single word  @  ]  ", result, info);
    for (size_t index = 0; index < size; ++index) {
        free(splitted[index]);
    }
    free(splitted);
    free(result);
}

static void test_case_trunc_start(test_info *info) {
    char *result;

    // NULL string
    result = trunc_start(NULL, 20);
    CINTA_ASSERT_NULL(result, info);

    // Empty string
    result = trunc_start("", 20);
    CINTA_ASSERT_STRING("", result, info);
    free(result);

    // 0 size
    result = trunc_start("balabala beuleubeuleu", 0);
    CINTA_ASSERT_STRING("", result, info);

    // Too long size
    result = trunc_start("Hellooo", 20);
    CINTA_ASSERT_STRING("Hellooo", result, info);
    free(result);

    // Empty string and 0 size
    result = trunc_start("", 0);
    CINTA_ASSERT_STRING("", result, info);

    // Size = string length
    result = trunc_start("123456", 6);
    CINTA_ASSERT_STRING("123456", result, info);
    free(result);

    // Classic sentences
    result = trunc_start("Pacman eats ghosts", 6);
    CINTA_ASSERT_STRING("ghosts", result, info);
    free(result);

    result = trunc_start("9876543210", 2);
    CINTA_ASSERT_STRING("10", result, info);
    free(result);
}

static void test_case_split_string_keep_trace(test_info *info) {
    char **result;
    size_t size;
    int last_index;

    // Empty string
    result = split_string_keep_trace("", "&", &size, &last_index);
    CINTA_ASSERT_INT(0, size, info);
    CINTA_ASSERT_INT(-1, last_index, info);
    free(result);

    // Only separator
    result = split_string_keep_trace("&&&&", "&", &size, &last_index);
    CINTA_ASSERT_INT(0, size, info);
    CINTA_ASSERT_INT(-1, last_index, info);
    free(result);

    // Single word
    result = split_string_keep_trace("word", "&", &size, &last_index);
    CINTA_ASSERT_INT(1, size, info);
    CINTA_ASSERT_INT(-1, last_index, info);
    free(result[0]);
    free(result);
    result = split_string_keep_trace("word&", "&", &size, &last_index);
    CINTA_ASSERT_INT(1, size, info);
    CINTA_ASSERT_INT(0, last_index, info);
    free(result[0]);
    free(result);

    // Multiple words
    result = split_string_keep_trace("wordword", "&", &size, &last_index);
    CINTA_ASSERT_INT(1, size, info);
    CINTA_ASSERT_INT(-1, last_index, info);
    free(result[0]);
    free(result);
    result = split_string_keep_trace("word1 & word2", "&", &size, &last_index);
    CINTA_ASSERT_INT(2, size, info);
    CINTA_ASSERT_INT(0, last_index, info);
    free(result[0]);
    free(result[1]);
    free(result);
    result = split_string_keep_trace("word1& word2&", "&", &size, &last_index);
    CINTA_ASSERT_INT(2, size, info);
    CINTA_ASSERT_INT(1, last_index, info);
    free(result[0]);
    free(result[1]);
    free(result);

    // Multiple words | Repeating separator
    result = split_string_keep_trace("word1&& word2", "&", &size, &last_index);
    CINTA_ASSERT_INT(2, size, info);
    CINTA_ASSERT_INT(0, last_index, info);
    free(result[0]);
    free(result[1]);
    free(result);
    result = split_string_keep_trace("word1&& word2&", "&", &size, &last_index);
    CINTA_ASSERT_INT(2, size, info);
    CINTA_ASSERT_INT(1, last_index, info);
    free(result[0]);
    free(result[1]);
    free(result);

    // A little bit longer
    result = split_string_keep_trace("a &&& b  &  c  &&&& d && e", "&", &size, &last_index);
    CINTA_ASSERT_INT(5, size, info);
    CINTA_ASSERT_INT(3, last_index, info);
    for (size_t index = 0; index < size; ++index) {
        free(result[index]);
    }
    free(result);
}

void test_case_null_trim_spaces(test_info *info) {
    char *result;

    result = trim_spaces(NULL);
    CINTA_ASSERT_NULL(result, info);
}

void test_case_empty_trim_spaces(test_info *info) {
    char *result;

    result = trim_spaces("");
    CINTA_ASSERT_STRING("", result, info);
    free(result);
}

void test_case_only_whitespace_trim_spaces(test_info *info) {
    char *result;

    result = trim_spaces("    ");
    CINTA_ASSERT_STRING("", result, info);
    free(result);
}

void test_case_nothing_to_trim_trim_spaces(test_info *info) {
    char *result;

    result = trim_spaces("aaaaa");
    CINTA_ASSERT_STRING("aaaaa", result, info);
    free(result);

    result = trim_spaces("a   a");
    CINTA_ASSERT_STRING("a   a", result, info);
    free(result);
}

void test_case_trim_start_trim_spaces(test_info *info) {
    char *result;

    result = trim_spaces(" a");
    CINTA_ASSERT_STRING("a", result, info);
    free(result);

    result = trim_spaces("      a");
    CINTA_ASSERT_STRING("a", result, info);
    free(result);

    result = trim_spaces("   a      a");
    CINTA_ASSERT_STRING("a      a", result, info);
    free(result);
}

void test_case_trim_end_trim_spaces(test_info *info) {
    char *result;

    result = trim_spaces("a ");
    CINTA_ASSERT_STRING("a", result, info);
    free(result);

    result = trim_spaces("a      ");
    CINTA_ASSERT_STRING("a", result, info);
    free(result);

    result = trim_spaces("a  a    ");
    CINTA_ASSERT_STRING("a  a", result, info);
    free(result);
}

void test_case_trim_both_trim_spaces(test_info *info) {
    char *result;

    result = trim_spaces(" a ");
    CINTA_ASSERT_STRING("a", result, info);
    free(result);

    result = trim_spaces("    a    ");
    CINTA_ASSERT_STRING("a", result, info);
    free(result);

    result = trim_spaces("   a a  ");
    CINTA_ASSERT_STRING("a a", result, info);
    free(result);
}

void test_case_string_made_of(test_info *info) {
    int result;

    result = is_only_composed_of("      ", " ");
    CINTA_ASSERT_INT(1, result, info);

    result = is_only_composed_of("", "a");
    CINTA_ASSERT_INT(1, result, info);

    result = is_only_composed_of("aaaaaaa", "a");
    CINTA_ASSERT_INT(1, result, info);

    result = is_only_composed_of("@@@", "@@");
    CINTA_ASSERT_INT(0, result, info);

    result = is_only_composed_of(" ", "a");
    CINTA_ASSERT_INT(0, result, info);

    result = is_only_composed_of("aaaabaa", "a");
    CINTA_ASSERT_INT(0, result, info);

    result = is_only_composed_of("baaaaaa", "a");
    CINTA_ASSERT_INT(0, result, info);

    result = is_only_composed_of("aaaaaab", "a");
    CINTA_ASSERT_INT(0, result, info);

    result = is_only_composed_of("aaaaaa", "a");
    CINTA_ASSERT_INT(1, result, info);

    result = is_only_composed_of("abababab", "ab");
    CINTA_ASSERT_INT(1, result, info);

    result = is_only_composed_of("ababababa", "ab");
    CINTA_ASSERT_INT(0, result, info);
}

void test_case_repeat(test_info *info) {
    char *result;

    result = repeat(" ", 0);
    CINTA_ASSERT_STRING("", result, info);
    free(result);

    result = repeat("a", 0);
    CINTA_ASSERT_STRING("", result, info);
    free(result);

    result = repeat("azerfgbhn", 0);
    CINTA_ASSERT_STRING("", result, info);
    free(result);

    result = repeat(" ", 1);
    CINTA_ASSERT_STRING(" ", result, info);
    free(result);

    result = repeat("a", 1);
    CINTA_ASSERT_STRING("a", result, info);
    free(result);

    result = repeat("azerfgbhn", 1);
    CINTA_ASSERT_STRING("azerfgbhn", result, info);
    free(result);

    result = repeat(" ", 3);
    CINTA_ASSERT_STRING("   ", result, info);
    free(result);

    result = repeat("a", 4);
    CINTA_ASSERT_STRING("aaaa", result, info);
    free(result);

    result = repeat("@#{", 6);
    CINTA_ASSERT_STRING("@#{@#{@#{@#{@#{@#{", result, info);
    free(result);
}
