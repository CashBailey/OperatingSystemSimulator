#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <string>
#include <vector>

namespace sched {

/** Process description */
struct Process {
    std::string pid;  // e.g., "P1"
    double arrival;   // arrival time
    double burst;     // CPU burst time (execution time), must be >= 0
};

/** Gantt chart segment */
struct Segment {
    std::string pid;  // running process id or "IDLE"
    double start;
    double end;
};

/** Per‑process computed metrics */
struct Metrics {
    std::string pid;
    double arrival;
    double burst;
    double completion;
    double turnaround;
    double waiting;
};

/** Result for one algorithm */
struct ScheduleResult {
    std::string name;
    std::vector<Segment> gantt;
    std::vector<Metrics> results;
    double avg_turnaround = 0.0;
    double avg_waiting    = 0.0;
};

/** Simulators */
ScheduleResult scheduleFCFS(const std::vector<Process>& processes);
ScheduleResult scheduleSJF (const std::vector<Process>& processes);     // non‑preemptive
ScheduleResult scheduleSRTF(const std::vector<Process>& processes);     // preemptive SJF

/** Convenience: run all three and return in order FCFS, SJF, SRTF */
std::vector<ScheduleResult> runAll(const std::vector<Process>& processes);

/** Pretty printers (optional) */
void printResults(const ScheduleResult& r);
void printGantt  (const ScheduleResult& r);

}  // namespace sched

#endif  // SCHEDULER_H_
