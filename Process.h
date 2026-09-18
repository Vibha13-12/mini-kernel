#ifndef PROCESS_H
#define PROCESS_H

// Represents the current state of a process.
enum class ProcessState {
    READY,
    RUNNING,
    TERMINATED
};

// Process Control Block
struct Process {
    int pid;
    int arrivalTime;
    int burstTime;
    int remainingTime;

    ProcessState state;

    int firstRunTime;
    int completionTime;
};

#endif