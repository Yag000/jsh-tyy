#include "internals.h"
#include "jobs.h"
#include "prompt.h"
#include "signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc > 1) {
        printf("Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    ignore_signals();

    init_internals();
    prompt();

    destroy_job_table();
    return last_exit_code;
}
