#ifndef READY_QUEUE_H
#define READY_QUEUE_H

#include <iostream>
#include <queue>
#include <string>
#include "process_manager.h"

using namespace std;

struct ReadyQueueItem {
    int pid;
    string processName;
    ProcessType processType;
    int priority;
};

class ReadyQueueManager {
private:
    queue<ReadyQueueItem> systemQueue;
    queue<ReadyQueueItem> interactiveQueue;
    queue<ReadyQueueItem> backgroundQueue;

public:
    /*
    Function: ReadyQueueManager
    Purpose: Initializes the ready queue manager.
    Parameters: None.
    Returns: Nothing.
    */
    ReadyQueueManager();

    /*
    Function: addProcessToReadyQueue
    Purpose: Adds a process to the correct ready queue based on its process type.
    Parameters: PID, process name, process type, and priority.
    Returns: Nothing.
    */
    void addProcessToReadyQueue(
        int pid,
        string processName,
        ProcessType processType,
        int priority
    );

    /*
    Function: hasReadyProcess
    Purpose: Checks whether any ready queue contains a process.
    Parameters: None.
    Returns: true if any queue has a process, otherwise false.
    */
    bool hasReadyProcess();

    /*
    Function: isSystemQueueEmpty
    Purpose: Checks whether system queue is empty.
    Parameters: None.
    Returns: true if empty, otherwise false.
    */
    bool isSystemQueueEmpty();

    /*
    Function: isInteractiveQueueEmpty
    Purpose: Checks whether interactive queue is empty.
    Parameters: None.
    Returns: true if empty, otherwise false.
    */
    bool isInteractiveQueueEmpty();

    /*
    Function: isBackgroundQueueEmpty
    Purpose: Checks whether background queue is empty.
    Parameters: None.
    Returns: true if empty, otherwise false.
    */
    bool isBackgroundQueueEmpty();

    /*
    Function: removeFromSystemQueue
    Purpose: Removes and returns the front process from system queue.
    Parameters: None.
    Returns: ReadyQueueItem.
    */
    ReadyQueueItem removeFromSystemQueue();

    /*
    Function: removeFromInteractiveQueue
    Purpose: Removes and returns the front process from interactive queue.
    Parameters: None.
    Returns: ReadyQueueItem.
    */
    ReadyQueueItem removeFromInteractiveQueue();

    /*
    Function: removeFromBackgroundQueue
    Purpose: Removes and returns the front process from background queue.
    Parameters: None.
    Returns: ReadyQueueItem.
    */
    ReadyQueueItem removeFromBackgroundQueue();

    /*
    Function: displayReadyQueues
    Purpose: Displays all ready queues with process details.
    Parameters: None.
    Returns: Nothing.
    */
    void displayReadyQueues();

    /*
    Function: getQueueName
    Purpose: Returns the queue name based on process type.
    Parameters: Process type.
    Returns: Queue name as string.
    */
    string getQueueName(ProcessType processType);
};

#endif