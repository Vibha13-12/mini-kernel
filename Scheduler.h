#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <string>
#include <vector>

#include "Process.h"

class Scheduler {
private:
    // Processes loaded from the workload file.
    std::vector<Process>& processes;

    // Current time in the CPU simulation.
    int currentTime;

    // Number of processes that have completed.
    int completedProcesses;

    // Add processes that have arrived to the ready queue.
    void addArrivedProcesses(
        std::queue<int>& readyQueue,
        std::vector<bool>& added);

    // Find the shortest available process for SJF.
    int selectSJFProcess(
        const std::vector<bool>& inReadyQueue) const;

    // Check whether all processes have finished.
    bool allProcessesCompleted() const;

    // Move the clock forward by one CPU time unit.
    void executeOneUnit(int processIndex);

    // Save the first time a process gets the CPU.
    void recordFirstRun(int processIndex);

    // Print when a process starts running.
    void printDispatch(int processIndex);

    // Print the final scheduling results.
    void printResults(const std::string& policy) const;

    // Move the clock to the next process arrival if the CPU is idle.
    bool moveToNextArrival(const std::vector<bool>& added);

public:
    explicit Scheduler(std::vector<Process>& processList);

    void runFCFS();
    void runSJF();
    void runRoundRobin(int quantum);
};

#endif