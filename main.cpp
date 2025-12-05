// main.cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

#include "auth.h"
#include "process.h"
#include "scheduler.h"
#include "VirtualMemory.h"

using namespace std;

int main()
{
    cout << "Booting Operating System Simulator..." << endl;

    // ---------- Phase 1: Authentication ----------
    if (!authenticateUser()) {
        cout << "Authentication failed. System shutting down." << endl;
        return 1;
    }

    cout << "Login successful. Welcome!" << endl;

    // ---------- Phase 2: Initialize subsystems ----------
    srand(static_cast<unsigned>(time(nullptr)));  // seed randomness for the whole sim

    // Initialize virtual memory (uses compile-time VM_PHYS_FRAMES / VM_PAGE_SIZE)
    vm_init();

    // ---------- Phase 3: Process creation ----------
    int random_number_of_processes = 1 + rand() % 10; // 1–10 processes

    vector<Process> processes;
    processes.reserve(random_number_of_processes);

    int quantumTime = 5;

    cout << "Number of processes generated: " << random_number_of_processes << endl;

    for (int i = 1; i <= random_number_of_processes; ++i)
    {
        // Create the process (arrival, CPU, IO, priority, memory)
        Process p = ProcessCreation(i);
        processes.push_back(p);

        // Register this process with the virtual memory subsystem
        vm_create_process(p.pid, static_cast<size_t>(p.memory_required));
    }

    // Sort the processes by arrival time (tie-breaker by pid)
    sort(processes.begin(), processes.end(),
         [](const Process &a, const Process &b)
         {
             if (a.arrival_time != b.arrival_time)
                 return a.arrival_time < b.arrival_time;
             return a.pid < b.pid;
         });

    // Print out the processes by arrival order (informational)
    for (const auto& p : processes)
    {
        cout << "\nProcess " << p.pid << " has arrived";
        cout << "\nPID = " << p.pid
             << "\nArrival = " << p.arrival_time
             << "\nCpu Burst = " << p.Cpu_burst
             << "\nPriority = " << p.priority
             << "\nMemory = " << p.memory_required << " bytes"
             << endl;
    }

    // ---------- Phase 4: Scheduling (all three algorithms) ----------
    run_fcfs(processes);                 // First-Come, First-Served
    run_round_robin(processes, quantumTime); // Round Robin
    run_sjf(processes);                  // Shortest Job First

    // ---------- Phase 5: Cleanup / shutdown ----------
    // Free per-process VM structures
    for (const auto& p : processes) {
        vm_free_process(p.pid);
    }

    // Optional: dump final VM state
    vm_dump_state();

    cout << "Operating System Simulator shutting down." << endl;
    return 0;
}
