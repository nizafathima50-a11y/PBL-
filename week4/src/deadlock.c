#include "deadlock.h"

#include <stddef.h>

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

            if (allocation < 0 || maximum < 0 || allocation > maximum ||
                allocation > state->total[j] - allocated[j]) {
                return false;
            }

            allocated[j] += allocation;
        }
    }

    return true;
}

int detect_deadlock(
    SystemState *state,
    bool deadlocked[MAX_PROCESSES]
)
{
    if (deadlocked == NULL) {
        return 0;
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        deadlocked[i] = false;
    }

    if (!valid_state(state) || !calculate_need(state)) {
        return 0;
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

            if (can_finish) {
                for (int j = 0; j < state->resource_count; j++) {
                    work[j] += state->allocation[i][j];
                }
                finished[i] = true;
                progress = true;
            }
        }
    } while (progress);

    int unable_to_finish = 0;
    for (int i = 0; i < state->process_count; i++) {
        if (!finished[i]) {
            deadlocked[i] = true;
            unable_to_finish++;
        }
    }

    return unable_to_finish;
}

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

    bool unable_to_finish[MAX_PROCESSES];
    if (detect_deadlock(state, unable_to_finish) == 0 ||
        !unable_to_finish[process_id]) {
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
    }

    return calculate_need(state);
}
