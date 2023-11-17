#include "string_utils.h"
#include <stdlib.h>

/** Returns a new string iterator for the given string and separator. */
string_iterator *new_string_iterator(char *string, char *separator) {
    string_iterator *iterator = malloc(sizeof(string_iterator));
    iterator->string = string;
    iterator->separator = separator;
    iterator->index = 0;
    return iterator;
}

/** Frees the memory allocated for the string iterator. */
void destroy_string_iterator(string_iterator *iterator) {
    free(iterator);
}

/** Returns 0 if there are no more words in the string, 1 otherwise. */
int has_next_word(const string_iterator *iterator) {
    // TODO: Implement this function.
    return 0;
}

/** Returns the next word in the string. */
char *next_word(string_iterator *iterator) {
    // TODO: Implement this function.
    return NULL;
}

/** Returns the number of words left in the string. */
int get_number_of_words_left(const string_iterator *iterator) {
    // TODO: Implement this function.
    return 0;
}

char **split_string(char *string, char *delimiter, size_t *size) {
    string_iterator *iterator = new_string_iterator(string, delimiter);
    int number_of_tokens = get_number_of_words_left(iterator);
    *size = number_of_tokens;

    char **tokens = malloc(sizeof(char *) * number_of_tokens);

    while (has_next_word(iterator)) {
        tokens[iterator->index] = next_word(iterator);
    }

    destroy_string_iterator(iterator);

    return tokens;
}
