# Operating System Simulator

## Team
- Cash Bailey  
- Alejandro Castineyra  
- Sylvia Ortega  
- Sarah Ramos Luna  

## Overview
**Operating System Simulator** is a compact, instructional OS environment.  
The current setup focuses on **process scheduling** and includes three algorithms wired into `main.cpp`:
- **FCFS** (First‑Come, First‑Served, non‑preemptive)
- **Round Robin** (preemptive with time quantum)
- **SJF** (Shortest Job First, non‑preemptive)

A boot/auth sample remains available in the repository and can be built separately. Additional modules—scheduling extensions, I/O, and memory management—are planned and partly scaffolded.

---

## Project Structure

```

.
├─ README.md
├─ main.cpp           # Entry: generates processes and runs FCFS, RR, and SJF
├─ scheduler.h        # Process model + scheduler API (FCFS, RR, SJF)
├─ scheduler.cpp      # Implementations for FCFS + RR + SJF and ProcessCreation()
├─ auth.h             # Declaration of authenticateUser()     (available, not used by current main)
├─ auth.cpp           # Implementation: prompts user & checks (available, not used by current main)
├─ process.h          # Process model (states, timing, I/O milestones)
└─ process.cpp        # (Planned) Process logic: constructors, transitions, accounting

````

> Note: The current `main.cpp` runs the scheduling demo (FCFS, Round Robin, SJF).  
> The original boot/auth sample remains in the repo and can be built separately.

---

## Build & Run

### Scheduling (current setup: FCFS + Round Robin + SJF)
```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp scheduler.cpp -o scheduler_demo
./scheduler_demo
````

### Boot + Auth only (uses the original boot/auth driver)

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp auth.cpp -o simpleos
./simpleos
```

### With `process.cpp` (when implemented)

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp auth.cpp process.cpp -o simpleos
./simpleos
```

### Build just the scheduling objects (for your own driver)

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -c scheduler.cpp
# Link scheduler.o with your custom driver later
```

---

## Running the Boot/Auth Sample

When prompted, use these credentials:

* **Username:** `admin`
* **Password:** `password123`

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

Each program is a `Process` with states and accounting fields.

**States**
`NEW → READY → RUNNING → WAITING → TERMINATED`

**Core Attributes**

* `pid`: unique process ID
* `arrival_time`: time the process enters the system
* `Cpu_burst`: total CPU time required
* `priority`: for priority scheduling (if used)
* `state`: current state

**Time Accounting**

* `remaining_time`: CPU time left
* `waiting_time`: total time spent READY
* `turnaround_time`: `completion_time - arrival_time`
* `completion_time`: time execution finished

**Memory & I/O**

* `memory_required`: estimated memory demand
* `io_milestone`: CPU-time checkpoints that trigger I/O (pointer)
* `io_count`: number of milestones
* `io_next_index`: next milestone index

**Execution Stats**

* `cpu_executed`: total CPU time consumed

---

## Scheduling Module (FCFS, Round Robin, SJF)

### What `main.cpp` does

1. Generates a random number of processes (1–10) with randomized attributes via `ProcessCreation(int pid)`.
2. Sorts processes by `arrival_time` and prints their attributes.
3. Executes **all three** schedulers in sequence:

   * `run_fcfs(processes);`
   * `run_round_robin(processes, quantumTime);`
   * `run_sjf(processes);`

### Algorithms

#### FCFS (First‑Come, First‑Served, non‑preemptive)

Implements the exact FCFS logic:

1. Sort by `arrival_time`.
2. For each process in arrival order:

   * `start = max(currentTime, arrival_time)`
   * `completion = start + Cpu_burst`
   * `turnaround = completion - arrival_time`
   * `waiting = turnaround - Cpu_burst`
3. Prints a tabular summary:

```
PID    Arrival  Burst  Start  Completion  Turnaround  Waiting
...    ...      ...    ...    ...         ...         ...
```

Also prints **Average Turnaround Time** and **Average Waiting Time**.

#### Round Robin (preemptive, arrival‑aware)

* Uses a ready queue and a fixed time quantum.
* If the CPU becomes idle and there are future arrivals, time fast‑forwards to the next arrival.
* Prints transitions and a per‑process summary with averages.
* Note: The log message string contains “quantam” to match the current code output.

#### SJF (Shortest Job First, non‑preemptive, arrival‑aware)

* Among the processes that have arrived, picks the smallest `remaining_time` (equal to `Cpu_burst` for non‑preemptive SJF).
* Runs that process to completion.
* Prints per‑process details and averages.

### Public API (`scheduler.h`)

```cpp
Process ProcessCreation(int pid);

void run_fcfs(const std::vector<Process>& processes);
void run_round_robin(const std::vector<Process>& processes, int quantumTime);
void run_sjf(const std::vector<Process>& processes);
```

### Output (per algorithm)

1. **Per‑process metrics**

   * **Completion Time (CT)**
   * **Turnaround Time (TAT) = CT − arrival_time**
   * **Waiting Time (WT) = TAT − Cpu_burst**
2. **Averages**: mean TAT and WT
3. **Logs**: state transitions and scheduling decisions (RR/SJF)

### Deterministic tie‑breaking

* Sorted by `arrival_time`; if equal, by `pid`.

---

## Example Round‑Robin Pseudocode

```cpp
while (terminated.size() < processes.size()) {
  // admit arrivals up to 'now'
  while (next_to_arrive < N && proc[next_to_arrive].arrival_time <= now) {
    ready.push(proc[next_to_arrive++]);
  }

  // if CPU idle, jump to next arrival
  if (ready.empty()) {
    if (next_to_arrive < N) now = proc[next_to_arrive].arrival_time;
    else break;
    continue;
  }

  auto p = ready.front(); ready.erase(ready.begin());
  int run_time = min(QUANTUM, p.remaining_time);
  now += run_time;
  p.remaining_time -= run_time;

  if (p.remaining_time > 0) ready.push_back(p);
  else record_completion(p, now);
}
```

---

## Roadmap

* Add **SRTF** (Shortest Remaining Time First).
* Implement `process.cpp` fully and integrate I/O wait queues.
* Add CLI flags for policy selection and quantum configuration.
* Export CSV summaries for metrics.

---

## License / Attribution

Add a license file at the repository root (e.g., MIT) or follow your course’s required licensing.
