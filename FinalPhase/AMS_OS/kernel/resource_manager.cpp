#include "resource_manager.h"

/*
Function: ResourceManager
Purpose: Default constructor that initializes all system resources to zero.
Parameters: None.
Returns: Nothing.
*/
ResourceManager::ResourceManager() {
    totalRAM = 0;
    availableRAM = 0;

    totalHDD = 0;
    availableHDD = 0;

    totalCores = 0;
    availableCores = 0;
}

/*
Function: ResourceManager
Purpose: Parameterized constructor that initializes RAM, HDD, and CPU cores.
Parameters: RAM size, HDD size, and number of CPU cores.
Returns: Nothing.
*/
ResourceManager::ResourceManager(int ram, int hdd, int cores) {
    initializeResources(ram, hdd, cores);
}

/*
Function: initializeResources
Purpose: Initializes the total and available system resources.
Parameters: RAM size, HDD size, and number of CPU cores.
Returns: Nothing.
*/
void ResourceManager::initializeResources(int ram, int hdd, int cores) {
    totalRAM = ram;
    availableRAM = ram;

    totalHDD = hdd;
    availableHDD = hdd;

    totalCores = cores;
    availableCores = cores;

memoryBlocks.clear();

MemoryBlock initialBlock;
initialBlock.startAddress = 0;
initialBlock.endAddress = totalRAM;
initialBlock.pid = -1;
initialBlock.processName = "FREE";
initialBlock.isFree = true;

memoryBlocks.push_back(initialBlock);
}

/*
Function: checkResources
Purpose: Checks whether requested RAM, HDD, and CPU cores are available.
Parameters: Required RAM, required HDD, and required CPU cores.
Returns: true if resources are available, otherwise false.
*/
bool ResourceManager::checkResources(int ramRequired, int hddRequired, int coresRequired) {
    if (ramRequired <= 0 || hddRequired <= 0 || coresRequired <= 0) {
        return false;
    }

    if (ramRequired > availableRAM) {
        return false;
    }

    if (hddRequired > availableHDD) {
        return false;
    }

    if (coresRequired > availableCores) {
        return false;
    }

    return true;
}

/*
Function: allocateResources
Purpose: Allocates RAM, HDD, and CPU cores to a process if available.
Parameters: Required RAM, required HDD, and required CPU cores.
Returns: true if allocation is successful, otherwise false.
*/
bool ResourceManager::allocateResources(int ramRequired, int hddRequired, int coresRequired) {
    if (!checkResources(ramRequired, hddRequired, coresRequired)) {
        cout << "\n[RESOURCE MANAGER] Resource allocation failed.\n";
        cout << "Reason: Not enough resources available or invalid request.\n";
        return false;
    }

    availableRAM = availableRAM - ramRequired;
    availableHDD = availableHDD - hddRequired;
    availableCores = availableCores - coresRequired;

    cout << "\n[RESOURCE MANAGER] Resources allocated successfully.\n";
    cout << "Allocated RAM: " << ramRequired << " MB\n";
    cout << "Allocated HDD: " << hddRequired << " MB\n";
    cout << "Allocated CPU Cores: " << coresRequired << "\n";

    return true;
}

/*
Function: releaseResources
Purpose: Releases RAM, HDD, and CPU cores after a process terminates.
Parameters: RAM amount, HDD amount, and CPU cores to release.
Returns: Nothing.
*/
void ResourceManager::releaseResources(int ramReleased, int hddReleased, int coresReleased) {
    if (ramReleased < 0 || hddReleased < 0 || coresReleased < 0) {
        cout << "\n[RESOURCE MANAGER] Invalid resource release request.\n";
        return;
    }

    availableRAM = availableRAM + ramReleased;
    availableHDD = availableHDD + hddReleased;
    availableCores = availableCores + coresReleased;

    if (availableRAM > totalRAM) {
        availableRAM = totalRAM;
    }

    if (availableHDD > totalHDD) {
        availableHDD = totalHDD;
    }

    if (availableCores > totalCores) {
        availableCores = totalCores;
    }

    cout << "\n[RESOURCE MANAGER] Resources released successfully.\n";
    cout << "Released RAM: " << ramReleased << " MB\n";
    cout << "Released HDD: " << hddReleased << " MB\n";
    cout << "Released CPU Cores: " << coresReleased << "\n";
}

/*
Function: displayResources
Purpose: Displays current total and available system resources.
Parameters: None.
Returns: Nothing.
*/
void ResourceManager::displayResources() {
    cout << "\n========== AMS OS RESOURCE STATUS ==========\n";
    cout << "RAM Available: " << availableRAM << " MB / " << totalRAM << " MB\n";
    cout << "HDD Available: " << availableHDD << " MB / " << totalHDD << " MB\n";
    cout << "CPU Cores Available: " << availableCores << " / " << totalCores << "\n";
    cout << "===========================================\n";
}

/*
Function: getAvailableRAM
Purpose: Returns currently available RAM.
Parameters: None.
Returns: Available RAM.
*/
int ResourceManager::getAvailableRAM() {
    return availableRAM;
}

/*
Function: getAvailableHDD
Purpose: Returns currently available hard drive space.
Parameters: None.
Returns: Available HDD.
*/
int ResourceManager::getAvailableHDD() {
    return availableHDD;
}

