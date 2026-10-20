
#include "deadlock.h"

#include <stddef.h>

/*
 * Validate the resource state and current outstanding requests.
 */
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
            int request = state->request[i][j];

            if (allocation < 0 ||
                maximum < 0 ||
                request < 0 ||
                allocation > maximum ||
                request > state->total[j]) {
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
 * Deadlock detection algorithm:
 * 1. Work = Available.
 * 2. Processes holding no resources are initially finished.
 * 3. Find an unfinished process whose current request <= Work.
 * 4. Assume it finishes and releases its allocation.
 * 5. Any process still unfinished is deadlocked.
 *
 * Returns:
 *   -1 = invalid state
 *    0 = no deadlocked processes
 *   >0 = number of deadlocked processes
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

    /*
     * A process holding no resources cannot be part of a
     * resource-allocation deadlock under this algorithm.
     */
    for (int i = 0; i < state->process_count; i++) {
        bool holds_resources = false;

        for (int j = 0; j < state->resource_count; j++) {
            if (state->allocation[i][j] > 0) {
                holds_resources = true;
                break;
            }
        }

        finished[i] = !holds_resources;
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
                if (state->request[i][j] > work[j]) {
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
 * Recover only if the selected process is currently deadlocked.
 * Recovery releases its allocated resources and clears its
 * outstanding request.
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

