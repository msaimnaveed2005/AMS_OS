#include <iostream>
#include <unistd.h>
#include <limits>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <signal.h>
#include <cerrno>
#include <cstring>
#include <vector>

#include "resource_manager.h"
#include "process_manager.h"
#include "task_catalog.h"
#include "ready_queue.h"
#include "scheduler.h"
#include "logger.h"
#include "deadlock_manager.h"
#include "sync_manager.h"

using namespace std;

enum OSMode {
    USER_MODE,
    KERNEL_MODE
};

struct IPCResourceRequest {
    char processName[100];
    int processType;
    int priority;
    int ramRequired;
    int hddRequired;
    int coresRequired;
};

struct IPCResourceResponse {
    int granted;
};

/*
Function: bootScreen
Purpose: Displays the AMS OS boot screen with loading effect.
Parameters: None.
Returns: Nothing.
*/
void bootScreen() {
    cout << "=====================================\n";
    cout << "              AMS OS\n";
    cout << "      Atomic Management System\n";
    cout << "=====================================\n";

    cout << "Booting AMS OS";

    for (int i = 0; i < 3; i++) {
        cout << ".";
        cout.flush();
        sleep(1);
    }

    cout << "\nSystem Loaded Successfully.\n";
}

/*
Function: getHardwareResourcesFromCommandLine
Purpose: Reads RAM, HDD, and CPU cores from command-line arguments before OS starts.
Parameters: argc, argv, and references to RAM, HDD, and CPU core variables.
Returns: true if valid resources are provided, otherwise false.
*/
bool getHardwareResourcesFromCommandLine(
    int argc,
    char* argv[],
    int &ram,
    int &hdd,
    int &cores
) {
    if (argc != 4) {
        cout << "\nInvalid startup command.\n";
        cout << "Usage: ./OS <RAM_GB> <HDD_GB> <CPU_CORES>\n";
        cout << "Example: ./OS 2 256 8\n";
        return false;
    }

    int ramGB = atoi(argv[1]);
    int hddGB = atoi(argv[2]);
    cores = atoi(argv[3]);

    if (ramGB <= 0 || hddGB <= 0 || cores <= 0) {
        cout << "\nInvalid hardware resources entered.\n";
        cout << "RAM, HDD, and CPU cores must be greater than zero.\n";
        return false;
    }

    ram = ramGB * 1024;
    hdd = hddGB * 1024;

    cout << "\n========== HARDWARE RESOURCE SETUP ==========\n";
    cout << "RAM Provided: " << ramGB << " GB (" << ram << " MB)\n";
    cout << "Hard Drive Provided: " << hddGB << " GB (" << hdd << " MB)\n";
    cout << "CPU Cores Provided: " << cores << "\n";
    cout << "=============================================\n";

    return true;
}

/*
Function: clearInputBuffer
Purpose: Clears invalid input from the input buffer to prevent program errors.
Parameters: None.
Returns: Nothing.
*/
void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

/*
Function: getValidatedInteger
Purpose: Takes integer input from the user and validates it.
Parameters: Message to display before taking input.
Returns: Valid integer entered by the user.
*/
int getValidatedInteger(string message) {
    int value;

    while (true) {
        cout << message;
        cin >> value;

        if (cin.fail()) {
            cout << "Invalid input. Please enter a valid number.\n";
            clearInputBuffer();
        } else {
            return value;
        }
    }
}
/*
Function: getModeName
Purpose: Converts OS mode into readable text.
Parameters: Current OS mode.
Returns: Mode name as string.
*/
string getModeName(OSMode mode) {
    if (mode == USER_MODE) {
        return "USER MODE";
    }

    return "KERNEL MODE";
}
/*
Function: showMainMenu
Purpose: Displays the main AMS OS menu according to current OS mode.
Parameters: Current OS mode.
Returns: Nothing.
*/
/*
Function: showMainMenu
Purpose: Displays the AMS OS menu according to current user or kernel mode.
Parameters: Current OS mode.
Returns: Nothing.
*/
void showMainMenu(OSMode currentMode) {
    cout << "\n========== AMS OS MAIN MENU ==========\n";
    cout << "Current Mode: " << getModeName(currentMode) << "\n";
    cout << "--------------------------------------\n";

    cout << "1. Show Task Catalog\n";
    cout << "2. Show Task Details\n";
    cout << "3. Launch Task\n";
    cout << "4. Show Resources\n";
    cout << "20. Show RAM Memory Layout\n";
    cout << "7. Show PCB Table\n";
    cout << "11. Run Scheduler\n";
    cout << "12. Show Ready Queues\n";
    cout << "17. Minimize Process\n";
    cout << "18. Resume Process\n";
    cout << "21. Close Process\n";
    cout << "22. Switch to Process\n";
    if (currentMode == USER_MODE) {
        cout << "13. Switch to Kernel Mode\n";
    }

    if (currentMode == KERNEL_MODE) {
        cout << "\n---------- KERNEL MODE TOOLS ----------\n";
        cout << "5. Diagnostic: Test Resource Allocation\n";
        cout << "6. Diagnostic: Test Resource Release\n";
        cout << "8. Diagnostic: Create Dummy PCB\n";
        cout << "9. Diagnostic: Update Process State\n";
        cout << "10. Diagnostic: Remove PCB\n";
        cout << "14. Switch to User Mode\n";
        cout << "15. View System Log\n";
        cout << "16. Kill Process\n";
        cout << "19. Deadlock Detection\n";
    }

    cout << "0. Shutdown AMS OS\n";
    cout << "======================================\n";
}

/*
Function: showComingSoonMessage
Purpose: Displays a message for modules that will be connected in future steps.
Parameters: Name of the selected module.
Returns: Nothing.
*/
void showComingSoonMessage(string moduleName) {
    cout << "\n[" << moduleName << "] module selected.\n";
    cout << "This module will be connected in the next implementation steps.\n";
}

/*
Function: testResourceAllocation
Purpose: Allows the user to manually test resource allocation.
Parameters: ResourceManager object reference.
Returns: Nothing.
*/
void testResourceAllocation(ResourceManager &resourceManager) {
    int reqRAM;
    int reqHDD;
    int reqCores;

    cout << "\n========== TEST RESOURCE ALLOCATION ==========\n";

    reqRAM = getValidatedInteger("Enter RAM required by process in MB: ");
    reqHDD = getValidatedInteger("Enter HDD required by process in MB: ");
    reqCores = getValidatedInteger("Enter CPU cores required by process: ");

    resourceManager.allocateResources(reqRAM, reqHDD, reqCores);
    resourceManager.displayResources();
}

/*
Function: testResourceRelease
Purpose: Allows the user to manually test resource release.
Parameters: ResourceManager object reference.
Returns: Nothing.
*/
void testResourceRelease(ResourceManager &resourceManager) {
    int relRAM;
    int relHDD;
    int relCores;

    cout << "\n========== TEST RESOURCE RELEASE ==========\n";

    relRAM = getValidatedInteger("Enter RAM to release in MB: ");
    relHDD = getValidatedInteger("Enter HDD to release in MB: ");
    relCores = getValidatedInteger("Enter CPU cores to release: ");

    resourceManager.releaseResources(relRAM, relHDD, relCores);
    resourceManager.displayResources();
}

