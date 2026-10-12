
#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCESSES 20
#define MAX_GANTT 500

typedef struct {
    int id;
    int arrival;      // input
    int burst;        // input
    int priority;     // input (lower number = higher priority)
    int remaining;    // used by preemptive algorithms (starts = burst)
    int start;        // first time it got the CPU (-1 if not yet)
    int completion;   // time it finished
    int waiting;      // WT  = TAT - burst
    int turnaround;   // TAT = completion - arrival
    int response;     // RT  = start - arrival
} Process;

// One block in the Gantt chart. pid = -1 means CPU was idle.
typedef struct {
    int pid;
    int start;
    int end;
} GanttBlock;

// Algorithms: fill the Gantt array, set start + completion for each process.
// Do NOT calculate WT/TAT/RT here - metrics.c does that.

// Wilona
void run_fcfs(Process p[], int n, GanttBlock g[], int *gcount);
void run_sjf(Process p[], int n, int preemptive, GanttBlock g[], int *gcount);

// Ausaf
void run_priority(Process p[], int n, int preemptive, GanttBlock g[], int *gcount);
void run_rr(Process p[], int n, int quantum, GanttBlock g[], int *gcount);

// Niza
void calculate_metrics(Process p[], int n);
void print_results(Process p[], int n);
float cpu_utilization(GanttBlock g[], int gcount);

// Najim
void print_gantt(GanttBlock g[], int gcount);
void get_input(Process p[], int *n);
void generate_random(Process p[], int *n);

#endif