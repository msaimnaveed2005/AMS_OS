#include <iostream>
#include <unistd.h>
#include "resource_manager.h"

using namespace std;

/*
Function: bootScreen
Purpose: Displays the AMS OS boot screen with loading effect.
Parameters: None.
Returns: Nothing.
*/
void bootScreen() {
    cout << "=====================================\n";
    cout << "              AMS OS\n";
    cout << "      Atomic Management System\n";
    cout << "=====================================\n";

    cout << "Booting AMS OS";

    for (int i = 0; i < 3; i++) {
        cout << ".";
        cout.flush();
        sleep(1);
    }

    cout << "\nSystem Loaded Successfully.\n";
}

/*
Function: getHardwareResources
Purpose: Takes RAM, HDD, and CPU core values from the user.
Parameters: References to RAM, HDD, and CPU core variables.
Returns: true if valid resources are entered, otherwise false.
*/
bool getHardwareResources(int &ram, int &hdd, int &cores) {
    cout << "\n========== HARDWARE RESOURCE SETUP ==========\n";

    cout << "Enter RAM in MB: ";
    cin >> ram;

    cout << "Enter Hard Drive in MB: ";
    cin >> hdd;

    cout << "Enter CPU Cores: ";
    cin >> cores;

    if (ram <= 0 || hdd <= 0 || cores <= 0) {
        cout << "\nInvalid hardware resources entered.\n";
        cout << "AMS OS cannot start with zero or negative resources.\n";
        return false;
    }

    return true;
}

/*
Function: showMainMenu
Purpose: Displays the main AMS OS menu.
Parameters: None.
Returns: Nothing.
*/
void showMainMenu() {
    cout << "\n========== AMS OS MAIN MENU ==========\n";
    cout << "1. Show Resources\n";
    cout << "2. Test Resource Allocation\n";
    cout << "3. Test Resource Release\n";
    cout << "0. Shutdown AMS OS\n";
    cout << "Enter your choice: ";
}

/*
Function: main
Purpose: Starts AMS OS, initializes resources, and handles the main menu.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    int ram;
    int hdd;
    int cores;

    bootScreen();

    if (!getHardwareResources(ram, hdd, cores)) {
        return 1;
    }

    ResourceManager resourceManager(ram, hdd, cores);

    cout << "\nAMS OS resources initialized successfully.\n";
    resourceManager.displayResources();

    int choice;

    do {
        showMainMenu();
        cin >> choice;

        switch (choice) {
            case 1:
                resourceManager.displayResources();
                break;

            case 2: {
                int reqRAM;
                int reqHDD;
                int reqCores;

                cout << "\nEnter RAM required by process in MB: ";
                cin >> reqRAM;

                cout << "Enter HDD required by process in MB: ";
                cin >> reqHDD;

                cout << "Enter CPU cores required by process: ";
                cin >> reqCores;

                resourceManager.allocateResources(reqRAM, reqHDD, reqCores);
                resourceManager.displayResources();
                break;
            }

            case 3: {
                int relRAM;
                int relHDD;
                int relCores;

                cout << "\nEnter RAM to release in MB: ";
                cin >> relRAM;

                cout << "Enter HDD to release in MB: ";
                cin >> relHDD;

                cout << "Enter CPU cores to release: ";
                cin >> relCores;

                resourceManager.releaseResources(relRAM, relHDD, relCores);
                resourceManager.displayResources();
                break;
            }

            case 0:
                cout << "\nShutting down AMS OS";
                for (int i = 0; i < 3; i++) {
                    cout << ".";
                    cout.flush();
                    sleep(1);
                }
                cout << "\nAMS OS shutdown completed.\n";
                break;

            default:
                cout << "\nInvalid choice. Please try again.\n";
        }

    } while (choice != 0);

    return 0;
}