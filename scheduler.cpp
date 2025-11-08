#include "scheduler.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <tuple>
#include <unordered_map>

namespace sched {

namespace {
constexpr double EPS = 1e-12;

inline bool isZero(double x) { return std::abs(x) <= EPS; }

void appendSegment(std::vector<Segment>& g, const std::string& pid, double start, double end) {
    if (end - start <= EPS) return; // ignore zero‑length slices
    if (!g.empty() && g.back().pid == pid && std::abs(g.back().end - start) <= EPS) {
        g.back().end = end;
    } else {
        g.push_back(Segment{pid, start, end});
    }
}

void validate(const std::vector<Process>& ps) {
    for (const auto& p : ps) {
        if (p.burst < -EPS) {
            throw std::invalid_argument("Burst time must be >= 0 for process " + p.pid);
        }
    }
}

ScheduleResult finalize(const std::string& name,
                        const std::vector<Process>& ps,
                        const std::vector<Segment>& gantt,
                        const std::unordered_map<std::string, double>& completion) {
    ScheduleResult out;
    out.name  = name;
    out.gantt = gantt;

    double sum_tat = 0.0, sum_wt = 0.0;
    out.results.reserve(ps.size());
    for (const auto& p : ps) {
        auto it = completion.find(p.pid);
        if (it == completion.end()) {
            throw std::runtime_error("Internal error: missing completion for " + p.pid);
        }
        double ct  = it->second;
        double tat = ct - p.arrival;
        double wt  = tat - p.burst;

        out.results.push_back(Metrics{
            p.pid, p.arrival, p.burst, ct, tat, wt
        });
        sum_tat += tat;
        sum_wt  += wt;
    }
    const double n = ps.empty() ? 1.0 : static_cast<double>(ps.size());
    out.avg_turnaround = sum_tat / n;
    out.avg_waiting    = sum_wt  / n;
    return out;
}

// Deterministic tie‑breaking: arrival ascending, then pid lexicographic
std::vector<Process> sortedByArrival(const std::vector<Process>& processes) {
    std::vector<Process> v = processes;
    std::sort(v.begin(), v.end(), [](const Process& a, const Process& b) {
        if (std::abs(a.arrival - b.arrival) > EPS) return a.arrival < b.arrival;
        return a.pid < b.pid;
    });
    return v;
}

} // namespace

// -------------------- FCFS --------------------

ScheduleResult scheduleFCFS(const std::vector<Process>& processes) {
    validate(processes);
    std::vector<Process> procs = sortedByArrival(processes);

    double time = 0.0;
    std::vector<Segment> gantt;
    std::unordered_map<std::string, double> completion;

    for (const auto& p : procs) {
        if (time + EPS < p.arrival) {
            appendSegment(gantt, "IDLE", time, p.arrival);
            time = p.arrival;
        }
        double start = time;
        double end   = time + p.burst;
        appendSegment(gantt, p.pid, start, end);
        time = end;
        completion[p.pid] = end;
    }
    return finalize("FCFS", processes, gantt, completion);
}

// -------------------- SJF (non‑preemptive) --------------------

ScheduleResult scheduleSJF(const std::vector<Process>& processes) {
    validate(processes);
    std::vector<Process> rem = sortedByArrival(processes);
    const std::size_t n = rem.size();
    std::size_t i = 0;

    double time = 0.0;
    std::vector<Segment> gantt;
    std::unordered_map<std::string, double> completion;

    using Entry = std::tuple<double,double,std::string,const Process*>; // (burst, arrival, pid, ptr)
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;

    auto pushArrived = [&] {
        while (i < n && rem[i].arrival <= time + EPS) {
            const Process* pp = &rem[i];
            pq.emplace(pp->burst, pp->arrival, pp->pid, pp);
            ++i;
        }
    };

    if (i < n && time + EPS < rem[i].arrival) {
        appendSegment(gantt, "IDLE", time, rem[i].arrival);
        time = rem[i].arrival;
    }
    pushArrived();

    while (!pq.empty() || i < n) {
        if (pq.empty()) {
            double next_at = rem[i].arrival;
            if (time + EPS < next_at) {
                appendSegment(gantt, "IDLE", time, next_at);
                time = next_at;
            }
            pushArrived();
            continue;
        }
        auto [bt, at, pid, pp] = pq.top(); pq.pop();
        double start = time;
        double end   = time + bt;
        appendSegment(gantt, pid, start, end);
        time = end;
        completion[pid] = end;
        pushArrived();
    }

    return finalize("SJF", processes, gantt, completion);
}

// -------------------- SRTF (preemptive SJF) --------------------

ScheduleResult scheduleSRTF(const std::vector<Process>& processes) {
    validate(processes);
    std::vector<Process> rem = sortedByArrival(processes);
    const std::size_t n = rem.size();
    std::size_t i = 0;

    double time = 0.0;
    std::vector<Segment> gantt;
    std::unordered_map<std::string, double> completion;

    // remaining times (by pid)
    std::unordered_map<std::string, double> remaining;
    remaining.reserve(n);
    for (const auto& p : processes) remaining[p.pid] = std::max(0.0, p.burst);

    using Entry = std::tuple<double,double,std::string,const Process*>; // (remaining, arrival, pid, ptr)
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;

    auto pushArrived = [&] {
        while (i < n && rem[i].arrival <= time + EPS) {
            const Process* pp = &rem[i];
            pq.emplace(remaining[pp->pid], pp->arrival, pp->pid, pp);
            ++i;
        }
    };

    // Ensure we start at the first arrival (with idle if needed)
    if (i < n && time + EPS < rem[i].arrival) {
        appendSegment(gantt, "IDLE", time, rem[i].arrival);
        time = rem[i].arrival;
    }
    pushArrived();

    while (!pq.empty() || i < n) {
        if (pq.empty()) {
            double next_at = rem[i].arrival;
            if (time + EPS < next_at) {
                appendSegment(gantt, "IDLE", time, next_at);
                time = next_at;
            }
            pushArrived();
            continue;
        }

        auto [r, at, pid, pp] = pq.top(); pq.pop();

        // time to next arrival
        double next_at = (i < n) ? rem[i].arrival : std::numeric_limits<double>::infinity();
        double finish_if_uninterrupted = time + remaining[pid];
        double run_until = std::min(finish_if_uninterrupted, next_at);

        double start = time;
        double end   = run_until;
        appendSegment(gantt, pid, start, end);

        double ran = end - start;
        remaining[pid] -= ran;
        time = end;

        // add newly arrived processes at 'time'
        pushArrived();

        if (remaining[pid] > EPS) {
            // not done; reinsert with its new remaining time
            pq.emplace(remaining[pid], at, pid, pp);
        } else {
            completion[pid] = time;
        }
    }

    return finalize("SRTF", processes, gantt, completion);
}

// -------------------- Aggregator & Printers --------------------

std::vector<ScheduleResult> runAll(const std::vector<Process>& processes) {
    std::vector<ScheduleResult> all;
    all.reserve(3);
    all.push_back(scheduleFCFS(processes));
    all.push_back(scheduleSJF (processes));
    all.push_back(scheduleSRTF(processes));
    return all;
}

void printResults(const ScheduleResult& r) {
    std::cout << std::string(r.name.size() + 9, '=') << "\n"
              << r.name << " Results\n"
              << std::string(r.name.size() + 9, '=') << "\n";
    std::cout << std::setw(6) << "PID" << " | "
              << std::setw(6) << "AT"  << " | "
              << std::setw(6) << "BT"  << " | "
              << std::setw(6) << "CT"  << " | "
              << std::setw(6) << "TAT" << " | "
              << std::setw(6) << "WT"  << "\n";
    std::cout << std::string(46, '-') << "\n";

    // deterministic order by PID for display
    std::vector<Metrics> rows = r.results;
    std::sort(rows.begin(), rows.end(), [](const Metrics& a, const Metrics& b) {
        return a.pid < b.pid;
    });

    std::cout << std::fixed << std::setprecision(2);
    for (const auto& m : rows) {
        std::cout << std::setw(6) << m.pid      << " | "
                  << std::setw(6) << m.arrival  << " | "
                  << std::setw(6) << m.burst    << " | "
                  << std::setw(6) << m.completion << " | "
                  << std::setw(6) << m.turnaround << " | "
                  << std::setw(6) << m.waiting    << "\n";
    }
    std::cout << std::string(46, '-') << "\n";
    std::cout << "Avg Turnaround: " << r.avg_turnaround << "\n";
    std::cout << "Avg Waiting   : " << r.avg_waiting    << "\n\n";
}

void printGantt(const ScheduleResult& r) {
    std::cout << "Gantt for " << r.name << ":\n";
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& s : r.gantt) {
        std::cout << "[" << s.start << " — " << s.pid << " — " << s.end << "] ";
    }
    std::cout << "\n\n";
}

} // namespace sched
