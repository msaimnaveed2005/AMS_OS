#ifndef TASK_CATALOG_H
#define TASK_CATALOG_H

#include <iostream>
#include <vector>
#include <string>
#include "process_manager.h"

struct TaskInfo {
    int taskID;
    std::string taskName;
    ProcessType processType;
    int priority;
    int ramRequired;
    int hddRequired;
    int coresRequired;
    std::string executablePath;
    std::string description;
};

class TaskCatalog {
private:
    std::vector<TaskInfo> taskList;

    void addTask(
        int taskID,
        const std::string &taskName,
        ProcessType processType,
        int priority,
        int ramRequired,
        int hddRequired,
        int coresRequired,
        const std::string &executablePath,
        const std::string &description
    );

public:
    /*
    Function: TaskCatalog
    Purpose: Initializes the task catalog with Phase 3 task metadata.
    Parameters: None.
    Returns: Nothing.
    */
    TaskCatalog();

    /*
    Function: initializePhase3Tasks
    Purpose: Adds the initial Phase 3 tasks into the task catalog.
    Parameters: None.
    Returns: Nothing.
    */
    void initializePhase3Tasks();

    /*
    Function: displayAvailableTasks
    Purpose: Displays all tasks currently available in AMS OS.
    Parameters: None.
    Returns: Nothing.
    */
    void displayAvailableTasks();

    /*
    Function: displayTaskDetails
    Purpose: Displays detailed information about one selected task.
    Parameters: Task ID.
    Returns: true if task is found, otherwise false.
    */
    bool displayTaskDetails(int taskID);

    /*
    Function: getTaskByID
    Purpose: Finds and returns a task using its task ID.
    Parameters: Task ID and reference variable to store found task.
    Returns: true if task is found, otherwise false.
    */
    bool getTaskByID(int taskID, TaskInfo &task) const;

    /*
    Function: getTaskCount
    Purpose: Returns the total number of tasks in the task catalog.
    Parameters: None.
    Returns: Total task count.
    */
    int getTaskCount() const;

    /*
    Function: getProcessTypeName
    Purpose: Converts process type enum into readable text.
    Parameters: Process type.
    Returns: Process type as string.
    */
    std::string getProcessTypeName(ProcessType type) const;
};

#endif