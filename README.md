# Operating System Simulator

## Team
- Cash Bailey  
- Alejandro Castineyra
- Sarah Ramos-Luna
- Sylvia Ortega 

---

## Overview

**Operating System Simulator** is a compact, instructional OS environment.

The current integrated project models several classic OS subsystems:

- **Boot & Authentication**  
  - Simulated “boot” message and login prompt (`auth.cpp`, `auth.h`).
- **Process Model**  
  - Process states, timing, and accounting (`process.cpp`, `process.h`).
- **CPU Scheduling**  
  - Three algorithms wired into `main.cpp` via `scheduler.cpp`, `scheduler.h`:
    - **FCFS** (First‑Come, First‑Served, non‑preemptive)
    - **Round Robin** (preemptive with time quantum)
    - **SJF** (Shortest Job First, non‑preemptive)
- **Virtual Memory Subsystem**  
  - Software paging, page tables, backing store, and FIFO page replacement (`VirtualMemory.cpp`, `VirtualMemory.h`).
- **OS “Kernel” Driver**  
  - `main.cpp` ties everything together: boot, login, VM init, process creation, scheduling, shutdown.

The whole system is designed to be small enough to understand in one sitting but rich enough to illustrate how real OS components interact.

---

## Project Structure

At the top level:

```text
.
├─ README.md
├─ main.cpp             # Entry point: boots, authenticates, init VM, creates processes, runs schedulers
├─ auth.h / auth.cpp    # Simple username/password authentication
├─ process.h / process.cpp
│                       # Process model: states, timing, basic I/O hooks
├─ scheduler.h / scheduler.cpp
│                       # FCFS, Round Robin, SJF + ProcessCreation()
├─ VirtualMemory.h / VirtualMemory.cpp
│                       # Virtual memory subsystem: paging, page faults, replacement
````

Key files and roles:

| File                   | Kind    | Role (short)                       |
| ---------------------- | ------- | ---------------------------------- |
| `main.cpp`             | C++     | OS driver; wires all modules       |
| `auth.cpp/.h`          | C / C++ | Login prompt and credential check  |
| `process.cpp/.h`       | C++     | Process data model and state logic |
| `scheduler.cpp/.h`     | C++     | FCFS, RR, SJF and process creation |
| `VirtualMemory.cpp/.h` | C++     | Paging, backing store, page faults |

---

## Build & Run

### Requirements

* C++17 compatible compiler

  * Example: `g++` (Linux, WSL, MinGW), `clang++`, or MSVC (with minor flag changes).
* Basic terminal / command prompt.

### Full OS Simulation (recommended)

From the project directory (where `main.cpp` lives), run:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
    main.cpp auth.cpp process.cpp scheduler.cpp VirtualMemory.cpp \
    -o main
```

Example with your path (Linux / WSL):

```bash
cd /home/c/VScode/OperatingSystemSimulator
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
    main.cpp auth.cpp process.cpp scheduler.cpp VirtualMemory.cpp \
    -o main
```

Then run:

```bash
./main
```

On Windows with MinGW:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic ^
    main.cpp auth.cpp process.cpp scheduler.cpp VirtualMemory.cpp ^
    -o main.exe
