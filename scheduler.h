// scheduler.h
#pragma once

#include <vector>
#include "process.h"

// Initializes a process with random values (arrival, CPU burst, IO burst, priority, memory).
Process ProcessCreation(int pid);

// === Scheduling runners (each prints its own timeline and summary) ===
void run_fcfs(const std::vector<Process>& processes);               // FCFS using your snippet's logic
void run_round_robin(const std::vector<Process>& processes, int quantumTime);
void run_sjf(const std::vector<Process>& processes);
