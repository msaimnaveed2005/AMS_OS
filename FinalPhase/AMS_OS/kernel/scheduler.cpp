#include "scheduler.h"
#include "ui.h"
#include <cstdio>
#include <sstream>

namespace {
bool sendSignalToScheduledProcess(int pid, int signalNumber) {
    if (pid <= 0) {
        return false;
    }

    if (kill(-pid, signalNumber) == 0) {
        return true;
    }

    return kill(pid, signalNumber) == 0;
}

bool commandHasOutput(const string &command) {
    FILE *pipe = popen(command.c_str(), "r");

    if (pipe == nullptr) {
        return false;
    }

    char buffer[128];
    bool hasOutput = fgets(buffer, sizeof(buffer), pipe) != nullptr;
    pclose(pipe);

    return hasOutput;
}

bool isTaskTerminalHidden(int pid) {
    if (pid <= 0) {
        return false;
    }

    ostringstream visibleCommand;
    visibleCommand << "xdotool search --onlyvisible --pid " << pid << " 2>/dev/null";

    ostringstream anyWindowCommand;
    anyWindowCommand << "xdotool search --pid " << pid << " 2>/dev/null";

    const bool hasVisibleWindow = commandHasOutput(visibleCommand.str());
    const bool hasAnyWindow = commandHasOutput(anyWindowCommand.str());

    return hasAnyWindow && !hasVisibleWindow;
}

void restoreVisibleMinimizedTasks(
    ProcessManager &processManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager
) {
    vector<int> pids = processManager.getAllPIDs();

    for (int pid : pids) {
        PCB pcb;

        if (!processManager.getPCB(pid, pcb)) {
            continue;
        }

        if (pcb.processState != MINIMIZED_STATE) {
            continue;
        }

        if (pcb.queueType != "Terminal Minimized") {
            continue;
        }

        if (isTaskTerminalHidden(pid)) {
            continue;
        }

        processManager.updateProcessState(pid, READY_STATE);
        processManager.updateQueueType(pid, readyQueueManager.getQueueName(pcb.priority));
        readyQueueManager.addProcessToReadyQueue(pid, pcb.processName, pcb.processType, pcb.priority);
        syncManager.notifyReadyQueue();

        logger.logProcessEvent(
            pid,
            pcb.processName,
            "Terminal restored from window controls, process moved back to READY"
        );
        UI::successLine(
            "[SCHEDULER] Terminal restored for PID " + to_string(pid) + ". Process returned to READY."
        );
    }
}
}

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
        UI::infoLine("[SCHEDULER] Selected process from System Queue using FCFS.");
        return true;
    }

    if (!readyQueueManager.isInteractiveQueueEmpty()) {
        selectedItem = readyQueueManager.removeFromInteractiveQueue();
        UI::infoLine("[SCHEDULER] Selected process from Interactive Queue using Round Robin.");
        return true;
    }

    if (!readyQueueManager.isBackgroundQueueEmpty()) {
        selectedItem = readyQueueManager.removeFromBackgroundQueue();
        UI::infoLine("[SCHEDULER] Selected process from Background Queue using FCFS.");
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
        UI::warnLine("[SCHEDULER] PCB not found. Cannot release resources.");
        return;
    }

    processManager.updateProcessState(pid, TERMINATED_STATE);
    logger.logProcessEvent(pid, pcb.processName, "Process terminated");
    UI::successLine("[SCHEDULER] Releasing resources for completed process.");
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
    SyncManager &syncManager
){
    int status;
    int quantum;
    vector<int> assignedCores;

    PCB pcb;

    if (!processManager.getPCB(item.pid, pcb)) {
        UI::warnLine("[SCHEDULER] Process not found in PCB table. Skipping dispatch.");
        return true;
    }
    assignedCores = syncManager.acquireCoreSet(pcb.coresRequired);
    int primaryCore = -1;

    if (!assignedCores.empty()) {
        primaryCore = assignedCores.front();
    }

    processManager.updateAssignedCore(item.pid, primaryCore);
    processManager.updateQueueType(item.pid, readyQueueManager.getQueueName(item.priority));

	logger.logSystemEvent(
	    "CPU execution slot assigned to PID " + to_string(item.pid) +
	    " | Process: " + item.processName +
        " | Core: " + (primaryCore == -1 ? string("N/A") : to_string(primaryCore))
	);
    UI::panelHeader("Context Switch", "Scheduler dispatch");
    UI::keyValue("PID", to_string(item.pid));
    UI::keyValue("Process Name", item.processName);
    UI::keyValue("Queue", readyQueueManager.getQueueName(item.priority));
    UI::panelFooter();

    processManager.updateProcessState(item.pid, RUNNING_STATE);
    logger.logProcessEvent(item.pid, item.processName, "Dispatched by scheduler");
    sendSignalToScheduledProcess(item.pid, SIGCONT);

    if (pcb.processType == SYSTEM_PROCESS || pcb.processType == KERNEL_PROCESS) {
        UI::infoLine("[SCHEDULER] System process running with FCFS until completion.");

       waitpid(item.pid, &status, 0);
        processManager.incrementTurnaroundTime(item.pid, 1);

    UI::successLine("[SCHEDULER] System process completed.");

    processManager.updateAssignedCore(item.pid, -1);
	syncManager.releaseCoreSet(assignedCores);

	logger.logSystemEvent(
	    "CPU execution slot released from PID " + to_string(item.pid)
	);

	releaseCompletedProcess(item.pid, processManager, resourceManager, logger);
	return true;
    }

    if (pcb.processType == BACKGROUND_PROCESS) {
        UI::infoLine("[SCHEDULER] Background process running with FCFS until completion.");

        waitpid(item.pid, &status, 0);
        processManager.incrementTurnaroundTime(item.pid, 1);

        UI::successLine("[SCHEDULER] Background process completed.");

        processManager.updateAssignedCore(item.pid, -1);
        syncManager.releaseCoreSet(assignedCores);

        logger.logSystemEvent(
            "CPU execution slot released from PID " + to_string(item.pid)
        );

        releaseCompletedProcess(item.pid, processManager, resourceManager, logger);
        return true;
    }

    if (pcb.processType == INTERACTIVE_PROCESS || pcb.processType == AUTO_RUNNING_PROCESS) {
        quantum = interactiveQuantum;
        UI::infoLine("[SCHEDULER] Interactive process running with quantum: " + to_string(quantum) + " seconds.");
    } else {
        quantum = interactiveQuantum;
        UI::infoLine("[SCHEDULER] Process running with quantum: " + to_string(quantum) + " seconds.");
    }

    for (int i = 1; i <= quantum; i++) {
        sleep(1);
        processManager.incrementTurnaroundTime(item.pid, 1);

        pid_t result = waitpid(item.pid, &status, WNOHANG);

       if (result == item.pid) {
        UI::successLine("[SCHEDULER] Process completed within time quantum.");

        processManager.updateAssignedCore(item.pid, -1);
	    syncManager.releaseCoreSet(assignedCores);

	    logger.logSystemEvent(
		"CPU execution slot released from PID " + to_string(item.pid)
	    );

	    releaseCompletedProcess(item.pid, processManager, resourceManager, logger);
	    return true;
	}

        if (isTaskTerminalHidden(item.pid)) {
            UI::warnLine("[SCHEDULER] Task terminal minimized/hidden. Pausing current task.");
            sendSignalToScheduledProcess(item.pid, SIGSTOP);

            processManager.updateProcessState(item.pid, MINIMIZED_STATE);
            processManager.updateQueueType(item.pid, "Terminal Minimized");
            processManager.updateAssignedCore(item.pid, -1);
            syncManager.releaseCoreSet(assignedCores);

            logger.logProcessEvent(
                item.pid,
                item.processName,
                "Terminal minimized from window controls, process moved to MINIMIZED"
            );

            logger.logSystemEvent(
                "CPU execution slot released after terminal minimize for PID " + to_string(item.pid)
            );

            return false;
        }

        cout << "[SCHEDULER] Time slice " << i << "/" << quantum
             << " completed for PID " << item.pid << "\n";
    }

    UI::warnLine("[SCHEDULER] Time quantum expired for PID " + to_string(item.pid) + ".");
    UI::infoLine("[SCHEDULER] Pausing process and moving it back to ready queue.");

    sendSignalToScheduledProcess(item.pid, SIGSTOP);
	pid_t stopWaitResult = waitpid(item.pid, &status, WUNTRACED);

	if (stopWaitResult == item.pid && (WIFEXITED(status) || WIFSIGNALED(status))) {
        UI::successLine("[SCHEDULER] Process finished while its quantum was expiring.");

        processManager.updateAssignedCore(item.pid, -1);
	    syncManager.releaseCoreSet(assignedCores);

	    logger.logSystemEvent(
		"CPU execution slot released after PID " + to_string(item.pid) + " finished"
	    );

	    releaseCompletedProcess(item.pid, processManager, resourceManager, logger);
	    return true;
	}

	if (stopWaitResult == -1) {
        UI::warnLine("[SCHEDULER] Process is no longer waitable. Cleaning PCB and resources.");
        processManager.updateAssignedCore(item.pid, -1);
        syncManager.releaseCoreSet(assignedCores);

	    logger.logSystemEvent(
		"CPU execution slot released after PID " + to_string(item.pid) + " became non-waitable"
	    );

	    releaseCompletedProcess(item.pid, processManager, resourceManager, logger);
	    return true;
	}

    processManager.updateAssignedCore(item.pid, -1);
	syncManager.releaseCoreSet(assignedCores);

	logger.logSystemEvent(
	    "CPU execution slot released after quantum expiry for PID " + to_string(item.pid)
	);

	processManager.updateProcessState(item.pid, READY_STATE);

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
    SyncManager &syncManager
) {
    UI::panelHeader("AMS OS Scheduler", "Started");
    logger.logSystemEvent("Scheduler started");
    UI::infoLine("[SCHEDULER] Multilevel queue scheduler started.");

    restoreVisibleMinimizedTasks(processManager, readyQueueManager, logger, syncManager);

    if (!readyQueueManager.hasReadyProcess()) {
    UI::warnLine("[SCHEDULER] No process available in ready queue.");
    UI::infoLine("[SCHEDULER] Waiting briefly for ready queue notification.");

    syncManager.waitForReadyQueueSignal(2);

    if (!readyQueueManager.hasReadyProcess()) {
        UI::warnLine("[SCHEDULER] Still no process available.");
        UI::panelFooter();
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
        syncManager
    );

    PCB scheduledProcess;
    if (processManager.getPCB(selectedItem.pid, scheduledProcess) &&
        scheduledProcess.processState == MINIMIZED_STATE) {
        UI::warnLine("[SCHEDULER] Scheduling paused because active task was minimized.");
        UI::infoLine("[SCHEDULER] Restore terminal and run scheduler again to continue.");
        logger.logSystemEvent(
            "Scheduler paused because PID " + to_string(selectedItem.pid) + " was minimized"
        );
        break;
    }

    UI::sectionBanner("Scheduler Resource Snapshot", UI::BRIGHT_GREEN);
    resourceManager.displayResources();

    UI::sectionBanner("Scheduler Queue Snapshot", UI::BRIGHT_BLUE);
    readyQueueManager.displayReadyQueues();
}
    logger.logSystemEvent("Scheduler finished");
    UI::panelHeader("AMS OS Scheduler", "Finished");
}

