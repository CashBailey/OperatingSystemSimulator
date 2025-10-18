# Operating System Simulator

## Team Members
- Cash Bailey  
- Alejandro Castineyra  
- Sylvia Ortega  
- Sarah Ramos Luna  

## Overview
**Operating System Simulator** is a simplified OS environment built for coursework.  
The current version handles the boot process and user authentication, validating input against a fixed administrator account.  
Future milestones will expand the simulator with modules for scheduling, I/O handling, and memory management.

---

## Project Structure
```

.
├── README.md
├── main.cpp          # Entry point: boot sequence + authentication
├── auth.h            # Declaration of authenticateUser()
├── auth.cpp          # Implementation: prompts user and checks credentials
├── process.h         # Process data model (states, timing, I/O milestones)
└── process.cpp       # (Planned) Process logic: constructors, state transitions, accounting

````

---

## Build Instructions

### Linux (g++)
```bash
g++ -std=c++17 -o main main.cpp auth.cpp process.cpp
./main
````

---

## Running the Program

When prompted, use the following credentials:

* **Username:** `admin`
* **Password:** `password123`

### Example Output

**Successful Login**

```
Booting SimpleOS...
Username: admin
Password: password123
Access granted.
Login successful. Welcome!
```

**Failed Login**

```
Booting SimpleOS...
Username: admin
Password: wrongpass
Access denied.
Authentication failed.
```

---

## Process Model (`process.h`)

Each simulated program is represented as a `Process` object with clearly defined states and accounting variables.

### States

`ProcessState` represents the typical OS lifecycle:
`NEW → READY → RUNNING → WAITING → TERMINATED`

### Core Attributes

* `pid`: Unique process ID
* `arrival_time`: Time the process enters the system
* `burst_time`: Total CPU time required
* `priority`: Used for priority-based scheduling
* `state`: Current process state

### Time Accounting

* `remaining_time`: CPU time left to execute
* `waiting_time`: Total time spent in READY state
* `turnaround_time`: `completion_time - arrival_time`
* `completion_time`: Time process finished execution

### Memory & I/O

* `memory_required`: Estimated memory demand
* `io_milestones`: CPU-time checkpoints that trigger I/O
* `io_count`: Number of I/O milestones
* `io_next_index`: Index of next milestone to check

### Execution Stats

* `cpu_executed`: Total CPU time consumed

---

## Process Implementation (`process.cpp`)

This file defines the behavior declared in `process.h`, keeping `main.cpp` and scheduling logic organized.

### Constructors & Setup

* `Process(int pid, int arrival, int burst, int prio, int mem, const int* io, std::size_t n_io)`
  Initializes all fields, sets initial timing values, and assigns state `NEW`.

### State Transitions

Functions to control lifecycle progression:

* `void to_ready(int now);`
* `void to_running(int now);`
* `void to_waiting(int now);`
* `void to_terminated(int now);`

These methods update time statistics and change the process state as appropriate.

### CPU Ticking & I/O Handling

* `bool tick_cpu(int dt)` – advances CPU time and returns `true` when finished.
* `bool check_io_request()` – detects I/O milestones and signals when I/O should occur.

### Ready-Queue Management

* `void on_enter_ready(int now)` – records when a process enters the READY queue.
* `void on_leave_ready(int now)` – accumulates waiting time based on duration spent ready.

### Utility Methods

* `bool is_done() const` – checks if process finished execution.
* `bool wants_io() const` – detects pending I/O requests.
* `int response_time(int first_scheduled_time) const` – optional metric for analysis.

---

## Example Scheduler (Round Robin Pseudocode)

```cpp
while (!ready.empty() || !io.empty() || running) {
  admit_new_arrivals(now);
  complete_io(now);

  if (!running && !ready.empty()) {
    auto& p = ready.front(); ready.pop();
    p.to_running(now);
    running = &p;
  }

  if (running) {
    if (running->tick_cpu(QUANTUM)) {
      running->to_terminated(now);
      running = nullptr;
    } else if (running->check_io_request()) {
      running->to_waiting(now);
      enqueue_io(*running);
      running = nullptr;
    } else {
      running->to_ready(now);
      ready.push(*running);
      running = nullptr;
    }
  }

  now += QUANTUM;
}
```

---

## Roadmap

* Implement `process.cpp` fully.
* Add a basic scheduler with `ready` and `waiting` queues.
* Simulate I/O devices with delay handling.
* Add CLI arguments for selecting scheduling policies (FCFS, SJF, RR, Priority).
* Generate statistics for waiting and turnaround time.
---