/*
Function: getProcessTypeFromChoice
Purpose: Converts user's process type choice into a ProcessType enum.
Parameters: User choice.
Returns: Selected process type.
*/
ProcessType getProcessTypeFromChoice(int choice) {
    switch (choice) {
        case 1:
            return SYSTEM_PROCESS;
        case 2:
            return INTERACTIVE_PROCESS;
        case 3:
            return BACKGROUND_PROCESS;
        case 4:
            return AUTO_RUNNING_PROCESS;
        case 5:
            return KERNEL_PROCESS;
        default:
            return INTERACTIVE_PROCESS;
    }
}

/*
Function: getProcessStateFromChoice
Purpose: Converts user's process state choice into a ProcessState enum.
Parameters: User choice.
Returns: Selected process state.
*/
ProcessState getProcessStateFromChoice(int choice) {
    switch (choice) {
        case 1:
            return NEW_STATE;
        case 2:
            return READY_STATE;
        case 3:
            return RUNNING_STATE;
        case 4:
            return BLOCKED_STATE;
        case 5:
            return TERMINATED_STATE;
        default:
            return READY_STATE;
    }
}

/*
Function: testDummyPCBCreation
Purpose: Creates a dummy PCB for testing before real fork based process creation.
Parameters: ProcessManager object reference.
Returns: Nothing.
*/
void testDummyPCBCreation(ProcessManager &processManager) {
    string processName;
    int typeChoice;
    int priority;
    int ramRequired;
    int hddRequired;
    int coresRequired;

    cout << "\n========== TEST DUMMY PCB CREATION ==========\n";

    cout << "Enter process name: ";
    cin >> processName;

    cout << "\nSelect Process Type:\n";
    cout << "1. System\n";
    cout << "2. Interactive\n";
    cout << "3. Background\n";
    cout << "4. Auto-running\n";
    cout << "5. Kernel\n";

    typeChoice = getValidatedInteger("Enter type choice: ");
    priority = getValidatedInteger("Enter process priority: ");
    ramRequired = getValidatedInteger("Enter RAM required in MB: ");
    hddRequired = getValidatedInteger("Enter HDD required in MB: ");
    coresRequired = getValidatedInteger("Enter CPU cores required: ");

    ProcessType selectedType = getProcessTypeFromChoice(typeChoice);

    int pid = processManager.createDummyPCB(
        processName,
        selectedType,
        priority,
        ramRequired,
        hddRequired,
        coresRequired
    );

    if (pid != -1) {
        cout << "\nDummy PCB created with PID: " << pid << "\n";
    }
}

/*
Function: testProcessStateUpdate
Purpose: Allows user to update the state of an existing process in PCB table.
Parameters: ProcessManager object reference.
Returns: Nothing.
*/
void testProcessStateUpdate(ProcessManager &processManager) {
    int pid;
    int stateChoice;

    cout << "\n========== TEST PROCESS STATE UPDATE ==========\n";

    pid = getValidatedInteger("Enter PID to update: ");

    cout << "\nSelect New Process State:\n";
    cout << "1. NEW\n";
    cout << "2. READY\n";
    cout << "3. RUNNING\n";
    cout << "4. BLOCKED\n";
    cout << "5. TERMINATED\n";

    stateChoice = getValidatedInteger("Enter state choice: ");

    ProcessState selectedState = getProcessStateFromChoice(stateChoice);

    processManager.updateProcessState(pid, selectedState);
}

/*
Function: testPCBRemoval
Purpose: Allows user to remove a process from the PCB table.
Parameters: ProcessManager object reference.
Returns: Nothing.
*/
void testPCBRemoval(ProcessManager &processManager) {
    int pid;

    cout << "\n========== TEST PCB REMOVAL ==========\n";

    pid = getValidatedInteger("Enter PID to remove: ");

    processManager.removeProcess(pid);
}

/*
Function: showTaskDetailsMenu
Purpose: Allows user to view complete details of a selected task.
Parameters: TaskCatalog object reference.
Returns: Nothing.
*/
void showTaskDetailsMenu(TaskCatalog &taskCatalog) {
    int taskID;

    cout << "\n========== TASK DETAILS MENU ==========\n";
    taskCatalog.displayAvailableTasks();

    taskID = getValidatedInteger("Enter Task ID to view details: ");

    taskCatalog.displayTaskDetails(taskID);
}

