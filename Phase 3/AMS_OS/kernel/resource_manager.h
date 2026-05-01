#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <iostream>
using namespace std;

class ResourceManager {
private:
    int totalRAM;
    int availableRAM;

    int totalHDD;
    int availableHDD;

    int totalCores;
    int availableCores;

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
};

#endif