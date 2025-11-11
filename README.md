# Operating System Simulator

## Team
- Cash Bailey  
- Alejandro Castineyra  
- Sylvia Ortega  
- Sarah Ramos Luna  

## Overview
**Operating System Simulator** is a compact, instructional OS environment.  
The current sample implements the boot sequence and a simple authentication flow against a fixed administrator account.  
Additional modules—scheduling, I/O, and memory management—are planned and partly scaffolded below. A standalone **Scheduling Module** (**Round Robin** and **SJF**) is included and wired through the current `main.cpp`. FCFS and SRTF are planned.

---

## Project Structure

```

.
├─ README.md
├─ main.cpp           # Current entry: scheduler demo driver (Round Robin + SJF)
├─ scheduler.h        # Process model + scheduler API (RR + SJF)
├─ scheduler.cpp      # Implementations for RR + SJF and ProcessCreation()
├─ auth.h             # Declaration of authenticateUser()     (available, not used by current main)
├─ auth.cpp           # Implementation: prompts user & checks (available, not used by current main)
├─ process.h          # Process model (states, timing, I/O milestones)
└─ process.cpp        # (Planned) Process logic: constructors, transitions, accounting

````

> Note: The current `main.cpp` runs the scheduling demo. The original boot/auth sample remains in the repo and can be built separately (see below).

---

## Build & Run

### Scheduling (current setup: Round Robin + SJF)
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

## Scheduling Module (Round Robin, SJF)

The current scheduling code provides:

* **Round Robin (arrival-aware)** — time-quantum based, with idle fast-forward when Ready is empty.
* **SJF (non-preemptive, arrival-aware)** — picks the process with the shortest remaining time (equal to `Cpu_burst` here) and runs it to completion.

**Public API (from `scheduler.h`)**

* `Process ProcessCreation(int pid);`
* `void run_round_robin(const vector<Process>& processes, int quantumTime);`
* `void run_sjf(const vector<Process>& processes);`

`main.cpp` generates a random set of processes, prints them in arrival order, and then invokes both schedulers.

**Output (per algorithm)**

1. Per-process metrics:

   * **Completion Time (CT)**
   * **Turnaround Time (TAT) = CT − arrival_time**
   * **Waiting Time (WT) = TAT − Cpu_burst**
2. Averages: mean TAT and WT
3. Timeline logs for state transitions

**Deterministic tie-breaking**

* Sorted by arrival time; if equal, by PID.

> Note: The Round Robin log prints “quantam” to match the current code’s `cout` string.

---

## Example Scheduler (Round-Robin pseudocode)

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

  auto p = ready.front(); ready.pop_front();
  run_time = min(QUANTUM, p.remaining_time);
  now += run_time;
  p.remaining_time -= run_time;

  if (p.remaining_time > 0) ready.push_back(p);
  else record_completion(p, now);
}
```

---

## Roadmap

* Add **FCFS** and **SRTF** implementations matching the current style.
* Implement `process.cpp` fully and integrate I/O wait queues.
* Add CLI flags for policy selection and quantum configuration.
* Generate and export CSV summaries for metrics.

---

## License / Attribution

Add a license file at the repository root (e.g., MIT) or follow your course’s required licensing.


