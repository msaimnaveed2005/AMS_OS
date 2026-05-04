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

    if (priority <= 1) {
        systemQueue.push(item);
        cout << "\n[READY QUEUE] Process added to System Queue based on priority.\n";
    } 
    else if (priority == 2) {
        interactiveQueue.push(item);
        cout << "\n[READY QUEUE] Process added to Interactive Queue based on priority.\n";
    } 
    else {
        backgroundQueue.push(item);
        cout << "\n[READY QUEUE] Process added to Background Queue based on priority.\n";
    }

    cout << "PID: " << pid << "\n";
    cout << "Process Name: " << processName << "\n";
    cout << "Priority: " << priority << "\n";
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
Function: insertAgedProcess
Purpose: Inserts process into a queue based on improved priority.
Parameters: ReadyQueueItem.
Returns: Nothing.
*/
void ReadyQueueManager::insertAgedProcess(ReadyQueueItem item) {
    if (item.priority <= 1) {
        systemQueue.push(item);
    } 
    else if (item.priority == 2) {
        interactiveQueue.push(item);
    } 
    else {
        backgroundQueue.push(item);
    }
}

/*
Function: applyAging
Purpose: Applies aging to all waiting processes in ready queues.
Parameters: ProcessManager reference, Logger reference, and aging threshold.
Returns: Nothing.
*/
void ReadyQueueManager::applyAging(
    ProcessManager &processManager,
    Logger &logger,
    int agingThreshold
) {
    queue<ReadyQueueItem> allProcesses;

    while (!systemQueue.empty()) {
        allProcesses.push(systemQueue.front());
        systemQueue.pop();
    }

    while (!interactiveQueue.empty()) {
        allProcesses.push(interactiveQueue.front());
        interactiveQueue.pop();
    }

    while (!backgroundQueue.empty()) {
        allProcesses.push(backgroundQueue.front());
        backgroundQueue.pop();
    }

    while (!allProcesses.empty()) {
        ReadyQueueItem item = allProcesses.front();
        allProcesses.pop();

        PCB pcb;

        if (!processManager.getPCB(item.pid, pcb)) {
            continue;
        }

        processManager.incrementWaitingTime(item.pid);
        processManager.getPCB(item.pid, pcb);

        if (pcb.waitingTime >= agingThreshold && pcb.priority > 1) {
            int oldPriority = pcb.priority;

            processManager.improvePriority(item.pid);
            processManager.getPCB(item.pid, pcb);

            item.priority = pcb.priority;

            cout << "\n[AGING] Priority improved for waiting process.\n";
            cout << "PID: " << item.pid << "\n";
            cout << "Process: " << item.processName << "\n";
            cout << "Old Priority: " << oldPriority << "\n";
            cout << "New Priority: " << pcb.priority << "\n";

            logger.logProcessEvent(
                item.pid,
                item.processName,
                "Aging applied, priority improved from " +
                to_string(oldPriority) + " to " + to_string(pcb.priority)
            );
        } else {
            item.priority = pcb.priority;
        }

        insertAgedProcess(item);
    }
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

    cout << "\n[Queue 1: System Queue, Highest Priority, FCFS]\n";
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

    cout << "\n[Queue 2: Interactive Queue, Medium Priority, Round Robin]\n";
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
string ReadyQueueManager::getQueueName(int priority) {
    if (priority <= 1) {
        return "System Queue";
    }

    if (priority == 2) {
        return "Interactive Queue";
    }

    return "Background Queue";
}

/*
Function: clearAllQueues
Purpose: Removes all processes from all ready queues.
Parameters: None.
Returns: Nothing.
*/
void ReadyQueueManager::clearAllQueues() {
    while (!systemQueue.empty()) {
        systemQueue.pop();
    }

    while (!interactiveQueue.empty()) {
        interactiveQueue.pop();
    }

    while (!backgroundQueue.empty()) {
        backgroundQueue.pop();
    }

    cout << "\n[READY QUEUE] All ready queues cleared.\n";
}