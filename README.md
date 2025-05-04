CMP SCI 4760 - Assignment 5: Resource Management
Author: Curtis Been  



## Overview

This project simulates an operating system's resource management system using shared memory and message queues. Multiple user processes (`worker`) randomly request or release resources, while a master process (`oss`) handles resource allocation, tracks requests, and periodically checks for deadlocks.

## Key Features

- Shared memory system clock.
- Resource descriptor table with 5 resources, 10 instances each.
- IPC using message queues between `oss` and `worker`.
- Dynamic forking of up to 18 concurrent user processes.
- Logging to both file and screen.
- Deadlock detection every simulated second.
- Recovery by terminating one process per detection round.
- Simulation terminates after 5 seconds or when all processes finish.

## Files

- `oss.c` – Master simulator that manages resources, workers, clock, and deadlock detection.
- `worker.c` – User process that requests/releases resources and terminates randomly.
- `shared.h` – Shared structures for messages and resource descriptors.
- `Makefile` – Builds `oss` and `worker`.
- `oss.log` – Output log file (generated after run).

## Compilation

```bash
make clean
make

-------

Use This command to check deadlock log
cat oss.log | grep -i deadlock

