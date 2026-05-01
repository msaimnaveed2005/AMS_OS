#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <iostream>
#include <string>
#include <map>

using namespace std;

enum ProcessState {
    NEW_STATE,
    READY_STATE,
    RUNNING_STATE,
    BLOCKED_STATE,
    TERMINATED_STATE
};

enum ProcessType {
    SYSTEM_PROCESS,
    INTERACTIVE_PROCESS,
    BACKGROUND_PROCESS,
    AUTO_RUNNING_PROCESS,
    KERNEL_PROCESS
};

struct PCB {
    int pid;
    string processName;
    ProcessType processType;
    ProcessState processState;
    int priority;
    int ramRequired;
    int hddRequired;
    int coresRequired;
    int waitingTime;
};

class ProcessManager {
private:
    map<int, PCB> processTable;
    int dummyPIDCounter;

public:
    /*
    Function: ProcessManager
    Purpose: Initializes the process manager and dummy PID counter.
    Parameters: None.
    Returns: Nothing.
    */
    ProcessManager();

    /*
    Function: createPCB
    Purpose: Creates a PCB and stores it in the process table.
    Parameters: PID, process name, process type, priority, RAM, HDD, and CPU cores.
    Returns: true if PCB is created successfully, otherwise false.
    */
    bool createPCB(
        int pid,
        string processName,
        ProcessType processType,
        int priority,
        int ramRequired,
        int hddRequired,
        int coresRequired
    );

    /*
    Function: createDummyPCB
    Purpose: Creates a dummy PCB for testing process tracking before fork is implemented.
    Parameters: Process name, process type, priority, RAM, HDD, and CPU cores.
    Returns: Generated dummy PID.
    */
    int createDummyPCB(
        string processName,
        ProcessType processType,
        int priority,
        int ramRequired,
        int hddRequired,
        int coresRequired
    );

    /*
    Function: updateProcessState
    Purpose: Updates the state of an existing process.
    Parameters: PID and new process state.
    Returns: true if state is updated, otherwise false.
    */
    bool updateProcessState(int pid, ProcessState newState);

    /*
    Function: removeProcess
    Purpose: Removes a process from the process table.
    Parameters: PID of the process.
    Returns: true if process is removed, otherwise false.
    */
    bool removeProcess(int pid);

    /*
    Function: processExists
    Purpose: Checks whether a process exists in the process table.
    Parameters: PID of the process.
    Returns: true if process exists, otherwise false.
    */
    bool processExists(int pid);

    /*
    Function: displayPCBTable
    Purpose: Displays all processes currently stored in the process table.
    Parameters: None.
    Returns: Nothing.
    */
    void displayPCBTable();

    /*
    Function: getProcessTypeName
    Purpose: Converts process type enum into readable text.
    Parameters: Process type.
    Returns: Process type as string.
    */
    string getProcessTypeName(ProcessType type);

    /*
    Function: getProcessStateName
    Purpose: Converts process state enum into readable text.
    Parameters: Process state.
    Returns: Process state as string.
    */
    string getProcessStateName(ProcessState state);
};

#endif