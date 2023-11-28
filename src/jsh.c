#include "internals.h"
#include "prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc > 1) {
        printf("Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    init_internals();
    prompt();

    return exit_code;
}
