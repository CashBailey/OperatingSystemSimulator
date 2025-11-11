#include <iostream>
#include <vector>
#include <algorithm> // for sorting
#include <cstdlib>   // for rand()
#include <ctime>     // for time()
#include "scheduler.h"
using namespace std;

int main()
{
	srand(time(nullptr)); // seed randomness

	int random_number_of_processes = 1 + rand() % 10; // this generates a random number of processes from 1 - 10

	vector<Process> processes; // vector for the processes
	int quantumTime = 5;

	cout << "Number of processes generated: " << random_number_of_processes << endl;

	// Generates Process
	for (int i = 1; i <= random_number_of_processes; i++)
	{
		processes.push_back(ProcessCreation(i));
	}

	// Sort the processes by arrival time
	sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
	{
		return a.arrival_time < b.arrival_time;
	});

	// Print out the processes by arrival order (informational)
	for (int i = 0; i < (int)processes.size(); i++)
	{
		cout << "\nProcess " << processes[i].pid << " has arrived";
		cout << "\nPID = " << processes[i].pid
			<< "\nArrival = " << processes[i].arrival_time
			<< "\nCpu Burst = " << processes[i].Cpu_burst
			<< "\nPriority = " << processes[i].priority
			<< "\nMemory = " << processes[i].memory_required << endl;
	}

	// Run Round Robin (minimal-change behavior)
	run_round_robin(processes, quantumTime);

	// Run SJF (non-preemptive)
	run_sjf(processes);

	return 0;
}
