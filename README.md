# Operating System Simulator

## Team
- Cash Bailey  
- Alejandro Castineyra  
- Sylvia Ortega  
- Sarah Ramos Luna  

## Overview
**Operating System Simulator** is a compact, instructional OS environment.  
The current sample implements the boot sequence and a simple authentication flow against a fixed administrator account.  
Additional modules—scheduling, I/O, and memory management—are planned and partly scaffolded below. A standalone **Scheduling Module** (FCFS, SJF, SRTF) is included as an optional library you can build and run separately.

---

## Project Structure

```

.
├─ README.md
├─ main.cpp           # Entry point: boot sequence + authentication
├─ auth.h             # Declaration of authenticateUser()
├─ auth.cpp           # Implementation: prompts user and checks credentials
├─ process.h          # Process model (states, timing, I/O milestones)
└─ process.cpp        # (Planned) Process logic: constructors, transitions, accounting

# Optional scheduling library (add these files if using the scheduler)

# ├─ scheduler.h      # Public API for FCFS, SJF, SRTF

# └─ scheduler.cpp    # Implementation (+ demo main when compiled with -DSCHEDULER_DEMO)

````

---

## Build & Run

### Boot + Auth only
```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp auth.cpp -o simpleos
./simpleos
````

### With `process.cpp` (when implemented)

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp auth.cpp process.cpp -o simpleos
./simpleos
```

### Scheduling Module (library or demo)

**Build the library only** (for use in your own driver):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -c scheduler.cpp
# Link 'scheduler.o' with your own driver later
```

**Run the built‑in demo** (enables a demo `main()` inside `scheduler.cpp`):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic scheduler.cpp -o scheduler_demo -DSCHEDULER_DEMO
./scheduler_demo
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
* `burst_time`: total CPU time required
* `priority`: for priority scheduling (if used)
* `state`: current state

**Time Accounting**

* `remaining_time`: CPU time left
* `waiting_time`: total time spent READY
* `turnaround_time`: `completion_time - arrival_time`
* `completion_time`: time execution finished

**Memory & I/O**

* `memory_required`: estimated memory demand
* `io_milestones`: CPU‑time checkpoints that trigger I/O
* `io_count`: number of milestones
* `io_next_index`: next milestone index

**Execution Stats**

* `cpu_executed`: total CPU time consumed

---

## Scheduling Module (FCFS, SJF, SRTF)

The scheduling library provides:

* **FCFS** – First‑Come, First‑Served (non‑preemptive)
* **SJF** – Shortest Job First (non‑preemptive)
* **SRTF** – Shortest Remaining Time First (preemptive SJF)

**Public API (from `scheduler.h`)**

* Data: `Process`, `Segment` (Gantt slice), `Metrics`, `ScheduleResult`
* Functions:
  `scheduleFCFS`, `scheduleSJF`, `scheduleSRTF`, `runAll`, `printResults`, `printGantt`

**Input format (for your own drivers)**
Each line describes one process (space‑separated or CSV):

```
pid arrival burst
# or
pid,arrival,burst
```

Lines starting with `#` are ignored.

**Output (per algorithm)**

1. Per‑process metrics:

   * **CT** (Completion Time)
   * **TAT** (Turnaround Time) = `CT − arrival`
   * **WT** (Waiting Time) = `TAT − burst`
2. Averages: mean TAT and WT
3. Gantt segments (including `"IDLE"` gaps), printed as:
   `[start — PID — end]`

**Reference numbers (demo dataset: P1(0,8), P2(1,4), P3(2,9), P4(3,5))**

* FCFS — Avg TAT **15.25**, Avg WT **8.75**
* SJF  — Avg TAT **14.25**, Avg WT **7.75**
* SRTF — Avg TAT **13.00**, Avg WT **6.50**

**Deterministic tie‑breaking**
Order by arrival; if equal, by PID (lexicographic).
SJF/SRTF pick the shortest burst/remaining time, then break ties by arrival, then PID.

---

## Example Scheduler (Round‑Robin pseudocode)

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
* Add a basic scheduler with READY and WAITING queues.
* Simulate I/O devices with delays.
* Add CLI for selecting scheduling policies (FCFS, SJF, RR, Priority).
* Generate statistics for waiting and turnaround time.

---

## License / Attribution

Add a license file at the repository root (e.g., MIT) or follow your course’s required licensing.
