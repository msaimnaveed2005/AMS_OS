#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <deque>
#include <unordered_map>
#include <vector>
#include <cstring>

using namespace std;

enum ProcessType {
    SYSTEM = 1,
    INTERACTIVE = 2,
    BACKGROUND = 3
};

enum ProcessState {
    READY,
    RUNNING,
    TERMINATED
};

struct TaskMeta {
    string name;
    ProcessType type;
    int ramRequired;
    int hddRequired;
    int cpuRequired;
    int duration;
};

struct ResourceRequest {
    char name[64];
    int type;
    int ramRequired;
    int hddRequired;
    int cpuRequired;
    int duration;
};

struct ResourceResponse {
    int granted;
};

struct PCB {
    pid_t pid;
    string name;
    ProcessType type;
    ProcessState state;
    int ramAllocated;
    int hddAllocated;
    int cpuAllocated;
    int duration;
    int priority;
    int waitingTime;
};

string getTypeName(ProcessType type) {
    if (type == SYSTEM) return "System";
    if (type == INTERACTIVE) return "Interactive";
    return "Background";
}

string getStateName(ProcessState state) {
    if (state == READY) return "READY";
    if (state == RUNNING) return "RUNNING";
    return "TERMINATED";
}

int getBasePriority(ProcessType type) {
    if (type == SYSTEM) return 1;
    if (type == INTERACTIVE) return 2;
    return 3;
}

class ResourceManager {
private:
    int totalRam;
    int totalHdd;
    int totalCpu;

    int freeRam;
    int freeHdd;
    int freeCpu;

public:
    ResourceManager(int ram, int hdd, int cpu) {
        totalRam = ram;
        totalHdd = hdd;
        totalCpu = cpu;

        freeRam = ram;
        freeHdd = hdd;
        freeCpu = cpu;
    }

    bool canAllocate(const ResourceRequest& request) {
        return request.ramRequired <= freeRam &&
               request.hddRequired <= freeHdd &&
               request.cpuRequired <= freeCpu;
    }

    void allocate(const ResourceRequest& request) {
        freeRam -= request.ramRequired;
        freeHdd -= request.hddRequired;
        freeCpu -= request.cpuRequired;
    }

    void release(const PCB& pcb) {
        freeRam += pcb.ramAllocated;
        freeHdd += pcb.hddAllocated;
        freeCpu += pcb.cpuAllocated;

        if (freeRam > totalRam) freeRam = totalRam;
        if (freeHdd > totalHdd) freeHdd = totalHdd;
        if (freeCpu > totalCpu) freeCpu = totalCpu;
    }

    void displayResources() {
        cout << "\n========== RESOURCE STATUS ==========\n";
        cout << "RAM: " << freeRam << "/" << totalRam << " MB free\n";
        cout << "HDD: " << freeHdd << "/" << totalHdd << " MB free\n";
        cout << "CPU Cores: " << freeCpu << "/" << totalCpu << " free\n";
        cout << "=====================================\n";
    }
};

class AMSKernel {
private:
    ResourceManager resourceManager;

    unordered_map<pid_t, PCB> pcbTable;

    deque<pid_t> systemQueue;
    deque<pid_t> interactiveQueue;
    deque<pid_t> backgroundQueue;

public:
    AMSKernel() : resourceManager(1024, 5000, 8) {}

    void addToReadyQueue(pid_t pid) {
        if (pcbTable.find(pid) == pcbTable.end()) {
            return;
        }

        PCB& pcb = pcbTable[pid];
        pcb.state = READY;

        if (pcb.type == SYSTEM) {
            systemQueue.push_back(pid);
        } else if (pcb.type == INTERACTIVE) {
            interactiveQueue.push_back(pid);
        } else {
            backgroundQueue.push_back(pid);
        }

        cout << "[READY QUEUE] Process " << pcb.name
             << " added to " << getTypeName(pcb.type) << " queue.\n";
    }

    pid_t selectNextProcess() {
        if (!systemQueue.empty()) {
            pid_t pid = systemQueue.front();
            systemQueue.pop_front();
            return pid;
        }

        if (!interactiveQueue.empty()) {
            pid_t pid = interactiveQueue.front();
            interactiveQueue.pop_front();
            return pid;
        }

        if (!backgroundQueue.empty()) {
            pid_t pid = backgroundQueue.front();
            backgroundQueue.pop_front();
            return pid;
        }

        return -1;
    }

