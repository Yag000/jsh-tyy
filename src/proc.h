#ifndef PROC_H
#define PROC_H

#include <sys/types.h>

/**
 * If the process exists, it returns an array with the pids of the
 * children of the process and size is then set to the size of the array.
 * size is then set to the size of the array.
 * Otherwise, returns NULL, and size is set to 0.
 */
int *get_children(pid_t pid, size_t *size);

/**
 * Returns
 *   *  1 if process exists and has at least one child.
 *   *  0 if it exists but has no child.
 *   * -1 if the process does not exist.
 */
int has_children(pid_t pid);

#endif // PROC_H