/*
Function: childSendResourceRequest
Purpose: Sends resource request from child process to parent kernel using IPC pipe.
Parameters: Write pipe, read pipe, and selected task metadata.
Returns: Response received from parent.
*/
IPCResourceResponse childSendResourceRequest(
    int requestWritePipe,
    int responseReadPipe,
    TaskInfo selectedTask
) {
    IPCResourceRequest request;
    IPCResourceResponse response;

    memset(&request, 0, sizeof(request));
    memset(&response, 0, sizeof(response));

    strncpy(request.processName, selectedTask.taskName.c_str(), sizeof(request.processName) - 1);
    request.processType = selectedTask.processType;
    request.priority = selectedTask.priority;
    request.ramRequired = selectedTask.ramRequired;
    request.hddRequired = selectedTask.hddRequired;
    request.coresRequired = selectedTask.coresRequired;

    cout << "\n[CHILD PROCESS] Sending IPC resource request to kernel.\n";
    write(requestWritePipe, &request, sizeof(request));

    read(responseReadPipe, &response, sizeof(response));

    return response;
}
/*
Function: executeTaskInSeparateTerminal
Purpose: Opens the selected task executable in a separate Xubuntu terminal window.
         If xfce4-terminal fails, it falls back to direct exec execution.
Parameters: Selected task metadata.
Returns: Nothing. If exec succeeds, this function does not return.
*/
void executeTaskInSeparateTerminal(TaskInfo selectedTask) {
    cout << "[CHILD PROCESS] Opening task in separate terminal window.\n";
    cout << "[CHILD PROCESS] Terminal: xfce4-terminal\n";
    cout << "[CHILD PROCESS] Executable Path: " << selectedTask.executablePath << "\n";

    /*
    Xubuntu uses xfce4-terminal.
    --execute runs the selected executable inside a new terminal window.
    */
    execlp(
        "xfce4-terminal",
        "xfce4-terminal",
        "--execute",
        selectedTask.executablePath.c_str(),
        selectedTask.taskName.c_str(),
        NULL
    );

    /*
    Fallback:
    If xfce4-terminal is not found or fails, run the executable directly.
    */
    perror("[CHILD PROCESS] xfce4-terminal failed");

    cout << "[CHILD PROCESS] Falling back to direct exec.\n";

    execl(
        selectedTask.executablePath.c_str(),
        selectedTask.executablePath.c_str(),
        selectedTask.taskName.c_str(),
        NULL
    );

    perror("[CHILD PROCESS] direct exec failed");
    exit(1);
}
/*
Function: launchTaskUsingIPCForkTest
Purpose: Creates a child process using fork, child sends resource request through IPC,
         parent grants or denies resources, and child runs only if granted.
Parameters: TaskCatalog, ProcessManager, and ResourceManager object references.
Returns: Nothing.
*/
void launchTaskUsingIPCForkTest(
    TaskCatalog &taskCatalog,
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager
) {
    int taskID;
    TaskInfo selectedTask;

    cout << "\n========== LAUNCH TASK USING IPC FORK TEST ==========\n";
    taskCatalog.displayAvailableTasks();

    taskID = getValidatedInteger("Enter Task ID to launch: ");

    if (!taskCatalog.getTaskByID(taskID, selectedTask)) {
        cout << "\nInvalid Task ID. No task found.\n";
        return;
    }
    logger.logProcessEvent(0, selectedTask.taskName, "Task selected for launch");
    cout << "\nSelected Task Information\n";
    cout << "Task Name: " << selectedTask.taskName << "\n";
    cout << "Task Type: " << taskCatalog.getProcessTypeName(selectedTask.processType) << "\n";
    cout << "RAM Required: " << selectedTask.ramRequired << " MB\n";
    cout << "HDD Required: " << selectedTask.hddRequired << " MB\n";
    cout << "CPU Required: " << selectedTask.coresRequired << "\n";

    int requestPipe[2];
    int responsePipe[2];

    if (pipe(requestPipe) == -1) {
        cout << "\n[AMS OS] Failed to create request pipe.\n";
        return;
    }

    if (pipe(responsePipe) == -1) {
        cout << "\n[AMS OS] Failed to create response pipe.\n";
        close(requestPipe[0]);
        close(requestPipe[1]);
        return;
    }

    cout.flush();

    pid_t pid = fork();

    if (pid < 0) {
        cout << "\n[AMS OS] Fork failed. Unable to create child process.\n";
        close(requestPipe[0]);
        close(requestPipe[1]);
        close(responsePipe[0]);
        close(responsePipe[1]);
        return;
    }

    if (pid == 0) {
        close(requestPipe[0]);
        close(responsePipe[1]);

        cout << "\n[CHILD PROCESS] Child created successfully.\n";
        cout << "[CHILD PROCESS] PID: " << getpid() << "\n";
        cout << "[CHILD PROCESS] Parent PID: " << getppid() << "\n";

        IPCResourceResponse response = childSendResourceRequest(
            requestPipe[1],
            responsePipe[0],
            selectedTask
        );

        close(requestPipe[1]);
        close(responsePipe[0]);

        if (response.granted == 0) {
            cout << "[CHILD PROCESS] Resource request denied by kernel.\n";
            cout << "[CHILD PROCESS] Terminating process.\n";
            exit(2);
        }

       cout << "[CHILD PROCESS] Resource request granted by kernel.\n";
	cout << "[CHILD PROCESS] Process is now waiting for scheduler dispatch.\n";

	raise(SIGSTOP);

	cout << "[CHILD PROCESS] Scheduler resumed this process.\n";
	cout << "[CHILD PROCESS] Loading task executable in separate terminal using exec.\n";

	executeTaskInSeparateTerminal(selectedTask);
}

   	close(requestPipe[1]);
   	close(responsePipe[0]);

    IPCResourceRequest request;
   IPCResourceResponse response;

    memset(&request, 0, sizeof(request));
    memset(&response, 0, sizeof(response));

    read(requestPipe[0], &request, sizeof(request));

    cout << "\n[KERNEL/PARENT] IPC resource request received.\n";
    cout << "Child PID: " << pid << "\n";
    cout << "Process Name: " << request.processName << "\n";
    cout << "RAM Requested: " << request.ramRequired << " MB\n";
    cout << "HDD Requested: " << request.hddRequired << " MB\n";
    cout << "CPU Requested: " << request.coresRequired << "\n";

    if (resourceManager.checkResources(
            request.ramRequired,
            request.hddRequired,
            request.coresRequired
        )) {
        response.granted = 1;

        cout << "\n[KERNEL/PARENT] Resources available. Granting request.\n";
	int memoryStart = -1;
int memoryEnd = -1;

bool memoryAllocated = resourceManager.allocateMemoryBlock(
    pid,
    request.processName,
    request.ramRequired,
    memoryStart,
    memoryEnd
);

if (!memoryAllocated) {
    response.granted = 0;

    cout << "\n[KERNEL/PARENT] RAM block allocation failed. Denying request.\n";

    logger.logResourceEvent(
        "RAM block allocation failed for " + string(request.processName)
    );

    write(responsePipe[1], &response, sizeof(response));

    close(requestPipe[0]);
    close(responsePipe[1]);

    int status;
    waitpid(pid, &status, 0);

    return;
}
        resourceManager.allocateResources(
            request.ramRequired,
            request.hddRequired,
            request.coresRequired
        );
	logger.logResourceEvent(
	    "Resources granted to " + string(request.processName) +
	    " | RAM: " + to_string(request.ramRequired) +
	    "MB | HDD: " + to_string(request.hddRequired) +
	    "MB | CPU: " + to_string(request.coresRequired)
	);
       processManager.createPCB(
	    pid,
	    request.processName,
	    static_cast<ProcessType>(request.processType),
	    request.priority,
	    request.ramRequired,
	    request.hddRequired,
	    request.coresRequired);
processManager.updateMemoryBlock(pid, memoryStart, memoryEnd);

logger.logResourceEvent(
    "RAM block assigned to PID " + to_string(pid) +
    " | " + string(request.processName) +
    " | Start: " + to_string(memoryStart) +
    "MB | End: " + to_string(memoryEnd) + "MB"
);

logger.logProcessEvent(pid, request.processName, "PCB Created");

processManager.updateProcessState(pid, READY_STATE);

readyQueueManager.addProcessToReadyQueue(
    pid,
    request.processName,
    static_cast<ProcessType>(request.processType),
    request.priority
);
syncManager.notifyReadyQueue();

logger.logProcessEvent(pid, request.processName, "Added to Ready Queue");
cout << "\n[KERNEL/PARENT] Current Ready Queue Status:\n";
readyQueueManager.displayReadyQueues();

    } else {
        response.granted = 0;

        cout << "\n[KERNEL/PARENT] Resources unavailable. Denying request.\n";
	logger.logResourceEvent("Resources denied for process " + string(request.processName));
    }

    write(responsePipe[1], &response, sizeof(response));

    close(requestPipe[0]);
    close(responsePipe[1]);

    int status;

	if (response.granted == 1) {
	    waitpid(pid, &status, WUNTRACED);

	    if (WIFSTOPPED(status)) {
		cout << "\n[KERNEL/PARENT] Child process is paused and waiting in ready queue.\n";
		cout << "[KERNEL/PARENT] Run scheduler from menu to execute this process.\n";
	    }

	    cout << "\n[KERNEL/PARENT] Current PCB Table:\n";
	    processManager.displayPCBTable();

	    cout << "\n[KERNEL/PARENT] Current Ready Queue Status:\n";
	    readyQueueManager.displayReadyQueues();
	} else {
	    waitpid(pid, &status, 0);

	    cout << "\n[KERNEL/PARENT] Denied child process has terminated.\n";
	    resourceManager.displayResources();
	}
}

/*
Function: shutdownScreen
Purpose: Displays shutdown animation and closing message.
Parameters: None.
Returns: Nothing.
*/
void shutdownScreen() {
    cout << "\nShutting down AMS OS";

    for (int i = 0; i < 3; i++) {
        cout << ".";
        cout.flush();
        sleep(1);
    }

    cout << "\nAMS OS shutdown completed successfully.\n";
}

/*
Function: createRequiredDirectories
Purpose: Creates required folders for build output and virtual disk storage.
Parameters: None.
Returns: Nothing.
*/
void createRequiredDirectories() {
    system("mkdir -p build");
    system("mkdir -p data");
    system("mkdir -p data/virtual_disk");
}

