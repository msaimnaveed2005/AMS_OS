#include <iostream>
#include <unistd.h>
#include <limits>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <filesystem>
#include <signal.h>
#include <cerrno>
#include <vector>

#include "resource_manager.h"
#include "process_manager.h"
#include "task_catalog.h"
#include "ready_queue.h"
#include "scheduler.h"
#include "logger.h"
#include "deadlock_manager.h"
#include "sync_manager.h"
#include "ui.h"

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
    UI::clearScreen();
    UI::panelHeader("AMS OS BIOS", "Retro startup sequence", 86);
    cout << "  " << UI::paint("Initializing display adapter", UI::LIGHT_GRAY) << "\n";
    cout << "  " << UI::paint("Loading kernel image...", UI::LIGHT_GRAY) << "\n";
    UI::terminalBell();
    sleep(1);

    const vector<string> bootFrames = {
        "[#---------] 10%",
        "[###-------] 30%",
        "[#####-----] 50%",
        "[#######---] 70%",
        "[#########-] 90%",
        "[##########] 100%"
    };

    cout << "\n  " << UI::paint("SYSTEM BOOT SEQUENCE", UI::BRIGHT_CYAN + UI::BOLD) << "\n";
    for (const string &frame : bootFrames) {
        cout << "  " << UI::paint(frame, UI::ROYAL_BLUE + UI::BOLD) << "\r";
        cout.flush();
        UI::terminalBell();
        usleep(220000);
    }
    cout << "\n";

    UI::asciiLogo();
    UI::panelHeader("AMS OS", "Atomic Management System");
    UI::bootStep("BOOT", "Kernel boot sequence initialized");
    UI::bootStep("RAM", "Memory manager online");
    UI::bootStep("CPU", "Multilevel scheduler online");
    UI::bootStep("IPC", "Fork/exec task bridge online");
    cout << "\n  " << UI::paint("Entering 90s terminal mode...", UI::LIGHT_BLUE + UI::BOLD) << "\n";
    UI::terminalBell();
    usleep(180000);
    UI::terminalBell();
    usleep(180000);
    UI::terminalBell();

    cout << "\n  " << UI::statusPill("READY", UI::GREEN)
         << " " << UI::paint("System loaded successfully.", UI::WHITE + UI::BOLD) << "\n";
    UI::playCue("boot");
}

