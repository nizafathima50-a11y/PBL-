
#include <stdio.h>
#include <stdbool.h>
#include "scheduler.h"

static void clear_input_buffer(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

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
    printf("10. Enter Current Resource Requests\n");
    printf("0. Exit\n");
    printf("====================================\n");
    printf("Enter choice: ");
}

int main(void)
{
    SystemState state;
    int choice;
    bool initialized = false;

    initialize_system(&state);

    while (1) {
        display_menu();

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clear_input_buffer();
            continue;
        }

        switch (choice) {
            case 1: {
                int processes;
                int resources;
                bool valid = true;

                printf("Enter number of processes (1-%d): ",
                       MAX_PROCESSES);

                if (scanf("%d", &processes) != 1 ||
                    processes < 1 ||
                    processes > MAX_PROCESSES) {
                    printf("Invalid number of processes.\n");
                    clear_input_buffer();
                    initialized = false;
                    break;
                }

                printf("Enter number of resource types (1-%d): ",
                       MAX_RESOURCES);

                if (scanf("%d", &resources) != 1 ||
                    resources < 1 ||
                    resources > MAX_RESOURCES) {
                    printf("Invalid number of resource types.\n");
                    clear_input_buffer();
                    initialized = false;
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
                        clear_input_buffer();
                        valid = false;
                        break;
                    }
                }

                if (!valid) {
                    initialize_system(&state);
                    initialized = false;
                    break;
                }

                initialized = true;
                printf("\nProcesses and resources created successfully.\n");
                printf("Enter the allocation and maximum matrices next.\n");
                break;
            }

            case 2: {
                if (!initialized) {
                    printf("Please create processes and resources first.\n");
                    break;
                }

                printf("\nEnter the Allocation Matrix:\n");

                bool valid = true;

                for (int i = 0; i < state.process_count && valid; i++) {
                    for (int j = 0; j < state.resource_count; j++) {
                        int value;

                        printf("Allocation of P%d for R%d: ", i, j);

                        if (scanf("%d", &value) != 1 || value < 0) {
                            printf("Invalid allocation value.\n");
                            clear_input_buffer();
                            valid = false;
                            break;
                        }

                        state.allocation[i][j] = value;
                    }
                }

                if (!valid) {
                    printf("Please re-enter the allocation matrix.\n");
                } else {
                    printf("Allocation matrix entered.\n");
                }
                break;
            }

            case 3: {
                if (!initialized) {
                    printf("Please create processes and resources first.\n");
                    break;
                }

                printf("\nEnter the Maximum Requirement Matrix:\n");

                bool valid = true;

                for (int i = 0; i < state.process_count && valid; i++) {
                    for (int j = 0; j < state.resource_count; j++) {
                        int value;

                        printf("Maximum requirement of P%d for R%d: ", i, j);

                        if (scanf("%d", &value) != 1 || value < 0) {
                            printf("Invalid maximum value.\n");
                            clear_input_buffer();
                            valid = false;
                            break;
                        }

                        state.maximum[i][j] = value;
                    }
                }

                if (!valid) {
                    printf("Please re-enter the maximum matrix.\n");
                } else {
                    printf("Maximum requirement matrix entered.\n");
                }
                break;
            }

            case 4: {
                if (!initialized || !calculate_need(&state)) {
                    printf("Invalid system data. Check allocation, maximum, "
                           "and total resource values.\n");
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

                if (!initialized || !calculate_need(&state)) {
                    printf("Please enter valid system data first.\n");
                    break;
                }

                int count = detect_deadlock(&state, deadlocked);

                if (count < 0) {
                if (count < 0) {
                    printf("Feasibility check could not be completed.\n");
                } else if (count == 0) {
                    printf("All processes can finish under the "
                           "need-based feasibility check.\n");
                } else {
                    printf("Processes unable to finish under the "
                           "need-based feasibility check: ");

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

                if (!initialized) {
                    printf("Please create processes first.\n");
                    break;
                }

                printf("Enter process ID to recover (0 to %d): ",
                       state.process_count - 1);

                if (scanf("%d", &process_id) != 1) {
                    printf("Invalid process ID.\n");
                    clear_input_buffer();
                    break;
                }

                if (process_id < 0 ||
                    process_id >= state.process_count) {
                    printf("Process ID is out of range.\n");
                    break;
                }

                if (recover_from_deadlock(&state, process_id)) {
                    printf("Recovery completed for P%d.\n", process_id);
                    printf("Resource data updated.\n");
                } else {
                    printf("Recovery not performed for P%d. The process "
                           "must be unable to finish under the need-based "
                           "check and hold allocated resources.\n",
                           process_id);
                           process_id);
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
                                   j, i, state.allocation[i][j]);
                        }
                    }
                }

                printf("\nCurrent outstanding requests:\n");

                for (int i = 0; i < state.process_count; i++) {
                    for (int j = 0; j < state.resource_count; j++) {
                        if (state.request[i][j] > 0) {
                            printf("P%d -> R%d (instances: %d)\n",
                                   i, j, state.request[i][j]);
                        }
                    }
                }

                printf("\nNote: This is a text representation of the graph.\n");
                break;
            }

            case 9: {
                int safe_sequence[MAX_PROCESSES];
                bool deadlocked[MAX_PROCESSES];

                if (!initialized || !calculate_need(&state)) {
                    printf("System status unavailable. "
                           "Please enter valid system data first.\n");
                    break;
                }

                int count = detect_deadlock(&state, deadlocked);

                if (count < 0) {
                    printf("System status unavailable: detection failed.\n");
                } else if (count > 0) {
                    printf("System status: UNSAFE under the need-based "
                           "feasibility check; this does not prove a "
                           "runtime deadlock.\n");
                } else if (check_safe_state(&state, safe_sequence)) {
                    printf("System status: SAFE.\n");
                } else {
                    printf("System status: UNSAFE.\n");
                }
                } else if (check_safe_state(&state, safe_sequence)) {
                    printf("System status: SAFE.\n");
                } else {
                    printf("System status: UNSAFE.\n");
                }
                break;
            }

            case 10: {
                if (!initialized) {
                    printf("Please create processes and resources first.\n");
                    break;
                }

                int new_requests[MAX_PROCESSES][MAX_RESOURCES] = {{0}};
                bool valid = true;

                printf("\nEnter CURRENT outstanding requests.\n");
                printf("Enter 0 for a resource the process is not waiting for.\n");

                for (int i = 0; i < state.process_count && valid; i++) {
                    printf("\nProcess P%d:\n", i);

                    for (int j = 0; j < state.resource_count; j++) {
                        printf("Current request of P%d for R%d: ", i, j);

                        if (scanf("%d", &new_requests[i][j]) != 1 ||
                            new_requests[i][j] < 0) {
                            printf("Invalid request value. "
                                   "Please enter the matrix again.\n");
                            clear_input_buffer();
                            valid = false;
                            break;
                        }
                    }
                }

                if (valid) {
                    for (int i = 0; i < state.process_count; i++) {
                        for (int j = 0; j < state.resource_count; j++) {
                            state.request[i][j] = new_requests[i][j];
                        }
                    }

                    printf("Current request matrix entered.\n");
                }
                break;
            }

            case 0:
                printf("Exiting Deadlock Simulation System.\n");
                return 0;

            default:
                printf("Invalid choice. Please select an option from 0 to 10.\n");
                break;
        }
    }

    return 0;
}

