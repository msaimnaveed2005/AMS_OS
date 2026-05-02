#include <iostream>
#include <unistd.h>

using namespace std;

/*
Function: main
Purpose: Displays task manager information as a separate task executable.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    cout << "\n========== TASK MANAGER TASK ==========\n";
    cout << "This task confirms Task Manager is running as a separate process.\n";
    cout << "For full PCB table and process states, use AMS OS kernel menu option 7.\n";
    cout << "Current Task PID: " << getpid() << "\n";
    cout << "Parent Kernel PID: " << getppid() << "\n";

    return 0;
}