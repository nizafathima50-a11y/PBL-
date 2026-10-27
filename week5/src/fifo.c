#include "fifo.h"

int fifo_simulate(const int refs[], int n, int f, Result *r)
{
    int frame[MAX_FRAMES];
    int next_to_replace = 0;
    int i;
    int j;

    if (refs == 0 || r == 0 || n < 1 || n > MAX_REF ||
        f < 1 || f > MAX_FRAMES) {
        return 0;
    }

    for (i = 0; i < n; i++) {
        if (refs[i] < 0) {
            return 0;
        }
    }

    r->n = n;
    r->f = f;
    r->hits = 0;
    r->faults = 0;

    for (j = 0; j < f; j++) {
        frame[j] = -1;
    }

    for (i = 0; i < n; i++) {
        int found = 0;
        int empty = -1;

        r->refs[i] = refs[i];

        for (j = 0; j < f; j++) {
            if (frame[j] == refs[i]) {
                found = 1;
                break;
            }
            if (frame[j] == -1 && empty == -1) {
                empty = j;
            }
        }

        if (found) {
            r->hit[i] = 1;
            r->hits++;
        } else {
            r->hit[i] = 0;
            r->faults++;

            if (empty != -1) {
                frame[empty] = refs[i];
            } else {
                frame[next_to_replace] = refs[i];
                next_to_replace = (next_to_replace + 1) % f;
            }
        }

        for (j = 0; j < f; j++) {
            r->frames[i][j] = frame[j];
        }
    }

    return 1;
}
