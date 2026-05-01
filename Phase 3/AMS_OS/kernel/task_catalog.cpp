#include "task_catalog.h"

/*
Function: TaskCatalog
Purpose: Initializes the task catalog with Phase 3 task metadata.
Parameters: None.
Returns: Nothing.
*/
TaskCatalog::TaskCatalog() {
    initializePhase3Tasks();
}

/*
Function: initializePhase3Tasks
Purpose: Adds the initial Phase 3 tasks into the task catalog.
Parameters: None.
Returns: Nothing.
*/
void TaskCatalog::initializePhase3Tasks() {
    taskList.clear();

    TaskInfo calculator;
    calculator.taskID = 1;
    calculator.taskName = "Calculator";
    calculator.processType = INTERACTIVE_PROCESS;
    calculator.priority = 2;
    calculator.ramRequired = 100;
    calculator.hddRequired = 20;
    calculator.coresRequired = 1;
    calculator.executablePath = "./build/calculator";
    calculator.description = "Interactive calculator task for basic arithmetic operations.";
    taskList.push_back(calculator);

    TaskInfo notepad;
    notepad.taskID = 2;
    notepad.taskName = "Notepad";
    notepad.processType = INTERACTIVE_PROCESS;
    notepad.priority = 2;
    notepad.ramRequired = 150;
    notepad.hddRequired = 50;
    notepad.coresRequired = 1;
    notepad.executablePath = "./build/notepad";
    notepad.description = "Interactive notepad task with file writing support.";
    taskList.push_back(notepad);

    TaskInfo clock;
    clock.taskID = 3;
    clock.taskName = "Digital Clock";
    clock.processType = AUTO_RUNNING_PROCESS;
    clock.priority = 1;
    clock.ramRequired = 50;
    clock.hddRequired = 10;
    clock.coresRequired = 1;
    clock.executablePath = "./build/clock";
    clock.description = "Auto-running digital clock task.";
    taskList.push_back(clock);

    TaskInfo musicPlayer;
    musicPlayer.taskID = 4;
    musicPlayer.taskName = "Music Player";
    musicPlayer.processType = BACKGROUND_PROCESS;
    musicPlayer.priority = 3;
    musicPlayer.ramRequired = 120;
    musicPlayer.hddRequired = 30;
    musicPlayer.coresRequired = 1;
    musicPlayer.executablePath = "./build/music_player";
    musicPlayer.description = "Background music player simulation using beep output.";
    taskList.push_back(musicPlayer);

    TaskInfo fileCopy;
    fileCopy.taskID = 5;
    fileCopy.taskName = "File Copy";
    fileCopy.processType = BACKGROUND_PROCESS;
    fileCopy.priority = 3;
    fileCopy.ramRequired = 200;
    fileCopy.hddRequired = 100;
    fileCopy.coresRequired = 1;
    fileCopy.executablePath = "./build/file_copy";
    fileCopy.description = "Background file copy simulation task.";
    taskList.push_back(fileCopy);
}

/*
Function: displayAvailableTasks
Purpose: Displays all tasks currently available in AMS OS.
Parameters: None.
Returns: Nothing.
*/
void TaskCatalog::displayAvailableTasks() {
    cout << "\n==================== AMS OS TASK CATALOG ====================\n";

    if (taskList.empty()) {
        cout << "No tasks are available in the task catalog.\n";
        cout << "=============================================================\n";
        return;
    }

    cout << "ID\tTask Name\t\tType\t\tPriority\tRAM\tHDD\tCPU\n";
    cout << "-------------------------------------------------------------\n";

    for (TaskInfo task : taskList) {
        cout << task.taskID << "\t"
             << task.taskName << "\t\t"
             << getProcessTypeName(task.processType) << "\t"
             << task.priority << "\t\t"
             << task.ramRequired << "MB\t"
             << task.hddRequired << "MB\t"
             << task.coresRequired << "\n";
    }

    cout << "=============================================================\n";
}

/*
Function: displayTaskDetails
Purpose: Displays detailed information about one selected task.
Parameters: Task ID.
Returns: true if task is found, otherwise false.
*/
bool TaskCatalog::displayTaskDetails(int taskID) {
    TaskInfo selectedTask;

    if (!getTaskByID(taskID, selectedTask)) {
        cout << "\n[TASK CATALOG] Task not found.\n";
        return false;
    }

    cout << "\n==================== TASK DETAILS ====================\n";
    cout << "Task ID: " << selectedTask.taskID << "\n";
    cout << "Task Name: " << selectedTask.taskName << "\n";
    cout << "Task Type: " << getProcessTypeName(selectedTask.processType) << "\n";
    cout << "Priority: " << selectedTask.priority << "\n";
    cout << "RAM Required: " << selectedTask.ramRequired << " MB\n";
    cout << "HDD Required: " << selectedTask.hddRequired << " MB\n";
    cout << "CPU Cores Required: " << selectedTask.coresRequired << "\n";
    cout << "Executable Path: " << selectedTask.executablePath << "\n";
    cout << "Description: " << selectedTask.description << "\n";
    cout << "======================================================\n";

    return true;
}

/*
Function: getTaskByID
Purpose: Finds and returns a task using its task ID.
Parameters: Task ID and reference variable to store found task.
Returns: true if task is found, otherwise false.
*/
bool TaskCatalog::getTaskByID(int taskID, TaskInfo &task) {
    for (TaskInfo currentTask : taskList) {
        if (currentTask.taskID == taskID) {
            task = currentTask;
            return true;
        }
    }

    return false;
}

/*
Function: getTaskCount
Purpose: Returns the total number of tasks in the task catalog.
Parameters: None.
Returns: Total task count.
*/
int TaskCatalog::getTaskCount() {
    return taskList.size();
}

/*
Function: getProcessTypeName
Purpose: Converts process type enum into readable text.
Parameters: Process type.
Returns: Process type as string.
*/
string TaskCatalog::getProcessTypeName(ProcessType type) {
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