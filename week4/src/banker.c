//wilona
#include "scheduler.h"

#include <stddef.h>
#include <string.h>

static bool valid_state(const SystemState *state)
{
    if (state == NULL ||
        state->process_count < 1 ||
        state->process_count > MAX_PROCESSES ||
        state->resource_count < 1 ||
        state->resource_count > MAX_RESOURCES) {
        return false;
    }

    for (int j = 0; j < state->resource_count; j++) {
        if (state->total[j] < 0) {
            return false;
        }
    }

    for (int i = 0; i < state->process_count; i++) {
        for (int j = 0; j < state->resource_count; j++) {
            int allocation = state->allocation[i][j];
            int maximum = state->maximum[i][j];
            int need = state->need[i][j];

            if (allocation < 0 || maximum < 0 || need < 0) {
                return false;
            }

            if (allocation > maximum) {
                return false;
            }

            if (allocation > state->total[j]) {
                return false;
            }
        }
    }

    return true;
}

bool check_safe_state(SystemState *state, int safe_sequence[MAX_PROCESSES])
{
    if (state == NULL || safe_sequence == NULL) {
        return false;
    }

    if (!valid_state(state) || !calculate_need(state)) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            safe_sequence[i] = -1;
        }
        return false;
    }

    int work[MAX_RESOURCES] = {0};
    bool finished[MAX_PROCESSES] = {false};
    int count = 0;

    for (int j = 0; j < state->resource_count; j++) {
        work[j] = state->available[j];
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        safe_sequence[i] = -1;
    }

    bool progress = true;
    while (progress) {
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
            safe_sequence[count++] = i;
            progress = true;
        }
    }

    if (count != state->process_count) {
        return false;
    }

    return true;
}

bool request_resources(SystemState *state, int process_id, const int request[MAX_RESOURCES])
{
    if (state == NULL || request == NULL) {
        return false;
    }

    if (!valid_state(state) ||
        process_id < 0 ||
        process_id >= state->process_count) {
        return false;
    }

    int available_copy[MAX_RESOURCES] = {0};
    int allocation_copy[MAX_PROCESSES][MAX_RESOURCES] = {{0}};
    int need_copy[MAX_PROCESSES][MAX_RESOURCES] = {{0}};

    for (int j = 0; j < state->resource_count; j++) {
        if (request[j] < 0) {
            return false;
        }

        if (request[j] > state->need[process_id][j]) {
            return false;
        }

        if (request[j] > state->available[j]) {
            return false;
        }

        available_copy[j] = state->available[j];
        allocation_copy[process_id][j] = state->allocation[process_id][j];
        need_copy[process_id][j] = state->need[process_id][j];
    }

    for (int j = 0; j < state->resource_count; j++) {
        state->available[j] -= request[j];
        state->allocation[process_id][j] += request[j];
        state->need[process_id][j] -= request[j];
    }

    int safe_sequence[MAX_PROCESSES] = {-1};
    if (check_safe_state(state, safe_sequence)) {
        return true;
    }

    for (int j = 0; j < state->resource_count; j++) {
        state->available[j] = available_copy[j];
        state->allocation[process_id][j] = allocation_copy[process_id][j];
        state->need[process_id][j] = need_copy[process_id][j];
    }

    return false;
}
