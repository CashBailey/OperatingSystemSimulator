// scheduler.cpp
#include "scheduler.h"
#include "VirtualMemory.h"

#include <iostream>
#include <vector>
#include <algorithm> // for sorting/scanning
#include <cstdlib>   // for rand()
#include <ctime>     // for time()
#include <iomanip>

using namespace std;

// Helper: simulate a single memory access for a process.
// Randomly picks an address in [0, memory_required) and does either a read or write.
static void simulate_memory_access(const Process& p)
{
    if (p.memory_required <= 0) {
        return;
    }

    // Pick a random virtual address within the process's memory
    std::size_t addr = static_cast<std::size_t>(rand() % p.memory_required);

    int rw = rand() % 2; // 0 = read, 1 = write
    if (rw == 0) {
        // Read and discard value (we just want the VM activity)
        (void) vm_read(p.pid, addr);
    } else {
        // Write some deterministic byte based on pid + address
        std::uint8_t value = static_cast<std::uint8_t>((addr + p.pid) & 0xFFu);
        vm_write(p.pid, addr, value);
    }
}

Process ProcessCreation(int pid)
{
    int arrival = rand() % 10;         // random 0 - 9
    int cpu     = 5 + rand() % 15;     // random 5 - 19
    int io      = rand() % 5;          // random 0 - 4
    int prio    = 1 + rand() % 3;      // Priority 1 - 3
    int mem     = 256 + rand() % 1024; // random 256 - 1279 bytes (or KB per your interpretation)

    return Process(pid, arrival, cpu, io, prio, mem);
}

// ---------------------- FCFS (non-preemptive, your logic) ----------------------
void run_fcfs(const vector<Process>& processes_in)
{
    // Work on a local copy so we don't mutate the caller's data
    vector<Process> processes = processes_in;

    // 1) Sort by arrival time (like: sort(..., compareArrival))
    sort(processes.begin(), processes.end(),
        [](const Process& a, const Process& b) {
            if (a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
            return a.pid < b.pid; // stable tie-breaker
        });

    // 2) Run to completion in arrival order, computing metrics
    int currentTime = 0;
    float totalTAT = 0.0f, totalWT = 0.0f;

    cout << "\n=== FCFS Execution (non-preemptive) ===\n";
    cout << "PID\tArrival\tBurst\tStart\tCompletion\tTurnaround\tWaiting\n";

    for (auto &p : processes)
    {
        // start = max(currentTime, arrival)
        int start = (currentTime > p.arrival_time) ? currentTime : p.arrival_time;

        // completion = start + burst
        int completion = start + p.Cpu_burst;

        // turnaround = completion - arrival
        int turnaround = completion - p.arrival_time;

        // waiting = turnaround - burst
        int waiting = turnaround - p.Cpu_burst;

        // Simulate CPU execution and memory usage for this process
        for (int tick = 0; tick < p.Cpu_burst; ++tick) {
            simulate_memory_access(p);
        }

        // Advance "CPU time"
        currentTime = completion;

        // Update stored accounting
        p.remaining_time   = 0;
        p.completion_time  = completion;
        p.turnaround_time  = turnaround;
        p.waiting_time     = waiting;
        p.state            = ProcessState::Terminated;

        totalTAT += turnaround;
        totalWT  += waiting;

        cout << "P" << p.pid << "\t"
             << p.arrival_time << "\t"
             << p.Cpu_burst << "\t"
             << start << "\t"
             << completion << "\t\t"
             << turnaround << "\t\t"
             << waiting << "\n";
    }

    int n = static_cast<int>(processes.size());
    cout << fixed << setprecision(2);
    if (n > 0)
    {
        cout << "\nAverage Turnaround Time: " << (totalTAT / n) << endl;
        cout << "Average Waiting Time: "   << (totalWT  / n) << endl;
    }
    cout << endl;
}

// ---------------------- Round Robin (preemptive) ----------------------
void run_round_robin(const vector<Process>& processes_in, int quantumTime)
{
    vector<Process> processes = processes_in; // local copy so we can mutate
    vector<Process> Ready;                    // ready queue
    vector<Process> Execution;                // executing processes (for prints)
    vector<Process> Terminated;               // terminated results

    // processes should already be sorted by arrival in main, but keep behavior consistent
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });

    int elapsedTime = 0;
    size_t next_to_arrive = 0;

    cout << "\n=== Round Robin Execution (quantum = " << quantumTime << ") ===\n";

    // Main loop
    while (Terminated.size() < processes.size())
    {
        // Enqueue arrivals up to current time
        while (next_to_arrive < processes.size() && processes[next_to_arrive].arrival_time <= elapsedTime)
        {
            processes[next_to_arrive].state = ProcessState::Ready;
            Ready.push_back(processes[next_to_arrive]);
            ++next_to_arrive;
        }

        // If CPU idle, fast-forward to the next arrival
        if (Ready.empty())
        {
            if (next_to_arrive < processes.size())
            {
                elapsedTime = processes[next_to_arrive].arrival_time;
                continue;
            }
            else
            {
                break;
            }
        }

        // Get first ready
        Process current = Ready.front();
        Ready.erase(Ready.begin());
        current.state = ProcessState::Running;

        Execution.push_back(current);
        cout << "\nProcess " << current.pid << " moved from Ready to Running.\n";

        int timeSlice = min(quantumTime, current.remaining_time);

        // Simulate each tick within the time slice, with a memory access
        for (int t = 0; t < timeSlice; ++t) {
            simulate_memory_access(current);
        }

        current.remaining_time -= timeSlice;
        elapsedTime += timeSlice;

        cout << "Process " << current.pid << " executed for " << timeSlice
             << " quantam bursts and now has " << current.remaining_time
             << " CPU bursts\n";

        Execution.pop_back();

        if (current.remaining_time > 0)
        {
            current.state = ProcessState::Ready;
            Ready.push_back(current);
            cout << "Process " << current.pid << " preempted and moved from Running to Ready.\n";
        }
        else
        {
            current.state = ProcessState::Terminated;
            current.completion_time = elapsedTime;
            current.turnaround_time = current.completion_time - current.arrival_time;
            current.waiting_time = current.turnaround_time - current.Cpu_burst;
            Terminated.push_back(current);
            cout << "Process " << current.pid << " completed and moved to Terminated queue.\n";
        }
    }

    // Summary
    double totalTurnaround = 0.0;
    double totalWaiting = 0.0;

    cout << "\nRound Robin has been completed!!!\n";
    for (int i = 0; i < (int)Terminated.size(); i++)
    {
        cout << "\nPID = " << Terminated[i].pid
             << "\nArrival = " << Terminated[i].arrival_time
             << "\nCompletion = " << Terminated[i].completion_time
             << "\nTurnaround = " << Terminated[i].turnaround_time
             << "\nWaiting = " << Terminated[i].waiting_time << endl;

        totalTurnaround += Terminated[i].turnaround_time;
        totalWaiting += Terminated[i].waiting_time;
    }

    cout << fixed << setprecision(2);
    if (!Terminated.empty())
    {
        double avgTurnaround = totalTurnaround / Terminated.size();
        double avgWaiting = totalWaiting / Terminated.size();
        cout << "\nAverage Turnaround Time = " << avgTurnaround << endl;
        cout << "Average Waiting Time = " << avgWaiting << endl;
    }
    cout << endl;
}

