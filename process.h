#pragma once
#include <cstddef>

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

class Process {
public:
    // --- Core attributes ---
    int pid;                 // Unique process ID
    int arrival_time;        // Time process enters the system
    int burst_time;          // Total CPU time required
    int priority;            // Scheduling priority
    ProcessState state;      // Current process state

    // --- Time tracking ---
    int remaining_time;      // CPU time left for completion
    int waiting_time;        // Time spent waiting in READY queue
    int turnaround_time;     // Time from arrival to completion
    int completion_time;     // Time process finished execution

    // --- Memory and I/O ---
    int memory_required;     // Bytes or KB required by the process
    const int* io_milestones; // Array of I/O request milestones
    std::size_t io_count;    // Number of I/O milestones
    std::size_t io_next_index; // Index of the next I/O operation

    // --- Execution statistics ---
    int cpu_executed;        // Total CPU time executed so far

    // (No constructors or methods — attributes only)
};