/*
Function: authenticateKernelMode
Purpose: Authenticates user before switching to kernel mode.
Parameters: None.
Returns: true if password is correct, otherwise false.
*/
bool authenticateKernelMode() {
    string password;

    cout << "\n========== KERNEL MODE AUTHENTICATION ==========\n";
    cout << "Enter kernel password: ";
    cin >> password;

    if (password == "admin") {
        cout << "Kernel mode access granted.\n";
        return true;
    }

    cout << "Incorrect password. Kernel mode access denied.\n";
    return false;
}


/*
Function: viewSystemLog
Purpose: Displays the system log file content.
Parameters: None.
Returns: Nothing.
*/
void viewSystemLog() {
    ifstream file("data/system_log.txt");

    if (!file) {
        cout << "\nNo system log file found.\n";
        return;
    }

    string line;

    cout << "\n==================== SYSTEM LOG ====================\n";

    while (getline(file, line)) {
        cout << line << "\n";
    }

    cout << "====================================================\n";

    file.close();
}

/*
Function: kernelKillProcess
Purpose: Allows Kernel Mode to terminate a process by PID, release its resources,
         remove it from ready queue, remove PCB, and log the event.
Parameters: ProcessManager, ResourceManager, ReadyQueueManager, and Logger references.
Returns: Nothing.
*/
void kernelKillProcess(
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger
) {
    int pid;
    char confirmation;
    PCB pcb;

    cout << "\n========== KERNEL MODE PROCESS KILLER ==========\n";

    processManager.displayPCBTable();

    pid = getValidatedInteger("Enter PID to kill: ");

    if (!processManager.getPCB(pid, pcb)) {
        cout << "\n[KERNEL] No process found with PID: " << pid << "\n";
        logger.logProcessEvent(pid, "Unknown", "Kernel kill failed, PID not found");
        return;
    }

    cout << "\nProcess selected for termination:\n";
    cout << "PID: " << pcb.pid << "\n";
    cout << "Process Name: " << pcb.processName << "\n";
    cout << "RAM: " << pcb.ramRequired << " MB\n";
    cout << "HDD: " << pcb.hddRequired << " MB\n";
    cout << "CPU Cores: " << pcb.coresRequired << "\n";

    cout << "\nAre you sure you want to kill this process? (y/n): ";
    cin >> confirmation;

    if (confirmation != 'y' && confirmation != 'Y') {
        cout << "\n[KERNEL] Process kill cancelled.\n";
        logger.logProcessEvent(pid, pcb.processName, "Kernel kill cancelled");
        return;
    }

   cout << "\n[KERNEL] Sending force termination signal to PID: " << pid << "\n";

	/*
	Reason:
	The process may be paused with SIGSTOP while waiting in the ready queue.
	SIGTERM may not terminate a stopped process immediately, so the program can get stuck at waitpid().
	SIGKILL is used here because Kernel Mode Process Killer should forcefully terminate the process.
	*/
	int killResult = kill(pid, SIGKILL);

	if (killResult == 0) {
	    int status;
	    int waitResult = waitpid(pid, &status, WNOHANG);

	    if (waitResult == 0) {
		cout << "[KERNEL] Process kill signal sent. Waiting briefly for cleanup.\n";
		sleep(1);
		waitResult = waitpid(pid, &status, WNOHANG);
	    }

	    if (waitResult == pid) {
		cout << "[KERNEL] Process terminated successfully using SIGKILL.\n";
		logger.logProcessEvent(pid, pcb.processName, "Terminated by Kernel Mode using SIGKILL");
	    } else {
		cout << "[KERNEL] Kill signal sent, but process was not collected immediately.\n";
		cout << "[KERNEL] Continuing AMS OS cleanup to avoid blocking.\n";
		logger.logProcessEvent(pid, pcb.processName, "SIGKILL sent, non-blocking cleanup continued");
	    }
	} else {
	    cout << "[KERNEL] Could not send SIGKILL.\n";
	    cout << "[KERNEL] Error: " << strerror(errno) << "\n";
	    cout << "[KERNEL] Cleaning AMS OS PCB and resource records only.\n";

	    logger.logProcessEvent(pid, pcb.processName, "SIGKILL failed, cleaning AMS OS records");
	}
    readyQueueManager.removeProcessByPID(pid);

    processManager.updateProcessState(pid, TERMINATED_STATE);

    resourceManager.releaseResources(
        pcb.ramRequired,
        pcb.hddRequired,
        pcb.coresRequired
    );

    logger.logResourceEvent(
        "Resources released after kernel kill | PID: " + to_string(pid) +
        " | RAM: " + to_string(pcb.ramRequired) +
        "MB | HDD: " + to_string(pcb.hddRequired) +
        "MB | CPU: " + to_string(pcb.coresRequired)
    );

    processManager.removeProcess(pid);

    cout << "\n[KERNEL] Process cleanup completed.\n";

    cout << "\nUpdated PCB Table:\n";
    processManager.displayPCBTable();

    cout << "\nUpdated Resource Status:\n";
    resourceManager.displayResources();

    cout << "\nUpdated Ready Queue Status:\n";
    readyQueueManager.displayReadyQueues();
}

/*
Function: minimizeProcess
Purpose: Simulates an interrupt by moving a process to BLOCKED state.
         The process is removed from ready queue but resources remain allocated.
Parameters: ProcessManager, ReadyQueueManager, and Logger references.
Returns: Nothing.
*/
void minimizeProcess(
    ProcessManager &processManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger
) {
    int pid;
    PCB pcb;

    cout << "\n========== MINIMIZE PROCESS ==========\n";

    processManager.displayPCBTable();

    pid = getValidatedInteger("Enter PID to minimize: ");

    if (!processManager.getPCB(pid, pcb)) {
        cout << "\n[INTERRUPT HANDLER] No process found with PID: " << pid << "\n";
        logger.logProcessEvent(pid, "Unknown", "Minimize failed, PID not found");
        return;
    }

    if (pcb.processState == TERMINATED_STATE) {
        cout << "\n[INTERRUPT HANDLER] Cannot minimize a terminated process.\n";
        return;
    }

    cout << "\n[INTERRUPT HANDLER] Minimizing process.\n";
    cout << "PID: " << pcb.pid << "\n";
    cout << "Process Name: " << pcb.processName << "\n";

    /*
    If process is currently waiting in ready queue, remove it.
    This prevents scheduler from selecting a minimized process.
    */
    readyQueueManager.removeProcessByPID(pid);

    /*
    Try to pause real child process.
    If it is already stopped, this does not break the program.
    */
    kill(pid, SIGSTOP);

    processManager.updateProcessState(pid, BLOCKED_STATE);

    logger.logProcessEvent(pid, pcb.processName, "Process minimized and moved to BLOCKED state");

    cout << "\n[INTERRUPT HANDLER] Process minimized successfully.\n";
    cout << "RAM is still allocated, but CPU execution is paused.\n";

    cout << "\nUpdated PCB Table:\n";
    processManager.displayPCBTable();

    cout << "\nUpdated Ready Queue Status:\n";
    readyQueueManager.displayReadyQueues();
}

