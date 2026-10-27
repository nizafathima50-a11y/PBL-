# Week 5 — Page Replacement Simulator (C + GTK4)


Example reference string: `7 0 1 2 0 3 0 4 2 3`; frames: `3`.

Choose FIFO, LRU, Optimal, or Compare all. The program shows frame states, hits, page faults, hit ratio, and fault ratio.

## FIFO component

The shared interface is declared in `src/page_replacement.h`. `Result.frames[i]`
stores all frame contents after reference `i`; an unused frame is `-1`, and
`Result.hit[i]` is `1` for a hit and `0` for a page fault. `fifo_simulate`
returns `1` on success and `0` for invalid input.

Run the standalone FIFO test from the project root (it does not build or start
the GTK application):

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Iweek5/src \
    week5/src/fifo.c week5/test/test_fifo.c -o /tmp/test_fifo
/tmp/test_fifo
```

For the example reference string with three frames, FIFO produces 1 hit and 9
page faults.

## team split
- Niza: GUI, integration and testing (`main.c`).
- Ausaf: FIFO algorithm.
- Wilona: LRU algorithm.
- Najim: Optimal algorithm.

.
