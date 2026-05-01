#include "process_manager.h"

/*
Function: ProcessManager
Purpose: Initializes the process manager and dummy PID counter.
Parameters: None.
Returns: Nothing.
*/
ProcessManager::ProcessManager() {
    dummyPIDCounter = 1000;
}

/*
Function: createPCB
Purpose: Creates a PCB and stores it in the process table.
Parameters: PID, process name, process type, priority, RAM, HDD, and CPU cores.
Returns: true if PCB is created successfully, otherwise false.
*/
bool ProcessManager::createPCB(
    int pid,
    string processName,
    ProcessType processType,
    int priority,
    int ramRequired,
    int hddRequired,
    int coresRequired
) {
    if (pid <= 0) {
        cout << "\n[PROCESS MANAGER] Invalid PID. PCB creation failed.\n";
        return false;
    }

    if (processName.empty()) {
        cout << "\n[PROCESS MANAGER] Invalid process name. PCB creation failed.\n";
        return false;
    }

    if (processTable.find(pid) != processTable.end()) {
        cout << "\n[PROCESS MANAGER] Process with this PID already exists.\n";
        return false;
    }

    PCB newProcess;

    newProcess.pid = pid;
    newProcess.processName = processName;
    newProcess.processType = processType;
    newProcess.processState = NEW_STATE;
    newProcess.priority = priority;
    newProcess.ramRequired = ramRequired;
    newProcess.hddRequired = hddRequired;
    newProcess.coresRequired = coresRequired;
    newProcess.waitingTime = 0;

    processTable[pid] = newProcess;

    cout << "\n[PROCESS MANAGER] PCB created successfully.\n";
    cout << "PID: " << pid << "\n";
    cout << "Process Name: " << processName << "\n";
    cout << "Process Type: " << getProcessTypeName(processType) << "\n";
    cout << "Initial State: " << getProcessStateName(NEW_STATE) << "\n";

    return true;
}

/*
Function: createDummyPCB
Purpose: Creates a dummy PCB for testing process tracking before fork is implemented.
Parameters: Process name, process type, priority, RAM, HDD, and CPU cores.
Returns: Generated dummy PID.
*/
int ProcessManager::createDummyPCB(
    string processName,
    ProcessType processType,
    int priority,
    int ramRequired,
    int hddRequired,
    int coresRequired
) {
    dummyPIDCounter++;

    bool created = createPCB(
        dummyPIDCounter,
        processName,
        processType,
        priority,
        ramRequired,
        hddRequired,
        coresRequired
    );

    if (!created) {
        return -1;
    }

    return dummyPIDCounter;
}

/*
Function: updateProcessState
Purpose: Updates the state of an existing process.
Parameters: PID and new process state.
Returns: true if state is updated, otherwise false.
*/
bool ProcessManager::updateProcessState(int pid, ProcessState newState) {
    if (processTable.find(pid) == processTable.end()) {
        cout << "\n[PROCESS MANAGER] Process not found. State update failed.\n";
        return false;
    }

    processTable[pid].processState = newState;

    cout << "\n[PROCESS MANAGER] Process state updated.\n";
    cout << "PID: " << pid << "\n";
    cout << "New State: " << getProcessStateName(newState) << "\n";

    return true;
}

/*
Function: removeProcess
Purpose: Removes a process from the process table.
Parameters: PID of the process.
Returns: true if process is removed, otherwise false.
*/
bool ProcessManager::removeProcess(int pid) {
    if (processTable.find(pid) == processTable.end()) {
        cout << "\n[PROCESS MANAGER] Process not found. Removal failed.\n";
        return false;
    }

    cout << "\n[PROCESS MANAGER] Removing process from PCB table.\n";
    cout << "PID: " << pid << "\n";
    cout << "Process Name: " << processTable[pid].processName << "\n";

    processTable.erase(pid);

    cout << "[PROCESS MANAGER] Process removed successfully.\n";

    return true;
}

/*
Function: processExists
Purpose: Checks whether a process exists in the process table.
Parameters: PID of the process.
Returns: true if process exists, otherwise false.
*/
bool ProcessManager::processExists(int pid) {
    return processTable.find(pid) != processTable.end();
}

/*
Function: displayPCBTable
Purpose: Displays all processes currently stored in the process table.
Parameters: None.
Returns: Nothing.
*/
void ProcessManager::displayPCBTable() {
    cout << "\n==================== PCB TABLE ====================\n";

    if (processTable.empty()) {
        cout << "No process exists in the PCB table.\n";
        cout << "===================================================\n";
        return;
    }

    cout << "PID\tName\t\tType\t\tState\t\tPriority\tRAM\tHDD\tCPU\n";
    cout << "---------------------------------------------------\n";

    for (auto process : processTable) {
        PCB pcb = process.second;

        cout << pcb.pid << "\t"
             << pcb.processName << "\t\t"
             << getProcessTypeName(pcb.processType) << "\t"
             << getProcessStateName(pcb.processState) << "\t\t"
             << pcb.priority << "\t\t"
             << pcb.ramRequired << "MB\t"
             << pcb.hddRequired << "MB\t"
             << pcb.coresRequired << "\n";
    }

    cout << "===================================================\n";
}

/*
Function: getProcessTypeName
Purpose: Converts process type enum into readable text.
Parameters: Process type.
Returns: Process type as string.
*/
string ProcessManager::getProcessTypeName(ProcessType type) {
    switch (type) {
        case SYSTEM_PROCESS:
            return "System";

        case INTERACTIVE_PROCESS:
            return "Interactive";

        case BACKGROUND_PROCESS:
            return "Background";

        case AUTO_RUNNING_PROCESS:
            return "Auto";

        case KERNEL_PROCESS:
            return "Kernel";

        default:
            return "Unknown";
    }
}

/*
Function: getProcessStateName
Purpose: Converts process state enum into readable text.
Parameters: Process state.
Returns: Process state as string.
*/
string ProcessManager::getProcessStateName(ProcessState state) {
    switch (state) {
        case NEW_STATE:
            return "NEW";

        case READY_STATE:
            return "READY";

        case RUNNING_STATE:
            return "RUNNING";

        case BLOCKED_STATE:
            return "BLOCKED";

        case TERMINATED_STATE:
            return "TERMINATED";

        default:
            return "UNKNOWN";
    }
}