/*
Function: resumeProcess
Purpose: Resumes a minimized process by moving it from BLOCKED state to READY state.
         The process is inserted back into the correct ready queue.
Parameters: ProcessManager, ReadyQueueManager, and Logger references.
Returns: Nothing.
*/
void resumeProcess(
    ProcessManager &processManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager
) {
    int pid;
    PCB pcb;

    cout << "\n========== RESUME PROCESS ==========\n";

    processManager.displayPCBTable();

    pid = getValidatedInteger("Enter PID to resume: ");

    if (!processManager.getPCB(pid, pcb)) {
        cout << "\n[INTERRUPT HANDLER] No process found with PID: " << pid << "\n";
        logger.logProcessEvent(pid, "Unknown", "Resume failed, PID not found");
        return;
    }

    if (pcb.processState != BLOCKED_STATE) {
        cout << "\n[INTERRUPT HANDLER] Only BLOCKED processes can be resumed.\n";
        cout << "Current State: " << processManager.getProcessStateName(pcb.processState) << "\n";
        return;
    }

    processManager.updateProcessState(pid, READY_STATE);

    readyQueueManager.addProcessToReadyQueue(
        pid,
        pcb.processName,
        pcb.processType,
        pcb.priority
    );
    syncManager.notifyReadyQueue();
    logger.logProcessEvent(pid, pcb.processName, "Process resumed and moved back to READY state");

    cout << "\n[INTERRUPT HANDLER] Process resumed successfully.\n";
    cout << "Run scheduler to continue execution.\n";

    cout << "\nUpdated PCB Table:\n";
    processManager.displayPCBTable();

    cout << "\nUpdated Ready Queue Status:\n";
    readyQueueManager.displayReadyQueues();
}

/*
Function: forceTerminateProcessByPID
Purpose: Forcefully terminates a process by PID, releases resources, removes it from ready queue,
         removes PCB, and logs the reason.
Parameters: PID, process manager, resource manager, ready queue manager, logger, and reason.
Returns: true if cleanup is performed, otherwise false.
*/
bool forceTerminateProcessByPID(
    int pid,
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    string reason
) {
    PCB pcb;

    if (!processManager.getPCB(pid, pcb)) {
        cout << "\n[KERNEL] No process found with PID: " << pid << "\n";
        logger.logProcessEvent(pid, "Unknown", "Force termination failed, PID not found");
        return false;
    }

    cout << "\n[KERNEL] Force terminating process.\n";
    cout << "PID: " << pid << "\n";
    cout << "Process Name: " << pcb.processName << "\n";
    cout << "Reason: " << reason << "\n";

    int killResult = kill(pid, SIGKILL);

    if (killResult == 0) {
        int status;
        int waitResult = waitpid(pid, &status, WNOHANG);

        if (waitResult == 0) {
            sleep(1);
            waitpid(pid, &status, WNOHANG);
        }

        cout << "[KERNEL] Process kill signal sent successfully.\n";
    } else {
        cout << "[KERNEL] Kill signal failed or process already ended.\n";
        cout << "[KERNEL] Continuing AMS OS cleanup.\n";
    }

    readyQueueManager.removeProcessByPID(pid);

    processManager.updateProcessState(pid, TERMINATED_STATE);
    resourceManager.releaseMemoryBlock(pid);
    resourceManager.releaseResources(
        pcb.ramRequired,
        pcb.hddRequired,
        pcb.coresRequired
    );

    logger.logProcessEvent(pid, pcb.processName, "Force terminated. Reason: " + reason);

    logger.logResourceEvent(
        "Resources released after force termination | PID: " + to_string(pid) +
        " | RAM: " + to_string(pcb.ramRequired) +
        "MB | HDD: " + to_string(pcb.hddRequired) +
        "MB | CPU: " + to_string(pcb.coresRequired)
    );

    processManager.removeProcess(pid);

    cout << "\n[KERNEL] Force termination cleanup completed.\n";

    return true;
}

/*
Function: runDeadlockDetectionSimulation
Purpose: Simulates circular wait between two active processes, detects deadlock,
         and recovers by terminating one victim process.
Parameters: ProcessManager, ResourceManager, ReadyQueueManager, DeadlockManager, and Logger references.
Returns: Nothing.
*/
void runDeadlockDetectionSimulation(
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    DeadlockManager &deadlockManager,
    Logger &logger
) {
    int firstPID;
    int secondPID;
    PCB firstPCB;
    PCB secondPCB;

    cout << "\n========== DEADLOCK DETECTION SIMULATION ==========\n";

    cout << "\nActive processes:\n";
    processManager.displayPCBTable();

    firstPID = getValidatedInteger("Enter first process PID: ");
    secondPID = getValidatedInteger("Enter second process PID: ");

    if (firstPID == secondPID) {
        cout << "\n[DEADLOCK MANAGER] Both PIDs cannot be the same.\n";
        return;
    }

    if (!processManager.getPCB(firstPID, firstPCB)) {
        cout << "\n[DEADLOCK MANAGER] First PID not found.\n";
        logger.logSystemEvent("Deadlock simulation failed, first PID not found");
        return;
    }

    if (!processManager.getPCB(secondPID, secondPCB)) {
        cout << "\n[DEADLOCK MANAGER] Second PID not found.\n";
        logger.logSystemEvent("Deadlock simulation failed, second PID not found");
        return;
    }

    /*
    Simple circular wait example:
    Process A holds Resource_A and waits for Resource_B.
    Process B holds Resource_B and waits for Resource_A.
    */
    string resourceA = "Resource_A";
    string resourceB = "Resource_B";

    deadlockManager.clearRecords();

    deadlockManager.addRecord(
        firstPCB.pid,
        firstPCB.processName,
        resourceA,
        resourceB
    );

    deadlockManager.addRecord(
        secondPCB.pid,
        secondPCB.processName,
        resourceB,
        resourceA
    );

    deadlockManager.displayResourceGraph();

    int victimPID = -1;
    string victimName = "";

    bool deadlockDetected = deadlockManager.detectDeadlock(victimPID, victimName);

    if (deadlockDetected) {
        cout << "\nDeadlock detected among processes\n";
        cout << "[DEADLOCK MANAGER] Circular wait condition found.\n";
        cout << "[DEADLOCK MANAGER] Victim selected for termination.\n";
        cout << "Victim PID: " << victimPID << "\n";
        cout << "Victim Process: " << victimName << "\n";

        logger.logSystemEvent(
            "Deadlock detected among processes. Victim PID: " +
            to_string(victimPID) + " | Process: " + victimName
        );

        forceTerminateProcessByPID(
            victimPID,
            processManager,
            resourceManager,
            readyQueueManager,
            logger,
            "Deadlock recovery victim"
        );

        cout << "\n[DEADLOCK MANAGER] Deadlock recovery completed.\n";

        cout << "\nUpdated PCB Table:\n";
        processManager.displayPCBTable();

        cout << "\nUpdated Ready Queue Status:\n";
        readyQueueManager.displayReadyQueues();

        cout << "\nUpdated Resource Status:\n";
        resourceManager.displayResources();
    } else {
        cout << "\n[DEADLOCK MANAGER] No deadlock detected.\n";
        logger.logSystemEvent("Deadlock check completed, no deadlock detected");
    }
}

