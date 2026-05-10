#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include "resource_manager.h"
#include "logger.h"

using namespace std;

/*
Class: SimpleSemaphore
Purpose: Implements a simple counting semaphore using mutex and condition variable.
*/
class SimpleSemaphore {
private:
    int availableSlots;
    mutex semaphoreMutex;
    condition_variable semaphoreCondition;

public:
    /*
    Function: SimpleSemaphore
    Purpose: Initializes semaphore with available slots.
    Parameters: Number of available slots.
    Returns: Nothing.
    */
    SimpleSemaphore(int slots);

    /*
    Function: acquire
    Purpose: Acquires one semaphore slot. Waits if no slot is available.
    Parameters: None.
    Returns: Nothing.
    */
    void acquire();

    /*
    Function: release
    Purpose: Releases one semaphore slot and wakes waiting thread.
    Parameters: None.
    Returns: Nothing.
    */
    void release();
};

/*
Class: SyncManager
Purpose: Handles synchronization for CPU dispatch, ready queue notification,
         and background monitoring thread.
*/
class SyncManager {
private:
    SimpleSemaphore cpuCoreSemaphore;
    mutex readyQueueMutex;
    condition_variable readyQueueCondition;
    atomic<bool> monitorRunning;
    thread resourceMonitorThread;

public:
    /*
    Function: SyncManager
    Purpose: Initializes synchronization manager with CPU core count.
    Parameters: Total CPU cores.
    Returns: Nothing.
    */
    SyncManager(int totalCores);

    /*
    Function: ~SyncManager
    Purpose: Stops monitor thread safely when object is destroyed.
    Parameters: None.
    Returns: Nothing.
    */
    ~SyncManager();

    /*
    Function: acquireCPUCores
    Purpose: Acquires CPU execution slots before dispatching a process.
    Parameters: Number of cores required.
    Returns: Nothing.
    */
    void acquireCPUCores(int coresRequired);

    /*
    Function: releaseCPUCores
    Purpose: Releases CPU execution slots after process stops or completes.
    Parameters: Number of cores released.
    Returns: Nothing.
    */
    void releaseCPUCores(int coresReleased);

    /*
    Function: notifyReadyQueue
    Purpose: Notifies scheduler that a process has entered ready queue.
    Parameters: None.
    Returns: Nothing.
    */
    void notifyReadyQueue();

    /*
    Function: waitForReadyQueueSignal
    Purpose: Waits briefly for ready queue notification.
    Parameters: Waiting time in seconds.
    Returns: Nothing.
    */
    void waitForReadyQueueSignal(int seconds);

    /*
    Function: startResourceMonitor
    Purpose: Starts a background thread that logs system resource status.
    Parameters: ResourceManager and Logger references.
    Returns: Nothing.
    */
    void startResourceMonitor(ResourceManager &resourceManager, Logger &logger);

    /*
    Function: stopResourceMonitor
    Purpose: Stops the background resource monitor thread safely.
    Parameters: None.
    Returns: Nothing.
    */
    void stopResourceMonitor();
};

#endif