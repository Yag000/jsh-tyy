#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/** Returns the result of 'getcwd' as it is. */
char *get_current_wd();

/** Returns the result of 'getcwd' keeping the last
 *  `size_limit` characters of the path.
 */
char *truncated_cwd(size_t size_limit);

/** Add a value to a set of determined size.
 * An empty spot needs to be marked with a negative value.
 * Returns 0 if `value` was correctly added, -1 otherwise.
 */
int add_set(int *set, size_t size, int value);

/** Returns 1 if the set contains the value, 0 otherwise. */
int contains(int *set, size_t size, int value);

/** Removes `value` from the set.
 * Returns 0 if `value` was properly removed, -1 otherwise.
 */
int remove_set(int *set, size_t size, int value);

/**
 * Returns a new string starting with `\001` following by a color tag,
 * ending with `\002` and contains in its center the string passed in
 * argument.
 */
char *colored(char *color, char *string);
/**
 * Returns the string passed in argument colored.
 * The color only affects the string and not what could
 * be next to it.
 */
char *in_white(char *string);
char *in_red(char *string);
char *in_green(char *string);
char *in_yellow(char *string);
char *in_blue(char *string);
char *in_purple(char *string);
char *in_cyan(char *string);

#endif // UTILS_H
