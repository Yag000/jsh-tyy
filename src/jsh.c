#include "internals.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc > 1) {
        printf("Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    init_internals();

    return EXIT_SUCCESS;
}
