#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

typedef struct {
    int process_count;
    int resource_count;

    int total[MAX_RESOURCES];
    int available[MAX_RESOURCES];

    int allocation[MAX_PROCESSES][MAX_RESOURCES];
    int maximum[MAX_PROCESSES][MAX_RESOURCES];
    int need[MAX_PROCESSES][MAX_RESOURCES];
    int request[MAX_PROCESSES][MAX_RESOURCES];
} SystemState;

typedef struct {
    int process_id;
    int request[MAX_RESOURCES];
} ResourceRequest;

typedef enum {
    STATE_SAFE,
    STATE_UNSAFE,
    STATE_DEADLOCKED
} SystemStatus;

void initialize_system(SystemState *state);
bool calculate_need(SystemState *state);
bool check_safe_state(
    SystemState *state,
    int safe_sequence[MAX_PROCESSES]
);
bool request_resources(
    SystemState *state,
    int process_id,
    const int request[MAX_RESOURCES]
);
int detect_deadlock(
    SystemState *state,
    bool deadlocked[MAX_PROCESSES]
);
bool recover_from_deadlock(
    SystemState *state,
    int process_id
);

#endif
