#include <assert.h>
#include <stdio.h>

#include "../src/scheduler.h"

static void set_up_classic_state(SystemState *state)
{
    initialize_system(state);
    state->process_count = 5;
    state->resource_count = 3;

    state->total[0] = 10;
    state->total[1] = 5;
    state->total[2] = 7;

    state->allocation[0][1] = 1;

    state->allocation[1][0] = 2;

    state->allocation[2][0] = 3;
    state->allocation[2][2] = 2;

    state->allocation[3][0] = 2;
    state->allocation[3][1] = 1;
    state->allocation[3][2] = 1;

    state->allocation[4][2] = 2;

    state->maximum[0][0] = 7;
    state->maximum[0][1] = 5;
    state->maximum[0][2] = 3;

    state->maximum[1][0] = 3;
    state->maximum[1][1] = 2;
    state->maximum[1][2] = 2;

    state->maximum[2][0] = 9;
    state->maximum[2][2] = 2;

    state->maximum[3][0] = 2;
    state->maximum[3][1] = 2;
    state->maximum[3][2] = 2;

    state->maximum[4][0] = 4;
    state->maximum[4][1] = 3;
    state->maximum[4][2] = 3;

    assert(calculate_need(state));
}

static void assert_safe_sequence(const SystemState *state, const int sequence[])
{
    int work[MAX_RESOURCES] = {0};
    bool seen[MAX_PROCESSES] = {false};

    for (int j = 0; j < state->resource_count; j++) {
        work[j] = state->available[j];
    }

    for (int position = 0; position < state->process_count; position++) {
        int process_id = sequence[position];
        assert(process_id >= 0 && process_id < state->process_count);
        assert(!seen[process_id]);

        for (int j = 0; j < state->resource_count; j++) {
            assert(state->need[process_id][j] <= work[j]);
            work[j] += state->allocation[process_id][j];
        }

        seen[process_id] = true;
    }
}

static void assert_request_unchanged(
    const SystemState *state,
    int process_id,
    const int available[],
    const int allocation[],
    const int need[]
)
{
    for (int j = 0; j < state->resource_count; j++) {
        assert(state->available[j] == available[j]);
        assert(state->allocation[process_id][j] == allocation[j]);
        assert(state->need[process_id][j] == need[j]);
    }
}

static void test_safe_state_returns_valid_sequence(void)
{
    SystemState state;
    int sequence[MAX_PROCESSES];
    set_up_classic_state(&state);

    assert(check_safe_state(&state, sequence));
    assert_safe_sequence(&state, sequence);
}

static void test_unsafe_state_is_rejected(void)
{
    SystemState state;
    int sequence[MAX_PROCESSES];

    initialize_system(&state);
    state.process_count = 2;
    state.resource_count = 1;
    state.total[0] = 2;
    state.allocation[0][0] = 1;
    state.allocation[1][0] = 1;
    state.maximum[0][0] = 2;
    state.maximum[1][0] = 2;
    assert(calculate_need(&state));

    assert(!check_safe_state(&state, sequence));
}

static void test_safe_request_is_granted_and_accounted(void)
{
    SystemState state;
    int request[MAX_RESOURCES] = {1, 0, 2};
    set_up_classic_state(&state);

    assert(request_resources(&state, 1, request));
    assert(state.available[0] == 2);
    assert(state.available[1] == 3);
    assert(state.available[2] == 0);
    assert(state.allocation[1][0] == 3);
    assert(state.allocation[1][1] == 0);
    assert(state.allocation[1][2] == 2);
    assert(state.need[1][0] == 0);
    assert(state.need[1][1] == 2);
    assert(state.need[1][2] == 0);
}

static void test_unsafe_request_is_denied_and_rolled_back(void)
{
    SystemState state;
    int request[MAX_RESOURCES] = {3, 3, 0};
    set_up_classic_state(&state);

    int available[MAX_RESOURCES];
    int allocation[MAX_RESOURCES];
    int need[MAX_RESOURCES];
    for (int j = 0; j < state.resource_count; j++) {
        available[j] = state.available[j];
        allocation[j] = state.allocation[4][j];
        need[j] = state.need[4][j];
    }

    assert(!request_resources(&state, 4, request));
    assert_request_unchanged(&state, 4, available, allocation, need);

    for (int j = 0; j < state.resource_count; j++) {
        assert(state.allocation[4][j] == (j == 2 ? 2 : 0));
        assert(state.need[4][j] == (j == 0 ? 4 : j == 1 ? 3 : 1));
    }
}

static void test_invalid_requests_are_rejected_without_mutation(void)
{
    SystemState state;
    set_up_classic_state(&state);

    int available[MAX_RESOURCES];
    int allocation[MAX_RESOURCES];
    int need[MAX_RESOURCES];
    for (int j = 0; j < state.resource_count; j++) {
        available[j] = state.available[j];
        allocation[j] = state.allocation[0][j];
        need[j] = state.need[0][j];
    }

    int negative[MAX_RESOURCES] = {-1, 0, 0};
    int exceeds_need[MAX_RESOURCES] = {8, 0, 0};
    int exceeds_available[MAX_RESOURCES] = {0, 4, 0};
    int zero[MAX_RESOURCES] = {0, 0, 0};

    assert(!request_resources(&state, 0, negative));
    assert_request_unchanged(&state, 0, available, allocation, need);

    assert(!request_resources(&state, 0, exceeds_need));
    assert_request_unchanged(&state, 0, available, allocation, need);

    assert(!request_resources(&state, 0, exceeds_available));
    assert_request_unchanged(&state, 0, available, allocation, need);

    assert(!request_resources(&state, -1, zero));
    assert_request_unchanged(&state, 0, available, allocation, need);

    assert(!request_resources(&state, state.process_count, zero));
    assert_request_unchanged(&state, 0, available, allocation, need);
}

int main(void)
{
    test_safe_state_returns_valid_sequence();
    test_unsafe_state_is_rejected();
    test_safe_request_is_granted_and_accounted();
    test_unsafe_request_is_denied_and_rolled_back();
    test_invalid_requests_are_rejected_without_mutation();

    puts("All Banker's Algorithm tests passed.");
    return 0;
}
