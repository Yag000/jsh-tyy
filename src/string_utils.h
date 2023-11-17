#ifndef STRING_UTILS_H
#define STRING_UTILS_H

/** Struct that represents an iterator over a string.
 * It iterates over the words in the string, separated by the separator.
 */
#include <stddef.h>
typedef struct string_iterator {
    char *string;
    char *separator;
    int index;
} string_iterator;

/** Returns a new string iterator for the given string and separator. */
string_iterator *new_string_iterator(char *, char *);

/** Frees the memory allocated for the string iterator. */
void destroy_string_iterator(string_iterator *);

/** Returns 0 if there are no more words in the string, 1 otherwise. */
int has_next_word(const string_iterator *);

/** Returns the next word in the string. */
char *next_word(string_iterator *);

/** Returns the number of words left in the string. */
int get_number_of_words_left(const string_iterator *);

/** Returns a new array of strings obtained by splitting the string on the
 * separator.
 */
char **split_string(char *, char *, size_t *);

/** Returns a new string that is the concatenation of the strings in the array
 * separated by the separator.
 */
char *join_strings(char **, char *);

/** Removes any whitespace character from the beginning and end of the string. */
void trim_string(char *);

#endif // STRING_UTILS_H
