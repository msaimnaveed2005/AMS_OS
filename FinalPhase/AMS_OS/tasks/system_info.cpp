#include <iostream>
#include <ctime>
#include <unistd.h>
#include <sys/utsname.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Displays detailed system information for AMS OS task simulation.
         Shows process details, host OS info, and environment variables.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("System Information", "AMS OS task executable");
    UI::taskControlHint(getpid());

    UI::sectionBanner("Process Details", UI::BRIGHT_CYAN);
    UI::keyValue("Current Process PID", to_string(getpid()));
    UI::keyValue("Parent Kernel PID", to_string(UI::parentProcessID()));
    UI::keyValue("Process Group ID", to_string(getpgrp()));
    UI::keyValue("User ID", to_string(getuid()));
    UI::keyValue("Session ID", to_string(getsid(0)));

    UI::sectionBanner("Host System", UI::BRIGHT_GREEN);
    struct utsname hostInfo;
    if (uname(&hostInfo) == 0) {
        UI::keyValue("OS Name", string(hostInfo.sysname));
        UI::keyValue("Hostname", string(hostInfo.nodename));
        UI::keyValue("Kernel Release", string(hostInfo.release));
        UI::keyValue("Kernel Version", string(hostInfo.version));
        UI::keyValue("Architecture", string(hostInfo.machine));
    } else {
        UI::warnLine("Could not retrieve host system info.");
    }

    UI::sectionBanner("AMS OS Environment", UI::BRIGHT_BLUE);
    UI::keyValue("Simulator", "AMS OS - Atomic Management System");
    UI::keyValue("Execution Mode", "User Task Executable");

    const char* amsOSPID = getenv("AMS_OS_PID");
    if (amsOSPID != nullptr) {
        UI::keyValue("AMS OS Kernel PID", string(amsOSPID));
    }

    const char* user = getenv("USER");
    if (user != nullptr) {
        UI::keyValue("Logged-in User", string(user));
    }

    const char* shell = getenv("SHELL");
    if (shell != nullptr) {
        UI::keyValue("Default Shell", string(shell));
    }

    const char* home = getenv("HOME");
    if (home != nullptr) {
        UI::keyValue("Home Directory", string(home));
    }

    time_t now = time(0);
    char timeBuffer[64];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
    UI::keyValue("Timestamp", string(timeBuffer));

    cout << "\n";
    UI::panelFooter();
    return 0;
}