/*
Function: getAvailableCores
Purpose: Returns currently available CPU cores.
Parameters: None.
Returns: Available CPU cores.
*/
int ResourceManager::getAvailableCores() {
    return availableCores;
}


/*
Function: allocateMemoryBlock
Purpose: Allocates a simulated RAM block to a process using first-fit allocation.
Parameters: PID, process name, RAM required, memory start reference, memory end reference.
Returns: true if memory block is allocated, otherwise false.
*/
bool ResourceManager::allocateMemoryBlock(
    int pid,
    string processName,
    int ramRequired,
    int &memoryStart,
    int &memoryEnd
) {
    if (pid <= 0 || ramRequired <= 0) {
        cout << "\n[MEMORY MANAGER] Invalid memory allocation request.\n";
        return false;
    }

    for (int i = 0; i < memoryBlocks.size(); i++) {
        int blockSize = memoryBlocks[i].endAddress - memoryBlocks[i].startAddress;

        if (memoryBlocks[i].isFree && blockSize >= ramRequired) {
            memoryStart = memoryBlocks[i].startAddress;
            memoryEnd = memoryStart + ramRequired;

            MemoryBlock allocatedBlock;
            allocatedBlock.startAddress = memoryStart;
            allocatedBlock.endAddress = memoryEnd;
            allocatedBlock.pid = pid;
            allocatedBlock.processName = processName;
            allocatedBlock.isFree = false;

            if (blockSize == ramRequired) {
                memoryBlocks[i] = allocatedBlock;
            } else {
                MemoryBlock remainingBlock;
                remainingBlock.startAddress = memoryEnd;
                remainingBlock.endAddress = memoryBlocks[i].endAddress;
                remainingBlock.pid = -1;
                remainingBlock.processName = "FREE";
                remainingBlock.isFree = true;

                memoryBlocks[i] = allocatedBlock;
                memoryBlocks.insert(memoryBlocks.begin() + i + 1, remainingBlock);
            }

            cout << "\n[MEMORY MANAGER] RAM block allocated successfully.\n";
            cout << "PID: " << pid << "\n";
            cout << "Process: " << processName << "\n";
            cout << "RAM Block: " << memoryStart << " MB to " << memoryEnd << " MB\n";

            return true;
        }
    }

    cout << "\n[MEMORY MANAGER] No suitable RAM block found.\n";
    return false;
}

/*
Function: releaseMemoryBlock
Purpose: Releases a simulated RAM block assigned to a process.
Parameters: PID.
Returns: true if memory block is released, otherwise false.
*/
bool ResourceManager::releaseMemoryBlock(int pid) {
    for (int i = 0; i < memoryBlocks.size(); i++) {
        if (!memoryBlocks[i].isFree && memoryBlocks[i].pid == pid) {
            cout << "\n[MEMORY MANAGER] Releasing RAM block.\n";
            cout << "PID: " << pid << "\n";
            cout << "RAM Block: " << memoryBlocks[i].startAddress
                 << " MB to " << memoryBlocks[i].endAddress << " MB\n";

            memoryBlocks[i].pid = -1;
            memoryBlocks[i].processName = "FREE";
            memoryBlocks[i].isFree = true;

            mergeFreeMemoryBlocks();

            return true;
        }
    }

    return false;
}

/*
Function: mergeFreeMemoryBlocks
Purpose: Merges adjacent free memory blocks to reduce fragmentation.
Parameters: None.
Returns: Nothing.
*/
void ResourceManager::mergeFreeMemoryBlocks() {
    for (int i = 0; i < memoryBlocks.size() - 1; i++) {
        if (memoryBlocks[i].isFree && memoryBlocks[i + 1].isFree) {
            memoryBlocks[i].endAddress = memoryBlocks[i + 1].endAddress;
            memoryBlocks.erase(memoryBlocks.begin() + i + 1);
            i--;
        }
    }
}

/*
Function: displayMemoryLayout
Purpose: Displays current simulated RAM layout.
Parameters: None.
Returns: Nothing.
*/
void ResourceManager::displayMemoryLayout() {
    cout << "\n==================== RAM MEMORY LAYOUT ====================\n";

    if (memoryBlocks.empty()) {
        cout << "No memory layout available.\n";
        cout << "===========================================================\n";
        return;
    }

    cout << "Start(MB)\tEnd(MB)\t\tStatus\t\tPID\tProcess\n";
    cout << "-----------------------------------------------------------\n";

    for (MemoryBlock block : memoryBlocks) {
        cout << block.startAddress << "\t\t"
             << block.endAddress << "\t\t";

        if (block.isFree) {
            cout << "FREE\t\t"
                 << "-\t"
                 << "Available\n";
        } else {
            cout << "ALLOCATED\t"
                 << block.pid << "\t"
                 << block.processName << "\n";
        }
    }

    cout << "===========================================================\n";
}



/*
Function: getTotalRAM
Purpose: Returns total RAM.
Parameters: None.
Returns: Total RAM.
*/
int ResourceManager::getTotalRAM() {
    return totalRAM;
}

/*
Function: getTotalHDD
Purpose: Returns total HDD.
Parameters: None.
Returns: Total HDD.
*/
int ResourceManager::getTotalHDD() {
    return totalHDD;
}

/*
Function: getTotalCores
Purpose: Returns total CPU cores.
Parameters: None.
Returns: Total CPU cores.
*/
int ResourceManager::getTotalCores() {
    return totalCores;
}