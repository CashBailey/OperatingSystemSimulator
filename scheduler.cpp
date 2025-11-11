#include "scheduler.h"

#include <iostream>
#include <vector>
#include <algorithm> // for sorting/scanning
#include <cstdlib>   // for rand()
#include <ctime>     // for time()
#include <iomanip>
using namespace std;

Process ProcessCreation(int pid)
{
	int arrival = rand() % 10;         // random 0 - 9
	int cpu = 5 + rand() % 15;         // random 5 - 19
	int io = rand() % 5;               // random 0 - 4
	int prio = 1 + rand() % 3;         // Priority 1 - 3
	int mem = 256 + rand() % 1024;     // random 256 - 1279 KB
	return Process(pid, arrival, cpu, io, prio, mem);
}

// ---------------------- Round Robin (minimal-change version) ----------------------
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
