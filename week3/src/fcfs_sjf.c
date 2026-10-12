#include <stdio.h>
#include "scheduler.h"

/* Add a Gantt block, merging adjacent blocks for the same process. */
static void add_block(GanttBlock g[], int *gcount,
                      int pid, int start, int end)
{
    if (start >= end)
        return;

    if (*gcount > 0 &&
        g[*gcount - 1].pid == pid &&
        g[*gcount - 1].end == start) {
        g[*gcount - 1].end = end;
        return;
    }

    if (*gcount < MAX_GANTT) {
        g[*gcount].pid = pid;
        g[*gcount].start = start;
        g[*gcount].end = end;
        (*gcount)++;
    }
}

/* Reset fields that must be recalculated for each simulation. */
static void reset_processes(Process p[], int n)
{
    for (int i = 0; i < n; i++) {
        p[i].remaining = p[i].burst;
        p[i].start = -1;
        p[i].completion = 0;
        p[i].waiting = 0;
        p[i].turnaround = 0;
        p[i].response = 0;
    }
}

/* FCFS: execute processes in arrival order. */
void run_fcfs(Process p[], int n, GanttBlock g[], int *gcount)
{
    reset_processes(p, n);
    *gcount = 0;

    int time = 0;
    int done[MAX_PROCESSES] = {0};

    for (int completed = 0; completed < n; completed++) {
        int chosen = -1;

        /* Find the earliest-arriving process not yet executed. */
        for (int i = 0; i < n; i++) {
            if (!done[i] &&
                (chosen == -1 ||
                 p[i].arrival < p[chosen].arrival)) {
                chosen = i;
            }
        }

        if (chosen == -1)
            break;

        /* Record CPU idle time if the next process hasn't arrived. */
        if (time < p[chosen].arrival) {
            add_block(g, gcount, -1, time, p[chosen].arrival);
            time = p[chosen].arrival;
        }

        p[chosen].start = time;
        add_block(g, gcount, p[chosen].id,
                  time, time + p[chosen].burst);

        time += p[chosen].burst;
        p[chosen].remaining = 0;
        p[chosen].completion = time;
        done[chosen] = 1;
    }
}

/*
 * SJF:
 * preemptive = 0: non-preemptive SJF
 * preemptive = 1: preemptive SJF (Shortest Remaining Time First)
 */
void run_sjf(Process p[], int n, int preemptive,
             GanttBlock g[], int *gcount)
{
    reset_processes(p, n);
    *gcount = 0;

    int time = 0;
    int completed = 0;

    if (!preemptive) {
        /* Non-preemptive SJF */
        while (completed < n) {
            int chosen = -1;

            /* Select the shortest job among arrived processes. */
            for (int i = 0; i < n; i++) {
                if (p[i].remaining > 0 &&
                    p[i].arrival <= time &&
                    (chosen == -1 ||
                     p[i].burst < p[chosen].burst)) {
                    chosen = i;
                }
            }

            if (chosen == -1) {
                /* No process is ready: jump to the next arrival. */
                int next = -1;

                for (int i = 0; i < n; i++) {
                    if (p[i].remaining > 0 &&
                        (next == -1 ||
                         p[i].arrival < p[next].arrival)) {
                        next = i;
                    }
                }

                if (next == -1)
                    break;

                add_block(g, gcount, -1, time, p[next].arrival);
                time = p[next].arrival;
                continue;
            }

            p[chosen].start = time;
            add_block(g, gcount, p[chosen].id,
                      time, time + p[chosen].remaining);

            time += p[chosen].remaining;
            p[chosen].remaining = 0;
            p[chosen].completion = time;
            completed++;
        }
    } else {
        /* Preemptive SJF: select the shortest remaining job each tick. */
        while (completed < n) {
            int chosen = -1;

            for (int i = 0; i < n; i++) {
                if (p[i].remaining > 0 &&
                    p[i].arrival <= time &&
                    (chosen == -1 ||
                     p[i].remaining < p[chosen].remaining)) {
                    chosen = i;
                }
            }

            if (chosen == -1) {
                /* CPU is idle until the next process arrives. */
                add_block(g, gcount, -1, time, time + 1);
                time++;
                continue;
            }

            if (p[chosen].start == -1)
                p[chosen].start = time;

            add_block(g, gcount, p[chosen].id, time, time + 1);

            p[chosen].remaining--;
            time++;

            if (p[chosen].remaining == 0) {
                p[chosen].completion = time;
                completed++;
            }
        }
    }
}