void Scheduler::runMultitaskingDispatch(
    ProcessManager &processManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager
) {
    if (!readyQueueManager.hasReadyProcess()) {
        UI::warnLine("[MULTITASK] No ready process available to dispatch.");
        return;
    }

    UI::panelHeader("Multitasking Dispatch", "Parallel execution enabled");
    logger.logSystemEvent("Multitasking dispatch started");

    while (readyQueueManager.hasReadyProcess()) {
        ReadyQueueItem selectedItem;

        if (!selectNextProcess(readyQueueManager, selectedItem)) {
            break;
        }

        PCB pcb;
        if (!processManager.getPCB(selectedItem.pid, pcb)) {
            continue;
        }

        processManager.resetWaitingTime(selectedItem.pid);
        processManager.updateProcessState(selectedItem.pid, RUNNING_STATE);
        processManager.updateAssignedCore(selectedItem.pid, -1);
        processManager.updateQueueType(selectedItem.pid, "Parallel Running");
        sendSignalToScheduledProcess(selectedItem.pid, SIGCONT);

        logger.logProcessEvent(
            selectedItem.pid,
            selectedItem.processName,
            "Dispatched in multitasking mode (parallel)"
        );
        UI::successLine(
            "[MULTITASK] PID " + to_string(selectedItem.pid) + " running in parallel."
        );
    }

    syncManager.notifyReadyQueue();
    logger.logSystemEvent("Multitasking dispatch completed");
    UI::panelFooter();
}

