// process.h
#pragma once
#include <cstddef>   // for size_t

// Process states exactly as in your original code
enum class ProcessState
{
    New,
    Ready,
    Running,
    Waiting,
    Terminated
};

class Process
{
public:
    // ---------------- Core Attributes ----------------
    int pid;             // Unique process ID
    int arrival_time;    // Time process enters the system
    int Cpu_burst;       // Total CPU time required
    int IO_burst;        // Total I/O time required
    int priority;        // Scheduling priority
    ProcessState state;  // Current process state

    // ---------------- Time Tracking ----------------
    int remaining_time;   // CPU time left for completion
    int waiting_time;     // Time spent waiting in Ready queue
    int turnaround_time;  // Time from arrival to completion
    int completion_time;  // Time process finished execution

    // ---------------- Memory and I/O ----------------
    int memory_required;       // Bytes or KB required by the process
    const int* io_milestone;   // Array of I/O request milestones (optional usage)
    size_t io_count;           // Number of I/O milestones
    size_t io_next_index;      // Index of the next I/O operation

    // ---------------- Execution Statistics ----------------
    int cpu_executed;          // Total CPU ticks executed so far

    // ---------------- Constructor ----------------
    Process(int pid, int arrival, int cpu, int io, int prio, int mem);

    // ---------------- Behaviors ----------------
    void set_state(ProcessState new_state);
    // Execute up to 'quanta' CPU ticks at time 'now' if state==RUNNING.
    // Returns the number of ticks actually executed.
    int tick_cpu(int now, int quanta = 1);

    // Accumulate waiting time when in READY state.
    void add_wait(int dt);

    // Simple I/O transitions (your scheduler can call these).
    void start_io();   // READY/RUNNING -> WAITING
    void finish_io();  // WAITING -> READY

    // Convenience helpers
    bool is_complete() const;          // true if TERMINATED
    const char* state_cstr() const;    // string view of current state
};
