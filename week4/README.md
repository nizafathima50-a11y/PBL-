# Week 4 - Deadlock Simulation System

An interactive Operating Systems deadlock simulator written in C.

## Planned Features
- Resource allocation and requests
- Allocation, Maximum, Need, and Available data
- Banker's Algorithm and safe sequence
- Deadlock detection and recovery
- Resource Allocation Graph
- Graphical user interface

## Detection and recovery note

The current C interface has no per-process current-request matrix. Its
need-based check compares each process's remaining maximum claim with
available resources, like Banker's safety algorithm. A process that cannot
finish in this simulation is not necessarily deadlocked at runtime.
Recovery therefore releases resources only for a process marked unable to
finish by this check; it does not terminate or remove the process.
