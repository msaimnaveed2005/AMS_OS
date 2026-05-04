#include <iostream>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Displays process killer information as a separate executable.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Process Killer", "Kernel tool information");
    UI::taskControlHint(getpid());
    cout << "  Process Killer is a privileged Kernel Mode operation.\n";
    cout << "  Use AMS OS Kernel Mode menu option 16 to kill real processes.\n";
    UI::keyValue("Current Task PID", to_string(getpid()));
    UI::keyValue("Parent Kernel PID", to_string(getppid()));
    UI::panelFooter();

    return 0;
}