/*
Function: gracefulShutdownCleanup
Purpose: Terminates all active child processes, releases resources, clears ready queues,
         removes PCBs, and writes shutdown logs.
Parameters: ProcessManager, ResourceManager, ReadyQueueManager, and Logger references.
Returns: Nothing.
*/
void gracefulShutdownCleanup(
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger
) {
    cout << "\n========== GRACEFUL SHUTDOWN CLEANUP ==========\n";

    vector<int> activePIDs = processManager.getAllPIDs();

    if (activePIDs.empty()) {
        cout << "No active processes found. Nothing to clean.\n";
        logger.logSystemEvent("Shutdown cleanup completed, no active processes");
        return;
    }

    for (int pid : activePIDs) {
        PCB pcb;

        if (!processManager.getPCB(pid, pcb)) {
            continue;
        }

        cout << "\n[SHUTDOWN] Cleaning process:\n";
        cout << "PID: " << pid << "\n";
        cout << "Process Name: " << pcb.processName << "\n";

        kill(pid, SIGKILL);

        int status;
        waitpid(pid, &status, WNOHANG);
	resourceManager.releaseMemoryBlock(pid);
        resourceManager.releaseResources(
            pcb.ramRequired,
            pcb.hddRequired,
            pcb.coresRequired
        );

        logger.logProcessEvent(pid, pcb.processName, "Terminated during graceful shutdown");

        logger.logResourceEvent(
            "Resources released during shutdown | PID: " + to_string(pid) +
            " | RAM: " + to_string(pcb.ramRequired) +
            "MB | HDD: " + to_string(pcb.hddRequired) +
            "MB | CPU: " + to_string(pcb.coresRequired)
        );

        processManager.removeProcess(pid);
    }

    readyQueueManager.clearAllQueues();

    logger.logSystemEvent("Graceful shutdown cleanup completed");

    cout << "\n[SHUTDOWN] All active processes cleaned successfully.\n";
}

/*
Function: autoStartDigitalClock
Purpose: Automatically launches the Digital Clock task after AMS OS boot.
         The task is created using the same fork, IPC, resource allocation,
         PCB creation, and ready queue flow used for normal task launching.
Parameters: TaskCatalog, ProcessManager, ResourceManager, ReadyQueueManager, Logger, and SyncManager references.
Returns: Nothing.
*/
void autoStartDigitalClock(
    TaskCatalog &taskCatalog,
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager
) {
    TaskInfo clockTask;
    int clockTaskID = 8;

    cout << "\n========== AUTO STARTUP TASK ==========\n";
    cout << "[AMS OS] Auto-starting Digital Clock after boot.\n";

    if (!taskCatalog.getTaskByID(clockTaskID, clockTask)) {
        cout << "[AMS OS] Digital Clock task not found in task catalog.\n";
        logger.logSystemEvent("Auto-start failed, Digital Clock task not found");
        return;
    }

    int requestPipe[2];
    int responsePipe[2];

    if (pipe(requestPipe) == -1) {
        cout << "[AMS OS] Failed to create request pipe for auto-start clock.\n";
        logger.logSystemEvent("Auto-start clock failed, request pipe creation error");
        return;
    }

    if (pipe(responsePipe) == -1) {
        cout << "[AMS OS] Failed to create response pipe for auto-start clock.\n";
        close(requestPipe[0]);
        close(requestPipe[1]);
        logger.logSystemEvent("Auto-start clock failed, response pipe creation error");
        return;
    }

    cout.flush();

    pid_t pid = fork();

    if (pid < 0) {
        cout << "[AMS OS] Fork failed for auto-start Digital Clock.\n";
        close(requestPipe[0]);
        close(requestPipe[1]);
        close(responsePipe[0]);
        close(responsePipe[1]);
        logger.logSystemEvent("Auto-start clock failed, fork error");
        return;
    }

    if (pid == 0) {
        close(requestPipe[0]);
        close(responsePipe[1]);

        IPCResourceResponse response = childSendResourceRequest(
            requestPipe[1],
            responsePipe[0],
            clockTask
        );

        close(requestPipe[1]);
        close(responsePipe[0]);

        if (response.granted == 0) {
            cout << "[CHILD CLOCK] Resource request denied by kernel.\n";
            exit(2);
        }

        cout << "[CHILD CLOCK] Digital Clock approved and waiting for scheduler.\n";

        raise(SIGSTOP);

        cout << "[CHILD CLOCK] Scheduler resumed Digital Clock.\n";
	cout << "[CHILD CLOCK] Loading Digital Clock in separate terminal using exec.\n";

	executeTaskInSeparateTerminal(clockTask);
    }

    close(requestPipe[1]);
    close(responsePipe[0]);

    IPCResourceRequest request;
    IPCResourceResponse response;

    memset(&request, 0, sizeof(request));
    memset(&response, 0, sizeof(response));

    read(requestPipe[0], &request, sizeof(request));

    cout << "\n[KERNEL/PARENT] Auto-start IPC resource request received.\n";
    cout << "Child PID: " << pid << "\n";
    cout << "Process Name: " << request.processName << "\n";
    cout << "RAM Requested: " << request.ramRequired << " MB\n";
    cout << "HDD Requested: " << request.hddRequired << " MB\n";
    cout << "CPU Requested: " << request.coresRequired << "\n";

    if (resourceManager.checkResources(
            request.ramRequired,
            request.hddRequired,
            request.coresRequired
        )) {
        response.granted = 1;

        cout << "[KERNEL/PARENT] Resources available. Auto-start request granted.\n";
int memoryStart = -1;
int memoryEnd = -1;

bool memoryAllocated = resourceManager.allocateMemoryBlock(
    pid,
    request.processName,
    request.ramRequired,
    memoryStart,
    memoryEnd
);

if (!memoryAllocated) {
    response.granted = 0;

    cout << "\n[KERNEL/PARENT] RAM block allocation failed for auto-start Digital Clock.\n";

    logger.logResourceEvent(
        "Auto-start RAM block allocation failed for " + string(request.processName)
    );

    write(responsePipe[1], &response, sizeof(response));

    close(requestPipe[0]);
    close(responsePipe[1]);

    int status;
    waitpid(pid, &status, 0);

    return;
}
        resourceManager.allocateResources(
            request.ramRequired,
            request.hddRequired,
            request.coresRequired
        );

        logger.logResourceEvent(
            "Resources granted to auto-start task " + string(request.processName) +
            " | RAM: " + to_string(request.ramRequired) +
            "MB | HDD: " + to_string(request.hddRequired) +
            "MB | CPU: " + to_string(request.coresRequired)
        );

        processManager.createPCB(
            pid,
            request.processName,
            static_cast<ProcessType>(request.processType),
            request.priority,
            request.ramRequired,
            request.hddRequired,
            request.coresRequired
        );
	processManager.updateMemoryBlock(pid, memoryStart, memoryEnd);

	logger.logResourceEvent(
	    "RAM block assigned to auto-start PID " + to_string(pid) +
	    " | " + string(request.processName) +
	    " | Start: " + to_string(memoryStart) +
	    "MB | End: " + to_string(memoryEnd) + "MB"
	);
		logger.logProcessEvent(pid, request.processName, "Auto-start PCB created");

        processManager.updateProcessState(pid, READY_STATE);

        readyQueueManager.addProcessToReadyQueue(
            pid,
            request.processName,
            static_cast<ProcessType>(request.processType),
            request.priority
        );

        syncManager.notifyReadyQueue();

        logger.logProcessEvent(pid, request.processName, "Auto-start task added to ready queue");

        cout << "[KERNEL/PARENT] Digital Clock auto-started and added to ready queue.\n";
    } else {
        response.granted = 0;

        cout << "[KERNEL/PARENT] Not enough resources for auto-start Digital Clock.\n";

        logger.logResourceEvent("Auto-start Digital Clock denied due to insufficient resources");
    }

    write(responsePipe[1], &response, sizeof(response));

    close(requestPipe[0]);
    close(responsePipe[1]);

    int status;

    if (response.granted == 1) {
        waitpid(pid, &status, WUNTRACED);

        if (WIFSTOPPED(status)) {
            cout << "[KERNEL/PARENT] Auto-start Digital Clock is waiting in ready queue.\n";
        }
    } else {
        waitpid(pid, &status, 0);
    }

    cout << "\n[AMS OS] Startup task status:\n";
    readyQueueManager.displayReadyQueues();
}

