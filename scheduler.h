#pragma once

#include <vector>
#include <cstddef>
using namespace std;

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
	// Core Attributes
	int pid;               // Unique process ID
	int arrival_time;      // Time process enters the system
	int Cpu_burst;         // Total CPU time required
	int IO_burst;          // Total I/O time required
	int priority;          // Scheduling priority
	ProcessState state;    // Current process state

	// Time Tracking
	int remaining_time;    // CPU time left for completion
	int waiting_time;      // Time spent waiting in Ready queue
	int turnaround_time;   // Time from arrival to completion
	int completion_time;   // Time process finished execution
	int average_turnaround;    // average time from arrival to completion
	int average_waitingtime;   // average time from time spent waiting in Ready

	// Memory and I/O
	int memory_required;       // Bytes or KB required by the process
	const int* io_milestone;   // Array of I/O request milestones
	size_t io_count;           // Number of I/O milestones
	size_t io_next_index;      // Index of the next I/O operation

	// Execution Statistics
	int cpu_executed;

	// constructor
	Process(int pid, int arrival, int cpu, int io, int prio, int mem)
		: pid(pid), arrival_time(arrival), Cpu_burst(cpu), IO_burst(io), priority(prio),
		  state(ProcessState::New), remaining_time(cpu), waiting_time(0),
		  turnaround_time(0), completion_time(0), average_turnaround(0),
		  average_waitingtime(0), memory_required(mem), io_milestone(nullptr),
		  io_count(0), io_next_index(0), cpu_executed(0) { }
};

// Initializes a process with random values
Process ProcessCreation(int pid);

// Scheduling runners (each prints its own timeline and summary)
void run_round_robin(const vector<Process>& processes, int quantumTime);
void run_sjf(const vector<Process>& processes);
