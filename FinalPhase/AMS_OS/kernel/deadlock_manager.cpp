#include "deadlock_manager.h"
#include <iomanip>
#include <map>
#include <set>
#include <functional>

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
    cout << "\n==================== RESOURCE WAIT GRAPH ====================\n";

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
    map<int, vector<int>> waitForGraph;
    map<int, string> pidToName;

    for (const DeadlockRecord &record : records) {
        pidToName[record.pid] = record.processName;
    }

    for (const DeadlockRecord &waitingRecord : records) {
        for (const DeadlockRecord &holdingRecord : records) {
            if (waitingRecord.pid == holdingRecord.pid) {
                continue;
            }

            if (waitingRecord.waitingForResource == holdingRecord.holdingResource) {
                waitForGraph[waitingRecord.pid].push_back(holdingRecord.pid);
            }
        }
    }

    set<int> globalVisited;
    set<int> activeStack;

    function<bool(int)> hasCycle = [&](int currentPID) {
        globalVisited.insert(currentPID);
        activeStack.insert(currentPID);

        for (int nextPID : waitForGraph[currentPID]) {
            if (activeStack.find(nextPID) != activeStack.end()) {
                victimPID = max(currentPID, nextPID);
                victimName = pidToName[victimPID];
                return true;
            }

            if (globalVisited.find(nextPID) == globalVisited.end() && hasCycle(nextPID)) {
                return true;
            }
        }

        activeStack.erase(currentPID);
        return false;
    };

    for (const auto &entry : pidToName) {
        if (globalVisited.find(entry.first) == globalVisited.end()) {
            if (hasCycle(entry.first)) {
                return true;
            }
        }
    }

    return false;
}
