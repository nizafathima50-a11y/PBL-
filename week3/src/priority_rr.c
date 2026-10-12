
#include <stdio.h>
#include "scheduler.h"

void run_priority(Process p[], int n, int preemptive,
                  GanttBlock g[], int *gcount) {
    int time = 0;
    int completed = 0;
    *gcount = 0;

    while (completed < n) {
        int best = -1;

        for (int i = 0; i < n; i++) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                if (best == -1 ||
                    p[i].priority < p[best].priority ||
                    (p[i].priority == p[best].priority &&
                     p[i].arrival < p[best].arrival)) {
                    best = i;
                }
            }
        }

        if (best == -1) {
            g[*gcount] = (GanttBlock){-1, time, time + 1};
            (*gcount)++;
            time++;
            continue;
        }

        if (p[best].start == -1)
            p[best].start = time;

        int duration = 1;

        if (!preemptive) {
            duration = p[best].remaining;
        }

        if (*gcount >= MAX_GANTT)
            return;

        g[*gcount] = (GanttBlock){p[best].id, time, time + duration};
        (*gcount)++;

        time += duration;
        p[best].remaining -= duration;

        if (p[best].remaining == 0) {
            p[best].completion = time;
            completed++;
        }
    }
}

void run_rr(Process p[], int n, int quantum,
            GanttBlock g[], int *gcount) {
    if (quantum <= 0) {
        *gcount = 0;
        printf("Time quantum must be positive.\n");
        return;
    }

    int queue[MAX_PROCESSES * 100];
    int front = 0, rear = 0;
    int time = 0, completed = 0;
    int added[MAX_PROCESSES] = {0};

    *gcount = 0;

    while (completed < n) {
        for (int i = 0; i < n; i++) {
            if (!added[i] && p[i].arrival <= time) {
                queue[rear++] = i;
                added[i] = 1;
            }
        }

        if (front == rear) {
            if (*gcount >= MAX_GANTT)
                return;

            g[*gcount] = (GanttBlock){-1, time, time + 1};
            (*gcount)++;
            time++;
            continue;
        }

        int current = queue[front++];

        if (p[current].start == -1)
            p[current].start = time;

        int duration = p[current].remaining < quantum
                       ? p[current].remaining : quantum;

        if (*gcount >= MAX_GANTT)
            return;

        g[*gcount] = (GanttBlock){
            p[current].id, time, time + duration
        };
        (*gcount)++;

        time += duration;
        p[current].remaining -= duration;

        for (int i = 0; i < n; i++) {
            if (!added[i] && p[i].arrival <= time) {
                queue[rear++] = i;
                added[i] = 1;
            }
        }

        if (p[current].remaining > 0) {
            queue[rear++] = current;
        } else {
            p[current].completion = time;
            completed++;
        }
    }
}
