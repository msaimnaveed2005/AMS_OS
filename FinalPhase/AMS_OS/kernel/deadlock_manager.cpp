#include "deadlock_manager.h"
#include <iomanip>
#include "console_colors.h"

/*
Function: DeadlockManager
Purpose: Initializes the deadlock manager.
Parameters: None.
Returns: Nothing.
*/
DeadlockManager::DeadlockManager() {
}

/*
Function: clearRecords
Purpose: Clears all previous deadlock records.
Parameters: None.
Returns: Nothing.
*/
void DeadlockManager::clearRecords() {
    records.clear();
}

/*
Function: addRecord
Purpose: Adds process resource holding and waiting information.
Parameters: PID, process name, held resource, and waiting resource.
Returns: Nothing.
*/
void DeadlockManager::addRecord(
    int pid,
    string processName,
    string holdingResource,
    string waitingForResource
) {
    DeadlockRecord record;

    record.pid = pid;
    record.processName = processName;
    record.holdingResource = holdingResource;
    record.waitingForResource = waitingForResource;

    records.push_back(record);
}

/*
Function: displayResourceGraph
Purpose: Displays the current resource allocation and waiting graph.
Parameters: None.
Returns: Nothing.
*/
void DeadlockManager::displayResourceGraph() {
    cout << Color::deadlock("\n==================== RESOURCE WAIT GRAPH ====================\n");

    if (records.empty()) {
        cout << "No resource wait records available.\n";
        cout << "=============================================================\n";
        return;
    }

    cout << left
         << setw(8)  << "PID"
         << setw(22) << "Process"
         << setw(22) << "Holding"
         << setw(22) << "Waiting For"
         << "\n";

    cout << "-------------------------------------------------------------\n";

    for (DeadlockRecord record : records) {
        cout << left
             << setw(8)  << record.pid
             << setw(22) << record.processName
             << setw(22) << record.holdingResource
             << setw(22) << record.waitingForResource
             << "\n";
    }

    cout << "=============================================================\n";
}

/*
Function: detectDeadlock
Purpose: Detects circular wait between processes.
Parameters: Victim PID and victim name references.
Returns: true if deadlock is detected, otherwise false.
*/
bool DeadlockManager::detectDeadlock(int &victimPID, string &victimName) {
    for (size_t i = 0; i < records.size(); i++) {
        for (size_t j = 0; j < records.size(); j++) {
            if (i == j) {
                continue;
            }

            bool firstWaitingForSecond =
                records[i].waitingForResource == records[j].holdingResource;

            bool secondWaitingForFirst =
                records[j].waitingForResource == records[i].holdingResource;

            if (firstWaitingForSecond && secondWaitingForFirst) {
                victimPID = records[j].pid;
                victimName = records[j].processName;
                return true;
            }
        }
    }

    return false;
}
