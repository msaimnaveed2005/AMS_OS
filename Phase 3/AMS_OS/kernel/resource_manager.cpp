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