#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "Process.h"
#include "Scheduler.h"

bool loadWorkload(const std::string& filename,
                  std::vector<Process>& processes) {

    std::ifstream inputFile(filename);

    if (!inputFile) {
        std::cerr << "Error: Could not open workload file: "
                  << filename << std::endl;
        return false;
    }

    std::string line;

    while (std::getline(inputFile, line)) {

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);

        int pid;
        int arrivalTime;
        int burstTime;

        if (!(iss >> pid >> arrivalTime >> burstTime)) {
            continue;
        }

        Process process;

        process.pid = pid;
        process.arrivalTime = arrivalTime;
        process.burstTime = burstTime;
        process.remainingTime = burstTime;
        process.state = ProcessState::READY;
        process.firstRunTime = -1;
        process.completionTime = -1;

        processes.push_back(process);
    }

    return true;
}

void printUsage() {
    std::cout << "Usage:\n";
    std::cout << "  ./scheduler <workload-file> <policy> [quantum]\n\n";

    std::cout << "Examples:\n";
    std::cout << "  ./scheduler workload1.txt FCFS\n";
    std::cout << "  ./scheduler workload1.txt SJF\n";
    std::cout << "  ./scheduler workload1.txt RR 3\n";
}

int main(int argc, char* argv[]) {

    if (argc < 3) {
        printUsage();
        return 1;
    }

    std::string filename = argv[1];
    std::string policy = argv[2];

    int quantum = 0;

    if (policy != "FCFS" &&
        policy != "SJF" &&
        policy != "RR") {

        std::cerr << "Error: Invalid scheduling policy.\n";
        std::cerr << "Valid policies are: FCFS, SJF, RR\n";
        return 1;
    }

    if (policy == "RR") {

        if (argc < 4) {
            std::cerr << "Error: Round Robin requires a time quantum.\n";
            std::cerr << "Example: ./scheduler workload1.txt RR 3\n";
            return 1;
        }

        try {
            quantum = std::stoi(argv[3]);
        }
        catch (...) {
            std::cerr << "Error: Quantum must be an integer.\n";
            return 1;
        }

        if (quantum <= 0) {
            std::cerr << "Error: Quantum must be greater than 0.\n";
            return 1;
        }
    }

    std::vector<Process> processes;

    if (!loadWorkload(filename, processes)) {
        return 1;
    }

    if (processes.empty()) {
        std::cerr << "Error: Workload contains no processes.\n";
        return 1;
    }

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "        MINI-KERNEL CPU SCHEDULER\n";
    std::cout << "========================================\n";

    std::cout << "Workload File : " << filename << "\n";
    std::cout << "Policy        : " << policy << "\n";

    if (policy == "RR") {
        std::cout << "Time Quantum  : " << quantum << "\n";
    }

    std::cout << "\nProcesses Loaded\n";
    std::cout << "----------------------------------------\n";
    std::cout << "PID   Arrival   Burst\n";
    std::cout << "----------------------------------------\n";

    for (const Process& process : processes) {
        std::cout << "P" << process.pid
                  << "    " << process.arrivalTime
                  << "         " << process.burstTime
                  << "\n";
    }

    std::cout << "\nTotal Processes: "
              << processes.size() << "\n";

    Scheduler scheduler(processes);

    if (policy == "FCFS") {
        scheduler.runFCFS();
    }
    else if (policy == "SJF") {
        scheduler.runSJF();
    }
    else {
        scheduler.runRoundRobin(quantum);
    }

    return 0;
}