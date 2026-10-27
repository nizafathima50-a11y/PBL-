#include <stdio.h>
#include "lru.h"

int main(void)
{
    int refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    LRUResult result;
    int i;

    if (!simulate_lru(refs, 10, 3, &result)) {
        printf("Simulation failed\n");
        return 1;
    }

    printf("Hits: %d\n", result.hits);
    printf("Faults: %d\n", result.faults);

    for (i = 0; i < result.n; i++) {
        printf("%d: ", result.refs[i]);

        for (int j = 0; j < result.f; j++) {
            if (result.frames[i][j] == -1)
                printf("- ");
            else
                printf("%d ", result.frames[i][j]);
        }

        printf("%s\n", result.hit[i] ? "HIT" : "FAULT");
    }

    return 0;
}
