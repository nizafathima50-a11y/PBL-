#include "lru.h"

int simulate_lru(const int refs[], int n, int f, LRUResult *r)
{
    int frame[LRU_MAX_FRAMES];
    int last_used[LRU_MAX_FRAMES];
    int i, j;

    if (refs == 0 || r == 0 ||
        n < 1 || n > LRU_MAX_REF ||
        f < 1 || f > LRU_MAX_FRAMES) {
        return 0;
    }

    r->n = n;
    r->f = f;
    r->hits = 0;
    r->faults = 0;

    for (j = 0; j < f; j++) {
        frame[j] = -1;
        last_used[j] = -1;
    }

    for (i = 0; i < n; i++) {
        int found = 0;
        int slot = -1;

        for (j = 0; j < f; j++) {
            if (frame[j] == refs[i]) {
                found = 1;
                slot = j;
                break;
            }
        }

        r->refs[i] = refs[i];
        r->hit[i] = found;

        if (found) {
            r->hits++;
            last_used[slot] = i;
        } else {
            int empty = -1;

            r->faults++;

            for (j = 0; j < f; j++) {
                if (frame[j] == -1) {
                    empty = j;
                    break;
                }
            }

            if (empty != -1) {
                slot = empty;
            } else {
                slot = 0;

                for (j = 1; j < f; j++) {
                    if (last_used[j] < last_used[slot]) {
                        slot = j;
                    }
                }
            }

            frame[slot] = refs[i];
            last_used[slot] = i;
        }

        for (j = 0; j < f; j++) {
            r->frames[i][j] = frame[j];
        }
    }

    return 1;
}