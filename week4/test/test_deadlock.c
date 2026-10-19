#include <assert.h>
#include <stdio.h>

#include "../src/deadlock.h"

static void set_up_state(SystemState *state, int process_count, int resource_count)
{
    initialize_system(state);
    state->process_count = process_count;
    state->resource_count = resource_count;
}

static void test_no_deadlock(void)
{
    SystemState state;
    bool deadlocked[MAX_PROCESSES];
    set_up_state(&state, 2, 1);

    state.total[0] = 3;
    state.allocation[0][0] = 1;
    state.allocation[1][0] = 1;
    state.maximum[0][0] = 2;
    state.maximum[1][0] = 2;

    assert(detect_deadlock(&state, deadlocked) == 0);
    assert(!deadlocked[0] && !deadlocked[1]);
}

static void test_need_based_feasibility_failure(void)
{
    SystemState state;
    bool deadlocked[MAX_PROCESSES];
    set_up_state(&state, 2, 1);

    state.total[0] = 2;
    state.allocation[0][0] = 1;
    state.allocation[1][0] = 1;
    state.maximum[0][0] = 2;
    state.maximum[1][0] = 2;

    assert(detect_deadlock(&state, deadlocked) == 2);
    assert(deadlocked[0] && deadlocked[1]);
}

static void test_multiple_resources_and_zero_allocation(void)
{
    SystemState state;
    bool deadlocked[MAX_PROCESSES];
    set_up_state(&state, 2, 2);

    state.total[0] = 2;
    state.total[1] = 2;
    state.maximum[0][0] = 1;
    state.maximum[0][1] = 1;
    state.allocation[1][0] = 1;
    state.maximum[1][0] = 2;
    state.maximum[1][1] = 1;

    assert(detect_deadlock(&state, deadlocked) == 0);
    assert(!deadlocked[0] && !deadlocked[1]);
}

static void test_recovery_releases_resources(void)
{
    SystemState state;
    set_up_state(&state, 2, 1);

    state.total[0] = 3;
    state.allocation[0][0] = 2;
    state.allocation[1][0] = 1;
    state.maximum[0][0] = 3;
    state.maximum[1][0] = 2;
    assert(calculate_need(&state));
    assert(state.available[0] == 0);

    assert(recover_from_deadlock(&state, 0));
    assert(state.allocation[0][0] == 0);
    assert(state.available[0] == 2);
    assert(state.need[0][0] == 3);
}

static void test_recovery_rejects_bad_process_ids_and_empty_allocation(void)
{
    SystemState state;
    set_up_state(&state, 1, 1);
    state.total[0] = 1;
    state.maximum[0][0] = 1;

    assert(!recover_from_deadlock(&state, -1));
    assert(!recover_from_deadlock(&state, 1));
    assert(!recover_from_deadlock(&state, 0));
}

static void test_invalid_state_is_rejected_without_mutation(void)
{
    SystemState state;
    bool deadlocked[MAX_PROCESSES];
    set_up_state(&state, 1, 1);

    state.total[0] = 1;
    state.allocation[0][0] = 2;
    state.maximum[0][0] = 2;
    deadlocked[0] = true;

    assert(detect_deadlock(&state, deadlocked) == 0);
    assert(!deadlocked[0]);
    assert(!recover_from_deadlock(&state, 0));
    assert(state.allocation[0][0] == 2);

    state.allocation[0][0] = 0;
    state.resource_count = MAX_RESOURCES + 1;
    assert(detect_deadlock(&state, deadlocked) == 0);
    assert(!recover_from_deadlock(&state, 0));
}

int main(void)
{
    test_no_deadlock();
    test_need_based_feasibility_failure();
    test_multiple_resources_and_zero_allocation();
    test_recovery_releases_resources();
    test_recovery_rejects_bad_process_ids_and_empty_allocation();
    test_invalid_state_is_rejected_without_mutation();

    puts("All deadlock tests passed.");
    return 0;
}
