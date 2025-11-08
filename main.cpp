#include "scheduler.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace sched;

namespace {

std::string toLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

void usage(const char* argv0) {
    std::cerr
        << "Usage:\n"
        << "  " << argv0 << " [--algo all|fcfs|sjf|srtf] [--file path]\n\n"
        << "Input file format (one process per line):\n"
        << "  pid arrival burst\n"
        << "  or CSV: pid,arrival,burst\n"
        << "Lines starting with '#' are ignored.\n\n"
        << "Examples:\n"
        << "  " << argv0 << "                (run built-in demo)\n"
        << "  " << argv0 << " --algo all     (demo across all algorithms)\n"
        << "  " << argv0 << " --file procs.txt --algo srtf\n";
}

bool parseLine(const std::string& line, Process& out) {
    // Strip comments beginning with '#'
    std::string s = line;
    auto pos = s.find('#');
    if (pos != std::string::npos) s = s.substr(0, pos);

    // Replace commas with spaces to support CSV
    for (char& ch : s) if (ch == ',') ch = ' ';

    // Collapse leading/trailing whitespace
    auto notspace = [](int ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notspace).base(), s.end());
    if (s.empty()) return false;

    std::stringstream ss(s);
    Process p;
    if (!(ss >> p.pid >> p.arrival >> p.burst)) return false;
    out = p;
    return true;
}

std::vector<Process> loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Failed to open file: " + path);
    std::vector<Process> ps;
    std::string line;
    while (std::getline(in, line)) {
        Process p;
        if (parseLine(line, p)) ps.push_back(p);
    }
    return ps;
}

std::vector<Process> demoData() {
    return {
        {"P1", 0, 8},
        {"P2", 1, 4},
        {"P3", 2, 9},
        {"P4", 3, 5},
    };
}

void runAndPrint(const std::string& algo, const std::vector<Process>& ps) {
    const std::string a = toLower(algo);
    if (a == "fcfs") {
        auto r = scheduleFCFS(ps);
        printResults(r);
        printGantt(r);
    } else if (a == "sjf") {
        auto r = scheduleSJF(ps);
        printResults(r);
        printGantt(r);
    } else if (a == "srtf") {
        auto r = scheduleSRTF(ps);
        printResults(r);
        printGantt(r);
    } else { // all
        auto all = runAll(ps);
        for (const auto& r : all) {
            printResults(r);
            printGantt(r);
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    std::string algo = "all";
    std::string file;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            usage(argv[0]);
            return 0;
        } else if (arg == "--algo" && i + 1 < argc) {
            algo = argv[++i];
        } else if (arg == "--file" && i + 1 < argc) {
            file = argv[++i];
        } else {
            std::cerr << "Unknown or incomplete option: " << arg << "\n";
            usage(argv[0]);
            return 1;
        }
    }

    std::vector<Process> ps;
    try {
        ps = file.empty() ? demoData() : loadFromFile(file);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    try {
        runAndPrint(algo, ps);
    } catch (const std::exception& e) {
        std::cerr << "Scheduler error: " << e.what() << "\n";
        return 2;
    }
    return 0;
}