void Scheduler::handleTerminalWindowState(
    ProcessManager &processManager,
    ReadyQueueManager &readyQueueManager,
    Logger &logger,
    SyncManager &syncManager,
    bool autoResumeToRunning
) {
    vector<int> pids = processManager.getAllPIDs();

    for (int pid : pids) {
        PCB pcb;
        if (!processManager.getPCB(pid, pcb)) {
            continue;
        }

        if (pcb.processState == RUNNING_STATE && isTaskTerminalHidden(pid)) {
            sendSignalToScheduledProcess(pid, SIGSTOP);
            processManager.updateProcessState(pid, MINIMIZED_STATE);
            processManager.updateQueueType(pid, "Terminal Minimized");
            processManager.updateAssignedCore(pid, -1);

            logger.logProcessEvent(
                pid,
                pcb.processName,
                "Terminal minimized from window controls, process paused"
            );
            continue;
        }

        if (pcb.processState != MINIMIZED_STATE) {
            continue;
        }

        if (pcb.queueType != "Terminal Minimized") {
            continue;
        }

        if (isTaskTerminalHidden(pid)) {
            continue;
        }

        if (autoResumeToRunning) {
            processManager.updateProcessState(pid, RUNNING_STATE);
            processManager.updateQueueType(pid, "Parallel Running");
            sendSignalToScheduledProcess(pid, SIGCONT);
            logger.logProcessEvent(
                pid,
                pcb.processName,
                "Terminal restored from window controls, process resumed"
            );
        } else {
            processManager.updateProcessState(pid, READY_STATE);
            processManager.updateQueueType(pid, readyQueueManager.getQueueName(pcb.priority));
            readyQueueManager.addProcessToReadyQueue(pid, pcb.processName, pcb.processType, pcb.priority);
            syncManager.notifyReadyQueue();
            logger.logProcessEvent(
                pid,
                pcb.processName,
                "Terminal restored from window controls, process returned to READY"
            );
        }
    }
}

void Scheduler::reapFinishedParallelTasks(
    ProcessManager &processManager,
    ResourceManager &resourceManager,
    Logger &logger
) {
    vector<int> pids = processManager.getAllPIDs();

    for (int pid : pids) {
        PCB pcb;
        if (!processManager.getPCB(pid, pcb)) {
            continue;
        }

        if (pcb.processState != RUNNING_STATE) {
            continue;
        }

        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);

        if (result == pid) {
            releaseCompletedProcess(pid, processManager, resourceManager, logger);
        }
    }
}
