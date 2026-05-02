#include "scheduler.h"

/*
Function: Scheduler
Purpose: Initializes scheduler time quantum values.
Parameters: None.
Returns: Nothing.
*/
Scheduler::Scheduler() {
    interactiveQuantum = 8;
    backgroundQuantum = 4;
}

/*
Function: selectNextProcess
Purpose: Selects the next process from multilevel ready queues.
Parameters: ReadyQueueManager reference and ReadyQueueItem reference.
Returns: true if a process is selected, otherwise false.
*/
bool Scheduler::selectNextProcess(
    ReadyQueueManager &readyQueueManager,
    ReadyQueueItem &selectedItem
) {
    if (!readyQueueManager.isSystemQueueEmpty()) {
        selectedItem = readyQueueManager.removeFromSystemQueue();
        cout << "\n[SCHEDULER] Selected process from System Queue using FCFS.\n";
        return true;
    }

    if (!readyQueueManager.isInteractiveQueueEmpty()) {
        selectedItem = readyQueueManager.removeFromInteractiveQueue();
        cout << "\n[SCHEDULER] Selected process from Interactive Queue using Round Robin.\n";
        return true;
    }

    if (!readyQueueManager.isBackgroundQueueEmpty()) {
        selectedItem = readyQueueManager.removeFromBackgroundQueue();
        cout << "\n[SCHEDULER] Selected process from Background Queue using low priority scheduling.\n";
        return true;
    }

    return false;
}

/*
Function: releaseCompletedProcess
Purpose: Releases resources and removes a completed process from PCB table.
Parameters: PID, ProcessManager, and ResourceManager references.
Returns: Nothing.
*/
void Scheduler::releaseCompletedProcess(
    int pid,
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    Logger &logger
) {
    PCB pcb;

    if (!processManager.getPCB(pid, pcb)) {
        cout << "\n[SCHEDULER] PCB not found. Cannot release resources.\n";
        return;
    }

    processManager.updateProcessState(pid, TERMINATED_STATE);
    logger.logProcessEvent(pid, pcb.processName, "Process terminated");
    cout << "\n[SCHEDULER] Releasing resources for completed process.\n";
    cout << "PID: " << pid << "\n";
    cout << "Process: " << pcb.processName << "\n";

    resourceManager.releaseResources(
        pcb.ramRequired,
        pcb.hddRequired,
        pcb.coresRequired
    );
    logger.logResourceEvent(
    "Resources released from PID " + to_string(pid) +
    " | RAM: " + to_string(pcb.ramRequired) +
    "MB | HDD: " + to_string(pcb.hddRequired) +
    "MB | CPU: " + to_string(pcb.coresRequired)
);
    processManager.removeProcess(pid);
}

/*
Function: runProcess
Purpose: Resumes a selected process and applies scheduling rules based on process type.
Parameters: ReadyQueueItem, ProcessManager, ResourceManager, ReadyQueueManager references.
Returns: true if process finished, otherwise false.
*/
bool Scheduler::runProcess(
    ReadyQueueItem item,
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger
) {
    int status;
    int quantum;

    PCB pcb;

    if (!processManager.getPCB(item.pid, pcb)) {
        cout << "\n[SCHEDULER] Process not found in PCB table. Skipping PID: " << item.pid << "\n";
        return true;
    }

    cout << "\n==================== CONTEXT SWITCH ====================\n";
    cout << "[SCHEDULER] Dispatching Process\n";
    cout << "PID: " << item.pid << "\n";
    cout << "Process Name: " << item.processName << "\n";
    cout << "========================================================\n";

    processManager.updateProcessState(item.pid, RUNNING_STATE);
    logger.logProcessEvent(item.pid, item.processName, "Dispatched by scheduler");
    kill(item.pid, SIGCONT);

    if (pcb.processType == SYSTEM_PROCESS || pcb.processType == KERNEL_PROCESS) {
        cout << "[SCHEDULER] System process running with FCFS until completion.\n";

        waitpid(item.pid, &status, 0);

        cout << "\n[SCHEDULER] System process completed.\n";

        releaseCompletedProcess(item.pid, processManager, resourceManager, logger);
        return true;
    }

    if (pcb.processType == INTERACTIVE_PROCESS || pcb.processType == AUTO_RUNNING_PROCESS) {
        quantum = interactiveQuantum;
        cout << "[SCHEDULER] Interactive process running with quantum: "
             << quantum << " seconds.\n";
    } else {
        quantum = backgroundQuantum;
        cout << "[SCHEDULER] Background process running with quantum: "
             << quantum << " seconds.\n";
    }

    for (int i = 1; i <= quantum; i++) {
        sleep(1);

        pid_t result = waitpid(item.pid, &status, WNOHANG);

        if (result == item.pid) {
            cout << "\n[SCHEDULER] Process completed within time quantum.\n";
            releaseCompletedProcess(item.pid, processManager, resourceManager, logger);
            return true;
        }

        cout << "[SCHEDULER] Time slice " << i << "/" << quantum
             << " completed for PID " << item.pid << "\n";
    }

    cout << "\n[SCHEDULER] Time quantum expired for PID: " << item.pid << "\n";
    cout << "[SCHEDULER] Pausing process and moving it back to ready queue.\n";

    kill(item.pid, SIGSTOP);
    waitpid(item.pid, &status, WUNTRACED);

    processManager.updateProcessState(item.pid, READY_STATE);

    readyQueueManager.addProcessToReadyQueue(
        item.pid,
        item.processName,
        item.processType,
        item.priority
    );

    return false;
}

/*
Function: runScheduler
Purpose: Runs all ready processes using multilevel queue scheduling.
Parameters: ProcessManager, ResourceManager, and ReadyQueueManager references.
Returns: Nothing.
*/
void Scheduler::runScheduler(
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger
) {
    cout << "\n==================== AMS OS SCHEDULER STARTED ====================\n";
    logger.logSystemEvent("Scheduler started");
    if (!readyQueueManager.hasReadyProcess()) {
        cout << "[SCHEDULER] No process available in ready queue.\n";
        cout << "==================================================================\n";
        return;
    }

    while (readyQueueManager.hasReadyProcess()) {
        ReadyQueueItem selectedItem;

        if (!selectNextProcess(readyQueueManager, selectedItem)) {
            break;
        }

	 runProcess(
	    selectedItem,
	    processManager,
	    resourceManager,
	    readyQueueManager,
	    logger
	);

        cout << "\n[SCHEDULER] Current Resource Status:\n";
        resourceManager.displayResources();

        cout << "\n[SCHEDULER] Current Ready Queue Status:\n";
        readyQueueManager.displayReadyQueues();
    }
    logger.logSystemEvent("Scheduler finished");
    cout << "\n==================== AMS OS SCHEDULER FINISHED ====================\n";
}