/*
Function: readPositiveStartupValue
Purpose: Reads and validates a positive integer startup resource value from terminal.
Parameters: Prompt shown to the user.
Returns: Positive integer entered by the user.
*/
int readPositiveStartupValue(const string &prompt) {
    int value;

    while (true) {
        cout << prompt;
        cin >> value;

        if (!cin.fail() && value > 0) {
            return value;
        }

        UI::errorLine("Invalid value. Enter a number greater than zero.");
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

/*
Function: getHardwareResourcesFromStartup
Purpose: Reads RAM, HDD, and CPU cores before OS boot from command-line arguments
         or from interactive Ubuntu terminal prompts.
Parameters: argc, argv, and references to RAM, HDD, and CPU core variables.
Returns: true if valid resources are provided, otherwise false.
*/
bool getHardwareResourcesFromStartup(
    int argc,
    char* argv[],
    int &ram,
    int &hdd,
    int &cores
) {
    int ramGB;
    int hddGB;

    if (argc == 4) {
        ramGB = atoi(argv[1]);
        hddGB = atoi(argv[2]);
        cores = atoi(argv[3]);

        if (ramGB <= 0 || hddGB <= 0 || cores <= 0) {
            cout << "\nInvalid hardware resources entered.\n";
            cout << "RAM, HDD, and CPU cores must be greater than zero.\n";
            return false;
        }
    } else if (argc == 1) {
        UI::clearScreen();
        UI::asciiLogo();
        UI::panelHeader("Hardware Resource Setup", "Enter resources before boot");
        cout << "  " << UI::paint("Recommended project instance:", UI::BOLD)
             << " 2 GB RAM, 256 GB HDD, 8 CPU cores.\n\n";

        ramGB = readPositiveStartupValue("Enter RAM size in GB: ");
        hddGB = readPositiveStartupValue("Enter hard drive size in GB: ");
        cores = readPositiveStartupValue("Enter number of CPU cores: ");
    } else {
        cout << "\nInvalid startup command.\n";
        cout << "Usage: ./OS <RAM_GB> <HDD_GB> <CPU_CORES>\n";
        cout << "Example: ./OS 2 256 8\n";
        cout << "Or run ./OS and enter resources interactively.\n";
        return false;
    }

    ram = ramGB * 1024;
    hdd = hddGB * 1024;

    UI::panelHeader("Hardware Resource Setup", "Accepted");
    cout << "  " << UI::statusPill("GRANTED", UI::GREEN)
         << " " << UI::paint("Hardware resources locked for AMS OS boot.\n", UI::WHITE);
    UI::keyValue("RAM Provided", to_string(ramGB) + " GB (" + to_string(ram) + " MB)");
    UI::keyValue("Hard Drive Provided", to_string(hddGB) + " GB (" + to_string(hdd) + " MB)");
    UI::keyValue("CPU Cores Provided", to_string(cores));
    UI::panelFooter();

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
Function: demoPause
Purpose: Adds a short delay so viva demonstrations remain readable.
Parameters: Pause duration in seconds.
Returns: Nothing.
*/
void demoPause(unsigned int seconds = 2) {
    sleep(seconds);
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
            UI::errorLine("Invalid input. Please enter a valid number.");
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
Function: getTaskExecutionModeName
Purpose: Returns readable name for current task execution mode.
Parameters: Separate terminal mode flag.
Returns: Execution mode name as string.
*/
string getTaskExecutionModeName(bool separateTerminalMode) {
    if (separateTerminalMode) {
        return "Separate Terminal Mode";
    }

    return "Scheduler-Controlled Mode";
}

/*
Function: sendSignalToProcessGroup
Purpose: Sends a signal to the AMS OS child process group, then falls back to
         the single PID if a process group is not available.
Parameters: PID and signal number.
Returns: true if either signal call succeeds, otherwise false.
*/
bool sendSignalToProcessGroup(int pid, int signalNumber) {
    if (pid <= 0) {
        return false;
    }

    if (kill(-pid, signalNumber) == 0) {
        return true;
    }

    return kill(pid, signalNumber) == 0;
}


/*
Function: showMainMenu
Purpose: Displays the AMS OS menu according to current user or kernel mode.
Parameters: Current OS mode.
Returns: Nothing.
*/
void showMainMenu(
    OSMode currentMode,
    bool separateTerminalMode,
    bool multitaskingModeEnabled,
    ResourceManager &resourceManager,
    ProcessManager &processManager,
    ReadyQueueManager &readyQueueManager,
    TaskCatalog &taskCatalog
) {
    int usedRAM = resourceManager.getTotalRAM() - resourceManager.getAvailableRAM();
    int usedHDD = resourceManager.getTotalHDD() - resourceManager.getAvailableHDD();
    int usedCores = resourceManager.getTotalCores() - resourceManager.getAvailableCores();

    string modeBadge;
    if (currentMode == KERNEL_MODE) {
        modeBadge = UI::statusPill("KERNEL MODE", UI::YELLOW);
    } else {
        modeBadge = UI::statusPill("USER MODE", UI::GREEN);
    }

    string terminalBadge;
    if (separateTerminalMode) {
        terminalBadge = UI::statusPill("UBUNTU TERMINALS", UI::BLUE);
    } else {
        terminalBadge = UI::statusPill("SCHEDULER CONTROLLED", UI::CYAN);
    }
    string multitaskingBadge = UI::statusPill(
        multitaskingModeEnabled ? "MULTITASKING ON" : "MULTITASKING OFF",
        multitaskingModeEnabled ? UI::GREEN : UI::SLATE_GRAY
    );

    UI::panelHeader(
        "AMS OS Control Center",
        "Live kernel dashboard"
    );

    UI::modeSplash(getModeName(currentMode), getTaskExecutionModeName(separateTerminalMode));
    cout << "  " << modeBadge << "  " << terminalBadge << "  " << multitaskingBadge << "\n\n";

    UI::sectionBanner("System Snapshot", UI::BRIGHT_CYAN);
    UI::metric("Tasks Loaded", to_string(taskCatalog.getTaskCount()), "catalog executables");
    UI::metric("PCB Entries", to_string(processManager.getProcessCount()), "active process records");
    UI::metric(
        "Ready Processes",
        to_string(readyQueueManager.getTotalReadyCount()),
        "S:" + to_string(readyQueueManager.getSystemQueueCount()) +
        " I:" + to_string(readyQueueManager.getInteractiveQueueCount()) +
        " B:" + to_string(readyQueueManager.getBackgroundQueueCount())
    );
    UI::metric(
        "States",
        "R:" + to_string(processManager.getProcessStateCount(RUNNING_STATE)) +
        " W:" + to_string(processManager.getProcessStateCount(READY_STATE)) +
        " M:" + to_string(processManager.getProcessStateCount(MINIMIZED_STATE)),
        "running / ready / minimized"
    );

    UI::sectionBanner("Resource Meters", UI::BRIGHT_GREEN);
    cout << "  " << left << setw(7) << "RAM" << UI::usageBar(usedRAM, resourceManager.getTotalRAM())
         << "  " << usedRAM << "/" << resourceManager.getTotalRAM() << " MB used\n";
    cout << "  " << left << setw(7) << "HDD" << UI::usageBar(usedHDD, resourceManager.getTotalHDD())
         << "  " << usedHDD << "/" << resourceManager.getTotalHDD() << " MB used\n";
    cout << "  " << left << setw(7) << "CPU" << UI::usageBar(usedCores, resourceManager.getTotalCores())
         << "  " << usedCores << "/" << resourceManager.getTotalCores() << " cores used\n";

    cout << "\n" << UI::paint(UI::repeat('-', 76) + "\n", UI::DIM);

    UI::sectionBanner("Applications", UI::BRIGHT_MAGENTA);
    UI::menuItem(1, "Task Catalog", "Browse available programs");
    UI::menuItem(2, "Task Details", "Inspect requirements");
    UI::menuItem(3, "Launch Task", "Fork + IPC resource request");
    UI::menuItem(23, "Instruction Guide", "How to run and control AMS OS");

    UI::sectionBanner("System Monitor", UI::BRIGHT_BLUE);
    UI::menuItem(4, "Resource Status", "RAM, HDD, and CPU usage");
    UI::menuItem(20, "RAM Memory Layout", "Allocation blocks");
    UI::menuItem(7, "PCB Table", "Active process metadata");
    UI::menuItem(12, "Ready Queues", "Scheduling queues");

    UI::sectionBanner("Process Control", UI::BRIGHT_YELLOW);
    UI::menuItem(11, "Run Scheduler", "Dispatch waiting tasks");
    UI::menuItem(17, "Minimize Process", "Pause execution");
    UI::menuItem(18, "Resume Process", "Return to ready queue");
    UI::menuItem(21, "Close Process", "Release resources");
    UI::menuItem(22, "Switch to Process", "Focus selected PID");
    UI::menuItem(24, "Running Tasks List", "View RUNNING processes");
    UI::menuItem(25, "Minimized Tasks List", "View MINIMIZED processes");
    UI::menuItem(26, "Input Interrupt", "Move process to BLOCKED");
    UI::menuItem(27, "Complete Interrupt", "Resume BLOCKED process");
    UI::menuItem(28, "Start Multitasking", "Run ready tasks in parallel and keep menu active");

    if (currentMode == USER_MODE) {
        UI::sectionBanner("Access", UI::CYAN);
        UI::menuItem(13, "Switch to Kernel Mode", "Requires password");
    }

    if (currentMode == KERNEL_MODE) {
        UI::sectionBanner("Kernel Tools", UI::YELLOW);
        UI::menuItem(14, "Switch to User Mode");
        UI::menuItem(15, "View System Log");
        UI::menuItem(16, "Kill Process");
        UI::menuItem(19, "Deadlock Detection");
    }

    cout << "\n";
    UI::menuItem(0, "Shutdown AMS OS", "Graceful cleanup");
    UI::panelFooter();
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
Function: showInstructionGuide
Purpose: Displays the final project guide, available task workflow, and process controls.
Parameters: Task catalog reference.
Returns: Nothing.
*/
void showInstructionGuide(TaskCatalog &taskCatalog) {
    UI::panelHeader("AMS OS Instruction Guide", "Final project workflow", 92);

    cout << "  " << UI::paint("Startup", UI::BOLD) << "\n";
    cout << "  Run: ./OS and enter hardware resources when prompted.\n";
    cout << "  Fast run: ./OS <RAM_GB> <HDD_GB> <CPU_CORES>\n";
    cout << "  Example: ./OS 2 256 8\n";
    cout << "  Hardware resources are accepted before the AMS OS boot screen.\n";
    cout << "  Digital Clock and Calendar are auto-started after boot.\n\n";

    cout << "  " << UI::paint("Task Flow", UI::BOLD) << "\n";
    cout << "  1. Select Launch Task.\n";
    cout << "  2. Child process sends RAM/HDD/CPU request to kernel through IPC pipe.\n";
    cout << "  3. Kernel grants resources, assigns a RAM block, creates PCB, and queues task.\n";
    cout << "  4. Run Scheduler to dispatch ready tasks using multilevel queue scheduling.\n";
    cout << "  5. Task executes through exec in a GNOME Terminal window by default.\n";
    cout << "     If GNOME Terminal is unavailable, AMS OS tries other Ubuntu terminal emulators.\n\n";

    cout << "  " << UI::paint("Task Controls", UI::BOLD) << "\n";
    cout << "  Close Process   : Menu 21 releases RAM/HDD/CPU and removes PCB.\n";
    cout << "  Minimize Process: Menu 17 pauses task execution and moves it to MINIMIZED.\n";
    cout << "  Resume Process  : Menu 18 returns MINIMIZED or BLOCKED tasks to READY.\n";
    cout << "  Input Interrupt : Menu 26 moves a task to BLOCKED; menu 27 completes it.\n";
    cout << "  Switch Process  : Menu 22 focuses a process already loaded in RAM.\n\n";
    cout << "  Start Multitasking : Menu 28 dispatches ready tasks in parallel.\n\n";

    cout << "  " << UI::paint("Kernel Tools", UI::BOLD) << "\n";
    cout << "  Kernel Mode password: admin\n";
    cout << "  Kernel Mode includes logs, deadlock detection, diagnostics, and force kill.\n";

    UI::panelFooter(92);

    taskCatalog.displayAvailableTasks();
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
            return MINIMIZED_STATE;
        case 6:
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
    cout << "5. MINIMIZED\n";
    cout << "6. TERMINATED\n";

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

    UI::panelHeader("Task Details Menu", "Choose a task to inspect");
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
Function: executeTaskExecutable
Purpose: Executes the selected task either directly under AMS OS scheduler control
         or inside a separate Ubuntu terminal window based on selected mode.
Parameters: Selected task metadata and separate terminal mode flag.
Returns: Nothing. If exec succeeds, this function does not return.
*/
void executeTaskExecutable(TaskInfo selectedTask, bool separateTerminalMode) {
    string amsOSPID = to_string(getpid());
    setenv("AMS_OS_PID", amsOSPID.c_str(), 1);

    if (separateTerminalMode) {
        cout << "[CHILD PROCESS] Opening task in separate terminal window.\n";
        cout << "[CHILD PROCESS] Terminal Mode: Separate Terminal\n";
        cout << "[CHILD PROCESS] Terminal preference: gnome-terminal, xfce4-terminal, x-terminal-emulator\n";
        cout << "[CHILD PROCESS] Executable Path: " << selectedTask.executablePath << "\n";

        execlp(
            "gnome-terminal",
            "gnome-terminal",
            "--title",
            selectedTask.taskName.c_str(),
            "--",
            selectedTask.executablePath.c_str(),
            selectedTask.taskName.c_str(),
            NULL
        );

        if (errno != ENOENT) {
            perror("[CHILD PROCESS] gnome-terminal failed");
        }

        execlp(
            "xfce4-terminal",
            "xfce4-terminal",
            "--disable-server",
            "--title",
            selectedTask.taskName.c_str(),
            "--execute",
            selectedTask.executablePath.c_str(),
            selectedTask.taskName.c_str(),
            NULL
        );

        if (errno != ENOENT) {
            perror("[CHILD PROCESS] xfce4-terminal failed");
        }

        execlp(
            "x-terminal-emulator",
            "x-terminal-emulator",
            "-T",
            selectedTask.taskName.c_str(),
            "-e",
            selectedTask.executablePath.c_str(),
            selectedTask.taskName.c_str(),
            NULL
        );

        perror("[CHILD PROCESS] x-terminal-emulator failed");
        cout << "[CHILD PROCESS] Separate terminal launch failed. Install gnome-terminal or x-terminal-emulator.\n";
        exit(1);
    }

    cout << "[CHILD PROCESS] Running task directly under AMS OS scheduler.\n";
    cout << "[CHILD PROCESS] Terminal Mode: Scheduler-Controlled\n";
    cout << "[CHILD PROCESS] Executable Path: " << selectedTask.executablePath << "\n";

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
    SyncManager &syncManager,
    bool separateTerminalMode
) {
    int taskID;
    TaskInfo selectedTask;

    UI::panelHeader("Launch Task", "Fork + IPC resource request");
    taskCatalog.displayAvailableTasks();

    taskID = getValidatedInteger("Enter Task ID to launch: ");

    if (!taskCatalog.getTaskByID(taskID, selectedTask)) {
        cout << "\nInvalid Task ID. No task found.\n";
        UI::playCue("error");
        return;
    }
    logger.logProcessEvent(0, selectedTask.taskName, "Task selected for launch");
    UI::sectionTitle("Selected Task");
    UI::keyValue("Task Name", selectedTask.taskName);
    UI::keyValue("Task Type", taskCatalog.getProcessTypeName(selectedTask.processType));
    UI::keyValue("RAM Required", to_string(selectedTask.ramRequired) + " MB");
    UI::keyValue("HDD Required", to_string(selectedTask.hddRequired) + " MB");
    UI::keyValue("CPU Required", to_string(selectedTask.coresRequired));

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
        setpgid(0, 0);

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

        executeTaskExecutable(selectedTask, separateTerminalMode);
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
        UI::playCue("granted");

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
            request.coresRequired
        );

        processManager.updateMemoryBlock(pid, memoryStart, memoryEnd);

        logger.logResourceEvent(
            "RAM block assigned to PID " + to_string(pid) +
            " | " + string(request.processName) +
            " | Start: " + to_string(memoryStart) +
            "MB | End: " + to_string(memoryEnd) + "MB"
        );

        logger.logProcessEvent(pid, request.processName, "PCB Created");

        processManager.updateProcessState(pid, READY_STATE);
        processManager.updateQueueType(pid, readyQueueManager.getQueueName(request.priority));

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
        UI::playCue("denied");
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
        UI::playCue("error");
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
    UI::panelHeader("Shutdown", "Retro power-off sequence");
    cout << "  " << UI::paint("Saving system state...", UI::LIGHT_GRAY) << "\n";
    cout << "  " << UI::paint("Stopping scheduler and services...", UI::LIGHT_GRAY) << "\n\n";

    const vector<string> shutdownFrames = {
        "[##########] 100%",
        "[########--] 80%",
        "[######----] 60%",
        "[####------] 40%",
        "[##--------] 20%",
        "[----------] 0%"
    };

    cout << "  " << UI::paint("POWER DOWN", UI::YELLOW + UI::BOLD) << "\n";
    for (const string &frame : shutdownFrames) {
        cout << "  " << UI::paint(frame, UI::RED + UI::BOLD) << "\r";
        cout.flush();
        UI::terminalBell();
        usleep(180000);
    }
    cout << "\n\n";
    cout << "  " << UI::paint("IT IS NOW SAFE TO CLOSE AMS OS TERMINAL.", UI::LIGHT_BLUE + UI::BOLD) << "\n";
    cout << "  " << UI::paint("AMS OS shutdown completed successfully.", UI::GREEN + UI::BOLD) << "\n";
    UI::terminalBell();
    usleep(200000);
    UI::terminalBell();
    UI::playCue("shutdown");
    UI::panelFooter();
}

/*
Function: createRequiredDirectories
Purpose: Creates required folders for build output and virtual disk storage.
Parameters: None.
Returns: Nothing.
*/
void createRequiredDirectories() {
    try {
        filesystem::create_directories("build");
        filesystem::create_directories("data");
        filesystem::create_directories("data/sounds");
        filesystem::create_directories("data/virtual_disk");
    } catch (const filesystem::filesystem_error &error) {
        cout << "[AMS OS] Failed to create required directories: "
             << error.what() << "\n";
    }
}

/*
Function: showSoundPackStatus
Purpose: Displays which optional sound cue files are available in data/sounds.
Parameters: None.
Returns: Nothing.
*/
void showSoundPackStatus() {
    const vector<string> cueNames = {
        "boot", "shutdown", "error", "granted", "denied", "minimize", "resume", "close"
    };

    UI::panelHeader("Sound Pack Status", "data/sounds", 86);

    int loadedCount = 0;

    for (const string &cueName : cueNames) {
        string cuePath = "data/sounds/" + cueName + ".wav";
        bool exists = filesystem::exists(cuePath);

        if (exists) {
            loadedCount++;
            cout << "  " << UI::statusPill("LOADED", UI::GREEN)
                 << " " << cueName << ".wav\n";
        } else {
            cout << "  " << UI::statusPill("MISSING", UI::YELLOW)
                 << " " << cueName << ".wav "
                 << UI::paint("(terminal bell fallback)", UI::DIM)
                 << "\n";
        }
    }

    cout << "\n  " << UI::paint("Loaded: ", UI::LIGHT_BLUE)
         << UI::paint(to_string(loadedCount) + "/" + to_string(cueNames.size()), UI::WHITE + UI::BOLD)
         << "\n";
    UI::panelFooter(86);
}

/*
Function: authenticateKernelMode
Purpose: Authenticates user before switching to kernel mode.
Parameters: None.
Returns: true if password is correct, otherwise false.
*/
bool authenticateKernelMode() {
    string password;

    UI::panelHeader("Kernel Mode Authentication");
    cout << "Enter kernel password: ";
    cin >> password;

    if (password == "admin") {
        cout << UI::paint("Kernel mode access granted.\n", UI::GREEN + UI::BOLD);
        return true;
    }

    cout << UI::paint("Incorrect password. Kernel mode access denied.\n", UI::RED + UI::BOLD);
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
        cout << "\n" << UI::paint("No system log file found.\n", UI::YELLOW + UI::BOLD);
        return;
    }

    string line;

    UI::panelHeader("System Log");

    while (getline(file, line)) {
        cout << line << "\n";
    }

    UI::panelFooter();

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
    int killResult = sendSignalToProcessGroup(pid, SIGKILL) ? 0 : -1;

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

    resourceManager.releaseMemoryBlock(pid);

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
Purpose: Simulates an interrupt by moving a process to MINIMIZED state.
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

    if (pcb.processState == BLOCKED_STATE) {
        cout << "\n[INTERRUPT HANDLER] Process is BLOCKED. Complete interrupt first.\n";
        return;
    }

    cout << "\n[INTERRUPT HANDLER] Minimizing process.\n";
    cout << "PID: " << pcb.pid << "\n";
    cout << "Process Name: " << pcb.processName << "\n";

    /*
    Remove stale queue entries first to avoid duplicates.
    */
    readyQueueManager.removeProcessByPID(pid);

    /*
    Pause process execution immediately. This simulates task minimize
    interrupting the running process.
    */
    sendSignalToProcessGroup(pid, SIGSTOP);

    /*
    Manual requirement:
    minimize keeps the task in RAM but hidden/paused.
    It should not stay dispatchable until user resumes or switches to it.
    */
    processManager.updateProcessState(pid, MINIMIZED_STATE);
    processManager.updateQueueType(pid, "Minimized");

    logger.logProcessEvent(pid, pcb.processName, "Process minimized: execution paused and moved to MINIMIZED state");
    UI::playCue("minimize");

    cout << "\n[INTERRUPT HANDLER] Process minimized successfully.\n";
    cout << "RAM is still allocated, CPU execution paused, process moved to MINIMIZED state.\n";

    cout << "\nUpdated PCB Table:\n";
    processManager.displayPCBTable();

    cout << "\nUpdated Ready Queue Status:\n";
    readyQueueManager.displayReadyQueues();
}

/*
Function: resumeProcess
Purpose: Resumes a minimized process by moving it from MINIMIZED state to READY state.
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

    if (pcb.processState != MINIMIZED_STATE && pcb.processState != BLOCKED_STATE) {
        cout << "\n[INTERRUPT HANDLER] Only MINIMIZED or BLOCKED processes can be resumed.\n";
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
    processManager.updateQueueType(pid, readyQueueManager.getQueueName(pcb.priority));
    syncManager.notifyReadyQueue();
    logger.logProcessEvent(pid, pcb.processName, "Process resumed and moved back to READY state");
    UI::playCue("resume");

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

    int killResult = sendSignalToProcessGroup(pid, SIGKILL) ? 0 : -1;

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

        sendSignalToProcessGroup(pid, SIGKILL);

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
Function: autoStartTask
Purpose: Automatically launches a startup task after AMS OS boot.
         The task is created using the same fork, IPC, resource allocation,
         PCB creation, and ready queue flow used for normal task launching.
Parameters: Task ID, startup label, TaskCatalog, ProcessManager, ResourceManager,
            ReadyQueueManager, Logger, SyncManager, and terminal mode flag.
Returns: Nothing.
*/
void autoStartTask(
    int startupTaskID,
    string startupLabel,
    TaskCatalog &taskCatalog,
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager,
    bool separateTerminalMode
) {
    TaskInfo startupTask;

    UI::panelHeader("Auto Startup Task", startupLabel);
    cout << "[AMS OS] Auto-starting " << startupLabel << " after boot.\n";

    if (!taskCatalog.getTaskByID(startupTaskID, startupTask)) {
        cout << "[AMS OS] " << startupLabel << " task not found in task catalog.\n";
        logger.logSystemEvent("Auto-start failed, " + startupLabel + " task not found");
        return;
    }

    int requestPipe[2];
    int responsePipe[2];

    if (pipe(requestPipe) == -1) {
        cout << "[AMS OS] Failed to create request pipe for auto-start " << startupLabel << ".\n";
        logger.logSystemEvent("Auto-start " + startupLabel + " failed, request pipe creation error");
        return;
    }

    if (pipe(responsePipe) == -1) {
        cout << "[AMS OS] Failed to create response pipe for auto-start " << startupLabel << ".\n";
        close(requestPipe[0]);
        close(requestPipe[1]);
        logger.logSystemEvent("Auto-start " + startupLabel + " failed, response pipe creation error");
        return;
    }

    cout.flush();

    pid_t pid = fork();

    if (pid < 0) {
        cout << "[AMS OS] Fork failed for auto-start " << startupLabel << ".\n";
        close(requestPipe[0]);
        close(requestPipe[1]);
        close(responsePipe[0]);
        close(responsePipe[1]);
        logger.logSystemEvent("Auto-start " + startupLabel + " failed, fork error");
        return;
    }

    if (pid == 0) {
        setpgid(0, 0);

        close(requestPipe[0]);
        close(responsePipe[1]);

        IPCResourceResponse response = childSendResourceRequest(
            requestPipe[1],
            responsePipe[0],
            startupTask
        );

        close(requestPipe[1]);
        close(responsePipe[0]);

        if (response.granted == 0) {
            cout << "[CHILD STARTUP] Resource request denied by kernel.\n";
            exit(2);
        }

        cout << "[CHILD STARTUP] " << startupLabel << " approved and waiting for scheduler.\n";

        raise(SIGSTOP);

        cout << "[CHILD STARTUP] Scheduler resumed " << startupLabel << ".\n";
        cout << "[CHILD STARTUP] Loading startup task using exec.\n";

        executeTaskExecutable(startupTask, separateTerminalMode);
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

            cout << "\n[KERNEL/PARENT] RAM block allocation failed for auto-start "
                 << startupLabel << ".\n";

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
        processManager.updateQueueType(pid, readyQueueManager.getQueueName(request.priority));

        readyQueueManager.addProcessToReadyQueue(
            pid,
            request.processName,
            static_cast<ProcessType>(request.processType),
            request.priority
        );

        syncManager.notifyReadyQueue();

        logger.logProcessEvent(pid, request.processName, "Auto-start task added to ready queue");

        cout << "[KERNEL/PARENT] " << startupLabel
             << " auto-started and added to ready queue.\n";
    } else {
        response.granted = 0;

        cout << "[KERNEL/PARENT] Not enough resources for auto-start "
             << startupLabel << ".\n";

        logger.logResourceEvent("Auto-start " + startupLabel + " denied due to insufficient resources");
    }

    write(responsePipe[1], &response, sizeof(response));

    close(requestPipe[0]);
    close(responsePipe[1]);

    int status;

    if (response.granted == 1) {
        waitpid(pid, &status, WUNTRACED);

        if (WIFSTOPPED(status)) {
            cout << "[KERNEL/PARENT] Auto-start " << startupLabel
                 << " is waiting in ready queue.\n";
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
    int killResult = sendSignalToProcessGroup(pid, SIGKILL) ? 0 : -1;

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
    UI::playCue("close");

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
        processManager.updateQueueType(pid, readyQueueManager.getQueueName(pcb.priority));

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
        processManager.updateQueueType(pid, readyQueueManager.getQueueName(pcb.priority));

        syncManager.notifyReadyQueue();

        logger.logProcessEvent(pid, pcb.processName, "Switched from NEW to READY");
    }
    else if (pcb.processState == MINIMIZED_STATE) {
        cout << "\n[SWITCH PROCESS] Process is currently MINIMIZED.\n";
        cout << "[SWITCH PROCESS] Moving process back to READY state.\n";

        processManager.updateProcessState(pid, READY_STATE);

        readyQueueManager.addProcessToReadyQueue(
            pid,
            pcb.processName,
            pcb.processType,
            pcb.priority
        );
        processManager.updateQueueType(pid, readyQueueManager.getQueueName(pcb.priority));

        syncManager.notifyReadyQueue();
        logger.logProcessEvent(pid, pcb.processName, "Switched from MINIMIZED to READY");
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
Function: handleInputInterrupt
Purpose: Simulates an input or external interrupt by moving a running/ready process to BLOCKED state.
Parameters: ProcessManager, ReadyQueueManager, and Logger references.
Returns: Nothing.
*/
void handleInputInterrupt(
    ProcessManager &processManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger
) {
    int pid = getValidatedInteger("Enter PID to interrupt (BLOCKED): ");
    PCB pcb;

    if (!processManager.getPCB(pid, pcb)) {
        cout << "\n[INTERRUPT] PID not found.\n";
        return;
    }

    if (pcb.processState == TERMINATED_STATE) {
        cout << "\n[INTERRUPT] Cannot interrupt terminated process.\n";
        return;
    }

    if (pcb.processState == BLOCKED_STATE) {
        cout << "\n[INTERRUPT] Process is already BLOCKED.\n";
        return;
    }

    readyQueueManager.removeProcessByPID(pid);
    sendSignalToProcessGroup(pid, SIGSTOP);
    processManager.updateProcessState(pid, BLOCKED_STATE);
    logger.logProcessEvent(pid, pcb.processName, "Input/External interrupt: moved to BLOCKED");
    cout << "\n[INTERRUPT] Process moved to BLOCKED state.\n";
}

/*
Function: completeInterrupt
Purpose: Completes interrupt handling by moving a blocked process back to READY queue.
Parameters: ProcessManager, ReadyQueueManager, Logger, and SyncManager references.
Returns: Nothing.
*/
void completeInterrupt(
    ProcessManager &processManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager
) {
    int pid = getValidatedInteger("Enter PID to resume from BLOCKED: ");
    PCB pcb;

    if (!processManager.getPCB(pid, pcb)) {
        cout << "\n[INTERRUPT] PID not found.\n";
        return;
    }

    if (pcb.processState != BLOCKED_STATE) {
        cout << "\n[INTERRUPT] Process is not in BLOCKED state.\n";
        return;
    }

    processManager.updateProcessState(pid, READY_STATE);
    readyQueueManager.addProcessToReadyQueue(pid, pcb.processName, pcb.processType, pcb.priority);
    processManager.updateQueueType(pid, readyQueueManager.getQueueName(pcb.priority));
    syncManager.notifyReadyQueue();
    logger.logProcessEvent(pid, pcb.processName, "Interrupt completed: moved BLOCKED to READY");
    cout << "\n[INTERRUPT] Process resumed to READY state.\n";
}
/*
Function: main
Purpose: Starts AMS OS, collects startup resources, boots the system, and controls the main menu.
Parameters: argc and argv for optional command-line resource input.
Returns: Program exit status.
*/
int main(int argc, char* argv[]) {
    int ram;
    int hdd;
    int cores;
    int choice;
    bool separateTerminalMode = true;
    bool multitaskingModeEnabled = false;
    OSMode currentMode = USER_MODE;

    createRequiredDirectories();

    if (!getHardwareResourcesFromStartup(argc, argv, ram, hdd, cores)) {
        return 1;
    }

    bootScreen();
    showSoundPackStatus();

    ResourceManager resourceManager(ram, hdd, cores);
    ProcessManager processManager;
    TaskCatalog taskCatalog;
    ReadyQueueManager readyQueueManager;
    Scheduler scheduler;
    DeadlockManager deadlockManager;
    Logger logger;
    SyncManager syncManager(cores);

    logger.logSystemEvent("AMS OS Booted");
    UI::sectionTitle("Startup Summary");
    UI::keyValue("Resource Manager", "Initialized");
    UI::keyValue("Loaded Tasks", to_string(taskCatalog.getTaskCount()));
    resourceManager.displayResources();

    syncManager.startResourceMonitor(resourceManager, logger);

    autoStartTask(
        8,
        "Digital Clock",
        taskCatalog,
        processManager,
        resourceManager,
        readyQueueManager,
        logger,
        syncManager,
        separateTerminalMode
    );

    autoStartTask(
        16,
        "Calendar",
        taskCatalog,
        processManager,
        resourceManager,
        readyQueueManager,
        logger,
        syncManager,
        separateTerminalMode
    );

    if (readyQueueManager.hasReadyProcess()) {
        UI::sectionTitle("Auto-start Dispatch");
        cout << "Dispatching startup tasks so clock and calendar run immediately after boot.\n";

        scheduler.runScheduler(
            processManager,
            resourceManager,
            readyQueueManager,
            logger,
            syncManager
        );
    }

    do {
        if (multitaskingModeEnabled) {
            scheduler.handleTerminalWindowState(
                processManager,
                readyQueueManager,
                logger,
                syncManager,
                true
            );
            scheduler.reapFinishedParallelTasks(processManager, resourceManager, logger);
        }

        showMainMenu(
            currentMode,
            separateTerminalMode,
            multitaskingModeEnabled,
            resourceManager,
            processManager,
            readyQueueManager,
            taskCatalog
        );
        UI::commandPrompt("Enter command: ");
        choice = getValidatedInteger("");

        switch (choice) {
            case 1:
                taskCatalog.displayAvailableTasks();
                demoPause();
                break;

            case 2:
                showTaskDetailsMenu(taskCatalog);
                demoPause();
                break;

            case 3:
                launchTaskUsingIPCForkTest(
                    taskCatalog,
                    processManager,
                    resourceManager,
                    readyQueueManager,
                    logger,
                    syncManager,
                    separateTerminalMode
                );
                break;

            case 4:
                resourceManager.displayResources();
                demoPause();
                break;

            case 7:
                processManager.displayPCBTable();
                demoPause();
                break;

            case 11:
                multitaskingModeEnabled = false;
                scheduler.runScheduler(
                    processManager,
                    resourceManager,
                    readyQueueManager,
                    logger,
                    syncManager
                );
                break;

            case 28:
                multitaskingModeEnabled = true;
                UI::infoLine("[MULTITASK] Running scheduler dispatch on main terminal for visibility.");
                scheduler.runScheduler(
                    processManager,
                    resourceManager,
                    readyQueueManager,
                    logger,
                    syncManager
                );
                scheduler.runMultitaskingDispatch(
                    processManager,
                    readyQueueManager,
                    logger,
                    syncManager
                );
                break;

            case 12:
                readyQueueManager.displayReadyQueues();
                demoPause();
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
                    demoPause();
                } else {
                    UI::errorLine("Access denied. System logs can only be viewed in Kernel Mode.");
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
                    UI::errorLine("Access denied. Process Killer can only be used in Kernel Mode.");
                    logger.logSystemEvent("User Mode tried to access Process Killer");
                }
                break;

            case 17:
                minimizeProcess(processManager, readyQueueManager, logger);
                break;

            case 18:
                resumeProcess(processManager, readyQueueManager, logger, syncManager);
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
                    UI::errorLine("Access denied. Deadlock detection can only be used in Kernel Mode.");
                    logger.logSystemEvent("User Mode tried to access Deadlock Detection");
                }
                break;

            case 20:
                resourceManager.displayMemoryLayout();
                demoPause();
                break;

            case 21:
                closeProcess(processManager, resourceManager, readyQueueManager, logger);
                break;

            case 22:
                switchToProcess(processManager, readyQueueManager, logger, syncManager);
                break;

            case 23:
                showInstructionGuide(taskCatalog);
                demoPause();
                break;

            case 24:
                processManager.displayProcessesByState(RUNNING_STATE, "Running Tasks List");
                demoPause();
                break;

            case 25:
                processManager.displayProcessesByState(MINIMIZED_STATE, "Minimized Tasks List");
                demoPause();
                break;

            case 26:
                handleInputInterrupt(processManager, readyQueueManager, logger);
                break;

            case 27:
                completeInterrupt(processManager, readyQueueManager, logger, syncManager);
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
                UI::errorLine("Invalid choice. Please select a valid option from the menu.");
        }

    } while (choice != 0);

    return 0;
}
