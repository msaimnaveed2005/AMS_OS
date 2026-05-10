#include "resource_manager.h"
#include "console_colors.h"
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
        cout << "\n" << Color::resource("[RESOURCE MANAGER]") << " Resource allocation failed.\n";
        cout << "Reason: Not enough resources available or invalid request.\n";
        return false;
    }

    availableRAM = availableRAM - ramRequired;
    availableHDD = availableHDD - hddRequired;
    availableCores = availableCores - coresRequired;

    cout << "\n" << Color::resource("[RESOURCE MANAGER]") << " Resources allocated successfully.\n";
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
        cout << "\n" << Color::resource("[RESOURCE MANAGER]") << " Invalid resource release request.\n";
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

    cout << "\n" << Color::resource("[RESOURCE MANAGER]") << " Resources released successfully.\n";
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
    cout << "\n";
Color::line('=', 58, Color::BRIGHT_YELLOW + Color::BOLD);
cout << Color::paint("              AMS OS RESOURCE STATUS\n", Color::BRIGHT_YELLOW + Color::BOLD);
Color::line('=', 58, Color::BRIGHT_YELLOW + Color::BOLD);
    cout << Color::paint("RAM Available: ", Color::BRIGHT_BLUE + Color::BOLD)
     << availableRAM << " MB / " << totalRAM << " MB\n";

cout << Color::paint("HDD Available: ", Color::BRIGHT_MAGENTA + Color::BOLD)
     << availableHDD << " MB / " << totalHDD << " MB\n";

cout << Color::paint("CPU Cores Available: ", Color::BRIGHT_GREEN + Color::BOLD)
     << availableCores << " / " << totalCores << "\n";
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
        cout << "\n" << Color::memory("[MEMORY MANAGER]") << " Invalid memory allocation request.\n";
        return false;
    }

    for (size_t i = 0; i < memoryBlocks.size(); i++) {
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

            cout << "\n" << Color::memory("[MEMORY MANAGER]") << " RAM block allocated successfully.\n";
            cout << "PID: " << pid << "\n";
            cout << "Process: " << processName << "\n";
            cout << "RAM Block: " << memoryStart << " MB to " << memoryEnd << " MB\n";

            return true;
        }
    }

    cout << "\n" << Color::memory("[MEMORY MANAGER]") << " No suitable RAM block found.\n";
    return false;
}

/*
Function: releaseMemoryBlock
Purpose: Releases a simulated RAM block assigned to a process.
Parameters: PID.
Returns: true if memory block is released, otherwise false.
*/
bool ResourceManager::releaseMemoryBlock(int pid) {
    for (size_t i = 0; i < memoryBlocks.size(); i++) {
        if (!memoryBlocks[i].isFree && memoryBlocks[i].pid == pid) {
            cout << "\n" << Color::memory("[MEMORY MANAGER]") << " Releasing RAM block.\n";
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
    if (memoryBlocks.size() < 2) {
        return;
    }

    for (size_t i = 0; i + 1 < memoryBlocks.size(); i++) {
        if (memoryBlocks[i].isFree && memoryBlocks[i + 1].isFree) {
            memoryBlocks[i].endAddress = memoryBlocks[i + 1].endAddress;
            memoryBlocks.erase(memoryBlocks.begin() + i + 1);
            if (i > 0) {
                i--;
            }
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
    cout << "\n";
Color::line('=', 74, Color::BRIGHT_MAGENTA + Color::BOLD);
cout << Color::paint("                         RAM MEMORY LAYOUT\n", Color::BRIGHT_MAGENTA + Color::BOLD);
Color::line('=', 74, Color::BRIGHT_MAGENTA + Color::BOLD);

    if (memoryBlocks.empty()) {
        cout << "No memory layout available.\n";
        cout << "===========================================================\n";
        return;
    }

    // Header row
Color::cell("START", 12, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("END", 12, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("STATUS", 16, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("PID", 10, Color::BRIGHT_CYAN + Color::BOLD);
Color::cell("PROCESS", 20, Color::BRIGHT_CYAN + Color::BOLD);
cout << "\n";

// Line separator for rows
Color::line('-', 74, Color::GRAY);
    cout << "-----------------------------------------------------------\n";

   for (auto block : memoryBlocks) {
    Color::cell(to_string(block.startAddress), 12, Color::WHITE);
    Color::cell(to_string(block.endAddress), 12, Color::WHITE);

    if (block.isFree) {
        Color::cell("FREE", 16, Color::BRIGHT_GREEN + Color::BOLD);
        Color::cell("-", 10, Color::GRAY);
        Color::cell("Available", 20, Color::BRIGHT_GREEN);
    } else {
        Color::cell("ALLOCATED", 16, Color::BRIGHT_RED + Color::BOLD);
        Color::cell(to_string(block.pid), 10, Color::BRIGHT_YELLOW);
        Color::cell(block.processName, 20, Color::BRIGHT_WHITE + Color::BOLD);
    }

    cout << "\n";
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
