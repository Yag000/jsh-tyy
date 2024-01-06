#ifndef STRING_UTILS_H
#define STRING_UTILS_H

/** Struct that represents an iterator over a string.
 * It iterates over the words in the string, separated by the separator.
 */
#include <inttypes.h>
#include <stddef.h>

typedef struct string_iterator {
    char *string;
    const char *separator;
    int index;
} string_iterator;

/** Returns a new string iterator for the given string and separator. */
string_iterator *new_string_iterator(char *, const char *);

/** Frees the memory allocated for the string iterator. */
void destroy_string_iterator(string_iterator *);

/** Trims start of string iterator from separator. */
void trim_start(string_iterator *);

/** Returns 1 if the first string starts with the prefix, 0 otherwise. */
int starts_with(char *, const char *prefix);

/** Returns 0 if there are no more words in the string, 1 otherwise. */
int has_next_word(string_iterator *);

/** Returns the next word in the string.
 *  Moreover, it moves the pointer of iterator->string to the index just after
 *  the next found word. If the iterator does not contain any next word, it
 *  returns an empty string.
 */
char *next_word(string_iterator *);

/** Returns the number of words left in the string. */
int get_number_of_words_left(const string_iterator *);

/** Returns a new array of strings obtained by splitting the string on the
 * separator. Set its length to size.
 */
char **split_string(char *, const char *, size_t *);

/**
 * Splits a string by a delimiter. Returns an array of string and sets `size` to it's length.
 * `last` will be set to the index of the last item that was preceded by the delimiter. In particular,
 * if the string ends by the delimiter then `last` = `size - 1`. If no delimiter is present,
 * `last` is set to `-1`.
 */
char **split_string_keep_trace(char *string, const char *delimiter, size_t *size, int *last);

/** Returns a new string that is the concatenation of the strings in the array
 * separated by the separator.
 */
char *join_strings(char **, size_t, char *);

/** Keep the last `to_keep` characters from a string. */
char *trunc_start(char *str, size_t to_keep);

/** Returns 1 if the array of size `size` contains `word`, 0 otherwise. */
int contains_string(const char *array[], size_t size, char *word);

/**
 * Returns a new string without the leading and trailing spaces.
 * If the string is only made of spaces, returns an empty string.
 * If the string is NULL, returns NULL.
 * If the string is empty, returns an empty string.
 */
char *trim_spaces(const char *str);

/**
 * Parses an integer from a string.
 * Stores the result in `res`.
 * Returns 0 on errors and prints the error message to `fd`.
 * Returns 1 on success.
 */
int parse_intmax_t(char *string, intmax_t *res, int fd);

#endif // STRING_UTILS_H
