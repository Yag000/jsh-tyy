#include "utils.h"
#include "string_utils.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *get_current_wd() {
    char *cwd = getcwd(NULL, PATH_MAX);
    if (cwd == NULL) {
        perror("getcwd");
        return "";
    }
    return cwd;
}

char *truncated_cwd(size_t size_limit) {
    char *cwd = get_current_wd();
    char *trunc_cwd = trunc_start(cwd, size_limit);
    if (strcmp(cwd, "") != 0) {
        free(cwd);
    }
    return trunc_cwd;
}

int add_set(int *set, size_t size, int value) {

    for (size_t index = 0; index < size; ++index) {
        if (set[index] < 0) {
            set[index] = value;
            return 0;
        }
        if (set[index] == value) {
            return -1;
        }
    }
    return -1;
}

int remove_set(int *set, size_t size, int value) {
    for (size_t index = 0; index < size; ++index) {
        if (set[index] == value) {
            set[index] = -1;
            return 0;
        }
    }
    return -1;
}

int contains(int *set, size_t size, int value) {
    for (size_t index = 0; index < size; ++index) {
        if (set[index] == value) {
            return 1;
        }
    }
    return 0;
}

int contains2(int **array, size_t size, size_t subsize, int value) {
    for (size_t index = 0; index < size; ++index) {
        for (size_t j = 0; j < subsize; j++) {

            if (array[index][j] == value) {
                return 1;
            }
        }
    }
    return 0;
}

char *colored(char *color, char *string) {

    if (color == NULL || string == NULL) {
        return NULL;
    }

    char *start = "\001";
    char *end = "\002";
    char *remove_color = "\033[0m";
    size_t colored_len = strlen(start) + strlen(color) + strlen(end) + strlen(string) + strlen(remove_color) + 1;
    char *colored = malloc(colored_len * sizeof(char));

    if (colored == NULL) {
        perror("malloc(): colored");
        return string;
    }
    sprintf(colored, "%s%s%s%s%s", start, color, end, string, remove_color);
    return colored;
}

char *in_white(char *string) {
    return colored("\033[0m", string);
}

char *in_red(char *string) {
    return colored("\033[0;31m", string);
}

char *in_green(char *string) {
    return colored("\033[0;32m", string);
}

char *in_yellow(char *string) {
    return colored("\033[0;33m", string);
}

char *in_blue(char *string) {
    return colored("\033[0;34m", string);
}

char *in_purple(char *string) {
    return colored("\033[0;35m", string);
}

char *in_cyan(char *string) {
    return colored("\033[0;36m", string);
}
