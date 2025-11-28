#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>

typedef struct SignalOps {
    volatile sig_atomic_t *flag;
    int (*install)(void);
} SignalOps;

SignalOps create_signal_ops(void);

#endif
