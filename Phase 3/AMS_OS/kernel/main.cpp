#include <iostream>
#include <unistd.h>
using namespace std;

void bootScreen() {
sleep(2)
    cout << "=====================================\n";
    cout << "        AMS OS\n";
    cout << " Atomic Management System\n";
sleep(1)
    cout << "=====================================\n";
    cout << "Booting AMS OS";
    
    for (int i = 0; i < 3; i++) 
    {
        cout << ".";
        cout.flush();
        sleep(1);
    }

    cout << "\nSystem Loaded Successfully.\n";

int totalRAM, totalHDD, totalCores;

cout << "Enter RAM in MB: ";
cin >> totalRAM;

cout << "Enter Hard Drive in MB: ";
cin >> totalHDD;

cout << "Enter CPU Cores: ";
cin >> totalCores;


if (totalRAM <= 0 || totalHDD <= 0 || totalCores <= 0) {
    cout << "Invalid hardware resources. OS cannot start.\n";
    return 1;
}
}

int main() {
    bootScreen();
    return 0;
}