/*
Function: closeProcess
Purpose: Allows the user to close a selected process. The process is terminated,
         removed from ready queue, resources and RAM block are released, PCB is removed,
         and the event is logged.
Parameters: ProcessManager, ResourceManager, ReadyQueueManager, and Logger references.
Returns: Nothing.
*/
void closeProcess(
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger
) {
    int pid;
    char confirmation;
    PCB pcb;

    cout << "\n========== CLOSE PROCESS ==========\n";

    processManager.displayPCBTable();

    pid = getValidatedInteger("Enter PID to close: ");

    if (!processManager.getPCB(pid, pcb)) {
        cout << "\n[CLOSE PROCESS] No process found with PID: " << pid << "\n";
        logger.logProcessEvent(pid, "Unknown", "Close failed, PID not found");
        return;
    }

    if (pcb.processState == TERMINATED_STATE) {
        cout << "\n[CLOSE PROCESS] Process is already terminated.\n";
        logger.logProcessEvent(pid, pcb.processName, "Close failed, process already terminated");
        return;
    }

    cout << "\nSelected process:\n";
    cout << "PID: " << pcb.pid << "\n";
    cout << "Process Name: " << pcb.processName << "\n";
    cout << "Current State: " << processManager.getProcessStateName(pcb.processState) << "\n";
    cout << "RAM Block: ";

    if (pcb.memoryStart == -1 || pcb.memoryEnd == -1) {
        cout << "N/A\n";
    } else {
        cout << pcb.memoryStart << " MB to " << pcb.memoryEnd << " MB\n";
    }

    cout << "\nAre you sure you want to close this process? (y/n): ";
    cin >> confirmation;

    if (confirmation != 'y' && confirmation != 'Y') {
        cout << "\n[CLOSE PROCESS] Close cancelled.\n";
        logger.logProcessEvent(pid, pcb.processName, "Close cancelled by user");
        return;
    }

    cout << "\n[CLOSE PROCESS] Closing process PID: " << pid << "\n";

    /*
    The process can be in READY, BLOCKED, or paused using SIGSTOP.
    SIGKILL ensures the simulator does not freeze while closing a stopped process.
    */
    int killResult = kill(pid, SIGKILL);

    if (killResult == 0) {
        int status;
        int waitResult = waitpid(pid, &status, WNOHANG);

        if (waitResult == 0) {
            sleep(1);
            waitpid(pid, &status, WNOHANG);
        }

        cout << "[CLOSE PROCESS] Process close signal sent successfully.\n";
    } else {
        cout << "[CLOSE PROCESS] Process may already be finished. Continuing cleanup.\n";
    }

    readyQueueManager.removeProcessByPID(pid);

    processManager.updateProcessState(pid, TERMINATED_STATE);

    resourceManager.releaseMemoryBlock(pid);

    resourceManager.releaseResources(
        pcb.ramRequired,
        pcb.hddRequired,
        pcb.coresRequired
    );

    logger.logProcessEvent(pid, pcb.processName, "Closed by user");

    logger.logResourceEvent(
        "Resources released after user close | PID: " + to_string(pid) +
        " | RAM: " + to_string(pcb.ramRequired) +
        "MB | HDD: " + to_string(pcb.hddRequired) +
        "MB | CPU: " + to_string(pcb.coresRequired)
    );

    processManager.removeProcess(pid);

    cout << "\n[CLOSE PROCESS] Process closed successfully.\n";

    cout << "\nUpdated PCB Table:\n";
    processManager.displayPCBTable();

    cout << "\nUpdated Ready Queue Status:\n";
    readyQueueManager.displayReadyQueues();

    cout << "\nUpdated Resource Status:\n";
    resourceManager.displayResources();

    cout << "\nUpdated RAM Layout:\n";
    resourceManager.displayMemoryLayout();
}
/*
Function: switchToProcess
Purpose: Allows the user to switch to an existing process in RAM.
         If the process is BLOCKED, it is moved back to READY state and added to the ready queue.
         If the process is already READY or RUNNING, its current status is displayed.
Parameters: ProcessManager, ReadyQueueManager, Logger, and SyncManager references.
Returns: Nothing.
*/
void switchToProcess(
    ProcessManager &processManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager
) {
    int pid;
    PCB pcb;

    cout << "\n========== SWITCH TO PROCESS ==========\n";

    processManager.displayPCBTable();

    pid = getValidatedInteger("Enter PID to switch to: ");

    if (!processManager.getPCB(pid, pcb)) {
        cout << "\n[SWITCH PROCESS] No process found with PID: " << pid << "\n";
        logger.logProcessEvent(pid, "Unknown", "Switch failed, PID not found");
        return;
    }

    cout << "\nSelected process found in RAM:\n";
    cout << "PID: " << pcb.pid << "\n";
    cout << "Process Name: " << pcb.processName << "\n";
    cout << "Current State: " << processManager.getProcessStateName(pcb.processState) << "\n";
    cout << "Priority: " << pcb.priority << "\n";
    cout << "RAM Required: " << pcb.ramRequired << " MB\n";

    if (pcb.memoryStart == -1 || pcb.memoryEnd == -1) {
        cout << "RAM Block: N/A\n";
    } else {
        cout << "RAM Block: " << pcb.memoryStart << " MB to "
             << pcb.memoryEnd << " MB\n";
    }

    if (pcb.processState == TERMINATED_STATE) {
        cout << "\n[SWITCH PROCESS] Cannot switch to a terminated process.\n";
        logger.logProcessEvent(pid, pcb.processName, "Switch failed, process already terminated");
        return;
    }

    if (pcb.processState == BLOCKED_STATE) {
        cout << "\n[SWITCH PROCESS] Process is currently BLOCKED.\n";
        cout << "[SWITCH PROCESS] Moving process back to READY state.\n";

        processManager.updateProcessState(pid, READY_STATE);

        readyQueueManager.addProcessToReadyQueue(
            pid,
            pcb.processName,
            pcb.processType,
            pcb.priority
        );

        syncManager.notifyReadyQueue();

        logger.logProcessEvent(pid, pcb.processName, "Switched from BLOCKED to READY");

        cout << "\n[SWITCH PROCESS] Process is now ready for scheduler.\n";
    }
    else if (pcb.processState == READY_STATE) {
        cout << "\n[SWITCH PROCESS] Process is already in READY state.\n";
        cout << "[SWITCH PROCESS] Run scheduler to continue this task.\n";

        logger.logProcessEvent(pid, pcb.processName, "Switch requested, process already READY");
    }
    else if (pcb.processState == RUNNING_STATE) {
        cout << "\n[SWITCH PROCESS] Process is already marked as RUNNING.\n";
        cout << "[SWITCH PROCESS] No state change required.\n";

        logger.logProcessEvent(pid, pcb.processName, "Switch requested, process already RUNNING");
    }
    else if (pcb.processState == NEW_STATE) {
        cout << "\n[SWITCH PROCESS] Process is in NEW state.\n";
        cout << "[SWITCH PROCESS] Moving it to READY state and adding to ready queue.\n";

        processManager.updateProcessState(pid, READY_STATE);

        readyQueueManager.addProcessToReadyQueue(
            pid,
            pcb.processName,
            pcb.processType,
            pcb.priority
        );

        syncManager.notifyReadyQueue();

        logger.logProcessEvent(pid, pcb.processName, "Switched from NEW to READY");
    }

    cout << "\nUpdated PCB Table:\n";
    processManager.displayPCBTable();

    cout << "\nUpdated Ready Queue Status:\n";
    readyQueueManager.displayReadyQueues();

    cout << "\nRAM Memory Layout:\n";
    // Memory layout is shown from main menu option 20.
    cout << "Use menu option 20 to view complete RAM memory layout.\n";
}
/*
Function: main
Purpose: Starts AMS OS, initializes resources from command-line arguments, and controls the main menu.
Parameters: argc and argv for command-line resource input.
Returns: Program exit status.
*/
int main(int argc, char* argv[]) {
    int ram;
    int hdd;
    int cores;
    int choice;

    OSMode currentMode = USER_MODE;
    bootScreen();
    createRequiredDirectories();
    if (!getHardwareResourcesFromCommandLine(argc, argv, ram, hdd, cores)) {
        return 1;
    }

        ResourceManager resourceManager(ram, hdd, cores);
	ProcessManager processManager;
	TaskCatalog taskCatalog;
	ReadyQueueManager readyQueueManager;
	Scheduler scheduler;
	DeadlockManager deadlockManager;
	Logger logger;
	SyncManager syncManager(cores);
	logger.logSystemEvent("AMS OS Booted");
    cout << "\nAMS OS resources initialized successfully.\n";
	cout << "Loaded Tasks: " << taskCatalog.getTaskCount() << "\n";
	resourceManager.displayResources();

	syncManager.startResourceMonitor(resourceManager, logger);

	autoStartDigitalClock(
	    taskCatalog,
	    processManager,
	    resourceManager,
	    readyQueueManager,
	    logger,
	    syncManager
	);
    do {
        showMainMenu(currentMode);
        choice = getValidatedInteger("Enter your choice: ");

        switch (choice) {
            case 1:
                taskCatalog.displayAvailableTasks();
                break;

            case 2:
                showTaskDetailsMenu(taskCatalog);
                break;

            case 3:
    		launchTaskUsingIPCForkTest(
		    taskCatalog,
		    processManager,
		    resourceManager,
		    readyQueueManager,
		    logger,
		    syncManager
		);
    		break;

            case 4:
                resourceManager.displayResources();
                break;

            case 5:
		    if (currentMode == KERNEL_MODE) {
			testResourceAllocation(resourceManager);
		    } else {
			cout << "\nAccess denied. Resource allocation test requires Kernel Mode.\n";
		    }
		    break;

           case 6:
		    if (currentMode == KERNEL_MODE) {
			testResourceRelease(resourceManager);
		    } else {
			cout << "\nAccess denied. Resource release test requires Kernel Mode.\n";
		    }
		    break;

            case 7:
                processManager.displayPCBTable();
                break;

            case 8:
		    if (currentMode == KERNEL_MODE) {
			testDummyPCBCreation(processManager);
		    } else {
			cout << "\nAccess denied. Dummy PCB creation requires Kernel Mode.\n";
		    }
		    break;

            case 9:
		    if (currentMode == KERNEL_MODE) {
			testProcessStateUpdate(processManager);
		    } else {
			cout << "\nAccess denied. Process state update requires Kernel Mode.\n";
		    }
		    break;

            case 10:
		    if (currentMode == KERNEL_MODE) {
			testPCBRemoval(processManager);
		    } else {
			cout << "\nAccess denied. PCB removal requires Kernel Mode.\n";
		    }
		    break;

            case 11:
		    scheduler.runScheduler(
			    processManager,
			    resourceManager,
			    readyQueueManager,
			    logger,
			    syncManager
			);
		    break;
		   
	    case 12:
	        readyQueueManager.displayReadyQueues();
	        break;
           case 13:
		    if (authenticateKernelMode()) {
			currentMode = KERNEL_MODE;
			logger.logSystemEvent("Switched to Kernel Mode");
		    } else {
			logger.logSystemEvent("Failed Kernel Mode authentication attempt");
		    }
		    break;
	   case 14:
		    currentMode = USER_MODE;
		    logger.logSystemEvent("Switched to User Mode");
		    cout << "\nSwitched back to User Mode.\n";
		    break;
           case 15:
		    if (currentMode == KERNEL_MODE) {
			viewSystemLog();
			logger.logSystemEvent("System log viewed in Kernel Mode");
		    } else {
			cout << "\nAccess denied. System logs can only be viewed in Kernel Mode.\n";
			logger.logSystemEvent("User Mode tried to access system log");
		    }
		    break;
	   case 16:
		    if (currentMode == KERNEL_MODE) {
			kernelKillProcess(
			    processManager,
			    resourceManager,
			    readyQueueManager,
			    logger
			);
		    } else {
			cout << "\nAccess denied. Process Killer can only be used in Kernel Mode.\n";
			logger.logSystemEvent("User Mode tried to access Process Killer");
		    }
		    break;
          case 17:
	    minimizeProcess(
		processManager,
		readyQueueManager,
		logger
	    );
	    break;

	  case 18:
	    resumeProcess(
		    processManager,
		    readyQueueManager,
		    logger,
		    syncManager
		);
	    break;
	  case 19:
	    if (currentMode == KERNEL_MODE) {
		runDeadlockDetectionSimulation(
		    processManager,
		    resourceManager,
		    readyQueueManager,
		    deadlockManager,
		    logger
		);
	    } else {
		cout << "\nAccess denied. Deadlock detection can only be used in Kernel Mode.\n";
		logger.logSystemEvent("User Mode tried to access Deadlock Detection");
	    }
	    break;
	  case 20:
	    resourceManager.displayMemoryLayout();
	    break;
          case 21:
	    closeProcess(
		processManager,
		resourceManager,
		readyQueueManager,
		logger
	    );
	    break;
	  case 22:
	    switchToProcess(
		processManager,
		readyQueueManager,
		logger,
		syncManager
	    );
	    break;
          case 0:
	    logger.logSystemEvent("AMS OS shutdown requested");
	    syncManager.stopResourceMonitor();
            logger.logSystemEvent("Resource monitor thread stopped");
	    gracefulShutdownCleanup(
		processManager,
		resourceManager,
		readyQueueManager,
		logger
	    );

	    shutdownScreen();

	    logger.logSystemEvent("AMS OS shutdown completed");
	    break;
            default:
                cout << "\nInvalid choice. Please select a valid option from the menu.\n";
        }

    } while (choice != 0);

    return 0;
}