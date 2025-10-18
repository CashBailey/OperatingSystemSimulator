// process.cpp
#include "process.h"
#include <algorithm> // for std::min

Process::Process(int pid_, int arrival, int cpu, int io, int prio, int mem)
    : pid(pid_),
      arrival_time(arrival),
      Cpu_burst(cpu),
      IO_burst(io),
      priority(prio),
      state(ProcessState::New),
      remaining_time(cpu),
      waiting_time(0),
      turnaround_time(0),
      completion_time(0),
      memory_required(mem),
      io_milestone(nullptr),
      io_count(0),
      io_next_index(0),
      cpu_executed(0)
{
}

void Process::set_state(ProcessState new_state)
{
    state = new_state;
}

int Process::tick_cpu(int now, int quanta)
{
    if (state != ProcessState::Running || remaining_time <= 0 || quanta <= 0)
        return 0;

    const int executed = std::min(remaining_time, quanta);
    remaining_time -= executed;
    cpu_executed += executed;

    if (remaining_time == 0) {
        completion_time = now + executed;
        turnaround_time = completion_time - arrival_time;
        state = ProcessState::Terminated;
    }
    return executed;
}

void Process::add_wait(int dt)
{
    if (state == ProcessState::Ready && dt > 0)
        waiting_time += dt;
}

void Process::start_io()
{
    // Typically called by a scheduler to move a running/ready process to WAITING.
    state = ProcessState::Waiting;
}

void Process::finish_io()
{
    // Typically called when I/O completes to return to READY.
    state = ProcessState::Ready;
}

bool Process::is_complete() const
{
    return state == ProcessState::Terminated;
}

const char* Process::state_cstr() const
{
    switch (state) {
        case ProcessState::New:        return "New";
        case ProcessState::Ready:      return "Ready";
        case ProcessState::Running:    return "Running";
        case ProcessState::Waiting:    return "Waiting";
        case ProcessState::Terminated: return "Terminated";
    }
    return "?";
}