// ---------------------- Non-preemptive SJF (arrival-aware) ----------------------
void run_sjf(const vector<Process>& processes_in)
{
    vector<Process> processes = processes_in; // local copy so we can mutate
    vector<Process> Ready;                    // ready queue (we will pick shortest)
    vector<Process> Execution;                // executing processes (for prints)
    vector<Process> Terminated;               // terminated results

    // Ensure arrival order
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });

    int elapsedTime = 0;
    size_t next_to_arrive = 0;

    cout << "\n=== SJF Execution (non-preemptive) ===\n";

    while (Terminated.size() < processes.size())
    {
        // Enqueue arrivals up to current time
        while (next_to_arrive < processes.size() && processes[next_to_arrive].arrival_time <= elapsedTime)
        {
            processes[next_to_arrive].state = ProcessState::Ready;
            Ready.push_back(processes[next_to_arrive]);
            ++next_to_arrive;
        }

        // If nothing ready, jump to next arrival
        if (Ready.empty())
        {
            if (next_to_arrive < processes.size())
            {
                elapsedTime = processes[next_to_arrive].arrival_time;
                continue;
            }
            else
            {
                break;
            }
        }

        // Pick the process with the shortest remaining_time (non-preemptive)
        int best = 0;
        for (int i = 1; i < (int)Ready.size(); i++)
        {
            if (Ready[i].remaining_time < Ready[best].remaining_time)
            {
                best = i;
            }
        }

        Process current = Ready[best];
        Ready.erase(Ready.begin() + best);
        current.state = ProcessState::Running;

        Execution.push_back(current);
        cout << "\nProcess " << current.pid << " moved from Ready to Running.\n";

        int run_time = current.remaining_time; // run to completion in SJF (non-preemptive)

        // Simulate CPU ticks and memory access for this process
        for (int t = 0; t < run_time; ++t) {
            simulate_memory_access(current);
        }

        current.remaining_time -= run_time;
        elapsedTime += run_time;

        cout << "Process " << current.pid << " executed for " << run_time
             << " bursts and now has " << current.remaining_time
             << " CPU bursts\n";

        Execution.pop_back();

        // Completed
        current.state = ProcessState::Terminated;
        current.completion_time = elapsedTime;
        current.turnaround_time = current.completion_time - current.arrival_time;
        current.waiting_time = current.turnaround_time - current.Cpu_burst;
        Terminated.push_back(current);
        cout << "Process " << current.pid << " completed and moved to Terminated queue.\n";
    }

    // Summary
    double totalTurnaround = 0.0;
    double totalWaiting = 0.0;

    cout << "\nSJF has been completed!!!\n";
    for (int i = 0; i < (int)Terminated.size(); i++)
    {
        cout << "\nPID = " << Terminated[i].pid
             << "\nArrival = " << Terminated[i].arrival_time
             << "\nCompletion = " << Terminated[i].completion_time
             << "\nTurnaround = " << Terminated[i].turnaround_time
             << "\nWaiting = " << Terminated[i].waiting_time << endl;

        totalTurnaround += Terminated[i].turnaround_time;
        totalWaiting += Terminated[i].waiting_time;
    }

    cout << fixed << setprecision(2);
    if (!Terminated.empty())
    {
        double avgTurnaround = totalTurnaround / Terminated.size();
        double avgWaiting = totalWaiting / Terminated.size();
        cout << "\nAverage Turnaround Time = " << avgTurnaround << endl;
        cout << "Average Waiting Time = " << avgWaiting << endl;
    }
    cout << endl;
}
