
#include "deadlock.h"

#include <stddef.h>

/* Validate the resource totals and allocation/maximum matrices. */
static bool valid_state(const SystemState *state)
{
    if (state == NULL ||
        state->process_count < 1 ||
        state->process_count > MAX_PROCESSES ||
        state->resource_count < 1 ||
        state->resource_count > MAX_RESOURCES) {
        return false;
    }

    int allocated[MAX_RESOURCES] = {0};

    for (int j = 0; j < state->resource_count; j++) {
        if (state->total[j] < 0) {
            return false;
        }
    }

    for (int i = 0; i < state->process_count; i++) {
        for (int j = 0; j < state->resource_count; j++) {
            int allocation = state->allocation[i][j];
            int maximum = state->maximum[i][j];

            if (allocation < 0 ||
                maximum < 0 ||
                allocation > maximum) {
                return false;
            }

            allocated[j] += allocation;

            if (allocated[j] > state->total[j]) {
                return false;
            }
        }
    }

    return true;
}

/*
 * Need-based completion-feasibility check:
 * 1. Work = Available.
 * 2. Find an unfinished process whose Need <= Work.
 * 3. Assume it finishes and releases its allocation.
 * 4. Any process still unfinished cannot be shown to finish in this order.
 *
 * This is a safety check, not proof of a runtime deadlock.
 *
 * Returns:
 *   -1 = invalid state
 *    0 = all processes can finish
 *   >0 = number of processes that cannot be shown to finish
 */
int detect_deadlock(
    SystemState *state,
    bool deadlocked[MAX_PROCESSES]
)
{
    if (deadlocked == NULL) {
        return -1;
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        deadlocked[i] = false;
    }

    if (!valid_state(state) || !calculate_need(state)) {
        return -1;
    }

    int work[MAX_RESOURCES] = {0};
    bool finished[MAX_PROCESSES] = {false};

    for (int j = 0; j < state->resource_count; j++) {
        work[j] = state->available[j];
    }

    bool progress;

    do {
        progress = false;

        for (int i = 0; i < state->process_count; i++) {
            if (finished[i]) {
                continue;
            }

            bool can_finish = true;

            for (int j = 0; j < state->resource_count; j++) {
                if (state->need[i][j] > work[j]) {
                    can_finish = false;
                    break;
                }
            }

            if (!can_finish) {
                continue;
            }

            for (int j = 0; j < state->resource_count; j++) {
                work[j] += state->allocation[i][j];
            }

            finished[i] = true;
            progress = true;
        }
    } while (progress);

    int count = 0;

    for (int i = 0; i < state->process_count; i++) {
        if (!finished[i]) {
            deadlocked[i] = true;
            count++;
        }
    }

    return count;
}

/*
 * Release resources only for a process unable to finish according to the
 * need-based feasibility check. This is a simulator action, not runtime
 * deadlock recovery.
 */
bool recover_from_deadlock(
    SystemState *state,
    int process_id
)
{
    if (!valid_state(state) ||
        process_id < 0 ||
        process_id >= state->process_count) {
        return false;
    }

    bool deadlocked[MAX_PROCESSES] = {false};

    int count = detect_deadlock(state, deadlocked);

    if (count <= 0 || !deadlocked[process_id]) {
        return false;
    }

    bool holds_resources = false;

    for (int j = 0; j < state->resource_count; j++) {
        if (state->allocation[process_id][j] > 0) {
            holds_resources = true;
            break;
        }
    }

    if (!holds_resources) {
        return false;
    }

    for (int j = 0; j < state->resource_count; j++) {
        state->allocation[process_id][j] = 0;
        state->request[process_id][j] = 0;
    }

    return calculate_need(state);
}
