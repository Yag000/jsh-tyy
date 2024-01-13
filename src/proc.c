#include "proc.h"
#include "string_utils.h"

#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int *get_children(int pid, size_t *size) {

    char path[PATH_MAX];
    *size = 0;

    snprintf(path, PATH_MAX, "/proc/%d/task/%d/children", pid, pid);

    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        return NULL;
    }

    struct stat st;
    if (lstat(path, &st) == -1) {
        perror("lstat");
        return NULL;
    }

    char *buf = malloc((st.st_size + 1) * sizeof(char));
    if (buf == NULL) {
        perror("malloc");
        return NULL;
    }

    if (read(fd, buf, st.st_size) == -1) {
        perror("read");
        return NULL;
    }

    buf[st.st_size] = '\0';

    close(fd);

    char **children_string = split_string(buf, " ", size);

    int *children_pid = malloc(*size * sizeof(int));
    if (children_pid == NULL) {
        perror("malloc");
        return NULL;
    }

    for (size_t index = 0; index < *size; ++index) {

        long cast;
        int parse_status = parse_intmax_t(children_string[index], &cast, STDERR_FILENO);

        if (parse_status && INT_MIN <= cast && cast <= INT_MAX) {
            children_pid[index] = (int)cast;
            continue;
        }

        // Handling invalid parsed value
        *size = 0;
        for (size_t index = 0; index < *size; ++index) {
            free(children_string[index]);
        }
        free(children_string);
        free(children_pid);
        return NULL;
    }

    for (size_t index = 0; index < *size; ++index) {
        free(children_string[index]);
    }
    free(children_string);

    return children_pid;
}

int has_children(pid_t pid) {

    size_t size = 0;
    int *children = get_children(pid, &size);

    if (children == NULL) {
        return -1;
    }

    free(children);

    return size > 0;
}