    bool hasReadyProcess() {
        return !systemQueue.empty() ||
               !interactiveQueue.empty() ||
               !backgroundQueue.empty();
    }

    void launchTask(const TaskMeta& task) {
        int requestPipe[2];
        int responsePipe[2];

        if (pipe(requestPipe) == -1 || pipe(responsePipe) == -1) {
            perror("Pipe creation failed");
            return;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            return;
        }

        if (pid == 0) {
            close(requestPipe[0]);
            close(responsePipe[1]);

            ResourceRequest request{};
            strncpy(request.name, task.name.c_str(), sizeof(request.name) - 1);
            request.type = task.type;
            request.ramRequired = task.ramRequired;
            request.hddRequired = task.hddRequired;
            request.cpuRequired = task.cpuRequired;
            request.duration = task.duration;

            write(requestPipe[1], &request, sizeof(request));

            ResourceResponse response{};
            read(responsePipe[0], &response, sizeof(response));

            if (!response.granted) {
                cout << "[CHILD] Resource request denied. Process exiting.\n";
                _exit(2);
            }

            raise(SIGSTOP);

            string durationText = to_string(task.duration);

            execl(
                "./task_runner",
                "task_runner",
                task.name.c_str(),
                durationText.c_str(),
                nullptr
            );

            perror("exec failed");
            _exit(1);
        }

        close(requestPipe[1]);
        close(responsePipe[0]);

        ResourceRequest request{};
        read(requestPipe[0], &request, sizeof(request));

        ResourceResponse response{};
        response.granted = 0;

        cout << "\n[IPC] Resource request received from child PID " << pid << endl;
        cout << "Task: " << request.name << endl;
        cout << "RAM Requested: " << request.ramRequired << " MB\n";
        cout << "HDD Requested: " << request.hddRequired << " MB\n";
        cout << "CPU Requested: " << request.cpuRequired << " core\n";

        if (resourceManager.canAllocate(request)) {
            response.granted = 1;
            resourceManager.allocate(request);

            cout << "[RESOURCE MANAGER] Resources granted.\n";
        } else {
            cout << "[RESOURCE MANAGER] Resources denied.\n";
        }

        write(responsePipe[1], &response, sizeof(response));

        close(requestPipe[0]);
        close(responsePipe[1]);

        if (!response.granted) {
            waitpid(pid, nullptr, 0);
            return;
        }

        int status;
        waitpid(pid, &status, WUNTRACED);

        PCB pcb;
        pcb.pid = pid;
        pcb.name = request.name;
        pcb.type = static_cast<ProcessType>(request.type);
        pcb.state = READY;
        pcb.ramAllocated = request.ramRequired;
        pcb.hddAllocated = request.hddRequired;
        pcb.cpuAllocated = request.cpuRequired;
        pcb.duration = request.duration;
        pcb.priority = getBasePriority(pcb.type);
        pcb.waitingTime = 0;

        pcbTable[pid] = pcb;

        cout << "[PROCESS MANAGER] PCB created for PID " << pid << endl;

        addToReadyQueue(pid);
    }

    bool runProcessForQuantum(pid_t pid, int quantum, bool allowPreemption) {
        if (pcbTable.find(pid) == pcbTable.end()) {
            return true;
        }

        PCB& pcb = pcbTable[pid];

        cout << "\n[SCHEDULER] Dispatching PID " << pid
             << " (" << pcb.name << ")" << endl;

        pcb.state = RUNNING;

        kill(pid, SIGCONT);

        int status;

        for (int i = 0; i < quantum; i++) {
            sleep(1);

            pid_t result = waitpid(pid, &status, WNOHANG);

            if (result == pid) {
                cout << "[SCHEDULER] Process " << pcb.name << " completed.\n";
                pcb.state = TERMINATED;
                resourceManager.release(pcb);
                pcbTable.erase(pid);
                return true;
            }
        }

        if (allowPreemption) {
            cout << "[SCHEDULER] Time quantum expired for " << pcb.name << endl;
            cout << "[CONTEXT SWITCH] Stopping PID " << pid << endl;

            kill(pid, SIGSTOP);
            waitpid(pid, &status, WUNTRACED);

            pcb.state = READY;
            return false;
        }

        waitpid(pid, &status, 0);

        cout << "[SCHEDULER] System process completed.\n";
        pcb.state = TERMINATED;
        resourceManager.release(pcb);
        pcbTable.erase(pid);

        return true;
    }

