#include "optimal.h"

int simulate_optimal(const int refs[], int n, int f, OptimalResult *r)
{
    int frame[OPTIMAL_MAX_FRAMES];
    int i, j;

    if (refs == 0 || r == 0 ||
        n < 1 || n > OPTIMAL_MAX_REF ||
        f < 1 || f > OPTIMAL_MAX_FRAMES) {
        return 0;
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
        int slot = -1;

        /* Check whether the requested page is already loaded. */
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
        } else {
            int empty = -1;

            r->faults++;

            /* Use an empty frame first, if one exists. */
            for (j = 0; j < f; j++) {
                if (frame[j] == -1) {
                    empty = j;
                    break;
                }
            }

            if (empty != -1) {
                slot = empty;
            } else {
                int farthest = -1;

                /*
                 * Find the page whose next use is farthest
                 * in the future. A page never used again
                 * can be replaced immediately.
                 */
                slot = 0;

                for (j = 0; j < f; j++) {
                    int k;
                    int next = -1;

                    for (k = i + 1; k < n; k++) {
                        if (refs[k] == frame[j]) {
                            next = k;
                            break;
                        }
                    }

                    if (next == -1) {
                        slot = j;
                        break;
                    }

                    if (next > farthest) {
                        farthest = next;
                        slot = j;
                    }
                }
            }

            frame[slot] = refs[i];
        }

        /* Save the frame contents after this request. */
        for (j = 0; j < f; j++) {
            r->frames[i][j] = frame[j];
        }
    }

    return 1;
}
