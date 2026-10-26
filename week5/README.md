# Week 5 — Page Replacement Simulator (C + GTK4)

## Build on macOS
Install GTK4 if necessary:
```bash
brew install gtk4
```
From the project folder:
```bash
cd src
clang -Wall -Wextra -std=c11 main.c $(pkg-config --cflags --libs gtk4) -o ../page_replacement
cd ..
./page_replacement
```

Example reference string: `7 0 1 2 0 3 0 4 2 3`; frames: `3`.

Choose FIFO, LRU, Optimal, or Compare all. The program shows frame states, hits, page faults, hit ratio, and fault ratio.

## Suggested team split
- Niza: GUI, integration and testing (`main.c`).
- Ausaf: FIFO algorithm.
- Wilona: LRU algorithm.
- Najim: Optimal algorithm.

This is a starter implementation. Compile and test it on your Mac before presenting it. The current version keeps the algorithms in one `main.c` file for easy first-time setup. After it runs, you can split the algorithm functions into separate files for individual commits and grading.
