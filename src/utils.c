#include "utils.h"
#include "errno.h"
#include "internals.h"
#include <linux/limits.h>

char *get_cwd_raw() {

    char *cwd = getcwd(NULL, PATH_MAX);
    return cwd;
}

char *get_cwd_trunc_start(size_t size_limit) {
    char *cwd = get_cwd_raw();

    if (size_limit >= strlen(cwd) || cwd == NULL) {
        return cwd;
    }

    if (size_limit == 0) {
        free(cwd);
        return "";
    }

    char *trunc_cwd = malloc(size_limit * sizeof(char));
    memmove(trunc_cwd, cwd + strlen(cwd) - size_limit, strlen(cwd) - size_limit);
    free(cwd);
    return trunc_cwd;
}
