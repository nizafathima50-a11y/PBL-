#include <stdio.h>
#include "../src/scheduler.h"

int main(void)
{
    SystemState state;
    initialize_system(&state);

    state.process_count = 2;
    state.resource_count = 2;

    state.total[0] = 5;
    state.total[1] = 4;

    state.allocation[0][0] = 1;
    state.allocation[0][1] = 1;
    state.allocation[1][0] = 2;
    state.allocation[1][1] = 0;

    state.maximum[0][0] = 3;
    state.maximum[0][1] = 2;
    state.maximum[1][0] = 3;
    state.maximum[1][1] = 2;

    if (!calculate_need(&state)) {
        printf("Invalid system data.\n");
        return 1;
    }

    printf("Available: %d %d\n",
           state.available[0], state.available[1]);

    printf("P0 Need: %d %d\n",
           state.need[0][0], state.need[0][1]);

    printf("P1 Need: %d %d\n",
           state.need[1][0], state.need[1][1]);

    return 0;
}
