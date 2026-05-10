#include <iostream>
#include <unistd.h>

using namespace std;

/*
Function: main
Purpose: Displays process killer information as a separate executable.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    cout << "\n========== PROCESS KILLER TASK ==========\n";
    cout << "Process Killer is a privileged Kernel Mode operation.\n";
    cout << "Use AMS OS Kernel Mode menu option 16 to kill real processes.\n";
    cout << "This executable exists to satisfy separate task file requirement.\n";
    cout << "Current Task PID: " << getpid() << "\n";
    cout << "Parent Kernel PID: " << getppid() << "\n";

    return 0;
}