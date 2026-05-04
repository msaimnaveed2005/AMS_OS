#include "task_catalog.h"
#include <iomanip>
#include "console_colors.h"
/*
Function: getCatalogTypeColor
Purpose: Returns color based on process type.
Parameters: Process type.
Returns: ANSI color code.
*/
string getCatalogTypeColor(ProcessType type) {
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
Function: getCatalogPriorityColor
Purpose: Returns color based on priority.
Parameters: Priority number.
Returns: ANSI color code.
*/
string getCatalogPriorityColor(int priority) {
    if (priority == 1) {
        return Color::BRIGHT_RED + Color::BOLD;
    }

    if (priority == 2) {
        return Color::BRIGHT_GREEN + Color::BOLD;
    }

    return Color::BRIGHT_YELLOW + Color::BOLD;
}
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

    TaskInfo createFile;
    createFile.taskID = 1;
    createFile.taskName = "Create File";
    createFile.processType = INTERACTIVE_PROCESS;
    createFile.priority = 2;
    createFile.ramRequired = 80;
    createFile.hddRequired = 20;
    createFile.coresRequired = 1;
    createFile.executablePath = "./build/create_file";
    createFile.description = "Creates a file inside AMS OS virtual disk.";
    taskList.push_back(createFile);

    TaskInfo deleteFile;
    deleteFile.taskID = 2;
    deleteFile.taskName = "Delete File";
    deleteFile.processType = INTERACTIVE_PROCESS;
    deleteFile.priority = 2;
    deleteFile.ramRequired = 70;
    deleteFile.hddRequired = 10;
    deleteFile.coresRequired = 1;
    deleteFile.executablePath = "./build/delete_file";
    deleteFile.description = "Deletes a file from AMS OS virtual disk.";
    taskList.push_back(deleteFile);

    TaskInfo copyFile;
    copyFile.taskID = 3;
    copyFile.taskName = "Copy File";
    copyFile.processType = BACKGROUND_PROCESS;
    copyFile.priority = 3;
    copyFile.ramRequired = 200;
    copyFile.hddRequired = 100;
    copyFile.coresRequired = 1;
    copyFile.executablePath = "./build/file_copy";
    copyFile.description = "Copies a file from one path to another.";
    taskList.push_back(copyFile);

    TaskInfo moveFile;
    moveFile.taskID = 4;
    moveFile.taskName = "Move File";
    moveFile.processType = BACKGROUND_PROCESS;
    moveFile.priority = 3;
    moveFile.ramRequired = 150;
    moveFile.hddRequired = 60;
    moveFile.coresRequired = 1;
    moveFile.executablePath = "./build/move_file";
    moveFile.description = "Moves or renames a file in virtual disk.";
    taskList.push_back(moveFile);

    TaskInfo fileInfo;
    fileInfo.taskID = 5;
    fileInfo.taskName = "File Info";
    fileInfo.processType = INTERACTIVE_PROCESS;
    fileInfo.priority = 2;
    fileInfo.ramRequired = 90;
    fileInfo.hddRequired = 20;
    fileInfo.coresRequired = 1;
    fileInfo.executablePath = "./build/file_info";
    fileInfo.description = "Displays basic information about a file.";
    taskList.push_back(fileInfo);

    TaskInfo notepad;
    notepad.taskID = 6;
    notepad.taskName = "Notepad";
    notepad.processType = INTERACTIVE_PROCESS;
    notepad.priority = 2;
    notepad.ramRequired = 150;
    notepad.hddRequired = 50;
    notepad.coresRequired = 1;
    notepad.executablePath = "./build/notepad";
    notepad.description = "Interactive notepad task with file writing support.";
    taskList.push_back(notepad);

    TaskInfo calculator;
    calculator.taskID = 7;
    calculator.taskName = "Calculator";
    calculator.processType = INTERACTIVE_PROCESS;
    calculator.priority = 2;
    calculator.ramRequired = 100;
    calculator.hddRequired = 20;
    calculator.coresRequired = 1;
    calculator.executablePath = "./build/calculator";
    calculator.description = "Interactive calculator task for basic arithmetic operations.";
    taskList.push_back(calculator);

    TaskInfo clock;
    clock.taskID = 8;
    clock.taskName = "Digital Clock";
    clock.processType = AUTO_RUNNING_PROCESS;
    clock.priority = 2;
    clock.ramRequired = 50;
    clock.hddRequired = 10;
    clock.coresRequired = 1;
    clock.executablePath = "./build/clock";
    clock.description = "Auto-running digital clock task.";
    taskList.push_back(clock);

    TaskInfo systemInfo;
    systemInfo.taskID = 9;
    systemInfo.taskName = "System Info";
    systemInfo.processType = INTERACTIVE_PROCESS;
    systemInfo.priority = 2;
    systemInfo.ramRequired = 80;
    systemInfo.hddRequired = 10;
    systemInfo.coresRequired = 1;
    systemInfo.executablePath = "./build/system_info";
    systemInfo.description = "Displays system information.";
    taskList.push_back(systemInfo);

    TaskInfo snake;
    snake.taskID = 10;
    snake.taskName = "Snake Game";
    snake.processType = INTERACTIVE_PROCESS;
    snake.priority = 2;
    snake.ramRequired = 250;
    snake.hddRequired = 40;
    snake.coresRequired = 1;
    snake.executablePath = "./build/snake";
    snake.description = "Simple text-based snake game.";
    taskList.push_back(snake);

    TaskInfo minesweeper;
    minesweeper.taskID = 11;
    minesweeper.taskName = "Minesweeper";
    minesweeper.processType = INTERACTIVE_PROCESS;
    minesweeper.priority = 2;
    minesweeper.ramRequired = 220;
    minesweeper.hddRequired = 40;
    minesweeper.coresRequired = 1;
    minesweeper.executablePath = "./build/minesweeper";
    minesweeper.description = "Simple text-based minesweeper game.";
    taskList.push_back(minesweeper);

    TaskInfo musicPlayer;
    musicPlayer.taskID = 12;
    musicPlayer.taskName = "Music Player";
    musicPlayer.processType = BACKGROUND_PROCESS;
    musicPlayer.priority = 3;
    musicPlayer.ramRequired = 120;
    musicPlayer.hddRequired = 30;
    musicPlayer.coresRequired = 1;
    musicPlayer.executablePath = "./build/music_player";
    musicPlayer.description = "Background music player simulation using beep output.";
    taskList.push_back(musicPlayer);

    TaskInfo downloadSimulator;
    downloadSimulator.taskID = 13;
    downloadSimulator.taskName = "Download Simulator";
    downloadSimulator.processType = BACKGROUND_PROCESS;
    downloadSimulator.priority = 3;
    downloadSimulator.ramRequired = 180;
    downloadSimulator.hddRequired = 200;
    downloadSimulator.coresRequired = 1;
    downloadSimulator.executablePath = "./build/download_simulator";
    downloadSimulator.description = "Simulates a background file download.";
    taskList.push_back(downloadSimulator);

    TaskInfo taskManager;
    taskManager.taskID = 14;
    taskManager.taskName = "Task Manager";
    taskManager.processType = SYSTEM_PROCESS;
    taskManager.priority = 1;
    taskManager.ramRequired = 100;
    taskManager.hddRequired = 20;
    taskManager.coresRequired = 1;
    taskManager.executablePath = "./build/task_manager";
    taskManager.description = "Task manager executable. Full process table is handled by kernel menu.";
    taskList.push_back(taskManager);

    TaskInfo processKiller;
    processKiller.taskID = 15;
    processKiller.taskName = "Process Killer";
    processKiller.processType = KERNEL_PROCESS;
    processKiller.priority = 1;
    processKiller.ramRequired = 100;
    processKiller.hddRequired = 20;
    processKiller.coresRequired = 1;
    processKiller.executablePath = "./build/process_killer";
    processKiller.description = "Kernel mode process killer information task.";
    taskList.push_back(processKiller);
}

