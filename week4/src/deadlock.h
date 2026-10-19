#ifndef DEADLOCK_H
#define DEADLOCK_H

#include "scheduler.h"

int detect_deadlock(
    SystemState *state,
    bool deadlocked[MAX_PROCESSES]
);

bool recover_from_deadlock(
    SystemState *state,
    int process_id
);

#endif
