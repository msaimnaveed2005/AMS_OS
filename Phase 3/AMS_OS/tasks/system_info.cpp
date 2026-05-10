#include <iostream>
#include <unistd.h>

using namespace std;

/*
Function: main
Purpose: Displays basic system information for AMS OS task simulation.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    cout << "\n========== SYSTEM INFORMATION VIEWER TASK ==========\n";
    cout << "Task running as separate executable.\n";
    cout << "Current Process PID: " << getpid() << "\n";
    cout << "Parent Kernel PID: " << getppid() << "\n";
    cout << "Operating System Simulation: AMS OS\n";
    cout << "Execution Mode: User Task Executable\n";
    cout << "Resource details are managed by AMS OS kernel.\n";

    return 0;
}