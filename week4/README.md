# Week 4 - Deadlock Simulation System

An interactive Operating Systems deadlock simulator written in C.

## Planned Features
- Resource allocation and requests, including GUI requests checked by
  Banker's Algorithm
- Allocation, Maximum, Need, and Available data
- Banker's Algorithm and safe sequence
- Need-based completion-feasibility check and simulated resource release
- Resource Allocation Graph visualization
- Graphical user interface

## Request, detection, and recovery notes

The GUI's Submit Request action invokes Banker's Algorithm and updates the
selected process's allocation only when the request keeps the system safe.
The completion-feasibility check compares each process's remaining Need with
Available resources, then simulates processes finishing and releasing their
allocations. This is Banker-style safety analysis, not runtime deadlock
detection: an unsafe result does not prove that processes are actually
deadlocked. The recovery control simulates releasing resources from a process
that cannot be shown to finish; it does not terminate or remove the process.
Submitted resource requests are evaluated immediately and are not retained
in the current-request matrix. The Resource Allocation Graph shows allocated
resource-to-process edges and any outstanding process-to-resource request
edges; because GUI requests are granted or denied immediately, it normally
shows allocation edges only.
