#include "process_manager.h"
#include <iomanip>
#include "console_colors.h"

/*
Function: getPCBStateColor
Purpose: Returns color based on process state.
Parameters: Process state.
Returns: ANSI color code.
*/
string getPCBStateColor(ProcessState state) {
    if (state == NEW_STATE) {
        return Color::BRIGHT_BLUE + Color::BOLD;
    }

    if (state == READY_STATE) {
        return Color::BRIGHT_CYAN + Color::BOLD;
    }

    if (state == RUNNING_STATE) {
        return Color::BRIGHT_GREEN + Color::BOLD;
    }

    if (state == BLOCKED_STATE) {
        return Color::BRIGHT_YELLOW + Color::BOLD;
    }

    if (state == TERMINATED_STATE) {
        return Color::BRIGHT_RED + Color::BOLD;
    }

    return Color::WHITE;
}

/*
Function: getPCBTypeColor
Purpose: Returns color based on process type.
Parameters: Process type.
Returns: ANSI color code.
*/
string getPCBTypeColor(ProcessType type) {
    if (type == SYSTEM_PROCESS || type == KERNEL_PROCESS) {
        return Color::BRIGHT_MAGENTA + Color::BOLD;
    }

    if (type == INTERACTIVE_PROCESS) {
        return Color::BRIGHT_GREEN + Color::BOLD;
    }

    if (type == BACKGROUND_PROCESS) {
        return Color::BRIGHT_YELLOW + Color::BOLD;
    }

    if (type == AUTO_RUNNING_PROCESS) {
        return Color::BRIGHT_CYAN + Color::BOLD;
    }

    return Color::WHITE;
}

/*
Function: getPCBPriorityColor
Purpose: Returns color based on priority.
Parameters: Priority.
Returns: ANSI color code.
*/
string getPCBPriorityColor(int priority) {
    if (priority == 1) {
        return Color::BRIGHT_RED + Color::BOLD;
    }

    if (priority == 2) {
        return Color::BRIGHT_GREEN + Color::BOLD;
    }

    return Color::BRIGHT_YELLOW + Color::BOLD;
}
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
        cout << "\n" << Color::process("[PROCESS MANAGER]") << " Invalid PID. PCB creation failed.\n";
        return false;
    }

    if (processName.empty()) {
        cout << "\n" << Color::process("[PROCESS MANAGER]") << " Invalid process name. PCB creation failed.\n";
        return false;
    }

    if (processTable.find(pid) != processTable.end()) {
        cout << "\n" << Color::process("[PROCESS MANAGER]") << " Process with this PID already exists.\n";
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
	newProcess.memoryStart = -1;
	newProcess.memoryEnd = -1;
    processTable[pid] = newProcess;

    cout << "\n" << Color::process("[PROCESS MANAGER]") << " PCB created successfully.\n";
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
        cout << "\n" << Color::process("[PROCESS MANAGER]") << " Process not found. State update failed.\n";
        return false;
    }

    processTable[pid].processState = newState;

    cout << "\n" << Color::process("[PROCESS MANAGER]") << " Process state updated.\n";
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
        cout << "\n" << Color::process("[PROCESS MANAGER]") << " Process not found. Removal failed.\n";
        return false;
    }

    cout << "\n" << Color::process("[PROCESS MANAGER]") << " Removing process from PCB table.\n";
    cout << "PID: " << pid << "\n";
    cout << "Process Name: " << processTable[pid].processName << "\n";

    processTable.erase(pid);

    cout << Color::process("[PROCESS MANAGER]") << " Process removed successfully.\n";

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
	Function: getPCB
	Purpose: Finds a PCB by PID and copies it into the reference variable.
	Parameters: PID and PCB reference variable.
	Returns: true if PCB exists, otherwise false.
	*/
	bool ProcessManager::getPCB(int pid, PCB &pcb) {
	    if (processTable.find(pid) == processTable.end()) {
		return false;
	    }

	    pcb = processTable[pid];
	    return true;
	}

/*
Function: incrementWaitingTime
Purpose: Increases waiting time of a process by one unit.
Parameters: PID.
Returns: true if waiting time is updated, otherwise false.
*/
bool ProcessManager::incrementWaitingTime(int pid) {
    if (processTable.find(pid) == processTable.end()) {
        return false;
    }

    processTable[pid].waitingTime++;
    return true;
}

/*
Function: resetWaitingTime
Purpose: Resets waiting time of a process to zero.
Parameters: PID.
Returns: true if waiting time is reset, otherwise false.
*/
bool ProcessManager::resetWaitingTime(int pid) {
    if (processTable.find(pid) == processTable.end()) {
        return false;
    }

    processTable[pid].waitingTime = 0;
    return true;
}

