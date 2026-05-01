#include <iostream>
#include <unistd.h>
#include <limits>
#include "resource_manager.h"
#include "process_manager.h"

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
bool getHardwareResources(int &ram, int &hdd, int &cores) {
    cout << "\n========== HARDWARE RESOURCE SETUP ==========\n";

    ram = getValidatedInteger("Enter RAM in MB: ");
    hdd = getValidatedInteger("Enter Hard Drive in MB: ");
    cores = getValidatedInteger("Enter CPU Cores: ");

    if (ram <= 0 || hdd <= 0 || cores <= 0) {
        cout << "\nInvalid hardware resources entered.\n";
        cout << "AMS OS cannot start with zero or negative resources.\n";
        return false;
    }

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
    cout << "1. Launch Calculator\n";
    cout << "2. Launch Notepad\n";
    cout << "3. Launch Digital Clock\n";
    cout << "4. Launch Music Player\n";
    cout << "5. Launch File Copy\n";
    cout << "6. Show Resources\n";
    cout << "7. Test Resource Allocation\n";
    cout << "8. Test Resource Release\n";
    cout << "9. Show PCB Table\n";
    cout << "10. Test Dummy PCB Creation\n";
    cout << "11. Test Process State Update\n";
    cout << "12. Test PCB Removal\n";
    cout << "13. Run Scheduler\n";
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
int main() {
    int ram;
    int hdd;
    int cores;
    int choice;

    bootScreen();

    if (!getHardwareResources(ram, hdd, cores)) {
        return 1;
    }

    ResourceManager resourceManager(ram, hdd, cores);
    ProcessManager processManager;

    cout << "\nAMS OS resources initialized successfully.\n";
    resourceManager.displayResources();

    do {
        showMainMenu();
        choice = getValidatedInteger("Enter your choice: ");

        switch (choice) {
            case 1:
                showComingSoonMessage("Calculator");
                break;

            case 2:
                showComingSoonMessage("Notepad");
                break;

            case 3:
                showComingSoonMessage("Digital Clock");
                break;

            case 4:
                showComingSoonMessage("Music Player");
                break;

            case 5:
                showComingSoonMessage("File Copy");
                break;

            case 6:
                resourceManager.displayResources();
                break;

            case 7:
                testResourceAllocation(resourceManager);
                break;

            case 8:
                testResourceRelease(resourceManager);
                break;

            case 9:
                processManager.displayPCBTable();
                break;

            case 10:
                testDummyPCBCreation(processManager);
                break;

            case 11:
                testProcessStateUpdate(processManager);
                break;

            case 12:
                testPCBRemoval(processManager);
                break;

            case 13:
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