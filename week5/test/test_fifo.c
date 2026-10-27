#include <assert.h>
#include <stdio.h>

#include "fifo.h"

int main(void)
{
    const int refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    const int expected_frames[][3] = {
        {7, -1, -1},
        {7, 0, -1},
        {7, 0, 1},
        {2, 0, 1},
        {2, 0, 1},
        {2, 3, 1},
        {2, 3, 0},
        {4, 3, 0},
        {4, 2, 0},
        {4, 2, 3}
    };
    const int expected_hits[] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
    const int n = (int)(sizeof(refs) / sizeof(refs[0]));
    Result result;
    int i;
    int j;

    assert(fifo_simulate(refs, n, 3, &result) == 1);
    assert(result.n == n);
    assert(result.f == 3);
    assert(result.hits == 1);
    assert(result.faults == 9);

    for (i = 0; i < n; i++) {
        assert(result.refs[i] == refs[i]);
        assert(result.hit[i] == expected_hits[i]);
        for (j = 0; j < 3; j++) {
            assert(result.frames[i][j] == expected_frames[i][j]);
        }

        printf("%d |", result.refs[i]);
        for (j = 0; j < 3; j++) {
            if (result.frames[i][j] == -1) {
                printf(" -");
            } else {
                printf(" %d", result.frames[i][j]);
            }
        }
        printf(" | %s\n", result.hit[i] ? "HIT" : "PAGE FAULT");
    }

    assert(fifo_simulate(refs, 0, 3, &result) == 0);
    assert(fifo_simulate(refs, n, 0, &result) == 0);
    assert(fifo_simulate(0, n, 3, &result) == 0);
    puts("PASS: FIFO reference trace and invalid-input checks");
    return 0;
}
