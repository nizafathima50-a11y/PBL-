#ifndef DEADLOCK_H
#define DEADLOCK_H

#include "scheduler.h"

/*
 * Legacy function name: this performs a Need-based completion-feasibility
 * check, not runtime deadlock detection. The array marks processes that
 * cannot be shown to finish from the current Available resources.
 */
int detect_deadlock(
    SystemState *state,
    bool deadlocked[MAX_PROCESSES]
);

/* Simulates releasing resources from a process that cannot be shown to finish. */
bool recover_from_deadlock(
    SystemState *state,
    int process_id
);

#endif