    void runScheduler() {
        if (!hasReadyProcess()) {
            cout << "\n[SCHEDULER] No process available in ready queue.\n";
            return;
        }

        cout << "\n========== SCHEDULER STARTED ==========\n";

        while (hasReadyProcess()) {
            pid_t selectedPid = selectNextProcess();

            if (selectedPid == -1) {
                break;
            }

            if (pcbTable.find(selectedPid) == pcbTable.end()) {
                continue;
            }

            PCB selectedPCB = pcbTable[selectedPid];

            int quantum;
            bool allowPreemption;

            if (selectedPCB.type == SYSTEM) {
                quantum = selectedPCB.duration + 2;
                allowPreemption = false;
            } else if (selectedPCB.type == INTERACTIVE) {
                quantum = 2;
                allowPreemption = true;
            } else {
                quantum = 1;
                allowPreemption = true;
            }

            bool finished = runProcessForQuantum(
                selectedPid,
                quantum,
                allowPreemption
            );

            if (!finished && pcbTable.find(selectedPid) != pcbTable.end()) {
                addToReadyQueue(selectedPid);
            }

            resourceManager.displayResources();
        }

        cout << "\n========== ALL READY PROCESSES FINISHED ==========\n";
    }

    void displayPCBTable() {
        cout << "\n========== PCB TABLE ==========\n";

        if (pcbTable.empty()) {
            cout << "No active process.\n";
        }

        for (auto& entry : pcbTable) {
            PCB pcb = entry.second;

            cout << "PID: " << pcb.pid
                 << " | Name: " << pcb.name
                 << " | Type: " << getTypeName(pcb.type)
                 << " | State: " << getStateName(pcb.state)
                 << " | RAM: " << pcb.ramAllocated
                 << " | HDD: " << pcb.hddAllocated
                 << " | CPU: " << pcb.cpuAllocated
                 << endl;
        }

        cout << "===============================\n";
    }

    void showResources() {
        resourceManager.displayResources();
    }
};

int main() {
    AMSKernel os;

    vector<TaskMeta> tasks = {
        {"Task Manager", SYSTEM, 100, 200, 1, 3},
        {"Calculator", INTERACTIVE, 150, 100, 1, 6},
        {"Notepad", INTERACTIVE, 200, 300, 1, 7},
        {"File Copy", BACKGROUND, 250, 800, 1, 5},
        {"Music Player", BACKGROUND, 180, 400, 1, 6}
    };

    int choice;

    while (true) {
        cout << "\n\n========== AMS OS PHASE 3 ==========\n";
	sleep(1);
        cout << "1. Launch Task Manager\n";
        cout << "2. Launch Calculator\n";
        cout << "3. Launch Notepad\n";
        cout << "4. Launch File Copy\n";
        cout << "5. Launch Music Player\n";
        cout << "6. Run Scheduler\n";
        cout << "7. Show PCB Table\n";
        cout << "8. Show Resources\n";
        cout << "9. Auto Demo\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 0) {
            cout << "Shutting down AMS OS.\n";
            break;
        }

        switch (choice) {
            case 1:
                os.launchTask(tasks[0]);
                break;

            case 2:
                os.launchTask(tasks[1]);
                break;

            case 3:
                os.launchTask(tasks[2]);
                break;

            case 4:
                os.launchTask(tasks[3]);
                break;

            case 5:
                os.launchTask(tasks[4]);
                break;

            case 6:
                os.runScheduler();
                break;

            case 7:
                os.displayPCBTable();
                break;

            case 8:
                os.showResources();
                break;

            case 9:
                os.launchTask(tasks[0]);
                os.launchTask(tasks[1]);
                os.launchTask(tasks[2]);
                os.launchTask(tasks[3]);
                os.launchTask(tasks[4]);
                os.runScheduler();
                break;

            default:
                cout << "Invalid choice.\n";
        }
    }

    return 0;
}