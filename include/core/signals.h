#ifndef SIGNALS
#define SIGNALS


#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

int init_signal_handlers(void);

#endif