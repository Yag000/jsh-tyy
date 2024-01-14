#include "proc.h"
#include "string_utils.h"

#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int *get_children(int pid, size_t *size) {

    char path[PATH_MAX] = "";
    *size = 0;

    snprintf(path, PATH_MAX, "/proc/%d/task/%d/children", pid, pid);

    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        return NULL;
    }

    char buf[BUFSIZ] = "";
    size_t total_size = 0;
    char *string = malloc(sizeof(char));
    string[0] = '\0';

    if (string == NULL) {
        perror("malloc");
        return NULL;
    }

    int nb_read = 1;
    while (nb_read) {
        nb_read = read(fd, buf, BUFSIZ);

        if (nb_read < 0) {
            perror("read");
            free(string);
            close(fd);
            return NULL;
        }

        char *new_string = malloc((total_size + nb_read + 1) * sizeof(char));

        if (new_string == NULL) {
            perror("malloc");
            free(string);
            close(fd);
            return NULL;
        }

        snprintf(new_string, total_size + nb_read, "%s%s", string, buf);
        total_size += nb_read;

        free(string);
        string = new_string;
    }
    string[total_size] = '\0';

    close(fd);

    char **children_string = split_string(string, " ", size);
    free(string);

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
