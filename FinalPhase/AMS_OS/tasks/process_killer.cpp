#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <limits>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Displays process killer utility with process listing and
         kill signal demonstration. Actual OS-level kill requires
         kernel mode from the main AMS OS terminal.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Process Killer", "Kernel tool");
    UI::taskControlHint(getpid());

    UI::sectionBanner("About", UI::BRIGHT_YELLOW);
    UI::infoLine("This tool can send signals to any PID on this system.");
    UI::infoLine("For AMS OS managed processes, use Kernel Mode menu option 16.");
    UI::warnLine("Sending signals to system processes may cause instability.");

    /*
    Show a sample of active processes for reference.
    */
    UI::sectionBanner("Active Processes (sample)", UI::BRIGHT_BLUE);

    DIR *procDir = opendir("/proc");
    if (procDir != nullptr) {
        struct dirent *entry;
        int shown = 0;

        while ((entry = readdir(procDir)) != nullptr && shown < 15) {
            string name(entry->d_name);
            bool isNumeric = true;
            for (char ch : name) {
                if (ch < '0' || ch > '9') { isNumeric = false; break; }
            }
            if (!isNumeric) continue;

            string commPath = "/proc/" + name + "/comm";
            ifstream commFile(commPath);
            if (commFile) {
                string processName;
                getline(commFile, processName);
                commFile.close();

                cout << "    " << UI::paint("PID " + name, UI::LIGHT_BLUE + UI::BOLD)
                     << "  " << UI::paint(processName, UI::WHITE) << "\n";
                shown++;
            }
        }
        closedir(procDir);
    }

    UI::sectionBanner("Signal Test", UI::BRIGHT_CYAN);
    UI::menuItem(1, "Send SIGTERM (15)", "Graceful termination");
    UI::menuItem(2, "Send SIGSTOP (19)", "Pause process");
    UI::menuItem(3, "Send SIGCONT (18)", "Resume process");
    UI::menuItem(0, "Exit without action");

    int choice = 0;
    cout << "\n  Select action: ";
    cin >> choice;

    if (cin.fail() || choice == 0) {
        UI::successLine("Process Killer exited without action.");
        UI::panelFooter();
        return 0;
    }

    int signalNumber;
    string signalName;

    switch (choice) {
        case 1: signalNumber = SIGTERM; signalName = "SIGTERM (15)"; break;
        case 2: signalNumber = SIGSTOP; signalName = "SIGSTOP (19)"; break;
        case 3: signalNumber = SIGCONT; signalName = "SIGCONT (18)"; break;
        default:
            UI::warnLine("Invalid action.");
            UI::panelFooter();
            return 0;
    }

    int targetPID = 0;
    cout << "  Enter target PID: ";
    cin >> targetPID;

    if (cin.fail() || targetPID <= 0) {
        UI::errorLine("Invalid PID.");
        UI::panelFooter();
        return 1;
    }

    UI::keyValue("Signal", signalName);
    UI::keyValue("Target PID", to_string(targetPID));

    if (kill(targetPID, signalNumber) == 0) {
        UI::successLine("Signal sent successfully to PID " + to_string(targetPID) + ".");
        UI::playCue("granted");
    } else {
        UI::errorLine("Failed to send signal: " + string(strerror(errno)));
        UI::playCue("error");
    }

    UI::panelFooter();
    return 0;
}
