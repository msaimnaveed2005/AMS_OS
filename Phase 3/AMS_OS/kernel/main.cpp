#include <iostream>
#include <unistd.h>
#include <limits>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>

#include "resource_manager.h"
#include "process_manager.h"
#include "task_catalog.h"

using namespace std;

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
Function: getHardwareResources
Purpose: Takes RAM, HDD, and CPU core values from the user.
Parameters: References to RAM, HDD, and CPU core variables.
Returns: true if valid resources are entered, otherwise false.
*/
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
Function: showMainMenu
Purpose: Displays the main AMS OS menu.
Parameters: None.
Returns: Nothing.
*/
void showMainMenu() {
    cout << "\n========== AMS OS MAIN MENU ==========\n";
    cout << "1. Show Task Catalog\n";
    cout << "2. Show Task Details\n";
    cout << "3. Launch Task Using Fork Test\n";
    cout << "4. Show Resources\n";
    cout << "5. Test Resource Allocation\n";
    cout << "6. Test Resource Release\n";
    cout << "7. Show PCB Table\n";
    cout << "8. Test Dummy PCB Creation\n";
    cout << "9. Test Process State Update\n";
    cout << "10. Test PCB Removal\n";
    cout << "11. Run Scheduler\n";
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
Function: launchTaskUsingForkTest
Purpose: Selects a task from the catalog, allocates resources, creates a child process using fork,
         creates a PCB using the real child PID, waits for completion, then releases resources.
Parameters: TaskCatalog, ProcessManager, and ResourceManager object references.
Returns: Nothing.
*/
void launchTaskUsingForkTest(
    TaskCatalog &taskCatalog,
    ProcessManager &processManager,
    ResourceManager &resourceManager
) {
    int taskID;
    TaskInfo selectedTask;

    cout << "\n========== LAUNCH TASK USING FORK TEST ==========\n";
    taskCatalog.displayAvailableTasks();

    taskID = getValidatedInteger("Enter Task ID to launch: ");

    if (!taskCatalog.getTaskByID(taskID, selectedTask)) {
        cout << "\nInvalid Task ID. No task found.\n";
        return;
    }

    cout << "\nSelected Task Information\n";
    cout << "Task Name: " << selectedTask.taskName << "\n";
    cout << "Task Type: " << taskCatalog.getProcessTypeName(selectedTask.processType) << "\n";
    cout << "RAM Required: " << selectedTask.ramRequired << " MB\n";
    cout << "HDD Required: " << selectedTask.hddRequired << " MB\n";
    cout << "CPU Required: " << selectedTask.coresRequired << "\n";

    if (!resourceManager.checkResources(
            selectedTask.ramRequired,
            selectedTask.hddRequired,
            selectedTask.coresRequired
        )) {
        cout << "\n[AMS OS] Task launch denied due to insufficient resources.\n";
        resourceManager.displayResources();
        return;
    }

    cout << "\n[AMS OS] Resources available. Allocating resources before process creation.\n";

    if (!resourceManager.allocateResources(
            selectedTask.ramRequired,
            selectedTask.hddRequired,
            selectedTask.coresRequired
        )) {
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        cout << "\n[AMS OS] Fork failed. Unable to create child process.\n";
        resourceManager.releaseResources(
            selectedTask.ramRequired,
            selectedTask.hddRequired,
            selectedTask.coresRequired
        );
        return;
    }

    if (pid == 0) {
        cout << "\n[CHILD PROCESS] Child created successfully.\n";
        cout << "[CHILD PROCESS] PID: " << getpid() << "\n";
        cout << "[CHILD PROCESS] Parent PID: " << getppid() << "\n";
        cout << "[CHILD PROCESS] Simulating task execution for: " << selectedTask.taskName << "\n";

        for (int i = 1; i <= 4; i++) {
            cout << "[CHILD PROCESS] " << selectedTask.taskName
                 << " is running... step " << i << "/4\n";
            sleep(1);
        }

        cout << "[CHILD PROCESS] " << selectedTask.taskName << " execution completed.\n";
        exit(0);
    } else {
        cout << "\n[PARENT PROCESS] Child process created successfully.\n";
        cout << "[PARENT PROCESS] Child PID: " << pid << "\n";

        processManager.createPCB(
            pid,
            selectedTask.taskName,
            selectedTask.processType,
            selectedTask.priority,
            selectedTask.ramRequired,
            selectedTask.hddRequired,
            selectedTask.coresRequired
        );

        processManager.updateProcessState(pid, READY_STATE);
        processManager.updateProcessState(pid, RUNNING_STATE);

        cout << "\n[PARENT PROCESS] Current PCB Table after child creation:\n";
        processManager.displayPCBTable();

        int status;
        waitpid(pid, &status, 0);

        cout << "\n[PARENT PROCESS] Child process execution finished.\n";

        processManager.updateProcessState(pid, TERMINATED_STATE);

        cout << "\n[PARENT PROCESS] Releasing resources.\n";
        resourceManager.releaseResources(
            selectedTask.ramRequired,
            selectedTask.hddRequired,
            selectedTask.coresRequired
        );

        processManager.removeProcess(pid);

        cout << "\n[PARENT PROCESS] Final resource status after cleanup:\n";
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
Function: main
Purpose: Starts AMS OS, initializes resources, and controls the main menu.
Parameters: None.
Returns: Program exit status.
*/
int main(int argc, char* argv[]) {
    int ram;
    int hdd;
    int cores;
    int choice;

    bootScreen();

    if (!getHardwareResourcesFromCommandLine(argc, argv, ram, hdd, cores)) {
        return 1;
    }

    ResourceManager resourceManager(ram, hdd, cores);
    ProcessManager processManager;
    TaskCatalog taskCatalog;

    cout << "\nAMS OS resources initialized successfully.\n";
    cout << "Loaded Tasks: " << taskCatalog.getTaskCount() << "\n";
    resourceManager.displayResources();


    do {
        showMainMenu();
        choice = getValidatedInteger("Enter your choice: ");

        switch (choice) {
            case 1:
                taskCatalog.displayAvailableTasks();
                break;

            case 2:
                showTaskDetailsMenu(taskCatalog);
                break;

            case 3:
                launchTaskUsingForkTest(taskCatalog, processManager, resourceManager);
                break;

            case 4:
                resourceManager.displayResources();
                break;

            case 5:
                testResourceAllocation(resourceManager);
                break;

            case 6:
                testResourceRelease(resourceManager);
                break;

            case 7:
                processManager.displayPCBTable();
                break;

            case 8:
                testDummyPCBCreation(processManager);
                break;

            case 9:
                testProcessStateUpdate(processManager);
                break;

            case 10:
                testPCBRemoval(processManager);
                break;

            case 11:
                showComingSoonMessage("Scheduler");
                break;

            case 0:
                shutdownScreen();
                break;

            default:
                cout << "\nInvalid choice. Please select a valid option from the menu.\n";
        }

    } while (choice != 0);

    return 0;
}