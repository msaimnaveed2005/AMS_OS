#ifndef READY_QUEUE_H
#define READY_QUEUE_H

#include <iostream>
#include <queue>
#include <string>
#include "process_manager.h"
#include "logger.h"

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
/*
Function: insertAgedProcess
Purpose: Inserts process into a queue based on improved priority.
Parameters: ReadyQueueItem.
Returns: Nothing.
*/
void insertAgedProcess(ReadyQueueItem item);

/*
Function: removeFromQueueByPID
Purpose: Removes a process from a specific queue using PID.
Parameters: Queue reference and PID.
Returns: true if process is removed, otherwise false.
*/
bool removeFromQueueByPID(queue<ReadyQueueItem> &targetQueue, int pid);
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
    string getQueueName(int priority);


	/*
	Function: removeProcessByPID
	Purpose: Removes a process from all ready queues using PID.
	Parameters: PID.
	Returns: true if process was found and removed, otherwise false.
	*/
	bool removeProcessByPID(int pid);

	/*
	Function: applyAging
	Purpose: Applies aging to all waiting processes in ready queues.
	Parameters: ProcessManager reference, Logger reference, and aging threshold.
	Returns: Nothing.
	*/
	void applyAging(ProcessManager &processManager, Logger &logger, int agingThreshold);


/*
Function: clearAllQueues
Purpose: Removes all processes from all ready queues.
Parameters: None.
Returns: Nothing.
*/
void clearAllQueues();
};

#endif