/*
Function: improvePriority
Purpose: Improves process priority by reducing priority number.
Parameters: PID.
Returns: true if priority is improved, otherwise false.
*/
bool ProcessManager::improvePriority(int pid) {
    if (processTable.find(pid) == processTable.end()) {
        return false;
    }

    if (processTable[pid].priority > 1) {
        processTable[pid].priority--;
        processTable[pid].waitingTime = 0;
        return true;
    }

    return false;
}

/*
Function: updateProcessPriority
Purpose: Updates process priority manually.
Parameters: PID and new priority.
Returns: true if priority is updated, otherwise false.
*/
bool ProcessManager::updateProcessPriority(int pid, int newPriority) {
    if (processTable.find(pid) == processTable.end()) {
        return false;
    }

    if (newPriority < 1) {
        newPriority = 1;
    }

    if (newPriority > 3) {
        newPriority = 3;
    }

    processTable[pid].priority = newPriority;
    return true;
}

/*
Function: updateMemoryBlock
Purpose: Updates the memory start and end address of a process.
Parameters: PID, memory start, and memory end.
Returns: true if memory block is updated, otherwise false.
*/
bool ProcessManager::updateMemoryBlock(int pid, int memoryStart, int memoryEnd) {
    if (processTable.find(pid) == processTable.end()) {
        return false;
    }

    processTable[pid].memoryStart = memoryStart;
    processTable[pid].memoryEnd = memoryEnd;

    return true;
}


/*
Function: displayPCBTable
Purpose: Displays all processes currently stored in the process table.
Parameters: None.
Returns: Nothing.
*/
void ProcessManager::displayPCBTable() {
   cout << "\n";
Color::line('=', 126, Color::BRIGHT_CYAN + Color::BOLD);
cout << Color::paint("                                           PCB TABLE\n", Color::BRIGHT_CYAN + Color::BOLD);
Color::line('=', 126, Color::BRIGHT_CYAN + Color::BOLD);

// Headers with color
Color::cell("PID", 8, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("NAME", 22, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("TYPE", 18, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("STATE", 15, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("PRI", 8, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("WAIT", 8, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("RAM BLOCK", 18, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("RAM", 10, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("HDD", 10, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("CPU", 6, Color::BRIGHT_CYAN + Color::BOLD);
cout << "\n";

// Line separator for table rows
Color::line('-', 126, Color::GRAY);

    for (auto process : processTable) {
        PCB pcb = process.second;

        string ramBlock;

        if (pcb.memoryStart == -1 || pcb.memoryEnd == -1) {
            ramBlock = "N/A";
        } else {
            ramBlock = to_string(pcb.memoryStart) + "-" + to_string(pcb.memoryEnd);
        }

        Color::cell(to_string(pcb.pid), 8, Color::WHITE);
        Color::cell(pcb.processName, 22, Color::BRIGHT_WHITE + Color::BOLD);
        Color::cell(getProcessTypeName(pcb.processType), 18, getPCBTypeColor(pcb.processType));
        Color::cell(getProcessStateName(pcb.processState), 15, getPCBStateColor(pcb.processState));
        Color::cell(to_string(pcb.priority), 8, getPCBPriorityColor(pcb.priority));
        Color::cell(to_string(pcb.waitingTime), 8, Color::BRIGHT_BLUE);
        Color::cell(ramBlock, 18, Color::BRIGHT_MAGENTA);
        Color::cell(to_string(pcb.ramRequired) + "MB", 10, Color::BRIGHT_BLUE);
        Color::cell(to_string(pcb.hddRequired) + "MB", 10, Color::BRIGHT_MAGENTA);
        Color::cell(to_string(pcb.coresRequired), 6, Color::BRIGHT_GREEN);
        cout << "\n";
    }

    Color::line('=', 126, Color::BRIGHT_CYAN + Color::BOLD);

    cout << Color::paint("State Colors: ", Color::WHITE + Color::BOLD)
         << Color::paint("READY", Color::BRIGHT_CYAN + Color::BOLD)
         << " | "
         << Color::paint("RUNNING", Color::BRIGHT_GREEN + Color::BOLD)
         << " | "
         << Color::paint("BLOCKED", Color::BRIGHT_YELLOW + Color::BOLD)
         << " | "
         << Color::paint("TERMINATED", Color::BRIGHT_RED + Color::BOLD)
         << "\n";
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

/*
Function: getAllPIDs
Purpose: Returns all process IDs currently stored in PCB table.
Parameters: None.
Returns: Vector of process IDs.
*/
vector<int> ProcessManager::getAllPIDs() {
    vector<int> pids;

    for (auto process : processTable) {
        pids.push_back(process.first);
    }

    return pids;
}

/*
Function: getAllPCBs
Purpose: Returns all process control blocks currently stored in the PCB table.
Parameters: None.
Returns: Vector of PCB records.
*/
vector<PCB> ProcessManager::getAllPCBs() {
    vector<PCB> pcbList;

    for (auto process : processTable) {
        pcbList.push_back(process.second);
    }

    return pcbList;
}