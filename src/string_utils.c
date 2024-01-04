#include "string_utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Returns a new string iterator for the given string and separator.
 *  A NULL value is returned if the separator is an empty string.
 */
string_iterator *new_string_iterator(char *string, const char *separator) {

    if (strcmp(separator, "") == 0) {
        return NULL;
    }

    string_iterator *iterator = malloc(sizeof(string_iterator));
    if (iterator == NULL) {
        perror("malloc");
        return NULL;
    }

    iterator->string = string;
    iterator->separator = separator;
    iterator->index = 0;
    return iterator;
}

/** Frees the memory allocated for the string iterator. */
void destroy_string_iterator(string_iterator *iterator) {
    free(iterator);
}

/** Returns iterator's string trimmed with its separator. */
void trim_start(string_iterator *iterator) {
    while (starts_with(iterator->string, iterator->separator)) {
        iterator->string += strlen(iterator->separator);
    }
}

/** Checks if a string starts with a special pattern. */
int starts_with(char *string, const char *start) {
    return strncmp(string, start, strlen(start)) == 0 ? 1 : 0;
}

/** Returns 0 if there are no more words in the string, 1 otherwise. */
int has_next_word(string_iterator *iterator) {
    trim_start(iterator);
    return strlen(iterator->string) > 0;
}

/** Returns the next word in the string.
 *  Moreover, it moves the pointer of iterator->string to the index just after
 *  the next found word. If the iterator does not contain any next word, it
 *  returns an empty string.
 */
char *next_word(string_iterator *iterator) {
    size_t current_word_size = 0;

    trim_start(iterator);

    if (!has_next_word(iterator)) {
        return "";
    }

    while (current_word_size < strlen(iterator->string) &&
           !starts_with(iterator->string + current_word_size, iterator->separator)) {
        ++current_word_size;
    }

    char *current_word = malloc((current_word_size + 1) * sizeof(char));
    if (current_word == NULL) {
        perror("malloc");
        return NULL;
    }

    memcpy(current_word, iterator->string, current_word_size);
    current_word[current_word_size] = '\0';

    iterator->string += current_word_size;
    iterator->index += 1;

    return current_word;
}

/** Returns the number of words left in the string. */
int get_number_of_words_left(const string_iterator *iterator) {
    unsigned int word = 0;
    unsigned int total_words = 0;

    for (size_t index = 0; index < strlen(iterator->string); ++index) {
        int is_delimiter = starts_with(iterator->string + index, iterator->separator);
        if (word) {
            if (is_delimiter) {
                word = 0;
                total_words += 1;
                index += strlen(iterator->separator) - 1;
            }
        } else {
            if (!is_delimiter) {
                word = 1;
            } else {
                index += strlen(iterator->separator) - 1;
            }
        }
    }
    return total_words + word;
}

/** Splits a string by a delimiter. Returns an array of string and set its length to size. */
char **split_string(char *string, const char *delimiter, size_t *size) {
    string_iterator *iterator = new_string_iterator(string, delimiter);
    int number_of_tokens = get_number_of_words_left(iterator);
    *size = number_of_tokens;

    if (number_of_tokens == 0) {
        destroy_string_iterator(iterator);
        return NULL;
    }

    char **tokens = malloc(sizeof(char *) * number_of_tokens);
    if (tokens == NULL) {
        perror("malloc");
        return NULL;
    }

    while (has_next_word(iterator)) {
        tokens[iterator->index] = next_word(iterator);
    }

    destroy_string_iterator(iterator);

    return tokens;
}

char **split_string_keep_trace(char *string, const char *delimiter, size_t *size, int *last) {
    string_iterator *iterator = new_string_iterator(string, delimiter);
    int number_of_tokens = get_number_of_words_left(iterator);
    *size = number_of_tokens;
    *last = -1;

    if (number_of_tokens == 0) {
        destroy_string_iterator(iterator);
        return NULL;
    }

    char **tokens = malloc(sizeof(char *) * number_of_tokens);
    if (tokens == NULL) {
        perror("malloc");
        return NULL;
    }

    while (has_next_word(iterator)) {
        tokens[iterator->index] = next_word(iterator);
        *last += starts_with(iterator->string, delimiter);
    }

    destroy_string_iterator(iterator);

    return tokens;
}

/** Returns a new string that is the concatenation of the strings in the array
 * separated by the separator.
 */
char *join_strings(char **tokens, size_t number_of_tokens, char *sep) {
    if (number_of_tokens == 0) {
        return "";
    }

    size_t total_size = 1 + (number_of_tokens - 1) * strlen(sep);

    for (size_t index = 0; index < number_of_tokens; ++index) {
        total_size += strlen(tokens[index]);
    }

    char *joined_string = malloc(total_size * sizeof(char));
    if (joined_string == NULL) {
        perror("malloc");
        return "";
    }
    joined_string[0] = '\0';

    for (size_t index = 0; index < number_of_tokens; ++index) {
        memcpy(joined_string + strlen(joined_string), tokens[index], strlen(tokens[index]) + 1);
        if (index != number_of_tokens - 1) {
            memcpy(joined_string + strlen(joined_string), sep, strlen(sep) + 1);
        }
    }
    return joined_string;
}

char *trunc_start(char *str, size_t to_keep) {
    if (str == NULL) {
        return str;
    }

    if (to_keep == 0) {
        return "";
    }

    char *truncated = malloc((to_keep + 1) * sizeof(char));

    if (truncated == NULL) {
        perror("malloc(): trunc string at start");
        return "";
    }

    if (to_keep >= strlen(str)) {
        memmove(truncated, str, to_keep + 1);
        truncated[to_keep] = '\0';
        return truncated;
    }

    memmove(truncated, str + strlen(str) - to_keep, to_keep + 1);
    truncated[to_keep] = '\0';

    return truncated;
}

int contains_string(const char *array[], size_t size, char *word) {
    for (size_t index = 0; index < size; ++index) {
        if (strcmp(array[index], word) == 0) {
            return 1;
        }
    }
    return 0;
}

char *trim_spaces(const char *str) {
    if (str == NULL) {
        return NULL;
    }

    if (strlen(str) == 0) {
        char *empty = malloc(sizeof(char));
        if (empty == NULL) {
            perror("malloc");
            return NULL;
        }

        empty[0] = '\0';
        return empty;
    }

    size_t trim_start_len = 0;
    while (str[trim_start_len] == ' ') {
        trim_start_len += 1;
    }

    size_t trim_end_len = 0;
    while (str[strlen(str) - 1 - trim_end_len] == ' ') {
        trim_end_len += 1;
    }

    if (trim_start_len == 0 && trim_end_len == 0) {
        char *dup = strdup(str);
        if (dup == NULL) {
            perror("strdup");
            return NULL;
        }
        return dup;
    }

    if (trim_start_len + trim_end_len >= strlen(str)) {
        char *empty = malloc(sizeof(char));
        if (empty == NULL) {
            perror("malloc");
            return NULL;
        }

        empty[0] = '\0';
        return empty;
    }

    size_t new_len = strlen(str) - trim_start_len - trim_end_len;

    char *trimmed = malloc((new_len + 1) * sizeof(char));
    if (trimmed == NULL) {
        perror("malloc");
        return NULL;
    }

    memmove(trimmed, str + trim_start_len, new_len);
    trimmed[new_len] = '\0';

    return trimmed;
}
