#include "command.h"
#include "internals.h"
#include <errno.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bool is_valid_path(char *path, struct stat *p_stat, int call_stderr);
bool is_valid_target_dir(char *target, int call_stderr);
bool set_new_cwd(char *new_cwd, int call_stderr);
bool is_lwd_set();

/**
 * Checks if a given path is valid and updates `p_stat` using `lstat`.
 *
 * @param path
 * @param p_stat A pointer to a `struct stat` object.
 * @param call_stderr the file descriptor of Standard error.
 *
 * @return true if the path is valid; false otherwise.
 */
bool is_valid_path(char *path, struct stat *p_stat, int call_stderr) {
    if ((lstat(path, p_stat)) == -1) {
        switch (errno) {
            case EACCES:
                dprintf(call_stderr, "cd: invalid path: %s no access.\n", path);
                return false;

            case ENOENT:
                dprintf(call_stderr, "cd: invalid path: %s does not exist.\n", path);
                return false;

            default:
                dprintf(call_stderr, "cd: lstat failure. (%s)\n", strerror(errno));
                return false;
        }
    }

    return true;
}

/**
 * Checks if a given path is a valid target directory.
 *
 * @param target
 * @param call_stderr the file descriptor of Standard error.
 *
 * @return true if `target` is a valid path and is a directory;
 *         false otherwise.
 */
bool is_valid_target_dir(char *target, int call_stderr) {
    struct stat p_stat;

    if (!is_valid_path(target, &p_stat, call_stderr)) {
        return false;
    }

    if (!S_ISDIR(p_stat.st_mode)) {
        dprintf(call_stderr, "cd: %s is not a directory.\n", target);
        return false;
    }

    return true;
}

/**
 * Sets a new current working directory and updates the previous working
 * directory.
 *
 * @param new_cwd The future working directory.
 * @param call_stderr the file descriptor of Standard error.
 *
 * @return true if `new_cwd` is a valid target directory and the working
 *         directory is successfully set to `new_cwd`; false otherwise.
 */
bool set_new_cwd(char *new_cwd, int call_stderr) {
    if (!is_valid_target_dir(new_cwd, call_stderr)) {
        return false;
    }

    char *cwd = getcwd(NULL, PATH_MAX);
    if (cwd == NULL) {
        dprintf(call_stderr, "cd: getcwd failure. (%s)\n", strerror(errno));
        return false;
    }

    // Set new previous working directory
    memset(lwd, '\0', PATH_MAX);
    memmove(lwd, cwd, strlen(cwd));
    free(cwd);

    // Set new working directory
    if (chdir(new_cwd) == -1) {
        dprintf(call_stderr, "cd: chdir failure. (%s)\n", strerror(errno));
        return false;
    }

    return true;
}

bool is_lwd_set() {
    for (size_t i = 0; i < PATH_MAX; ++i) {
        if (lwd[i] != '\0') {
            return true;
        }
    }

    return false;
}

int cd(command_call *command_call) {
    char *path;
    bool res;

    // Go to $HOME
    if (command_call->argc == 1) {
        char *env_home;
        if ((env_home = getenv("HOME")) == NULL) {
            dprintf(command_call->stderr, "cd: HOME environment variable is not defined.\n");
            return 1;
        }

        path = strdup(env_home);
        if (path == NULL) {
            dprintf(command_call->stderr, "cd: strdup failure. (%s)\n", strerror(errno));
            return 1;
        }

        goto valid_path;
    }

    // Go back to previous working directory
    if (command_call->argc == 2 && strcmp(command_call->argv[1], "-") == 0) {
        if (is_lwd_set()) {
            path = strdup(lwd);
            if (path == NULL) {
                dprintf(command_call->stderr, "cd: strdup failure. (%s)\n", strerror(errno));
                return 1;
            }
        } else {
            path = getcwd(NULL, PATH_MAX);
            if (path == NULL) {
                dprintf(command_call->stderr, "cd: getcwd failure. (%s)\n", strerror(errno));
                return 1;
            }
        }

        goto valid_path;
    }

    // Go to ref
    if (command_call->argc == 2) {
        path = strdup(command_call->argv[1]);
        if (path == NULL) {
            dprintf(command_call->stderr, "cd: strdup failure. (%s)\n", strerror(errno));
            return 1;
        }

        goto valid_path;
    }

    dprintf(command_call->stderr, "cd: incorrect usage of cd command.\n");
    dprintf(command_call->stderr, "cd: correct usage: cd [directory|-]\n");

    perror("cd");
    return 1;

valid_path:
    res = set_new_cwd(path, command_call->stderr);
    free(path);
    return res ? 0 : 1;
}