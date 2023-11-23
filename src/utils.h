#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/** Returns the result of 'getcwd' as it is. */
char *get_cwd_raw();

/** Returns the result of 'getcwd' keeping the last
 *  `size_limit` characters of the path.
 */
char *get_cwd_trunc_start(size_t size_limit);

#endif // UTILS_H
