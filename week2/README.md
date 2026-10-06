# SusurKaddi — Week 2: Multi-Process Simulator & IPC

## What we did
Split our simulator into three separate processes (UI, Core, Logger) that
communicate using POSIX Message Queues instead of running as one program.

## Team Roles
| Member | Role | File |
|---|---|---|
| Najim | UI Process | `src/najim_ui.c` |
| Wilona | Core Process (CPU, Memory, Stack, Queue) | `src/wilona_core.c` |
| Ausaf | Logging Process | `src/Ausaf_log.c` |
| Niza | Team Leader — Integration | `launcher.c`, `Makefile`, `benchmark.sh` |

## IPC Mechanism
POSIX Message Queues. UI sends commands to Core, Core processes them and sends
results to Logger, Logger saves everything to `simulator.log` with a timestamp.

## Commands Supported
`ADD x y`, `SUB x y`, `MUL x y`, `DIV x y`, `PUSH value`, `POP`,
`STORE addr value`, `LOAD addr`, `QUIT`

## How to Run
```
make
./launcher
```
This starts Logger, Core, and UI in order automatically. Type commands at the
UI prompt. Type `QUIT` to exit.

## Files
- `src/` — source code for all three processes
- `launcher.c` — starts all three processes together
- `benchmark.sh` — compares single-process vs multi-process performance
- `simulator.log` — generated log of every command and result