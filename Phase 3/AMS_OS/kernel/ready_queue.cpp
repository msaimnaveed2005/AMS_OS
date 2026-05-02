#include "ready_queue.h"
#include <iomanip>

/*
Function: ReadyQueueManager
Purpose: Initializes the ready queue manager.
Parameters: None.
Returns: Nothing.
*/
ReadyQueueManager::ReadyQueueManager() {
}

/*
Function: addProcessToReadyQueue
Purpose: Adds a process to the correct ready queue based on its process type.
Parameters: PID, process name, process type, and priority.
Returns: Nothing.
*/
void ReadyQueueManager::addProcessToReadyQueue(
    int pid,
    string processName,
    ProcessType processType,
    int priority
) {
    ReadyQueueItem item;

    item.pid = pid;
    item.processName = processName;
    item.processType = processType;
    item.priority = priority;

    if (processType == SYSTEM_PROCESS || processType == KERNEL_PROCESS) {
        systemQueue.push(item);
        cout << "\n[READY QUEUE] Process added to System Queue.\n";
    } 
    else if (processType == INTERACTIVE_PROCESS || processType == AUTO_RUNNING_PROCESS) {
        interactiveQueue.push(item);
        cout << "\n[READY QUEUE] Process added to Interactive Queue.\n";
    } 
    else if (processType == BACKGROUND_PROCESS) {
        backgroundQueue.push(item);
        cout << "\n[READY QUEUE] Process added to Background Queue.\n";
    } 
    else {
        interactiveQueue.push(item);
        cout << "\n[READY QUEUE] Unknown type. Process added to Interactive Queue by default.\n";
    }

    cout << "PID: " << pid << "\n";
    cout << "Process Name: " << processName << "\n";
    cout << "Queue: " << getQueueName(processType) << "\n";
}

/*
Function: hasReadyProcess
Purpose: Checks whether any ready queue contains a process.
Parameters: None.
Returns: true if any queue has a process, otherwise false.
*/
bool ReadyQueueManager::hasReadyProcess() {
    return !systemQueue.empty() || !interactiveQueue.empty() || !backgroundQueue.empty();
}

/*
Function: isSystemQueueEmpty
Purpose: Checks whether system queue is empty.
Parameters: None.
Returns: true if empty, otherwise false.
*/
bool ReadyQueueManager::isSystemQueueEmpty() {
    return systemQueue.empty();
}

/*
Function: isInteractiveQueueEmpty
Purpose: Checks whether interactive queue is empty.
Parameters: None.
Returns: true if empty, otherwise false.
*/
bool ReadyQueueManager::isInteractiveQueueEmpty() {
    return interactiveQueue.empty();
}

/*
Function: isBackgroundQueueEmpty
Purpose: Checks whether background queue is empty.
Parameters: None.
Returns: true if empty, otherwise false.
*/
bool ReadyQueueManager::isBackgroundQueueEmpty() {
    return backgroundQueue.empty();
}

/*
Function: removeFromSystemQueue
Purpose: Removes and returns the front process from system queue.
Parameters: None.
Returns: ReadyQueueItem.
*/
ReadyQueueItem ReadyQueueManager::removeFromSystemQueue() {
    ReadyQueueItem item = systemQueue.front();
    systemQueue.pop();
    return item;
}

/*
Function: removeFromInteractiveQueue
Purpose: Removes and returns the front process from interactive queue.
Parameters: None.
Returns: ReadyQueueItem.
*/
ReadyQueueItem ReadyQueueManager::removeFromInteractiveQueue() {
    ReadyQueueItem item = interactiveQueue.front();
    interactiveQueue.pop();
    return item;
}

/*
Function: removeFromBackgroundQueue
Purpose: Removes and returns the front process from background queue.
Parameters: None.
Returns: ReadyQueueItem.
*/
ReadyQueueItem ReadyQueueManager::removeFromBackgroundQueue() {
    ReadyQueueItem item = backgroundQueue.front();
    backgroundQueue.pop();
    return item;
}

