
#include <stdio.h>
#include <stdbool.h>
#include "scheduler.h"

static void display_menu(void)
{
    printf("\n");
    printf("====================================\n");
    printf("   DEADLOCK SIMULATION SYSTEM\n");
    printf("====================================\n");
    printf("1. Create Processes and Resources\n");
    printf("2. Enter Allocation Matrix\n");
    printf("3. Enter Maximum Matrix\n");
    printf("4. Display Need and Available Resources\n");
    printf("5. Run Banker's Algorithm\n");
    printf("6. Detect Deadlock\n");
    printf("7. Recover from Deadlock\n");
    printf("8. Display Resource Allocation Graph\n");
    printf("9. Display System Status\n");
    printf("0. Exit\n");
    printf("====================================\n");
    printf("Enter choice: ");
}

int main(void)
{
    SystemState state;
    int choice;
    int initialized = 0;

    initialize_system(&state);

    while (1) {
        display_menu();

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");

            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }

            continue;
        }

        switch (choice) {
            case 1: {
                int processes;
                int resources;

                printf("Enter number of processes (1-%d): ",
                       MAX_PROCESSES);

                if (scanf("%d", &processes) != 1 ||
                    processes < 1 ||
                    processes > MAX_PROCESSES) {
                    printf("Invalid number of processes.\n");
                    initialized = 0;
                    break;
                }

                printf("Enter number of resource types (1-%d): ",
                       MAX_RESOURCES);

                if (scanf("%d", &resources) != 1 ||
                    resources < 1 ||
                    resources > MAX_RESOURCES) {
                    printf("Invalid number of resource types.\n");
                    initialized = 0;
                    break;
                }

                initialize_system(&state);

                state.process_count = processes;
                state.resource_count = resources;

                printf("\nEnter total instances of each resource:\n");

                for (int j = 0; j < resources; j++) {
                    printf("Total instances of R%d: ", j);

                    if (scanf("%d", &state.total[j]) != 1 ||
                        state.total[j] < 0) {
                        printf("Invalid resource count.\n");
                        initialize_system(&state);
                        initialized = 0;
                        break;
                    }
                }

                if (state.process_count == processes &&
                    state.resource_count == resources) {
                    initialized = 1;
                    printf("\nProcesses and resources created successfully.\n");
                    printf("Enter the allocation and maximum matrices next.\n");
                }

                break;
            }

            case 2: {
                if (!initialized) {
                    printf("Please create processes and resources first.\n");
                    break;
                }

                printf("\nEnter the Allocation Matrix:\n");

                for (int i = 0; i < state.process_count; i++) {
                    for (int j = 0; j < state.resource_count; j++) {
                        printf("Allocation of P%d for R%d: ", i, j);

                        if (scanf("%d", &state.allocation[i][j]) != 1 ||
                            state.allocation[i][j] < 0) {
                            printf("Invalid allocation value.\n");
                            state.allocation[i][j] = 0;
                        }
                    }
                }

                printf("Allocation matrix entered.\n");
                break;
            }

            case 3: {
                if (!initialized) {
                    printf("Please create processes and resources first.\n");
                    break;
                }

                printf("\nEnter the Maximum Requirement Matrix:\n");

                for (int i = 0; i < state.process_count; i++) {
                    for (int j = 0; j < state.resource_count; j++) {
                        printf("Maximum requirement of P%d for R%d: ", i, j);

                        if (scanf("%d", &state.maximum[i][j]) != 1 ||
                            state.maximum[i][j] < 0) {
                            printf("Invalid maximum value.\n");
                            state.maximum[i][j] = 0;
                        }
                    }
                }

                printf("Maximum requirement matrix entered.\n");
                break;
            }

            case 4: {
                if (!initialized) {
                    printf("Please create processes and resources first.\n");
                    break;
                }

                if (!calculate_need(&state)) {
                    printf("Invalid system data. Check the allocation, "
                           "maximum, and total resource values.\n");
                    break;
                }

                printf("\nAvailable Resources:\n");

                for (int j = 0; j < state.resource_count; j++) {
                    printf("R%d=%d ", j, state.available[j]);
                }

                printf("\n\nNeed Matrix:\n");

                for (int i = 0; i < state.process_count; i++) {
                    printf("P%d: ", i);

                    for (int j = 0; j < state.resource_count; j++) {
                        printf("%d ", state.need[i][j]);
                    }

                    printf("\n");
                }

                break;
            }

            case 5: {
                int safe_sequence[MAX_PROCESSES];

                if (!initialized || !calculate_need(&state)) {
                    printf("Please enter valid process, resource, "
                           "allocation, and maximum data first.\n");
                    break;
                }

                if (check_safe_state(&state, safe_sequence)) {
                    printf("\nSystem is SAFE.\n");
                    printf("Safe sequence: ");

                    for (int i = 0; i < state.process_count; i++) {
                        printf("P%d", safe_sequence[i]);

                        if (i < state.process_count - 1) {
                            printf(" -> ");
                        }
                    }

                    printf("\n");
                } else {
                    printf("\nSystem is UNSAFE or the input is invalid.\n");
                }

                break;
            }

            case 6: {
                bool deadlocked[MAX_PROCESSES];
                int count;

                if (!initialized || !calculate_need(&state)) {
                    printf("Please enter valid system data first.\n");
                    break;
                }

                count = detect_deadlock(&state, deadlocked);

                if (count < 0) {
                    printf("Deadlock detection could not be completed.\n");
                } else if (count == 0) {
                    printf("No deadlocked processes detected.\n");
                } else {
                    printf("Deadlocked processes: ");

                    for (int i = 0; i < state.process_count; i++) {
                        if (deadlocked[i]) {
                            printf("P%d ", i);
                        }
                    }

                    printf("\n");
                }

                break;
            }

            case 7: {
                int process_id;

                if (!initialized || state.process_count < 1) {
                    printf("Please create processes first.\n");
                    break;
                }

                printf("Enter process ID to recover (0 to %d): ",
                       state.process_count - 1);

                if (scanf("%d", &process_id) != 1) {
                    printf("Invalid process ID.\n");
                    break;
                }

                if (process_id < 0 ||
                    process_id >= state.process_count) {
                    printf("Process ID is out of range.\n");
                    break;
                }

                if (recover_from_deadlock(&state, process_id)) {
                    printf("Recovery completed for P%d.\n", process_id);

                    if (calculate_need(&state)) {
                        printf("Resource data updated.\n");
                    }
                } else {
                    printf("Recovery failed for P%d.\n", process_id);
                }

                break;
            }

            case 8: {
                if (!initialized || !calculate_need(&state)) {
                    printf("Please enter valid system data first.\n");
                    break;
                }

                printf("\nRESOURCE ALLOCATION GRAPH\n");
                printf("-------------------------\n");
                printf("Allocated resources:\n");

                for (int i = 0; i < state.process_count; i++) {
                    for (int j = 0; j < state.resource_count; j++) {
                        if (state.allocation[i][j] > 0) {
                            printf("R%d -> P%d (instances: %d)\n",
                                   j,
                                   i,
                                   state.allocation[i][j]);
                        }
                    }
                }

                printf("\nMaximum resource claims:\n");

                for (int i = 0; i < state.process_count; i++) {
                    for (int j = 0; j < state.resource_count; j++) {
                        if (state.maximum[i][j] > 0) {
                            printf("P%d --claims up to %d of--> R%d\n",
                                   i,
                                   state.maximum[i][j],
                                   j);
                        }
                    }
                }

                printf("\nNote: Maximum claims are not the same as "
                       "current outstanding requests.\n");
                break;
            }

            case 9: {
                int safe_sequence[MAX_PROCESSES];
                bool deadlocked[MAX_PROCESSES];
                int deadlock_count;

                if (!initialized || !calculate_need(&state)) {
                    printf("System status unavailable. "
                           "Please enter valid system data first.\n");
                    break;
                }

                deadlock_count = detect_deadlock(&state, deadlocked);

                if (deadlock_count > 0) {
                    printf("System status: DEADLOCKED "
                           "(according to the detection module).\n");
                } else if (check_safe_state(&state, safe_sequence)) {
                    printf("System status: SAFE.\n");
                } else {
                    printf("System status: UNSAFE or invalid.\n");
                }

                break;
            }

            case 0:
                printf("Exiting Deadlock Simulation System.\n");
                return 0;

            default:
                printf("Invalid choice. Please select an option from 0 to 9.\n");
                break;
        }
    }

    return 0;
}
