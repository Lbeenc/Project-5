CMP SCI 4760 - Assignment 5: Resource Management
Author: Curtis Been  



 Description
This project simulates a resource management system in an operating system using:
- Shared memory (for system clock and resource descriptors)
- Message queues (for communication between `oss` and `worker` processes)
- Deadlock detection and recovery mechanism

`oss` serves as the operating system simulator and handles process management, clock advancement, resource granting, logging, and deadlock detection. `worker` simulates user processes making resource requests and releases at random intervals.

---

Compilation
Run the following to build the project:

```bash
make all
```

This creates two executables: `oss` and `worker`.

---

Execution
To run the simulation:

```bash
./oss
```

Optional parameters can be added for extensions (if implemented).

---

Files
- `oss.c`: Main controller, forks children, manages clock, resources, and logs
- `worker.c`: Simulated user process
- `shared.h`: Common definitions and data structures
- `Makefile`: Compiles the programs

---

 Resource Management Strategy
- 5 types of resources, each with 10 instances
- `oss` grants requests if available, otherwise blocks the process
- Resource descriptors track allocations and requests per process

---

 Deadlock Detection Policy
- Runs every 1 simulated second
- Uses a variation of the Banker’s algorithm
- If a deadlock is detected, `oss` terminates the process with the highest PID first
- Releases all resources held by terminated processes

---






