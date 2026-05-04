#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>

#include "process_manager.h"
#include "resource_manager.h"
#include "ready_queue.h"
#include "logger.h"
#include "sync_manager.h"

using namespace std;

class Scheduler {
private:
    int interactiveQuantum;
    int backgroundQuantum;

public:
    /*
    Function: Scheduler
    Purpose: Initializes scheduler time quantum values.
    Parameters: None.
    Returns: Nothing.
    */
    Scheduler();

    /*
    Function: runScheduler
    Purpose: Runs all ready processes using multilevel queue scheduling.
    Parameters: ProcessManager, ResourceManager, and ReadyQueueManager references.
    Returns: Nothing.
    */
   void runScheduler(
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager,
    string osModeName,
    string taskModeName
);

    /*
    Function: runProcess
    Purpose: Resumes a selected process and applies scheduling rules based on process type.
    Parameters: ReadyQueueItem, ProcessManager, ResourceManager, ReadyQueueManager references.
    Returns: true if process finished, otherwise false.
    */
   bool runProcess(
    ReadyQueueItem item,
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager,
    string osModeName,
    string taskModeName
);

    /*
    Function: releaseCompletedProcess
    Purpose: Releases resources and removes a completed process from PCB table.
    Parameters: PID, ProcessManager, and ResourceManager references.
    Returns: Nothing.
    */
    void releaseCompletedProcess(
        int pid,
        ProcessManager &processManager,
        ResourceManager &resourceManager,
	Logger &logger
    );

    /*
    Function: selectNextProcess
    Purpose: Selects the next process from multilevel ready queues.
    Parameters: ReadyQueueManager reference and ReadyQueueItem reference.
    Returns: true if a process is selected, otherwise false.
    */
    bool selectNextProcess(
        ReadyQueueManager &readyQueueManager,
        ReadyQueueItem &selectedItem
    );

/*
Function: writeSchedulerGUIStatus
Purpose: Writes scheduler-side live process status to GUI status file.
Parameters: ResourceManager, ProcessManager, OS mode name, and task mode name.
Returns: Nothing.
*/
void writeSchedulerGUIStatus(
    ResourceManager &resourceManager,
    ProcessManager &processManager,
    string osModeName,
    string taskModeName
);

/*
Function: runSingleProcessByPID
Purpose: Dispatches one selected READY process by PID, mainly for GUI click execution.
Parameters: PID and core AMS OS managers.
Returns: true if process dispatch is attempted, otherwise false.
*/
bool runSingleProcessByPID(
    int pid,
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager,
    string osModeName,
    string taskModeName
);
};

#endif