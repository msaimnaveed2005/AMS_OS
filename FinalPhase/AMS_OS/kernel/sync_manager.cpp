#include "sync_manager.h"

/*
Function: SimpleSemaphore
Purpose: Initializes semaphore with available slots.
Parameters: Number of available slots.
Returns: Nothing.
*/
SimpleSemaphore::SimpleSemaphore(int slots) {
    availableSlots = slots;
}

/*
Function: acquire
Purpose: Acquires one semaphore slot. Waits if no slot is available.
Parameters: None.
Returns: Nothing.
*/
void SimpleSemaphore::acquire() {
    unique_lock<mutex> lock(semaphoreMutex);

    while (availableSlots <= 0) {
        semaphoreCondition.wait(lock);
    }

    availableSlots--;
}

/*
Function: release
Purpose: Releases one semaphore slot and wakes waiting thread.
Parameters: None.
Returns: Nothing.
*/
void SimpleSemaphore::release() {
    unique_lock<mutex> lock(semaphoreMutex);

    availableSlots++;

    semaphoreCondition.notify_one();
}

/*
Function: SyncManager
Purpose: Initializes synchronization manager with CPU core count.
Parameters: Total CPU cores.
Returns: Nothing.
*/
SyncManager::SyncManager(int totalCores) : cpuCoreSemaphore(totalCores) {
    monitorRunning = false;

    for (int coreIndex = 0; coreIndex < totalCores; coreIndex++) {
        availableCoreIDs.push(coreIndex);
    }
}

/*
Function: ~SyncManager
Purpose: Stops monitor thread safely when object is destroyed.
Parameters: None.
Returns: Nothing.
*/
SyncManager::~SyncManager() {
    stopResourceMonitor();
}

/*
Function: acquireCPUCores
Purpose: Acquires CPU execution slots before dispatching a process.
Parameters: Number of cores required.
Returns: Nothing.
*/
void SyncManager::acquireCPUCores(int coresRequired) {
    for (int i = 0; i < coresRequired; i++) {
        cpuCoreSemaphore.acquire();
    }

    cout << "\n[SYNC MANAGER] CPU execution slot acquired.\n";
    cout << "CPU Cores Acquired: " << coresRequired << "\n";
}

/*
Function: releaseCPUCores
Purpose: Releases CPU execution slots after process stops or completes.
Parameters: Number of cores released.
Returns: Nothing.
*/
void SyncManager::releaseCPUCores(int coresReleased) {
    for (int i = 0; i < coresReleased; i++) {
        cpuCoreSemaphore.release();
    }

    cout << "\n[SYNC MANAGER] CPU execution slot released.\n";
    cout << "CPU Cores Released: " << coresReleased << "\n";
}

/*
Function: acquireCoreSet
Purpose: Acquires a set of CPU core IDs for process execution.
Parameters: Number of cores required.
Returns: Vector of assigned core IDs.
*/
vector<int> SyncManager::acquireCoreSet(int coresRequired) {
    vector<int> assignedCores;

    if (coresRequired <= 0) {
        return assignedCores;
    }

    acquireCPUCores(coresRequired);

    unique_lock<mutex> lock(cpuCoreMutex);

    for (int i = 0; i < coresRequired; i++) {
        if (availableCoreIDs.empty()) {
            break;
        }

        assignedCores.push_back(availableCoreIDs.front());
        availableCoreIDs.pop();
    }

    return assignedCores;
}

/*
Function: releaseCoreSet
Purpose: Releases assigned CPU core IDs back to scheduler pool.
Parameters: Vector of core IDs.
Returns: Nothing.
*/
void SyncManager::releaseCoreSet(const vector<int> &coreIDs) {
    if (coreIDs.empty()) {
        return;
    }

    {
        unique_lock<mutex> lock(cpuCoreMutex);

        for (int coreID : coreIDs) {
            availableCoreIDs.push(coreID);
        }
    }

    releaseCPUCores(static_cast<int>(coreIDs.size()));
}

/*
Function: notifyReadyQueue
Purpose: Notifies scheduler that a process has entered ready queue.
Parameters: None.
Returns: Nothing.
*/
void SyncManager::notifyReadyQueue() {
    readyQueueCondition.notify_one();
}

/*
Function: waitForReadyQueueSignal
Purpose: Waits briefly for ready queue notification.
Parameters: Waiting time in seconds.
Returns: Nothing.
*/
void SyncManager::waitForReadyQueueSignal(int seconds) {
    unique_lock<mutex> lock(readyQueueMutex);

    readyQueueCondition.wait_for(
        lock,
        chrono::seconds(seconds)
    );
}

/*
Function: startResourceMonitor
Purpose: Starts a background thread that logs system resource status.
Parameters: ResourceManager and Logger references.
Returns: Nothing.
*/
void SyncManager::startResourceMonitor(ResourceManager &resourceManager, Logger &logger) {
    if (monitorRunning) {
        return;
    }

    monitorRunning = true;

    resourceMonitorThread = thread([this, &resourceManager, &logger]() {
        while (monitorRunning) {
            this_thread::sleep_for(chrono::seconds(5));

            if (!monitorRunning) {
                break;
            }

            logger.logResourceEvent(
                "Monitor Thread | Available RAM: " +
                to_string(resourceManager.getAvailableRAM()) +
                "MB | Available HDD: " +
                to_string(resourceManager.getAvailableHDD()) +
                "MB | Available CPU Cores: " +
                to_string(resourceManager.getAvailableCores())
            );
        }
    });

    logger.logSystemEvent("Resource monitor thread started");
}

/*
Function: stopResourceMonitor
Purpose: Stops the background resource monitor thread safely.
Parameters: None.
Returns: Nothing.
*/
void SyncManager::stopResourceMonitor() {
    monitorRunning = false;

    if (resourceMonitorThread.joinable()) {
        resourceMonitorThread.join();
    }
}