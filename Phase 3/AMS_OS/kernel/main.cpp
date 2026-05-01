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
}

int main() {
    bootScreen();
    return 0;
}