/*
Function: displayAvailableTasks
Purpose: Displays all tasks currently available in AMS OS.
Parameters: None.
Returns: Nothing.
*/
void TaskCatalog::displayAvailableTasks() {
    cout << "\n";
    Color::line('=', 104, Color::BRIGHT_CYAN + Color::BOLD);
    cout << Color::paint("                              AMS OS TASK CATALOG\n", Color::BRIGHT_CYAN + Color::BOLD);
    Color::line('=', 104, Color::BRIGHT_CYAN + Color::BOLD);

    if (taskList.empty()) {
        cout << Color::warning("No tasks are available in the task catalog.") << "\n";
        Color::line('=', 104, Color::BRIGHT_CYAN + Color::BOLD);
        return;
    }

    Color::cell("ID", 6, Color::BRIGHT_CYAN + Color::BOLD);
    Color::cell("TASK NAME", 24, Color::BRIGHT_CYAN + Color::BOLD);
    Color::cell("TYPE", 18, Color::BRIGHT_CYAN + Color::BOLD);
    Color::cell("PRI", 8, Color::BRIGHT_CYAN + Color::BOLD);
    Color::cell("RAM", 10, Color::BRIGHT_CYAN + Color::BOLD);
    Color::cell("HDD", 10, Color::BRIGHT_CYAN + Color::BOLD);
    Color::cell("CPU", 8, Color::BRIGHT_CYAN + Color::BOLD);
    Color::cell("STATUS", 14, Color::BRIGHT_CYAN + Color::BOLD);
    cout << "\n";

    Color::line('-', 104, Color::GRAY);

    for (TaskInfo task : taskList) {
        Color::cell(to_string(task.taskID), 6, Color::WHITE);
        Color::cell(task.taskName, 24, Color::BRIGHT_WHITE + Color::BOLD);
        Color::cell(getProcessTypeName(task.processType), 18, getCatalogTypeColor(task.processType));
        Color::cell(to_string(task.priority), 8, getCatalogPriorityColor(task.priority));
        Color::cell(to_string(task.ramRequired) + "MB", 10, Color::BRIGHT_BLUE);
        Color::cell(to_string(task.hddRequired) + "MB", 10, Color::BRIGHT_MAGENTA);
        Color::cell(to_string(task.coresRequired), 8, Color::BRIGHT_GREEN);
        Color::cell("READY", 14, Color::BRIGHT_GREEN + Color::BOLD);
        cout << "\n";
    }

    Color::line('=', 104, Color::BRIGHT_CYAN + Color::BOLD);

    cout << Color::paint("Legend: ", Color::WHITE + Color::BOLD)
         << Color::paint("System/Kernel", Color::BRIGHT_MAGENTA + Color::BOLD)
         << " | "
         << Color::paint("Interactive", Color::BRIGHT_GREEN + Color::BOLD)
         << " | "
         << Color::paint("Background", Color::BRIGHT_YELLOW + Color::BOLD)
         << " | "
         << Color::paint("Auto", Color::BRIGHT_CYAN + Color::BOLD)
         << "\n";
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