#include "Scheduler.h"

#include <iomanip>
#include <iostream>
#include <limits>

Scheduler::Scheduler(std::vector<Process>& processList)
    : processes(processList), currentTime(0), completedProcesses(0) {
}

// Add every process that has arrived to the ready queue.
void Scheduler::addArrivedProcesses(
    std::queue<int>& readyQueue,
    std::vector<bool>& added) {

    for (int i = 0; i < static_cast<int>(processes.size()); ++i) {
        if (!added[i] &&
            processes[i].arrivalTime <= currentTime) {

            processes[i].state = ProcessState::READY;
            readyQueue.push(i);
            added[i] = true;

            std::cout << "Time " << currentTime
                      << ": P" << processes[i].pid
                      << " -> READY" << std::endl;
        }
    }
}

// Move the simulated clock forward when the CPU has nothing to run.
bool Scheduler::moveToNextArrival(
    const std::vector<bool>& added) {

    int nextArrival = std::numeric_limits<int>::max();

    for (int i = 0; i < static_cast<int>(processes.size()); ++i) {
        if (!added[i] &&
            processes[i].arrivalTime < nextArrival) {

            nextArrival = processes[i].arrivalTime;
        }
    }

    if (nextArrival == std::numeric_limits<int>::max()) {
        return false;
    }

    std::cout << "Time " << currentTime
              << ": CPU IDLE" << std::endl;

    currentTime = nextArrival;

    return true;
}

bool Scheduler::allProcessesCompleted() const {
    return completedProcesses ==
           static_cast<int>(processes.size());
}

// Save the first time a process gets CPU time.
void Scheduler::recordFirstRun(int processIndex) {
    if (processes[processIndex].firstRunTime == -1) {
        processes[processIndex].firstRunTime = currentTime;
    }
}

// Print when a process starts running.
void Scheduler::printDispatch(int processIndex) {
    std::cout << "Time " << currentTime
              << ": P" << processes[processIndex].pid
              << " -> RUNNING" << std::endl;
}

// Execute one unit of CPU time for the selected process.
void Scheduler::executeOneUnit(int processIndex) {
    Process& process = processes[processIndex];

    process.state = ProcessState::RUNNING;
    --process.remainingTime;
    ++currentTime;

    if (process.remainingTime == 0) {
        process.state = ProcessState::TERMINATED;
        process.completionTime = currentTime;
        ++completedProcesses;

        std::cout << "Time " << currentTime
                  << ": P" << process.pid
                  << " -> TERMINATED" << std::endl;
    }
}

// Select the shortest available process for SJF.
// Arrival time and PID are used to break ties.
int Scheduler::selectSJFProcess(
    const std::vector<bool>& inReadyQueue) const {

    int selected = -1;

    for (int i = 0; i < static_cast<int>(processes.size()); ++i) {
        if (!inReadyQueue[i]) {
            continue;
        }

        if (selected == -1) {
            selected = i;
            continue;
        }

        const Process& current = processes[i];
        const Process& best = processes[selected];

        if (current.burstTime < best.burstTime) {
            selected = i;
        }
        else if (current.burstTime == best.burstTime) {
            if (current.arrivalTime < best.arrivalTime) {
                selected = i;
            }
            else if (current.arrivalTime == best.arrivalTime &&
                     current.pid < best.pid) {
                selected = i;
            }
        }
    }

    return selected;
}

void Scheduler::runFCFS() {
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "          FCFS SIMULATION" << std::endl;
    std::cout << "========================================" << std::endl;

    currentTime = 0;
    completedProcesses = 0;

    std::queue<int> readyQueue;
    std::vector<bool> added(processes.size(), false);

    while (!allProcessesCompleted()) {
        addArrivedProcesses(readyQueue, added);

        if (readyQueue.empty()) {
            if (moveToNextArrival(added)) {
                continue;
            }
        }

        if (!readyQueue.empty()) {
            int processIndex = readyQueue.front();
            readyQueue.pop();

            printDispatch(processIndex);
            recordFirstRun(processIndex);

            // FCFS keeps the CPU with the selected process until it finishes.
            while (processes[processIndex].remainingTime > 0) {
                executeOneUnit(processIndex);
                addArrivedProcesses(readyQueue, added);
            }
        }
    }

    printResults("FCFS");
}

