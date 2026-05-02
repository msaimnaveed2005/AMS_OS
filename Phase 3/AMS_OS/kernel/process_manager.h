#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

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
int memoryStart;
int memoryEnd;
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
	Function: getPCB
	Purpose: Finds a PCB by PID and copies it into the reference variable.
	Parameters: PID and PCB reference variable.
	Returns: true if PCB exists, otherwise false.
	*/
	bool getPCB(int pid, PCB &pcb);

/*
Function: incrementWaitingTime
Purpose: Increases waiting time of a process by one unit.
Parameters: PID.
Returns: true if waiting time is updated, otherwise false.
*/
bool incrementWaitingTime(int pid);

/*
Function: resetWaitingTime
Purpose: Resets waiting time of a process to zero.
Parameters: PID.
Returns: true if waiting time is reset, otherwise false.
*/
bool resetWaitingTime(int pid);

/*
Function: improvePriority
Purpose: Improves process priority by reducing priority number.
Parameters: PID.
Returns: true if priority is improved, otherwise false.
*/
bool improvePriority(int pid);

/*
Function: updateProcessPriority
Purpose: Updates process priority manually.
Parameters: PID and new priority.
Returns: true if priority is updated, otherwise false.
*/
bool updateProcessPriority(int pid, int newPriority);


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

    /*
	Function: getAllPIDs
	Purpose: Returns all process IDs currently stored in PCB table.
	Parameters: None.
	Returns: Vector of process IDs.
	*/
	vector<int> getAllPIDs();
/*
Function: updateMemoryBlock
Purpose: Updates the memory start and end address of a process.
Parameters: PID, memory start, and memory end.
Returns: true if memory block is updated, otherwise false.
*/
bool updateMemoryBlock(int pid, int memoryStart, int memoryEnd);
};


#endif