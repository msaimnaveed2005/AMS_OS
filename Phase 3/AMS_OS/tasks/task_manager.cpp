#include <iostream>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Displays task manager information as a separate task executable.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Task Manager", "Process monitor information");
    UI::taskControlHint(getpid());
    cout << "  This task confirms Task Manager is running as a separate process.\n";
    cout << "  For full PCB table and process states, use AMS OS kernel menu option 7.\n";
    UI::keyValue("Current Task PID", to_string(getpid()));
    UI::keyValue("Parent Kernel PID", to_string(UI::parentProcessID()));
    UI::panelFooter();

    return 0;
}
