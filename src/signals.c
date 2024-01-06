#include "signals.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

sigset_t original_mask;

void safe_sigaddset(sigset_t *set, int sig) {
    if (sigaddset(set, sig) == -1) {
        perror("sigaddset");
        exit(1);
    }
}

void ignore_signals() {
    sigset_t block_mask;
    if (sigemptyset(&block_mask) == -1) {
        perror("sigemptyset");
        exit(1);
    }

    safe_sigaddset(&block_mask, SIGINT);
    safe_sigaddset(&block_mask, SIGTERM);
    safe_sigaddset(&block_mask, SIGTTIN);
    safe_sigaddset(&block_mask, SIGQUIT);
    safe_sigaddset(&block_mask, SIGTTOU);
    safe_sigaddset(&block_mask, SIGTSTP);

    /*
        NOTE:
        This might not be a good idea if we will need to wait for a signal to be
        delivered as this does not guarantee atomicity.
        If we need to wait atomically, see sigsuspend.
    */
    // Modify the signal mask to block every signal in block_mask
    if (sigprocmask(SIG_BLOCK, &block_mask, &original_mask) == -1) {
        perror("sigprocmask");
        exit(1);
    }
}

void restore_signals() {
    if (sigprocmask(SIG_SETMASK, &original_mask, NULL) == -1) {
        perror("sigprocmask");
        exit(1);
    }
}
