#include "signals.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>

#undef ARRAY_LEN
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

static const int TERMINATION_SIGNALS[] = {SIGINT, SIGTERM};
static const int IGNORE_SIGNALS[] = {SIGPIPE};

static volatile sig_atomic_t internal_flag = 1;

static void handle_signal(int signum) {
    size_t i = 0;
    while (i < ARRAY_LEN(IGNORE_SIGNALS)) {
        if (IGNORE_SIGNALS[i] == signum) {
            return;
        }
        i++;
    }

    i = 0;
    while (i < ARRAY_LEN(TERMINATION_SIGNALS)) {
        if (TERMINATION_SIGNALS[i] == signum) {
            internal_flag = 0;
            break;
        }
        i++;
    }
}

static int install_signal_list(const int *signals, size_t count, struct sigaction *action) {
    size_t i = 0;
    while (i < count) {
        if (sigaction(signals[i], action, NULL) != 0) {
            perror("sigaction");
            return -1;
        }
        i++;
    }
    return 0;
}

static int install_termination_handlers(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_handler = handle_signal;
    // Avoid SA_RESTART so accept() returns EINTR and shutdown can proceed.
    action.sa_flags = 0;

    if (install_signal_list(TERMINATION_SIGNALS, ARRAY_LEN(TERMINATION_SIGNALS), &action) != 0) {
        return -1;
    }

    if (install_signal_list(IGNORE_SIGNALS, ARRAY_LEN(IGNORE_SIGNALS), &action) != 0) {
        return -1;
    }

    return 0;
}

SignalOps create_signal_ops(void) {
    SignalOps ops = {
        .flag = &internal_flag,
        .install = install_termination_handlers,
    };
    return ops;
}

#undef ARRAY_LEN
