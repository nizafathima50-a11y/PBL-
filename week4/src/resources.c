#include "scheduler.h"
#include <string.h>

void initialize_system(SystemState *state)
{
    if (state == NULL) {
        return;
    }

    memset(state, 0, sizeof(SystemState));
}

bool calculate_need(SystemState *state)
{
    if (state == NULL ||
        state->process_count < 1 ||
        state->process_count > MAX_PROCESSES ||
        state->resource_count < 1 ||
        state->resource_count > MAX_RESOURCES) {
        return false;
    }

    int allocated[MAX_RESOURCES] = {0};

    /* Validate resource totals and allocation data first. */
    for (int j = 0; j < state->resource_count; j++) {
        if (state->total[j] < 0) {
            return false;
        }
    }

    for (int i = 0; i < state->process_count; i++) {
        for (int j = 0; j < state->resource_count; j++) {
            int a = state->allocation[i][j];
            int m = state->maximum[i][j];

            if (a < 0 || m < 0 || a > m) {
                return false;
            }

            allocated[j] += a;
        }
    }

    for (int j = 0; j < state->resource_count; j++) {
        if (allocated[j] > state->total[j]) {
            return false;
        }
    }

    /* Calculate Need and Available. */
    for (int i = 0; i < state->process_count; i++) {
        for (int j = 0; j < state->resource_count; j++) {
            state->need[i][j] =
                state->maximum[i][j] - state->allocation[i][j];
        }
    }

    for (int j = 0; j < state->resource_count; j++) {
        state->available[j] = state->total[j] - allocated[j];
    }

    return true;
}