main.exe
```

### What the program does on startup

1. Prints a boot banner:

   ```text
   Booting Operating System Simulator...
   ```
2. Prompts for credentials:

   ```text
   Username:
   Password:
   ```
3. If authentication succeeds:

   * Initializes virtual memory (`vm_init`).
   * Creates a random number of processes via `ProcessCreation`.
   * Registers each process with the VM (`vm_create_process`).
   * Runs:

     * `run_fcfs(processes);`
     * `run_round_robin(processes, quantumTime);`
     * `run_sjf(processes);`
   * Frees all process memory (`vm_free_process`) and dumps VM state (`vm_dump_state`).
4. If authentication fails:

   * Prints an error and exits.

### Default credentials

When prompted, use:

* **Username:** `admin`
* **Password:** `password123`

On successful login you will see:

```text
Access granted.
Login successful. Welcome!
```

---

## Execution Flow

High‑level flow inside `main.cpp`:

1. **Boot message**
2. **User authentication** via `authenticateUser()`
3. **Virtual memory initialization** via `vm_init()`
4. **Random process creation**

   * Number of processes: `1–10`
   * Each process gets:

     * Arrival time
     * CPU burst
     * I/O burst (reserved)
     * Priority
     * Memory requirement
   * Each process is registered with the VM using `vm_create_process(pid, memory_required)`.
5. **Scheduling**

   * FCFS run (non‑preemptive)
   * Round Robin run (preemptive, fixed quantum)
   * SJF run (non‑preemptive, arrival‑aware)
6. **VM cleanup**

   * Free per‑process VM state (`vm_free_process(pid)`).
   * Dump final VM state (`vm_dump_state()`).
7. **Shutdown message**

This makes `main.cpp` feel like a tiny OS kernel: boot, login, init subsystems, run workload, shut down.

---

## Module Details

### 1. Authentication (`auth.h`, `auth.cpp`)

**Goal:** Simulate a simple secure login before the “OS” starts doing anything.

* `int authenticateUser(void);`

  * Prompts:

    * `Username:`
    * `Password:`
  * Compares input to hardcoded credentials:

    * `expectedUsername = "admin"`
    * `expectedPassword = "password123"`
  * Returns `1` on success, `0` on failure.

`main.cpp` uses this to gate the rest of the system. If authentication fails, scheduling and VM never start.

---

### 2. Process Model (`process.h`, `process.cpp`)

This is the core abstraction for a running program.

**States**

`ProcessState` enum:

* `New`
* `Ready`
* `Running`
* `Waiting`
* `Terminated`

**Key attributes in `Process`**

* Identification and scheduling:

  * `int pid;`
  * `int arrival_time;`
  * `int Cpu_burst;`
  * `int IO_burst;`
  * `int priority;`
  * `ProcessState state;`

* Time accounting:

  * `int remaining_time;`
  * `int waiting_time;`
  * `int turnaround_time;`
  * `int completion_time;`

* Memory and I/O:

  * `int memory_required;`
  * `const int* io_milestone;`
  * `size_t io_count;`
  * `size_t io_next_index;`

* Execution stats:

  * `int cpu_executed;`

**Core methods**

* `Process(int pid, int arrival, int cpu, int io, int prio, int mem);`
* `void set_state(ProcessState new_state);`
* `int tick_cpu(int now, int quanta = 1);`
* `void add_wait(int dt);`
* `void start_io();`  // Ready/Running → Waiting
* `void finish_io();` // Waiting → Ready
* `bool is_complete() const;`
* `const char* state_cstr() const;`

Schedulers use these fields and helpers to move processes between states and compute metrics like turnaround and waiting time.

---

### 3. Scheduler API (`scheduler.h`, `scheduler.cpp`)

`scheduler.h` declares:

* `Process ProcessCreation(int pid);`

  * Creates a `Process` with randomized fields:

    * Arrival time
    * CPU burst
    * I/O burst placeholder
    * Priority
    * Memory requirement

* Scheduling entry points:

  ```cpp
  void run_fcfs(const std::vector<Process>& processes);
  void run_round_robin(const std::vector<Process>& processes, int quantumTime);
  void run_sjf(const std::vector<Process>& processes);
  ```

#### 3.1 FCFS (First‑Come, First‑Served, non‑preemptive)

`run_fcfs`:

* Sorts processes by `arrival_time` (ties by `pid`).

* For each process:

  * `start = max(currentTime, arrival_time)`
  * `completion = start + Cpu_burst`
  * `turnaround = completion - arrival_time`
  * `waiting = turnaround - Cpu_burst`

* Simulates CPU work by calling a helper that performs VM read/write per tick.

* Prints a table:

  ```text
  PID   Arrival   Burst   Start   Completion   Turnaround   Waiting
  ...
  ```

* Prints average turnaround and waiting times.

#### 3.2 Round Robin (preemptive)

`run_round_robin`:

* Uses a Ready queue and a fixed quantum (e.g., 5).
* Steps:

  * Admit processes whose `arrival_time <= elapsedTime` into Ready queue.
  * If Ready empty but more arrivals exist, jump time forward to the next arrival.
  * Dequeue a process, set `Running`, run up to `quantum` ticks or until `remaining_time` hits zero.
  * Each tick triggers a VM memory access (read or write).
  * If `remaining_time > 0` after the quantum, process is preempted back to Ready.
  * Otherwise, mark it `Terminated`, compute stats.
* Prints:

  * Transitions (`Ready → Running`, `Running → Ready`, `Running → Terminated`)
  * Per‑process metrics at the end.
  * Average turnaround and waiting times.

#### 3.3 SJF (Shortest Job First, non‑preemptive)

`run_sjf`:

* Maintains a Ready list of processes that have arrived.
* Whenever CPU is free:

  * If Ready empty but more arrivals exist, fast‑forward time to next arrival.
  * Else pick the Ready process with smallest `remaining_time` (equal to `Cpu_burst` here).
* Runs that process to completion:

  * Each tick triggers a VM memory access.
* Computes and prints per‑process stats and averages like FCFS.

---

### 4. Virtual Memory (`VirtualMemory.h`, `VirtualMemory.cpp`)

This module turns `memory_required` into a real virtual address space with paging.

**Configuration macros**

Defined in `VirtualMemory.h` (overridable before include):

* `VM_PAGE_SIZE` (default `256` bytes)
* `VM_PHYS_FRAMES` (default `8` frames)
* `VM_MAX_PROCESSES` (default `128`)

**Public API**

```cpp
void       vm_init(std::size_t total_frames = VM_PHYS_FRAMES);
void       vm_create_process(int pid, std::size_t memory_bytes);
std::uint8_t vm_read(int pid, std::size_t address);
void       vm_write(int pid, std::size_t address, std::uint8_t value);
void       vm_free_process(int pid);
void       vm_dump_state();
```

**What it simulates**

* **Per‑process virtual address space**

  * Divided into pages of size `VM_PAGE_SIZE`.
  * Number of pages = `ceil(memory_required / PAGE_SIZE)`.

* **Page tables**

  * Each process gets a `PageTable` with `PageTableEntry` array.
  * Each entry:

    * `frame_number`
    * `valid` bit
    * `dirty` bit

* **Physical memory**

  * Simulated array: `VM_PHYS_FRAMES * VM_PAGE_SIZE` bytes.
  * Track which frame is:

    * In use
    * Owned by which PID
    * Holds which virtual page

* **Backing store**

  * Per‑process array allocated with `calloc`.
  * Acts like a swap file.

* **Page faults and replacement**

  * On `vm_read` / `vm_write`:

    * Split address into `(page_number, offset)`.
    * If page not `valid`, trigger page fault.
    * Find a free frame, or evict a victim using **FIFO**:

      * If victim page is dirty, write it back to its backing store.
      * Mark victim’s PTE invalid.
    * Load required page from backing store into the chosen frame.
    * Update PTE and frame metadata.
  * `vm_write` sets the dirty bit.

Schedulers trigger this by calling a helper in `scheduler.cpp` that randomly reads or writes addresses in `[0, memory_required)` for the current PID each tick.

**Debug logging**

The VM prints:

* Page faults:

  * `VM: Page fault on read by process N, page P`
* Loads:

  * `VM: Loading page P of process N into frame F`
* Evictions:

  * `VM: Evicting page P of process N from frame F`
* Reads/writes:

  * `VM: Read Pn[addr=...] -> page, frame, offset = value`
  * `VM: Wrote Pn[addr=...] -> page, frame, offset = value`
* Cleanup:

  * `VM: Freed all memory for process N`
* Final dump via `vm_dump_state()`.

---

### 5. Main Driver (`main.cpp`)

`main.cpp` is the glue that makes it all feel like an OS.

Core steps:

1. Print boot banner.
2. Call `authenticateUser()` and exit on failure.
3. Call `vm_init()` to bring up virtual memory.
4. Generate a random number of processes (`1–10`):

   * Use `ProcessCreation(i)` from `scheduler.cpp`.
   * Immediately call `vm_create_process(pid, memory_required)` for each.
5. Sort processes by `arrival_time` and print their attributes.
6. Run:

   * `run_fcfs(processes);`
   * `run_round_robin(processes, quantumTime);`
   * `run_sjf(processes);`
7. After scheduling runs, free all processes’ VM state (`vm_free_process(pid)`).
8. Call `vm_dump_state()` to show final memory status.
9. Print shutdown message.

---

## Extending the Project

Some natural extensions:

* Add **SRTF** (Shortest Remaining Time First) scheduler.
* Use `IO_burst` and `io_milestone` fields to simulate actual I/O waits:

  * Move processes into `Waiting` state and back to `Ready`.
* Simulate **page fault delay** by:

  * Accounting extra “I/O time” in the scheduler when a fault occurs.
* Implement a different page replacement policy:

  * LRU, Clock, or variants.
* Add a **TLB** simulation:

  * Cache a small number of PTEs (pid + page → frame).

The current design keeps modules decoupled enough that you can evolve one subsystem (e.g., scheduler or VM) without completely rewriting the others.