/*
Function: removeFromQueueByPID
Purpose: Removes a process from a specific queue using PID.
Parameters: Queue reference and PID.
Returns: true if process is removed, otherwise false.
*/
bool ReadyQueueManager::removeFromQueueByPID(queue<ReadyQueueItem> &targetQueue, int pid) {
    queue<ReadyQueueItem> tempQueue;
    bool found = false;

    while (!targetQueue.empty()) {
        ReadyQueueItem item = targetQueue.front();
        targetQueue.pop();

        if (item.pid == pid) {
            found = true;
        } else {
            tempQueue.push(item);
        }
    }

    targetQueue = tempQueue;

    return found;
}

/*
Function: removeProcessByPID
Purpose: Removes a process from all ready queues using PID.
Parameters: PID.
Returns: true if process was found and removed, otherwise false.
*/
bool ReadyQueueManager::removeProcessByPID(int pid) {
    bool removedFromSystem = removeFromQueueByPID(systemQueue, pid);
    bool removedFromInteractive = removeFromQueueByPID(interactiveQueue, pid);
    bool removedFromBackground = removeFromQueueByPID(backgroundQueue, pid);

    if (removedFromSystem || removedFromInteractive || removedFromBackground) {
        cout << "\n[READY QUEUE] Process removed from ready queue.\n";
        cout << "PID: " << pid << "\n";
        return true;
    }

    return false;
}



/*
Function: displayReadyQueues
Purpose: Displays all ready queues with process details.
Parameters: None.
Returns: Nothing.
*/
void ReadyQueueManager::displayReadyQueues() {
    cout << "\n============================== READY QUEUES ==============================\n";

    if (!hasReadyProcess()) {
        cout << "No process currently exists in ready queues.\n";
        cout << "=========================================================================\n";
        return;
    }

    queue<ReadyQueueItem> tempSystemQueue = systemQueue;
    queue<ReadyQueueItem> tempInteractiveQueue = interactiveQueue;
    queue<ReadyQueueItem> tempBackgroundQueue = backgroundQueue;

    cout << "\n[Queue 1: System Queue, FCFS]\n";
    if (tempSystemQueue.empty()) {
        cout << "Empty\n";
    } else {
        cout << left
             << setw(8) << "PID"
             << setw(22) << "Process Name"
             << setw(12) << "Priority"
             << "\n";

        while (!tempSystemQueue.empty()) {
            ReadyQueueItem item = tempSystemQueue.front();
            tempSystemQueue.pop();

            cout << left
                 << setw(8) << item.pid
                 << setw(22) << item.processName
                 << setw(12) << item.priority
                 << "\n";
        }
    }

    cout << "\n[Queue 2: Interactive Queue, Round Robin]\n";
    if (tempInteractiveQueue.empty()) {
        cout << "Empty\n";
    } else {
        cout << left
             << setw(8) << "PID"
             << setw(22) << "Process Name"
             << setw(12) << "Priority"
             << "\n";

        while (!tempInteractiveQueue.empty()) {
            ReadyQueueItem item = tempInteractiveQueue.front();
            tempInteractiveQueue.pop();

            cout << left
                 << setw(8) << item.pid
                 << setw(22) << item.processName
                 << setw(12) << item.priority
                 << "\n";
        }
    }

    cout << "\n[Queue 3: Background Queue, Low Priority]\n";
    if (tempBackgroundQueue.empty()) {
        cout << "Empty\n";
    } else {
        cout << left
             << setw(8) << "PID"
             << setw(22) << "Process Name"
             << setw(12) << "Priority"
             << "\n";

        while (!tempBackgroundQueue.empty()) {
            ReadyQueueItem item = tempBackgroundQueue.front();
            tempBackgroundQueue.pop();

            cout << left
                 << setw(8) << item.pid
                 << setw(22) << item.processName
                 << setw(12) << item.priority
                 << "\n";
        }
    }

    cout << "=========================================================================\n";
}

/*
Function: getQueueName
Purpose: Returns the queue name based on process type.
Parameters: Process type.
Returns: Queue name as string.
*/
string ReadyQueueManager::getQueueName(ProcessType processType) {
    if (processType == SYSTEM_PROCESS || processType == KERNEL_PROCESS) {
        return "System Queue";
    }

    if (processType == INTERACTIVE_PROCESS || processType == AUTO_RUNNING_PROCESS) {
        return "Interactive Queue";
    }

    if (processType == BACKGROUND_PROCESS) {
        return "Background Queue";
    }

    return "Interactive Queue";
}