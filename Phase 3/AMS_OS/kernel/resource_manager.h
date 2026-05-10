#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <iostream>
#include <vector>
#include <string>
using namespace std;

struct MemoryBlock {
    int startAddress;
    int endAddress;
    int pid;
    string processName;
    bool isFree;
};

class ResourceManager {
private:
    int totalRAM;
    int availableRAM;

    int totalHDD;
    int availableHDD;

    int totalCores;
    int availableCores;
    vector<MemoryBlock> memoryBlocks;

    /*
    Function: mergeFreeMemoryBlocks
    Purpose: Merges adjacent free memory blocks to reduce fragmentation.
    Parameters: None.
    Returns: Nothing.
    */
    void mergeFreeMemoryBlocks();

public:
    /*
    Function: ResourceManager
    Purpose: Default constructor that initializes all system resources to zero.
    Parameters: None.
    Returns: Nothing.
    */
    ResourceManager();

    /*
    Function: ResourceManager
    Purpose: Parameterized constructor that initializes RAM, HDD, and CPU cores.
    Parameters: RAM size, HDD size, and number of CPU cores.
    Returns: Nothing.
    */
    ResourceManager(int ram, int hdd, int cores);

    /*
    Function: initializeResources
    Purpose: Initializes the total and available system resources.
    Parameters: RAM size, HDD size, and number of CPU cores.
    Returns: Nothing.
    */
    void initializeResources(int ram, int hdd, int cores);

    /*
    Function: checkResources
    Purpose: Checks whether requested RAM, HDD, and CPU cores are available.
    Parameters: Required RAM, required HDD, and required CPU cores.
    Returns: true if resources are available, otherwise false.
    */
    bool checkResources(int ramRequired, int hddRequired, int coresRequired);

    /*
    Function: allocateResources
    Purpose: Allocates RAM, HDD, and CPU cores to a process if available.
    Parameters: Required RAM, required HDD, and required CPU cores.
    Returns: true if allocation is successful, otherwise false.
    */
    bool allocateResources(int ramRequired, int hddRequired, int coresRequired);

    /*
    Function: releaseResources
    Purpose: Releases RAM, HDD, and CPU cores after a process terminates.
    Parameters: RAM amount, HDD amount, and CPU cores to release.
    Returns: Nothing.
    */
    void releaseResources(int ramReleased, int hddReleased, int coresReleased);

    /*
    Function: displayResources
    Purpose: Displays current total and available system resources.
    Parameters: None.
    Returns: Nothing.
    */
    void displayResources();

    /*
    Function: getAvailableRAM
    Purpose: Returns currently available RAM.
    Parameters: None.
    Returns: Available RAM.
    */
    int getAvailableRAM();

    /*
    Function: getAvailableHDD
    Purpose: Returns currently available hard drive space.
    Parameters: None.
    Returns: Available HDD.
    */
    int getAvailableHDD();

    /*
    Function: getAvailableCores
    Purpose: Returns currently available CPU cores.
    Parameters: None.
    Returns: Available CPU cores.
    */
    int getAvailableCores();

    /*
    Function: getTotalRAM
    Purpose: Returns total configured RAM.
    Parameters: None.
    Returns: Total RAM.
    */
    int getTotalRAM();

    /*
    Function: getTotalHDD
    Purpose: Returns total configured hard drive space.
    Parameters: None.
    Returns: Total HDD.
    */
    int getTotalHDD();

    /*
    Function: getTotalCores
    Purpose: Returns total configured CPU cores.
    Parameters: None.
    Returns: Total CPU cores.
    */
    int getTotalCores();

    /*
    Function: allocateMemoryBlock
    Purpose: Allocates a simulated RAM block to a process using first-fit allocation.
    Parameters: PID, process name, RAM required, memory start reference, memory end reference.
    Returns: true if memory block is allocated, otherwise false.
    */
    bool allocateMemoryBlock(
        int pid,
        string processName,
        int ramRequired,
        int &memoryStart,
        int &memoryEnd
    );

    /*
    Function: releaseMemoryBlock
    Purpose: Releases a simulated RAM block assigned to a process.
    Parameters: PID.
    Returns: true if memory block is released, otherwise false.
    */
    bool releaseMemoryBlock(int pid);

    /*
    Function: displayMemoryLayout
    Purpose: Displays current simulated RAM layout.
    Parameters: None.
    Returns: Nothing.
    */
    void displayMemoryLayout();
};

#endif
