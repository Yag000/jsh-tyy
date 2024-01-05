#include "signals.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void ignore_signals() {
    // TODO: Ignore every shell sigaction
    struct sigaction sa = {0};
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGTTOU, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGTTIN, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

void restore_signals() {
    // TODO: Restore every sigaction
    struct sigaction sa = {0};
    sa.sa_handler = SIG_DFL;
    if (sigaction(SIGTTOU, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    if (sigaction(SIGTTIN, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}
