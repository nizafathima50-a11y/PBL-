#ifndef LRU_H
#define LRU_H

#define LRU_MAX_REF 200
#define LRU_MAX_FRAMES 20

typedef struct {
    int n;
    int f;
    int refs[LRU_MAX_REF];
    int frames[LRU_MAX_REF][LRU_MAX_FRAMES];
    int hit[LRU_MAX_REF];
    int hits;
    int faults;
} LRUResult;

int simulate_lru(const int refs[], int n, int f, LRUResult *r);

#endif
