#include "signals.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define NB_SIGNALS_TO_IGNORE 6
const int signals_to_ignore[NB_SIGNALS_TO_IGNORE] = {SIGINT, SIGTERM, SIGTTIN, SIGQUIT, SIGTTOU, SIGTSTP};

void set_signal_actions(struct sigaction *sa) {
    for (int i = 0; i < NB_SIGNALS_TO_IGNORE; i++) {
        if (sigaction(signals_to_ignore[i], sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }
    }
}

void ignore_signals() {
    struct sigaction sa = {0};
    sa.sa_handler = SIG_IGN;
    set_signal_actions(&sa);
}

void restore_signals() {
    struct sigaction sa = {0};
    sa.sa_handler = SIG_DFL;
    set_signal_actions(&sa);
}
