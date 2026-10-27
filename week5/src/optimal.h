#ifndef OPTIMAL_H
#define OPTIMAL_H

#define OPTIMAL_MAX_REF 200
#define OPTIMAL_MAX_FRAMES 20

typedef struct {
    int n;
    int f;
    int refs[OPTIMAL_MAX_REF];
    int frames[OPTIMAL_MAX_REF][OPTIMAL_MAX_FRAMES];
    int hit[OPTIMAL_MAX_REF];
    int hits;
    int faults;
} OptimalResult;

int simulate_optimal(const int refs[], int n, int f, OptimalResult *r);

#endif
