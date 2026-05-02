#ifndef DEADLOCK_MANAGER_H
#define DEADLOCK_MANAGER_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct DeadlockRecord {
    int pid;
    string processName;
    string holdingResource;
    string waitingForResource;
};

class DeadlockManager {
private:
    vector<DeadlockRecord> records;

public:
    /*
    Function: DeadlockManager
    Purpose: Initializes the deadlock manager.
    Parameters: None.
    Returns: Nothing.
    */
    DeadlockManager();

    /*
    Function: clearRecords
    Purpose: Clears all previous deadlock records.
    Parameters: None.
    Returns: Nothing.
    */
    void clearRecords();

    /*
    Function: addRecord
    Purpose: Adds process resource holding and waiting information.
    Parameters: PID, process name, held resource, and waiting resource.
    Returns: Nothing.
    */
    void addRecord(
        int pid,
        string processName,
        string holdingResource,
        string waitingForResource
    );

    /*
    Function: displayResourceGraph
    Purpose: Displays the current resource allocation and waiting graph.
    Parameters: None.
    Returns: Nothing.
    */
    void displayResourceGraph();

    /*
    Function: detectDeadlock
    Purpose: Detects circular wait between processes.
    Parameters: Victim PID and victim name references.
    Returns: true if deadlock is detected, otherwise false.
    */
    bool detectDeadlock(int &victimPID, string &victimName);
};

#endif