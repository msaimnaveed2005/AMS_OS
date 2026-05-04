#include <iostream>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Displays basic system information for AMS OS task simulation.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("System Information", "AMS OS task executable");
    UI::taskControlHint(getpid());
    UI::keyValue("Current Process PID", to_string(getpid()));
    UI::keyValue("Parent Kernel PID", to_string(getppid()));
    UI::keyValue("Operating System", "AMS OS Simulation");
    UI::keyValue("Execution Mode", "User Task Executable");
    cout << "  " << UI::paint("Resource details are managed by AMS OS kernel.\n", UI::DIM);
    UI::panelFooter();

    return 0;
}
