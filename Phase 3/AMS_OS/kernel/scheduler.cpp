#include "scheduler.h"
#include <fstream>
#include "console_colors.h"

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
        cout << "\n" << Color::scheduler("[SCHEDULER]") << " Selected process from System Queue using FCFS.\n";
        return true;
    }

    if (!readyQueueManager.isInteractiveQueueEmpty()) {
        selectedItem = readyQueueManager.removeFromInteractiveQueue();
        cout << "\n" << Color::scheduler("[SCHEDULER]") << " Selected process from Interactive Queue using Round Robin.\n";
        return true;
    }

    if (!readyQueueManager.isBackgroundQueueEmpty()) {
        selectedItem = readyQueueManager.removeFromBackgroundQueue();
        cout << "\n" << Color::scheduler("[SCHEDULER]") << " Selected process from Background Queue using low priority scheduling.\n";
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
        cout << "\n" << Color::scheduler("[SCHEDULER]") << " PCB not found. Cannot release resources.\n";
        return;
    }

    processManager.updateProcessState(pid, TERMINATED_STATE);
    logger.logProcessEvent(pid, pcb.processName, "Process terminated");
    cout << "\n" << Color::scheduler("[SCHEDULER]") << " Releasing resources for completed process.\n";
    cout << "PID: " << pid << "\n";
    cout << "Process: " << pcb.processName << "\n";
    resourceManager.releaseMemoryBlock(pid);
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
Function: writeSchedulerGUIStatus
Purpose: Writes scheduler-side live process status to GUI status file.
Parameters: ResourceManager, ProcessManager, OS mode name, and task mode name.
Returns: Nothing.
*/
void Scheduler::writeSchedulerGUIStatus(
    ResourceManager &resourceManager,
    ProcessManager &processManager,
    string osModeName,
    string taskModeName
) {
    ofstream file("data/gui_status.txt");

    if (!file) {
        return;
    }

    file << "OS_MODE=" << osModeName << "\n";
    file << "TASK_MODE=" << taskModeName << "\n";

    file << "RAM_AVAILABLE=" << resourceManager.getAvailableRAM() << "\n";
    file << "RAM_TOTAL=" << resourceManager.getTotalRAM() << "\n";

    file << "HDD_AVAILABLE=" << resourceManager.getAvailableHDD() << "\n";
    file << "HDD_TOTAL=" << resourceManager.getTotalHDD() << "\n";

    file << "CORES_AVAILABLE=" << resourceManager.getAvailableCores() << "\n";
    file << "CORES_TOTAL=" << resourceManager.getTotalCores() << "\n";

    vector<PCB> pcbList = processManager.getAllPCBs();

    for (PCB pcb : pcbList) {
        string ramBlock;

        if (pcb.memoryStart == -1 || pcb.memoryEnd == -1) {
            ramBlock = "N/A";
        } else {
            ramBlock = to_string(pcb.memoryStart) + "-" + to_string(pcb.memoryEnd) + " MB";
        }

        file << "PROCESS="
             << pcb.pid << "|"
             << pcb.processName << "|"
             << processManager.getProcessStateName(pcb.processState) << "|"
             << pcb.priority << "|"
             << ramBlock
             << "\n";
    }

    file.close();
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
    Logger &logger,
    SyncManager &syncManager,
    string osModeName,
    string taskModeName
){
    int status;
    int quantum;

    PCB pcb;

    if (!processManager.getPCB(item.pid, pcb)) {
        cout << "\n" << Color::scheduler("[SCHEDULER]") << " Process not found in PCB table. Skipping PID: " << item.pid << "\n";
        return true;
    }
	syncManager.acquireCPUCores(pcb.coresRequired);

	logger.logSystemEvent(
	    "CPU execution slot assigned to PID " + to_string(item.pid) +
	    " | Process: " + item.processName
	);
    cout << Color::scheduler("\n==================== CONTEXT SWITCH ====================\n");
    cout  << Color::scheduler("[SCHEDULER]") << " Dispatching Process\n";
    cout << "PID: " << item.pid << "\n";
    cout << "Process Name: " << item.processName << "\n";
    cout << "========================================================\n";

    processManager.updateProcessState(item.pid, RUNNING_STATE);
    writeSchedulerGUIStatus(
	    resourceManager,
	    processManager,
	    osModeName,
	    taskModeName
	);
    logger.logProcessEvent(item.pid, item.processName, "Dispatched by scheduler");
    kill(item.pid, SIGCONT);

    if (pcb.processType == SYSTEM_PROCESS || pcb.processType == KERNEL_PROCESS) {
        cout << Color::scheduler("[SCHEDULER]") << " System process running with FCFS until completion.\n";

       waitpid(item.pid, &status, 0);

	cout << "\n" << Color::success("[SCHEDULER]") << " System process completed.\n";

	syncManager.releaseCPUCores(pcb.coresRequired);

	logger.logSystemEvent(
	    "CPU execution slot released from PID " + to_string(item.pid)
	);

	releaseCompletedProcess(item.pid, processManager, resourceManager, logger);
	writeSchedulerGUIStatus(
	    resourceManager,
	    processManager,
	    osModeName,
	    taskModeName
	);
	return true;
    }

    if (pcb.processType == INTERACTIVE_PROCESS || pcb.processType == AUTO_RUNNING_PROCESS) {
        quantum = interactiveQuantum;
        cout << Color::scheduler("[SCHEDULER]") << " Interactive process running with quantum: "
             << quantum << " seconds.\n";
    } else {
        quantum = backgroundQuantum;
        cout <<  Color::scheduler("[SCHEDULER]") << " Background process running with quantum: "
             << quantum << " seconds.\n";
    }

    for (int i = 1; i <= quantum; i++) {
        sleep(1);

        pid_t result = waitpid(item.pid, &status, WNOHANG);

       if (result == item.pid) {
	    cout << "\n" << Color::success("[SCHEDULER]") << " Process completed within time quantum.\n";

	    syncManager.releaseCPUCores(pcb.coresRequired);

	    logger.logSystemEvent(
		"CPU execution slot released from PID " + to_string(item.pid)
	    );

	    releaseCompletedProcess(item.pid, processManager, resourceManager, logger);
            writeSchedulerGUIStatus(
		    resourceManager,
		    processManager,
		    osModeName,
		    taskModeName
		);
	    return true;
	}

        cout << Color::scheduler("[SCHEDULER]") << " Time slice " << i << "/" << quantum
             << " completed for PID " << item.pid << "\n";
    }

    cout << "\n" << Color::scheduler("[SCHEDULER]") << " Time quantum expired for PID: " << item.pid << "\n";
    cout <<  Color::scheduler("[SCHEDULER]") << " Pausing process and moving it back to ready queue.\n";

    kill(item.pid, SIGSTOP);
	waitpid(item.pid, &status, WUNTRACED);

	syncManager.releaseCPUCores(pcb.coresRequired);

	logger.logSystemEvent(
	    "CPU execution slot released after quantum expiry for PID " + to_string(item.pid)
	);

	processManager.updateProcessState(item.pid, READY_STATE);
        writeSchedulerGUIStatus(
		    resourceManager,
		    processManager,
		    osModeName,
		    taskModeName
		);
	readyQueueManager.addProcessToReadyQueue(
	    item.pid,
	    item.processName,
	    item.processType,
	    item.priority
	);

	syncManager.notifyReadyQueue();

	return false;
}
/*
Function: runSingleProcessByPID
Purpose: Dispatches one selected READY process by PID, mainly for GUI click execution.
Parameters: PID and core AMS OS managers.
Returns: true if process dispatch is attempted, otherwise false.
*/
bool Scheduler::runSingleProcessByPID(
    int pid,
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager,
    string osModeName,
    string taskModeName
) {
    PCB pcb;

    if (!processManager.getPCB(pid, pcb)) {
        cout << "\n[GUI COMMAND] PID not found in PCB table.\n";
        logger.logProcessEvent(pid, "Unknown", "GUI dispatch failed, PID not found");
        return false;
    }

    if (pcb.processState != READY_STATE) {
        cout << "\n[GUI COMMAND] Only READY processes can be dispatched from GUI.\n";
        cout << "PID: " << pid << "\n";
        cout << "Current State: " << processManager.getProcessStateName(pcb.processState) << "\n";

        logger.logProcessEvent(pid, pcb.processName, "GUI dispatch rejected, process not READY");
        return false;
    }

    readyQueueManager.removeProcessByPID(pid);

    ReadyQueueItem item;
    item.pid = pcb.pid;
    item.processName = pcb.processName;
    item.processType = pcb.processType;
    item.priority = pcb.priority;

    cout << "\n[GUI COMMAND] Dispatching selected process from GUI.\n";
    cout << "PID: " << pid << "\n";
    cout << "Process: " << pcb.processName << "\n";

    logger.logProcessEvent(pid, pcb.processName, "GUI requested process dispatch");

    writeSchedulerGUIStatus(
        resourceManager,
        processManager,
        osModeName,
        taskModeName
    );

    runProcess(
        item,
        processManager,
        resourceManager,
        readyQueueManager,
        logger,
        syncManager,
        osModeName,
        taskModeName
    );

    writeSchedulerGUIStatus(
        resourceManager,
        processManager,
        osModeName,
        taskModeName
    );

    return true;
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
    Logger &logger,
    SyncManager &syncManager,
    string osModeName,
    string taskModeName
) {
    cout << Color::scheduler("\n==================== AMS OS SCHEDULER STARTED ====================\n");
    logger.logSystemEvent("Scheduler started");
    if (!readyQueueManager.hasReadyProcess()) {
    cout <<  Color::scheduler("[SCHEDULER]") << " No process available in ready queue.\n";
    cout <<Color::scheduler("[SCHEDULER]") << " Waiting briefly for ready queue notification.\n";

    syncManager.waitForReadyQueueSignal(2);

    if (!readyQueueManager.hasReadyProcess()) {
        cout <<  Color::scheduler("[SCHEDULER]") << " Still no process available.\n";
        cout << "==================================================================\n";
        return;
    }
}

    while (readyQueueManager.hasReadyProcess()) {
    ReadyQueueItem selectedItem;

    /*
    Aging is applied before selecting the next process.
    This increases waiting time of processes already sitting in ready queues.
    If a process waits too long, its priority improves.
    */
    readyQueueManager.applyAging(processManager, logger, 2);

    if (!selectNextProcess(readyQueueManager, selectedItem)) {
        break;
    }

    processManager.resetWaitingTime(selectedItem.pid);

    runProcess(
    selectedItem,
    processManager,
    resourceManager,
    readyQueueManager,
    logger,
    syncManager,
    osModeName,
    taskModeName
       );

    cout << "\n" << Color::scheduler("[SCHEDULER]") << " Current Resource Status:\n";
    resourceManager.displayResources();

    cout << "\n" << Color::scheduler("[SCHEDULER]") << " Current Ready Queue Status:\n";
    readyQueueManager.displayReadyQueues();
}
    logger.logSystemEvent("Scheduler finished");
    cout << "\n==================== AMS OS SCHEDULER FINISHED ====================\n";
}
