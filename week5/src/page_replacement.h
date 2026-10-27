#ifndef PAGE_REPLACEMENT_H
#define PAGE_REPLACEMENT_H

#define MAX_REF 200
#define MAX_FRAMES 20

typedef enum {
    FIFO = 0,
    LRU = 1,
    OPTIMAL = 2
} Algorithm;

typedef struct {
    int n;
    int f;
    int refs[MAX_REF];
    int frames[MAX_REF][MAX_FRAMES];
    int hit[MAX_REF];
    int hits;
    int faults;
} Result;

int fifo_simulate(const int refs[], int n, int f, Result *r);

#endif
