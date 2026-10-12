// metrics.c - Niza's part
#include <stdio.h>
#include "scheduler.h"

void calculate_metrics(Process p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting    = p[i].turnaround - p[i].burst;
        p[i].response   = p[i].start - p[i].arrival;
    }
}

void print_results(Process p[], int n) {
    float total_wt = 0, total_tat = 0, total_rt = 0;

    printf("\n%-5s %-8s %-7s %-9s %-11s %-9s %-11s %-9s\n",
           "PID", "Arrival", "Burst", "Priority", "Completion", "Waiting", "Turnaround", "Response");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < n; i++) {
        printf("P%-4d %-8d %-7d %-9d %-11d %-9d %-11d %-9d\n",
               p[i].id, p[i].arrival, p[i].burst, p[i].priority,
               p[i].completion, p[i].waiting, p[i].turnaround, p[i].response);
        total_wt  += p[i].waiting;
        total_tat += p[i].turnaround;
        total_rt  += p[i].response;
    }

    printf("--------------------------------------------------------------------------------\n");
    printf("Average Waiting Time    : %.2f\n", total_wt / n);
    printf("Average Turnaround Time : %.2f\n", total_tat / n);
    printf("Average Response Time   : %.2f\n", total_rt / n);
}

float cpu_utilization(GanttBlock g[], int gcount) {
    if (gcount == 0) return 0;

    int busy = 0;
    for (int i = 0; i < gcount; i++) {
        if (g[i].pid != -1) busy += g[i].end - g[i].start;
    }

    int total = g[gcount - 1].end - g[0].start;
    if (total == 0) return 0;

    return (busy * 100.0f) / total;
}

#ifdef TEST_METRICS
int main() {
    Process p[3] = {
        {1, 0, 4, 1, 4, 0, 4, 0, 0, 0},
        {2, 1, 3, 2, 3, 4, 7, 0, 0, 0},
        {3, 2, 2, 3, 2, 7, 9, 0, 0, 0}
    };
    GanttBlock g[3] = { {1, 0, 4}, {2, 4, 7}, {3, 7, 9} };

    calculate_metrics(p, 3);
    print_results(p, 3);
    printf("CPU Utilization         : %.2f%%\n", cpu_utilization(g, 3));
    return 0;
}
#endif