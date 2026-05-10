#include "task_catalog.h"
#include "ui.h"
#include <iomanip>
#include <utility>

void TaskCatalog::addTask(
    int taskID,
    const std::string &taskName,
    ProcessType processType,
    int priority,
    int ramRequired,
    int hddRequired,
    int coresRequired,
    const std::string &executablePath,
    const std::string &description
) {
    TaskInfo task {
        taskID,
        taskName,
        processType,
        priority,
        ramRequired,
        hddRequired,
        coresRequired,
        executablePath,
        description
    };
    taskList.push_back(std::move(task));
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
    addTask(1, "Create File", INTERACTIVE_PROCESS, 2, 80, 20, 1, "./build/create_file",
            "Creates a file inside AMS OS virtual disk.");
    addTask(2, "Delete File", INTERACTIVE_PROCESS, 2, 70, 10, 1, "./build/delete_file",
            "Deletes a file from AMS OS virtual disk.");
    addTask(3, "Copy File", BACKGROUND_PROCESS, 3, 200, 100, 1, "./build/file_copy",
            "Copies a file from one path to another.");
    addTask(4, "Move File", BACKGROUND_PROCESS, 3, 150, 60, 1, "./build/move_file",
            "Moves or renames a file in virtual disk.");
    addTask(5, "File Info", INTERACTIVE_PROCESS, 2, 90, 20, 1, "./build/file_info",
            "Displays basic information about a file.");
    addTask(6, "Notepad", INTERACTIVE_PROCESS, 2, 150, 50, 1, "./build/notepad",
            "Interactive notepad task with file writing support.");
    addTask(7, "Calculator", INTERACTIVE_PROCESS, 2, 100, 20, 1, "./build/calculator",
            "Interactive calculator task for basic arithmetic operations.");
    addTask(8, "Digital Clock", AUTO_RUNNING_PROCESS, 2, 50, 10, 1, "./build/clock",
            "Auto-running digital clock task.");
    addTask(9, "System Info", INTERACTIVE_PROCESS, 2, 80, 10, 1, "./build/system_info",
            "Displays system information.");
    addTask(10, "Snake Game", INTERACTIVE_PROCESS, 2, 250, 40, 1, "./build/snake",
            "Simple text-based snake game.");
    addTask(11, "Minesweeper", INTERACTIVE_PROCESS, 2, 220, 40, 1, "./build/minesweeper",
            "Simple text-based minesweeper game.");
    addTask(12, "Music Player", BACKGROUND_PROCESS, 3, 120, 30, 1, "./build/music_player",
            "Background music player simulation using beep output.");
    addTask(13, "Download Simulator", BACKGROUND_PROCESS, 3, 180, 200, 1, "./build/download_simulator",
            "Simulates a background file download.");
    addTask(14, "Task Manager", SYSTEM_PROCESS, 1, 100, 20, 1, "./build/task_manager",
            "Task manager executable. Full process table is handled by kernel menu.");
    addTask(15, "Process Killer", KERNEL_PROCESS, 1, 100, 20, 1, "./build/process_killer",
            "Kernel mode process killer information task.");
    addTask(16, "Calendar", AUTO_RUNNING_PROCESS, 2, 50, 10, 1, "./build/calendar",
            "Auto-running monthly calendar task.");
}

/*
Function: displayAvailableTasks
Purpose: Displays all tasks currently available in AMS OS.
Parameters: None.
Returns: Nothing.
*/
void TaskCatalog::displayAvailableTasks() {
    UI::panelHeader("Task Catalog", std::to_string(taskList.size()) + " available tasks", 92);

    if (taskList.empty()) {
        UI::emptyState("No tasks are available in the task catalog.", 92);
        return;
    }

    cout << "  ";
    cout << UI::paint("ID", UI::WHITE + UI::BOLD);
    cout << string(4, ' ');
    cout << UI::paint("Task Name", UI::WHITE + UI::BOLD);
    cout << string(13, ' ');
    cout << UI::paint("Type", UI::WHITE + UI::BOLD);
    cout << string(12, ' ');
    cout << UI::paint("Priority", UI::WHITE + UI::BOLD);
    cout << string(4, ' ');
    cout << UI::paint("RAM", UI::WHITE + UI::BOLD);
    cout << string(8, ' ');
    cout << UI::paint("HDD", UI::WHITE + UI::BOLD);
    cout << string(8, ' ');
    cout << UI::paint("CPU", UI::WHITE + UI::BOLD);
    cout << string(3, ' ');
    cout << UI::paint("Description", UI::WHITE + UI::BOLD) << "\n";

    cout << "  " << UI::paint(UI::repeat('-', 86) + "\n", UI::DIM);

    for (const TaskInfo &task : taskList) {
        cout << "  ";
        cout << left
             << setw(6)  << UI::paint(std::to_string(task.taskID), UI::WHITE + UI::BOLD)
             << setw(22) << UI::paint(UI::fit(task.taskName, 20), UI::WHITE + UI::BOLD)
             << setw(16) << UI::paint(getProcessTypeName(task.processType), UI::WHITE)
             << setw(10) << UI::paint(std::to_string(task.priority), UI::WHITE)
             << setw(11) << UI::paint(std::to_string(task.ramRequired) + " MB", UI::WHITE)
             << setw(11) << UI::paint(std::to_string(task.hddRequired) + " MB", UI::WHITE)
             << setw(6)  << UI::paint(std::to_string(task.coresRequired), UI::WHITE)
             << UI::paint(UI::fit(task.description, 26), UI::WHITE)
             << "\n";
    }

    UI::panelFooter(92);
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

    UI::panelHeader("Task Details", selectedTask.taskName);
    UI::keyValue("Task ID", std::to_string(selectedTask.taskID));
    UI::keyValue("Task Type", getProcessTypeName(selectedTask.processType));
    UI::keyValue("Priority", std::to_string(selectedTask.priority));
    UI::keyValue("RAM Required", std::to_string(selectedTask.ramRequired) + " MB");
    UI::keyValue("HDD Required", std::to_string(selectedTask.hddRequired) + " MB");
    UI::keyValue("CPU Cores Required", std::to_string(selectedTask.coresRequired));
    UI::keyValue("Executable Path", selectedTask.executablePath);
    UI::keyValue("Description", selectedTask.description);
    UI::panelFooter();

    return true;
}

/*
Function: getTaskByID
Purpose: Finds and returns a task using its task ID.
Parameters: Task ID and reference variable to store found task.
Returns: true if task is found, otherwise false.
*/
bool TaskCatalog::getTaskByID(int taskID, TaskInfo &task) const {
    for (const TaskInfo &currentTask : taskList) {
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
int TaskCatalog::getTaskCount() const {
    return static_cast<int>(taskList.size());
}

/*
Function: getProcessTypeName
Purpose: Converts process type enum into readable text.
Parameters: Process type.
Returns: Process type as string.
*/
std::string TaskCatalog::getProcessTypeName(ProcessType type) const {
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