void Scheduler::runSJF() {
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "           SJF SIMULATION" << std::endl;
    std::cout << "========================================" << std::endl;

    currentTime = 0;
    completedProcesses = 0;

    std::vector<bool> added(processes.size(), false);
    std::vector<bool> inReadyQueue(processes.size(), false);

    while (!allProcessesCompleted()) {

        // Add newly arrived processes to the SJF ready set.
        for (int i = 0; i < static_cast<int>(processes.size()); ++i) {
            if (!added[i] &&
                processes[i].arrivalTime <= currentTime) {

                processes[i].state = ProcessState::READY;
                added[i] = true;
                inReadyQueue[i] = true;

                std::cout << "Time " << currentTime
                          << ": P" << processes[i].pid
                          << " -> READY" << std::endl;
            }
        }

        int processIndex = selectSJFProcess(inReadyQueue);

        if (processIndex == -1) {
            if (moveToNextArrival(added)) {
                continue;
            }
        }

        if (processIndex != -1) {
            inReadyQueue[processIndex] = false;

            printDispatch(processIndex);
            recordFirstRun(processIndex);

            // SJF is non-preemptive, so run until completion.
            while (processes[processIndex].remainingTime > 0) {
                executeOneUnit(processIndex);

                for (int i = 0;
                     i < static_cast<int>(processes.size());
                     ++i) {

                    if (!added[i] &&
                        processes[i].arrivalTime <= currentTime) {

                        processes[i].state = ProcessState::READY;
                        added[i] = true;
                        inReadyQueue[i] = true;

                        std::cout << "Time " << currentTime
                                  << ": P" << processes[i].pid
                                  << " -> READY" << std::endl;
                    }
                }
            }
        }
    }

    printResults("SJF");
}

void Scheduler::runRoundRobin(int quantum) {
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "       ROUND ROBIN SIMULATION" << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "Time Quantum: "
              << quantum << std::endl;

    currentTime = 0;
    completedProcesses = 0;

    std::queue<int> readyQueue;
    std::vector<bool> added(processes.size(), false);

    while (!allProcessesCompleted()) {
        addArrivedProcesses(readyQueue, added);

        if (readyQueue.empty()) {
            if (moveToNextArrival(added)) {
                continue;
            }
        }

        if (!readyQueue.empty()) {
            int processIndex = readyQueue.front();
            readyQueue.pop();

            printDispatch(processIndex);
            recordFirstRun(processIndex);

            int timeUsed = 0;

            while (timeUsed < quantum &&
                   processes[processIndex].remainingTime > 0) {

                executeOneUnit(processIndex);
                ++timeUsed;

                addArrivedProcesses(readyQueue, added);
            }

            // Put the process back at the end if its quantum expired.
            if (processes[processIndex].remainingTime > 0) {
                processes[processIndex].state = ProcessState::READY;
                readyQueue.push(processIndex);

                std::cout << "Time " << currentTime
                          << ": P" << processes[processIndex].pid
                          << " -> READY (quantum expired)"
                          << std::endl;
            }
        }
    }

    printResults("RR");
}

void Scheduler::printResults(
    const std::string& policy) const {

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "           " << policy << " RESULTS" << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << std::left
              << std::setw(8) << "PID"
              << std::setw(12) << "Arrival"
              << std::setw(10) << "Burst"
              << std::setw(14) << "Completion"
              << std::setw(14) << "Turnaround"
              << std::setw(12) << "Response"
              << std::endl;

    std::cout << "------------------------------------------------------------"
              << std::endl;

    double totalTurnaround = 0.0;
    double totalResponse = 0.0;

    for (const Process& process : processes) {
        int turnaround =
            process.completionTime - process.arrivalTime;

        int response =
            process.firstRunTime - process.arrivalTime;

        totalTurnaround += turnaround;
        totalResponse += response;

        std::cout << std::left
                  << std::setw(8)
                  << ("P" + std::to_string(process.pid))
                  << std::setw(12)
                  << process.arrivalTime
                  << std::setw(10)
                  << process.burstTime
                  << std::setw(14)
                  << process.completionTime
                  << std::setw(14)
                  << turnaround
                  << std::setw(12)
                  << response
                  << std::endl;
    }

    double averageTurnaround =
        totalTurnaround / processes.size();

    double averageResponse =
        totalResponse / processes.size();

    std::cout << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    std::cout << "Average Turnaround Time: "
              << averageTurnaround << std::endl;

    std::cout << "Average Response Time:   "
              << averageResponse << std::endl;

    std::cout << "========================================"
              << std::endl;
}