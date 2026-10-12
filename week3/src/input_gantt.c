```c
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

void get_input(Process p[], int *n) {
    printf("Enter number of processes (1-%d): ", MAX_PROCESSES);
    scanf("%d", n);

    if (*n < 1 || *n > MAX_PROCESSES) {
        printf("Invalid number of processes.\n");
        *n = 0;
        return;
    }

    for (int i = 0; i < *n; i++) {
        p[i].id = i + 1;

        printf("\nProcess P%d\n", p[i].id);
        printf("Arrival time: ");
        scanf("%d", &p[i].arrival);

        printf("Burst time: ");
        scanf("%d", &p[i].burst);

        printf("Priority: ");
        scanf("%d", &p[i].priority);

        if (p[i].arrival < 0 || p[i].burst <= 0) {
            printf("Invalid arrival or burst time.\n");
            *n = 0;
            return;
        }

        p[i].remaining = p[i].burst;
        p[i].start = -1;
        p[i].completion = 0;
        p[i].waiting = 0;
        p[i].turnaround = 0;
        p[i].response = 0;
    }
}

void print_gantt(GanttBlock g[], int gcount) {
    if (gcount <= 0) {
        printf("No Gantt chart to display.\n");
        return;
    }

    printf("\nGantt Chart:\n");

    for (int i = 0; i < gcount; i++) {
        if (g[i].pid == -1)
            printf("| Idle ");
        else
            printf("| P%d ", g[i].pid);
    }

    printf("|\n%d", g[0].start);

    for (int i = 0; i < gcount; i++) {
        printf("     %d", g[i].end);
    }

    printf("\n");
}

void generate_random(Process p[], int *n) {
    printf("Enter number of processes (1-%d): ", MAX_PROCESSES);
    scanf("%d", n);

    if (*n < 1 || *n > MAX_PROCESSES) {
        printf("Invalid number of processes.\n");
        *n = 0;
        return;
    }

    for (int i = 0; i < *n; i++) {
        p[i].id = i + 1;
        p[i].arrival = rand() % 10;
        p[i].burst = 1 + rand() % 10;
        p[i].priority = 1 + rand() % 5;
        p[i].remaining = p[i].burst;
        p[i].start = -1;
        p[i].completion = 0;
        p[i].waiting = 0;
        p[i].turnaround = 0;
        p[i].response = 0;
    }
}
```