
#include <stdio.h>
#include "scheduler.h"

int main(void) {
    Process p[MAX_PROCESSES];
    GanttBlock g[MAX_GANTT];
    int n = 0;
    int gcount = 0;
    int choice;
    int quantum;

    printf("=== Process Scheduling Simulator ===\n");

    get_input(p, &n);

    if (n <= 0) {
        printf("No valid processes entered.\n");
        return 1;
    }

    printf("\nChoose a scheduling algorithm:\n");
    printf("1. FCFS\n");
    printf("2. SJF Non-preemptive\n");
    printf("3. SJF Preemptive\n");
    printf("4. Priority Non-preemptive\n");
    printf("5. Priority Preemptive\n");
    printf("6. Round Robin\n");
    printf("Enter choice: ");

    if (scanf("%d", &choice) != 1) {
        printf("Invalid choice.\n");
        return 1;
    }

    switch (choice) {
        case 1:
            run_fcfs(p, n, g, &gcount);
            break;

        case 2:
            run_sjf(p, n, 0, g, &gcount);
            break;

        case 3:
            run_sjf(p, n, 1, g, &gcount);
            break;

        case 4:
            run_priority(p, n, 0, g, &gcount);
            break;

        case 5:
            run_priority(p, n, 1, g, &gcount);
            break;

        case 6:
            printf("Enter time quantum: ");
            if (scanf("%d", &quantum) != 1 || quantum <= 0) {
                printf("Invalid time quantum.\n");
                return 1;
            }
            run_rr(p, n, quantum, g, &gcount);
            break;

        default:
            printf("Invalid choice.\n");
            return 1;
    }

    if (gcount <= 0) {
        printf("Scheduling failed or produced no output.\n");
        return 1;
    }

    printf("\n");
    print_gantt(g, gcount);

    calculate_metrics(p, n);
    print_results(p, n);

    printf("CPU Utilization: %.2f%%\n",
           cpu_utilization(g, gcount));

    return 0;
}

